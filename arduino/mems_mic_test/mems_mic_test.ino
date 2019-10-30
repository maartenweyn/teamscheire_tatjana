#include <driver/i2s.h>
#include "BLESerial.h"
#include <string.h>
#include <soc/rtc.h>
#include "iir_filter.h"

const i2s_port_t I2S_PORT = I2S_NUM_0;
const int SAMPLE_RATE = 16000; //48000; // 32kHz
const int SAMPLES_SHORT = SAMPLE_RATE / 8; //SAMPLE_RATE / 8; // 125 ms
const int BLOCK_SIZE = 1000; // < 1024
const int SAMPLE_LENGTH = 48000; // 1 second of data //128;
const int MIN_RECORDING_INTERVAL = 60000; //60*1000;

#define MIC_OFFSET_DB     3.0103    // Default offset (sine-wave RMS vs. dBFS)
#define MIC_REF_DB        94.0      // dB(SPL)
//#define MIC_REF_AMPL      420426// 0.050118684768677 //420426/8388608    // Amplitude at 94dB(SPL) (-26dBFS from datasheet, i.e. (2^23-1)*10^(-26/20) )
#define MIC_REF_AMPL      6569

#define LED 13

#define POSTDATA_BUFFER_SIZE  60 * 12
#define CORR_X0   30.7 // watch
#define CORR_X1   76.0 // watch
#define CORR_REF0 30.7 // ref
#define CORR_REF1 76.0 // ref

typedef struct
{
  int ts;
  int id;
  double sound_level;
  double leq_min;
  double leq_hour;
  double leq_8hours;
  double leq_day;
  int response;
} post_data_s;

//
//static const double AFILTER_Acoef[] = {1.0, -4.0195761811158315, 6.1894064429206921, -4.4531989035441155,
//                    1.4208429496218764, -0.14182547383030436,
//                    0.0043511772334950787};
//static const double AFILTER_Bcoef[] = {0.2557411252042574, -0.51148225040851436,
//                    -0.25574112520425807, 1.0229645008170318,
//                    -0.25574112520425918, -0.51148225040851414,
//                    0.25574112520425729};
//static double AFILTER_conditions[] = {0, 0, 0, 0, 0, 0};

//
// IIR Filters
//
//
// A-weighting 6th order IIR Filter, Fs = 48KHz 
// (By Dr. Matt L., Source: https://dsp.stackexchange.com/a/36122)
//
const double A_weighting_B[] = {0.169994948147430, 0.280415310498794, -1.120574766348363, 0.131562559965936, 0.974153561246036, -0.282740857326553, -0.152810756202003};
const double A_weighting_A[] = {1.0, -2.12979364760736134, 0.42996125885751674, 1.62132698199721426, -0.96669962900852902, 0.00121015844426781, 0.04400300696788968};

////48000 Hz
//const double A_weighting_A[] =  { 1.0000000000000000D,  -4.113043408775872D,
//                      6.5531217526550503D,  -4.9908492941633842D,
//                      1.7857373029375754D,  -0.24619059531948789D,
//                      0.011224250033231334D };
//const double A_weighting_B[] =  {  0.2343017922995132D,  -0.4686035845990264D,
//                      -0.23430179229951431D,  0.9372071691980528D,
//                      -0.23430179229951364D,  -0.46860358459902524D,
//                      0.23430179229951273D };

////32000 Hz
//const double A_weighting_A[] =   {  1.0000000000, -3.6564460432,
//                        4.8314684507, -2.5575974966,
//                        0.2533680394, 0.1224430322,
//                        0.0067640722 };
//const double A_weighting_B[] = {  0.3434583387, -0.6869166774,
//                        -0.3434583387,  1.3738333547,
//                        -0.3434583387,  -0.6869166774,
//                        0.3434583387 };

IIR_FILTER A_weighting(A_weighting_B, A_weighting_A);




