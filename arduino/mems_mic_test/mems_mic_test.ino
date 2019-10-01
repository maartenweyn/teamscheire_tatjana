#include <driver/i2s.h>
#include "BLESerial.h"
#include <string.h>

const i2s_port_t I2S_PORT = I2S_NUM_0;
const int SAMPLE_RATE = 16000; // 16kHz
const int BLOCK_SIZE = 1024;
const int SAMPLE_LENGTH = SAMPLE_RATE * 1; // 1 second of data //128;
const int MIN_RECORDING_INTERVAL = 60000; //60*1000;

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

static const double AFILTER_Acoef[] = {1.0, -4.0195761811158315, 6.1894064429206921, -4.4531989035441155,
                    1.4208429496218764, -0.14182547383030436,
                    0.0043511772334950787};
static const double AFILTER_Bcoef[] = {0.2557411252042574, -0.51148225040851436,
                    -0.25574112520425807, 1.0229645008170318,
                    -0.25574112520425918, -0.51148225040851414,
                    0.25574112520425729};
static double AFILTER_conditions[] = {0, 0, 0, 0, 0, 0};

static double cumulativeSoundLevel;
static int cumulativeSoundCounter = 0;
static unsigned long start_ts = 0;
static double last_day_ts[24*60] = {0.0, };
static double last_day_SoundLevel[24*60] = {0.0, };
static int last_day_index = 0;

static post_data_s postdata[POSTDATA_BUFFER_SIZE];
static int post_data_length  = 0;
static char dataString[200];

static BLESerial bleSerial;

void push_average_values(double ts, int id, double sound_level, double leq_min, double leq_hour, double leq_8hours, double leq_day);

void setup() {
  pinMode(LED, OUTPUT);
  
  Serial.begin(115200);
  Serial.println("Configuring I2S...");
  esp_err_t err;

  // The I2S config as per the example
  const i2s_config_t i2s_config = {
      .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX), // Receive, not transfer
      .sample_rate = SAMPLE_RATE,                         // 16KHz
      .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT, // could only get it to work with 32bits
      .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT, // although the SEL config should be left, it seems to transmit on right
      .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,     // Interrupt level 1
      .dma_buf_count = 4,                           // number of buffers
      .dma_buf_len = BLOCK_SIZE                     // samples per buffer
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
  Serial.println("The device started, now you can pair it with bluetooth!");

  start_ts = millis();
}

double a_filter(double input) {
  double output = input * AFILTER_Bcoef[0] + AFILTER_conditions[0];
  for (int j = 0; j < 5; j++) {
    AFILTER_conditions[j] = input * AFILTER_Bcoef[j + 1] - output * AFILTER_Acoef[j + 1] + AFILTER_conditions[j + 1];
  }
  AFILTER_conditions[5] = input * AFILTER_Bcoef[6] - output * AFILTER_Acoef[6];

  return output;
}

double correctdB(double input) {
  double corrected = ((input - CORR_X0) * ((CORR_REF1 - CORR_REF0) / (CORR_X1 - CORR_X0))) + CORR_REF0;
  return corrected;
}

void get_samples() {
  // Read multiple samples at once and calculate the sound pressure
  int32_t samples[BLOCK_SIZE] = {0};
  int total_samples = 0;
  double sumsquare = 0;
  
  while (total_samples < SAMPLE_LENGTH) {
    int num_bytes_read = i2s_read_bytes(I2S_PORT, 
                                        (char *)samples, 
                                        BLOCK_SIZE*4,     // the doc says bytes, but its elements.
                                        portMAX_DELAY); // no timeout
  
//    for(int i = 0; i < BLOCK_SIZE; i++)
//    {
//      Serial.println(samples[i]);
//    }
  

    //Serial.print("num_bytes_read "); Serial.println(num_bytes_read);
    int samples_read = num_bytes_read /4;
    for (int i=0; i<samples_read; i++) {
      double fraction = 0.0;
      if (samples[i] < 0) 
        fraction = samples[i] / 2147483648.0;
      else
        fraction = samples[i] / 2147483647.0;
  
     // Serial.println(fraction);
      
      double filtered = a_filter(fraction);
      sumsquare += filtered * filtered;
    }

    total_samples += samples_read;
  }

  double soundLevel = sumsquare / total_samples;
  //Serial.print("soundLevel: "); Serial.println(soundLevel);
  cumulativeSoundLevel += soundLevel;
  cumulativeSoundCounter++;


  double leq = ((10.0 * log10(soundLevel)) + 93.97940008672037609572522210551);
  double correctedLeq = leq; //correctdB(leq);

  process_data(leq, correctedLeq);
                                    
}