//static double cumulativeSoundLevel;
//static int cumulativeSoundCounter = 0;
static unsigned long start_ts = 0;
////static double last_day_ts[24*60] = {0.0, };
////static double last_day_SoundLevel[24*60] = {0.0, };
//static int last_day_index = 0;

//static post_data_s postdata[POSTDATA_BUFFER_SIZE];
static int post_data_length  = 0;
static char dataString[200];

int32_t samples[SAMPLES_SHORT] = {0};

static BLESerial bleSerial;

void push_average_values(double ts, int id, double sound_level, double leq_min, double leq_hour, double leq_8hours, double leq_day);

void setup() {
  pinMode(LED, OUTPUT);
  
  Serial.begin(2000000);
  Serial.println("Configuring I2S...");
  esp_err_t err;

  // The I2S config as per the example
  const i2s_config_t i2s_config = {
      .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX), // Receive, not transfer
      .sample_rate = SAMPLE_RATE,                         // 32KHz
      .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT, // could only get it to work with 32bits
      .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // although the SEL config should be left, it seems to transmit on right
      .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,     // Interrupt level 1
      .dma_buf_count = 4,                           // number of buffers
      .dma_buf_len = BLOCK_SIZE,                    // samples per buffer
    .use_apll = true,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };

  // The pin config as per the setup
  const i2s_pin_config_t pin_config = {
      .bck_io_num = 14,   // BCKL
      .ws_io_num = 15,    // LRCL
      .data_out_num = -1, // not used (only for speakers)
      .data_in_num = 32   // DOUT
  };

  // Configuring the I2S driver and pins.
  // This function must be called before any I2S driver read/write operations.
  err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  if (err != ESP_OK) {
    Serial.printf("Failed installing driver: %d\n", err);
    while (true);
  }
  err = i2s_set_pin(I2S_PORT, &pin_config);
  if (err != ESP_OK) {
    Serial.printf("Failed setting pin: %d\n", err);
    while (true);
  }
  Serial.println("I2S driver installed.");

  bleSerial.begin("TatySound"); //Bluetooth device name
  //Serial.println("The device started, now you can pair it with bluetooth!");

  start_ts = millis();

  //FIXME: There is a known issue with esp-idf and sampling rates, see:
  //       https://github.com/espressif/esp-idf/issues/2634
  //       Update (when available) to the latest versions of esp-idf *should* fix it
  //       In the meantime, the below line seems to set sampling rate at ~47999.992Hz
  //       fifs_req=24576000, sdm0=149, sdm1=212, sdm2=5, odir=2 -> fifs_reached=24575996
  rtc_clk_apll_enable(1, 149, 212, 5, 2);
}

//double a_filter(double input) {
//  double output = input * AFILTER_Bcoef[0] + AFILTER_conditions[0];
//  for (int j = 0; j < 5; j++) {
//    AFILTER_conditions[j] = input * AFILTER_Bcoef[j + 1] - output * AFILTER_Acoef[j + 1] + AFILTER_conditions[j + 1];
//  }
//  AFILTER_conditions[5] = input * AFILTER_Bcoef[6] - output * AFILTER_Acoef[6];
//
//  return output;
//}
//
//double correctdB(double input) {
//  double corrected = ((input - CORR_X0) * ((CORR_REF1 - CORR_REF0) / (CORR_X1 - CORR_X0))) + CORR_REF0;
//  return corrected;
//}

void get_samples() {
  // Read multiple samples at once and calculate the sound pressure

  int total_samples = 0;
  double sumsquare = 0;
  size_t num_bytes_read;
  uint64_t sum_sqr_SPL = 0;
  uint64_t sum_sqr_weighted = 0;
  int32_t s;

  double avg = 0;

  //discard samples 
       i2s_read(I2S_PORT, (char *)samples, 
                                        BLOCK_SIZE*4,     // the doc says bytes, but its elements.
                                        &num_bytes_read,
                                        portMAX_DELAY); // no timeout

  int64_t meanval = 0;
  while (total_samples < SAMPLES_SHORT) {
     i2s_read(I2S_PORT, (char *)&samples[total_samples], 
                                        BLOCK_SIZE*4,     // the doc says bytes, but its elements.
                                        &num_bytes_read,
                                        portMAX_DELAY); // no timeout

    int samples_read = num_bytes_read /4;

    for (int i=0; i<samples_read; i++) {
      samples[total_samples+i] = samples[total_samples+i]>>14; 
    }

    for (int i=0; i<samples_read; i++) {
      //samples[total_samples+i] = samples[total_samples+i]>>6; // only first 18 bits have value
      //meanval += samples[total_samples+i];
      avg = 0.999  * avg + 0.001 * samples[total_samples+i];
      //printf("%d: %ld, %lld\n", i, samples[total_samples+i], meanval);
    }

    total_samples += samples_read;
    //printf("+%d = %d\n", samples_read, total_samples);
  }

  //meanval /= total_samples;
  meanval = avg;
  //printf("Meanvalue of %d =  %lld\n", total_samples, meanval);

    
  for (int i=0; i<total_samples; i++) {
      samples[i] -= meanval;
      
      //printf("%d\n", samples[i]);

      //samples[i] = ICS43434.filter(samples[i]);
      
//      Serial.println (samples[i], DEC);
      //printf("%d, %d\n", samples[i], s);

      
      sum_sqr_SPL += samples[i] * samples[i];

      //printf("%d, %f, %d\n", samples[i], weighted, sum_sqr_SPL);
      //printf("%f, %f\n", fraction, sum_sqr_SPL);
      
      int32_t weighted = A_weighting.filter(samples[i]);
      sum_sqr_weighted += weighted * weighted;
  
      //Serial.println(fraction);
      //Serial.println(weighted);
      
      //double filtered = a_filter(fraction);
      //Serial.println(fraction);
      //sumsquare += filtered * filtered;
  }

  double short_SPL_dB = MIC_OFFSET_DB + MIC_REF_DB + 20 * log10(sqrt(sum_sqr_SPL / total_samples) / MIC_REF_AMPL);
  double short_SPL_dBA = MIC_OFFSET_DB + MIC_REF_DB + 20 * log10(sqrt(sum_sqr_weighted / total_samples) / MIC_REF_AMPL);
  //printf("%lld, %llu, %f\n", meanval, sum_sqr_SPL, short_SPL_dB);
  Serial.println(short_SPL_dBA);
  //printf("%f,%f,%f,%f\n", sqrt(sum_sqr_SPL / total_samples), short_SPL_dB, sqrt(sum_sqr_weighted / total_samples), short_SPL_dBA);
  //Serial.println(short_SPL_dBA);
  
  //printf("%f, %f\n", short_SPL_dB, short_SPL_dBA);
//  Serial.print("sum_sqr_SPL: "); Serial.print(sum_sqr_SPL);
//  Serial.print(" - "); Serial.print(double(sum_sqr_SPL) / SAMPLES_SHORT);
//  Serial.print(" - "); Serial.print(sqrt(double(sum_sqr_SPL) / SAMPLES_SHORT));
//  Serial.print(" - "); Serial.print(sqrt(double(sum_sqr_SPL) / SAMPLES_SHORT)/ MIC_REF_AMPL);
//  Serial.print(" - "); Serial.print(log10(sqrt(double(sum_sqr_SPL) / SAMPLES_SHORT) / MIC_REF_AMPL));
//  Serial.print(" - "); Serial.println(20 * log10(sqrt(double(sum_sqr_SPL) / SAMPLES_SHORT) / MIC_REF_AMPL));
//  Serial.print("short_SPL_dB: "); Serial.println(short_SPL_dB);
  // Calculate dB values relative to MIC_REF_AMPL and adjust for reference dB
    

//  double soundLevel = sumsquare / total_samples;
//  //Serial.print("soundLevel: "); Serial.println(soundLevel);
//  cumulativeSoundLevel += soundLevel;
//  cumulativeSoundCounter++;
//
//
//  double leq = ((10.0 * log10(soundLevel)) + 93.97940);
//  //Serial.println(leq);
//  double correctedLeq  = soundLevel;
//  //double correctedLeq = leq; //correctdB(leq);
//
//  process_data(leq, correctedLeq);
                                    
}