void process_data(double leq, double correctedLeq) {

  unsigned long ts = millis();
    
//  snprintf(dataString, sizeof(dataString), "1,%.1f,%.1f\n", leq, correctedLeq);
//  if (SerialBT.hasClient()) {   
//    SerialBT.print(dataString);
//    Serial.print("BT:");
//  } else {
//    // store
//  }
//  Serial.print(dataString);

  if (ts - start_ts >= MIN_RECORDING_INTERVAL) {
    double avg_sound_level = cumulativeSoundLevel / cumulativeSoundCounter;
    double avg_leq = ((10.0 * log10(avg_sound_level)) + 93.97940008672037609572522210551);
    double corr_avg_leqmin = correctdB(avg_leq);

    //snprintf(dataString, sizeof(dataString), "1,%.1f,%.1f\n", avg_leq, corr_avg_leqmin);
//    if (SerialBT.hasClient()) {   
//      SerialBT.print(dataString);
//      Serial.print("BT:");
//    }
//    Serial.print(dataString);

    if (last_day_index == 60*24) {
      //rotate
      for (int i = 0; i < last_day_index-1; i++)
      {
        last_day_ts[i] = last_day_ts[i+1];
        last_day_SoundLevel[i] = last_day_SoundLevel[i+1];
      }
      last_day_index--;
    }
    last_day_ts[last_day_index] = start_ts;
    last_day_SoundLevel[last_day_index] = avg_sound_level;

    int last_hour_counter = last_day_index < 60 ? last_day_index : 60;
    double sum_last_hour = 0;
    for (int i = last_day_index - last_hour_counter; i < last_day_index; i++) {
      sum_last_hour += last_day_SoundLevel[i];
    }
    double last_hour = last_hour_counter > 0 ? sum_last_hour/ last_hour_counter : avg_sound_level;
    double leq_last_hour = correctdB(((10.0 * log10(last_hour)) + 93.97940008672037609572522210551));

    int last_8hour_counter = last_day_index < 8 * 60 ? last_day_index : 8 * 60;
    int last_8hour_start = last_8hour_counter < 60 ? last_8hour_counter : 60;
    double sum_last_8hour = sum_last_hour;
    for (int i = last_day_index - last_8hour_counter + last_8hour_start; i < last_day_index; i++) {
      sum_last_8hour += last_day_SoundLevel[i];
    }

    double last_8hour = last_8hour_counter > 0 ? sum_last_8hour / last_8hour_counter : avg_sound_level;
    double leq_last_8hour = correctdB(((10.0 * log10(last_8hour)) + 93.97940008672037609572522210551));

    int last_day_counter = last_day_index < 24 * 60 ? last_day_index : 24 * 60;
    int last_day_start = last_day_counter < 8 * 60 ? last_day_counter : 8 * 60;
    double sum_last_day = sum_last_8hour;
    for (int i = last_day_index - last_day_counter + last_day_start; i < last_day_index; i++) {
      sum_last_day += last_day_SoundLevel[i];
    }
    double last_day = last_day_counter > 0 ? sum_last_day / last_day_counter : avg_sound_level;
    double leq_last_day = correctdB(((10.0 * log10(last_day)) + 93.97940008672037609572522210551));

    //snprintf(dataString, sizeof(dataString), "2,%.1f,%.1f,%.1f\n", leq_last_hour, leq_last_8hour, leq_last_day);
//    if (SerialBT.hasClient()) {   
//      SerialBT.print(dataString);
//      Serial.print("BT:");
//    }
//    Serial.print(dataString);
    
    cumulativeSoundLevel = 0;
    cumulativeSoundCounter = 0;
    last_day_index++;
    start_ts += MIN_RECORDING_INTERVAL;

    push_average_values((int) (ts / 1000), last_day_index-1, avg_leq, corr_avg_leqmin, leq_last_hour, leq_last_8hour, leq_last_day);
  } else {
    
    
    if (bleSerial.connected())
    {
      int lenght = snprintf(dataString, sizeof(dataString), "1,%.1f,%.1f;", leq, correctedLeq);
      bleSerial.write((uint8_t*) dataString, lenght);
      
      Serial.println(dataString);
      
      static int led_on = 0;
      if (led_on)
        digitalWrite(LED, LOW);
      else
        digitalWrite(LED, HIGH);

     led_on = ~led_on;
      
    } else {
      digitalWrite(LED, LOW);
    }
  }
}