//void process_data(double leq, double correctedLeq) {
//
//  unsigned long ts = millis();
//    
////  snprintf(dataString, sizeof(dataString), "1,%.1f,%.1f\n", leq, correctedLeq);
////  if (SerialBT.hasClient()) {   
////    SerialBT.print(dataString);
////    Serial.print("BT:");
////  } else {
////    // store
////  }
////  Serial.print(dataString);
//
//  if (ts - start_ts >= MIN_RECORDING_INTERVAL) {
//    double avg_sound_level = cumulativeSoundLevel / cumulativeSoundCounter;
//    double avg_leq = ((10.0 * log10(avg_sound_level)) + 93.97940008672037609572522210551);
//   // double corr_avg_leqmin = correctdB(avg_leq);
//
//    //snprintf(dataString, sizeof(dataString), "1,%.1f,%.1f\n", avg_leq, corr_avg_leqmin);
////    if (SerialBT.hasClient()) {   
////      SerialBT.print(dataString);
////      Serial.print("BT:");
////    }
////    Serial.print(dataString);
//
//    if (last_day_index == 60*24) {
//      //rotate
//      for (int i = 0; i < last_day_index-1; i++)
//      {
//        last_day_ts[i] = last_day_ts[i+1];
//        last_day_SoundLevel[i] = last_day_SoundLevel[i+1];
//      }
//      last_day_index--;
//    }
//    last_day_ts[last_day_index] = start_ts;
//    last_day_SoundLevel[last_day_index] = avg_sound_level;
//
//    int last_hour_counter = last_day_index < 60 ? last_day_index : 60;
//    double sum_last_hour = 0;
//    for (int i = last_day_index - last_hour_counter; i < last_day_index; i++) {
//      sum_last_hour += last_day_SoundLevel[i];
//    }
//    double last_hour = last_hour_counter > 0 ? sum_last_hour/ last_hour_counter : avg_sound_level;
//    double leq_last_hour = correctdB(((10.0 * log10(last_hour)) + 93.97940008672037609572522210551));
//
//    int last_8hour_counter = last_day_index < 8 * 60 ? last_day_index : 8 * 60;
//    int last_8hour_start = last_8hour_counter < 60 ? last_8hour_counter : 60;
//    double sum_last_8hour = sum_last_hour;
//    for (int i = last_day_index - last_8hour_counter + last_8hour_start; i < last_day_index; i++) {
//      sum_last_8hour += last_day_SoundLevel[i];
//    }
//
//    double last_8hour = last_8hour_counter > 0 ? sum_last_8hour / last_8hour_counter : avg_sound_level;
//    double leq_last_8hour = correctdB(((10.0 * log10(last_8hour)) + 93.97940008672037609572522210551));
//
//    int last_day_counter = last_day_index < 24 * 60 ? last_day_index : 24 * 60;
//    int last_day_start = last_day_counter < 8 * 60 ? last_day_counter : 8 * 60;
//    double sum_last_day = sum_last_8hour;
//    for (int i = last_day_index - last_day_counter + last_day_start; i < last_day_index; i++) {
//      sum_last_day += last_day_SoundLevel[i];
//    }
//    double last_day = last_day_counter > 0 ? sum_last_day / last_day_counter : avg_sound_level;
//    double leq_last_day = correctdB(((10.0 * log10(last_day)) + 93.97940008672037609572522210551));
//
//    //snprintf(dataString, sizeof(dataString), "2,%.1f,%.1f,%.1f\n", leq_last_hour, leq_last_8hour, leq_last_day);
////    if (SerialBT.hasClient()) {   
////      SerialBT.print(dataString);
////      Serial.print("BT:");
////    }
////    Serial.print(dataString);
//    
//    cumulativeSoundLevel = 0;
//    cumulativeSoundCounter = 0;
//    last_day_index++;
//    start_ts += MIN_RECORDING_INTERVAL;
//
//    push_average_values((int) (ts / 1000), last_day_index-1, avg_leq, corr_avg_leqmin, leq_last_hour, leq_last_8hour, leq_last_day);
//  } else {
//    
//    
//    if (bleSerial.connected())
//    {
//      int lenght = snprintf(dataString, sizeof(dataString), "1,%.1f,%.8f;", leq, correctedLeq);
//      bleSerial.write((uint8_t*) dataString, lenght);
//      
//      Serial.println(dataString);
//      
//      static int led_on = 0;
//      if (led_on)
//        digitalWrite(LED, LOW);
//      else
//        digitalWrite(LED, HIGH);
//
//     led_on = ~led_on;
//      
//    } else {
//      digitalWrite(LED, LOW);
//    }
//  }
//}

//bool send_to_bluetooth() {
//  while (post_data_length > -1)
//  {
//    int lenght = snprintf(dataString, sizeof(dataString), "0,%d,%d,%.1f,%.1f,%.1f,%.1f,%.1f,%d,%d;", postdata[post_data_length].ts, postdata[post_data_length].id, postdata[post_data_length].sound_level, postdata[post_data_length].leq_min, postdata[post_data_length].leq_hour, postdata[post_data_length].leq_8hours, postdata[post_data_length].leq_day, postdata[post_data_length].response, post_data_length);
//    Serial.println(dataString);
//    size_t ret = bleSerial.write((uint8_t*) dataString, lenght);
//    if (ret == 0) return false;
//
//    post_data_length--;
//    delay(50);
//  }
//    
////  for (int i = 0; i < post_data_length+1; i++) {
////    snprintf(dataString, sizeof(dataString), "0,%d,%d,%.1f,%.1f,%.1f,%.1f,%.1f,%d\n", postdata[i].ts, postdata[i].id, postdata[i].sound_level, postdata[i].leq_min, postdata[i].leq_hour, postdata[i].leq_8hours, postdata[i].leq_day, postdata[i].response);
////    Serial.print(dataString);
////    size_t ret = bleSerial.print(dataString);
////    if (ret == 0) return false;
////  }
//
//  return true;
//}
//
//void push_average_values(int ts, int id, double sound_level, double leq_min, double leq_hour, double leq_8hours, double leq_day) {
//  char dataString[200];
//  snprintf(dataString, sizeof(dataString), "%d %d %d (%.1f) %.1f %.1f %.1f %.1f", post_data_length, ts, id, sound_level, leq_min, leq_hour, leq_8hours, leq_day);
//  Serial.println(dataString);
//
//  postdata[post_data_length].ts = ts;
//  postdata[post_data_length].id = id;
//  postdata[post_data_length].sound_level = sound_level;
//  postdata[post_data_length].leq_min = leq_min;
//  postdata[post_data_length].leq_hour = leq_hour;
//  postdata[post_data_length].leq_8hours = leq_8hours;
//  postdata[post_data_length].leq_day = leq_day;
//  postdata[post_data_length].response = 0;
//  bool transmitted = false; 
//  if (bleSerial.connected()) {
//    transmitted = send_to_bluetooth();
//  }
//  
//  if (transmitted) {
//      post_data_length = 0;
//  } else {
//    postdata[post_data_length].response = 1;
//    if (post_data_length > 0)
//      postdata[post_data_length].response += postdata[post_data_length-1].response;
//
//    post_data_length++;
//  }
//}


void loop() {
  //Serial.println("Sampling...");
  get_samples();

  delay(500);
  esp_sleep_enable_timer_wakeup(5000 * 1000);
  esp_light_sleep_start(); 
}