bool send_to_bluetooth() {
  while (post_data_length > -1)
  {
    int lenght = snprintf(dataString, sizeof(dataString), "0,%d,%d,%.1f,%.1f,%.1f,%.1f,%.1f,%d,%d;", postdata[post_data_length].ts, postdata[post_data_length].id, postdata[post_data_length].sound_level, postdata[post_data_length].leq_min, postdata[post_data_length].leq_hour, postdata[post_data_length].leq_8hours, postdata[post_data_length].leq_day, postdata[post_data_length].response, post_data_length);
    Serial.println(dataString);
    size_t ret = bleSerial.write((uint8_t*) dataString, lenght);
    if (ret == 0) return false;

    post_data_length--;
    delay(50);
  }
    
//  for (int i = 0; i < post_data_length+1; i++) {
//    snprintf(dataString, sizeof(dataString), "0,%d,%d,%.1f,%.1f,%.1f,%.1f,%.1f,%d\n", postdata[i].ts, postdata[i].id, postdata[i].sound_level, postdata[i].leq_min, postdata[i].leq_hour, postdata[i].leq_8hours, postdata[i].leq_day, postdata[i].response);
//    Serial.print(dataString);
//    size_t ret = bleSerial.print(dataString);
//    if (ret == 0) return false;
//  }

  return true;
}

void push_average_values(int ts, int id, double sound_level, double leq_min, double leq_hour, double leq_8hours, double leq_day) {
  char dataString[200];
  snprintf(dataString, sizeof(dataString), "%d %d %d (%.1f) %.1f %.1f %.1f %.1f", post_data_length, ts, id, sound_level, leq_min, leq_hour, leq_8hours, leq_day);
  Serial.println(dataString);

  postdata[post_data_length].ts = ts;
  postdata[post_data_length].id = id;
  postdata[post_data_length].sound_level = sound_level;
  postdata[post_data_length].leq_min = leq_min;
  postdata[post_data_length].leq_hour = leq_hour;
  postdata[post_data_length].leq_8hours = leq_8hours;
  postdata[post_data_length].leq_day = leq_day;
  postdata[post_data_length].response = 0;
  bool transmitted = false; 
  if (bleSerial.connected()) {
    transmitted = send_to_bluetooth();
  }
  
  if (transmitted) {
      post_data_length = 0;
  } else {
    postdata[post_data_length].response = 1;
    if (post_data_length > 0)
      postdata[post_data_length].response += postdata[post_data_length-1].response;

    post_data_length++;
  }
}


void loop() {
  //Serial.println("Sampling...");
  get_samples();

  delay(4000);
  //esp_sleep_enable_timer_wakeup(4000 * 1000);
  //esp_light_sleep_start(); 
}
