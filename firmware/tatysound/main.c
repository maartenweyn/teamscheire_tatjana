
#include "app_timer.h"
#include "app_uart.h"
#include "app_util_platform.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "ble_hci.h"
#include "ble_nus.h"
#include "bsp_btn_ble.h"
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_ble_gatt.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_soc.h"
#include <stdint.h>
#include <string.h>

#include "math.h"
#include "nrf_drv_pdm.h"

#include "app_pwm.h"

#include "i2c.h"
#include "lp55231.h"
#include "nrf_delay.h"
#include "nrf_drv_twi.h"
#include "nrf_drv_i2s.h"

#include "ble_app.h"
#include "config.h"
#include "lsm303_agr.h"
#include "battery.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "nrfx_pdm.h"
#include "nrf_calendar.h"

#include <nrf_drv_gpiote.h>
#include <nrf_pwr_mgmt.h>

APP_TIMER_DEF(m_button_timer_id);
APP_TIMER_DEF(m_light_timer_id);

extern nrf_twi_mngr_t m_nrf_twi_mngr;
static bsp_indication_t m_stable_state = BSP_INDICATE_IDLE;
static bool m_leds_clear = false;
static bool m_alert_on = false;
APP_TIMER_DEF(m_app_leds_tmr);
APP_TIMER_DEF(m_app_alert_tmr);
APP_TIMER_DEF(m_sound_tmr);

#define on(led)     nrf_gpio_pin_set(led)
#define off(led)    nrf_gpio_pin_clear(led)

typedef struct {
  uint8_t type;
  uint32_t time;
  uint16_t cdBSPL;
} sound_data_element_t;

sound_data_element_t ble_data[MAX_BLE_DATA];
uint16_t ble_data_size = 0;

static volatile uint8_t led_status[8][3] = {0};
static volatile bool transmitting = false;
static uint8_t led_driver_status[3] = {0};
static uint8_t light_brightness = 50;
static uint8_t front_light_status = 0;

static uint32_t time_stamp = 0;
static uint32_t prev_stamp = 0;

static uint8_t front_light_brightness_changed = 0;
static uint8_t button_pushed = 0;

uint32_t app_indication_set(bsp_indication_t indicate);
static void app_leds_timer_handler(void *p_context);
static void app_alert_timer_handler(void *p_context);
static void sound_timer_handler(void *p_context);
void set_led(uint8_t led_nr, uint8_t color, uint8_t brightness);
void set_front_light(uint8_t status);
static uint32_t send_data_to_ble(uint8_t *data, uint16_t length);
void set_all_leds_color(uint8_t color, uint8_t brightness);

uint8_t sleep_status = 0; // 0 can go to sleep,  1 keep awake
extern uint8_t ble_state;

static bool first_sample = true;

                                                                           
//static uint32_t m_buffer_rx[];
//static int32_t soundbuffer[SHORT_SAMPLE];
static int16_t soundbuffer[3][SOUND_BUFFER_SIZE];
static uint16_t buffer_offset = 0;
static uint8_t buffer_index = 0;
static volatile bool sampling_running = true;

static nrfx_pdm_config_t pdm_cfg = NRFX_PDM_DEFAULT_CONFIG(CONFIG_IO_PDM_CLK, CONFIG_IO_PDM_DATA);

static volatile int soundbuffer_position = -SKIP_BUFFERS;
static double prev_avg = 0;
static volatile bool parse_sound = false;

static volatile uint8_t     pwm_ready = 0;                                                                       // A PWM ready status

static void pdm_event_handler(nrfx_pdm_evt_t const * const p_evt);

uint32_t start_sound_measurement() {

  NRF_LOG_DEBUG("Starting pdm");
  
  app_indication_set(BSP_INDICATE_USER_STATE_2);

  //buffer_index = 0;
  int32_t err_code = 0;
  //uint32_t err_code = nrfx_pdm_buffer_set(soundbuffer[buffer_index], SOUND_BUFFER_SIZE);
  //APP_ERROR_CHECK(err_code);

 // err_code = nrfx_pdm_start();

  err_code = nrfx_pdm_init(&pdm_cfg, pdm_event_handler);

    if (err_code != 0) {
    NRF_LOG_INFO("cannot nrfx_pdm_init\n");
  }    

  err_code = nrfx_pdm_start();
  
  if (err_code != 0) {
    NRF_LOG_INFO("cannot nrfx_pdm_start\n");
  }    

  if (err_code == NRFX_SUCCESS)
    sampling_running = true;


  return err_code;

}

void stop_sound_measurement() {

  NRF_LOG_DEBUG("Stopping pdm");

  nrfx_pdm_uninit();

  sampling_running = false;
}

/**@brief Function for handling the data from the Nordic UART Service.
 *
 * @details This function will process the data received from the Nordic UART BLE Service and send
 *          it to the UART module.
 *
 * @param[in] p_nus    Nordic UART Service structure.
 * @param[in] p_data   Data to be send to UART module.
 * @param[in] length   Length of the data.
 */
/**@snippet [Handling the data received over BLE] */
static void nus_data_handler(ble_nus_evt_t *p_evt) {
  NRF_LOG_DEBUG("nus_data_handler %d", p_evt->type);
  if (p_evt->type == BLE_NUS_EVT_RX_DATA) {
    uint32_t err_code;

    NRF_LOG_DEBUG("Received data from BLE NUS., length %d:", p_evt->params.rx_data.length);
    NRF_LOG_HEXDUMP_DEBUG(p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);

    uint8_t type = p_evt->params.rx_data.p_data[2];
    uint8_t length = p_evt->params.rx_data.p_data[3];

    switch (type) {
      case 1:
      {
        uint32_t* ts = (uint32_t*) &p_evt->params.rx_data.p_data[4];
        NRF_LOG_INFO("Received time: %d", *ts);
        time_stamp = *ts;
        break;
      }
      default:
        NRF_LOG_ERROR("Unkown data type received: %d", type);
    }

  }
}

/* BUTTONS */

void in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
  //nrf_drv_gpiote_out_toggle(PIN_OUT);
  int32_t pin_state = nrf_gpio_pin_read(pin);
  NRF_LOG_INFO("in_pin_handler %d - %d / %d", pin, action, pin_state);

//   if (pin_state == 0) {
//      set_all_leds_color(GREEN, 50);
//    } else {
//      set_all_leds_color(GREEN, 0);
//    }
}

ret_code_t init_buttons() {
  ret_code_t err_code;

  err_code = nrfx_gpiote_init();
  APP_ERROR_CHECK(err_code);

  nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
  in_config.pull = NRF_GPIO_PIN_PULLUP;

  err_code = nrf_drv_gpiote_in_init(BUTTON1, &in_config, in_pin_handler);
  APP_ERROR_CHECK(err_code);

  nrf_drv_gpiote_in_event_enable(BUTTON1, true);

  return err_code;
}

/* LEDS */


static uint32_t send_data_to_ble(uint8_t *data, uint16_t length) {
  NRF_LOG_INFO("send_data_to_ble");

  uint32_t err_code;

  do {
    err_code = send_data(data, length);
    if (err_code != NRF_ERROR_BUSY) {
      if (err_code != NRF_SUCCESS) {
        NRF_LOG_INFO("send_data error %d\n", err_code);
      }
    }
  } while (err_code == NRF_ERROR_BUSY);

  return err_code;
}

/* TIMERS */

/**@brief Function for initializing the timer module and the application timers
 */
static void timers_init(void) {
  // Initialize timer module.
  ret_code_t err_code = app_timer_init();
  APP_ERROR_CHECK(err_code);

  // Create timers.
  err_code = app_timer_create(&m_app_leds_tmr, APP_TIMER_MODE_SINGLE_SHOT, app_leds_timer_handler);
  APP_ERROR_CHECK(err_code);

  err_code = app_timer_create(&m_sound_tmr, APP_TIMER_MODE_REPEATED, sound_timer_handler);
  APP_ERROR_CHECK(err_code);
}

/**@brief Handle events from leds timer.
 *
 * @note Timer handler does not support returning an error code.
 * Errors from app_indication_set() are not propagated.
 *
 * @param[in]   p_context   parameter registered in timer start function.
 */
static void app_leds_timer_handler(void *p_context) {
  //UNUSED_PARAMETER(p_context);
  app_indication_set(m_stable_state);
}


/**@brief Handle events from alert timer.
 *
 * @param[in]   p_context   parameter registered in timer start function.
 */

static void sound_timer_handler(void *p_context) {

  time_stamp += SOUND_TIMER_INTERVAL_SEC;

  UNUSED_PARAMETER(p_context);

  NRF_LOG_INFO("sound_timer_handler  %d", sampling_running);

  if (!sampling_running) {
    start_sound_measurement();
  } else {
    NRF_LOG_INFO("sound_timer_handler ERROR: %d", sampling_running);
    app_indication_set(BSP_INDICATE_SEND_ERROR);
    stop_sound_measurement();
  }
}

/**@brief Function for initializing the nrf log module.
 */
static void log_init(void) {
  ret_code_t err_code = NRF_LOG_INIT(NULL);
  APP_ERROR_CHECK(err_code);

  NRF_LOG_DEFAULT_BACKENDS_INIT();
}

/**@brief Function for initializing power management.
 */
static void power_management_init(void) {
  ret_code_t err_code;
  err_code = nrf_pwr_mgmt_init();
  APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
static void idle_state_handle(void) {
  UNUSED_RETURN_VALUE(NRF_LOG_PROCESS());
  nrf_pwr_mgmt_run();
}

static void application_timers_start(void) {
  ret_code_t err_code;
  //err_code = app_timer_start(m_app_timer_id, TIMER_INTERVAL, NULL);
  APP_ERROR_CHECK(err_code);
}

/**@brief Configure indicators to required state.
 */
uint32_t app_indication_set(bsp_indication_t indicate) {
  uint32_t err_code = NRF_SUCCESS;
  uint32_t next_delay = 0;

  if (m_leds_clear) {
    m_leds_clear = false;
    leds_set_all(0);
  }

  switch (indicate) {
  case BSP_INDICATE_IDLE:
    off(LED_RED);
    off(LED_GREEN);
    off(LED_BLUE);
    m_stable_state = indicate;
    break;

  case BSP_INDICATE_SCANNING:
  case BSP_INDICATE_ADVERTISING:
    // in advertising blink LED_0
//    if (led_status[LED_INDICATE_ADVERTISING][LED_INDICATE_ADVERTISING_COLOR_ID]) {
//      set_led(LED_INDICATE_ADVERTISING, LED_INDICATE_ADVERTISING_COLOR, 0);
//      next_delay = indicate ==
//                           BSP_INDICATE_ADVERTISING
//                       ? ADVERTISING_LED_OFF_INTERVAL
//                       : ADVERTISING_SLOW_LED_OFF_INTERVAL;
//    } else {
//      set_led(LED_INDICATE_ADVERTISING, LED_INDICATE_ADVERTISING_COLOR, LED_BRIGHTNESS);
//      next_delay = indicate ==
//                           BSP_INDICATE_ADVERTISING
//                       ? ADVERTISING_LED_ON_INTERVAL
//                       : ADVERTISING_SLOW_LED_ON_INTERVAL;
//    }

    m_stable_state = indicate;
    err_code = app_timer_start(m_app_leds_tmr, APP_TIMER_TICKS(next_delay), NULL);
    break;
    //
    //        case BSP_INDICATE_ADVERTISING_WHITELIST:
    //            // in advertising quickly blink LED_0
    //            if (bsp_board_led_state_get(BSP_LED_INDICATE_ADVERTISING_WHITELIST))
    //            {
    //                bsp_board_led_off(BSP_LED_INDICATE_ADVERTISING_WHITELIST);
    //                next_delay = indicate ==
    //                             BSP_INDICATE_ADVERTISING_WHITELIST ?
    //                             ADVERTISING_WHITELIST_LED_OFF_INTERVAL :
    //                             ADVERTISING_SLOW_LED_OFF_INTERVAL;
    //            }
    //            else
    //            {
    //                bsp_board_led_on(BSP_LED_INDICATE_ADVERTISING_WHITELIST);
    //                next_delay = indicate ==
    //                             BSP_INDICATE_ADVERTISING_WHITELIST ?
    //                             ADVERTISING_WHITELIST_LED_ON_INTERVAL :
    //                             ADVERTISING_SLOW_LED_ON_INTERVAL;
    //            }
    //            m_stable_state = indicate;
    //            err_code       = app_timer_start(m_app_leds_tmr, APP_TIMER_TICKS(next_delay), NULL);
    //            break;
    //
    //        case BSP_INDICATE_ADVERTISING_SLOW:
    //            // in advertising slowly blink LED_0
    //            if (bsp_board_led_state_get(BSP_LED_INDICATE_ADVERTISING_SLOW))
    //            {
    //                bsp_board_led_off(BSP_LED_INDICATE_ADVERTISING_SLOW);
    //                next_delay = indicate ==
    //                             BSP_INDICATE_ADVERTISING_SLOW ? ADVERTISING_SLOW_LED_OFF_INTERVAL :
    //                             ADVERTISING_SLOW_LED_OFF_INTERVAL;
    //            }
    //            else
    //            {
    //                bsp_board_led_on(BSP_LED_INDICATE_ADVERTISING_SLOW);
    //                next_delay = indicate ==
    //                             BSP_INDICATE_ADVERTISING_SLOW ? ADVERTISING_SLOW_LED_ON_INTERVAL :
    //                             ADVERTISING_SLOW_LED_ON_INTERVAL;
    //            }
    //            m_stable_state = indicate;
    //            err_code       = app_timer_start(m_app_leds_tmr, APP_TIMER_TICKS(next_delay), NULL);
    //            break;
    //
    //        case BSP_INDICATE_ADVERTISING_DIRECTED:
    //            // in advertising very quickly blink LED_0
    //            if (bsp_board_led_state_get(BSP_LED_INDICATE_ADVERTISING_DIRECTED))
    //            {
    //                bsp_board_led_off(BSP_LED_INDICATE_ADVERTISING_DIRECTED);
    //                next_delay = indicate ==
    //                             BSP_INDICATE_ADVERTISING_DIRECTED ?
    //                             ADVERTISING_DIRECTED_LED_OFF_INTERVAL :
    //                             ADVERTISING_SLOW_LED_OFF_INTERVAL;
    //            }
    //            else
    //            {
    //                bsp_board_led_on(BSP_LED_INDICATE_ADVERTISING_DIRECTED);
    //                next_delay = indicate ==
    //                             BSP_INDICATE_ADVERTISING_DIRECTED ?
    //                             ADVERTISING_DIRECTED_LED_ON_INTERVAL :
    //                             ADVERTISING_SLOW_LED_ON_INTERVAL;
    //            }
    //            m_stable_state = indicate;
    //            err_code       = app_timer_start(m_app_leds_tmr, APP_TIMER_TICKS(next_delay), NULL);
    //            break;
    //
    //        case BSP_INDICATE_BONDING:
    //            // in bonding fast blink LED_0
    //            bsp_board_led_invert(BSP_LED_INDICATE_BONDING);
    //
    //            m_stable_state = indicate;
    //            err_code       =
    //                app_timer_start(m_app_leds_tmr, APP_TIMER_TICKS(BONDING_INTERVAL), NULL);
    //            break;
    //
  case BSP_INDICATE_CONNECTED:
    NRF_LOG_INFO("BSP_INDICATE_CONNECTED");
//    if (led_status[LED_INDICATE_CONNECTED][LED_INDICATE_ADVERTISING_COLOR_ID])
//      set_led(LED_INDICATE_ADVERTISING, LED_INDICATE_ADVERTISING_COLOR, 0);

//    if (led_status[LED_INDICATE_CONNECTED][LED_INDICATE_CONNECTED_COLOR_ID]) {
//      set_led(LED_INDICATE_CONNECTED, LED_INDICATE_CONNECTED_COLOR, 0);
//      next_delay = indicate ==
//                           BSP_INDICATE_CONNECTED
//                       ? CONNECTED_LED_OFF_INTERVAL
//                       : CONNECTED_LED_OFF_INTERVAL;
//    } else {
//      set_led(LED_INDICATE_CONNECTED, LED_INDICATE_CONNECTED_COLOR, LED_BRIGHTNESS);
//      next_delay = indicate ==
//                           BSP_INDICATE_CONNECTED
//                       ? CONNECTED_LED_ON_INTERVAL
//                       : CONNECTED_LED_ON_INTERVAL;
//    }

    m_stable_state = indicate;
    err_code = app_timer_start(m_app_leds_tmr, APP_TIMER_TICKS(next_delay), NULL);
    break;

  case BSP_INDICATE_SENT_OK:  
    on(LED_GREEN);

    m_stable_state = BSP_INDICATE_IDLE;
    err_code = app_timer_start(m_app_leds_tmr, APP_TIMER_TICKS(100), NULL);
    break;
//    if (led_status[LED_INDICATE_SENT_OK][LED_INDICATE_SENT_OK_COLOR_ID]) {
//      set_led(LED_INDICATE_SENT_OK, LED_INDICATE_SENT_OK_COLOR, 0);
//      m_stable_state = BSP_INDICATE_IDLE;
//    } else {
//      set_led(LED_INDICATE_SENT_OK, LED_INDICATE_SENT_OK_COLOR, LED_BRIGHTNESS);
//      m_stable_state = indicate;
//      err_code = app_timer_start(m_app_leds_tmr, APP_TIMER_TICKS(SENT_OK_INTERVAL), NULL);
//    }
    break;

  case BSP_INDICATE_SEND_ERROR:
    on(LED_RED);

    m_stable_state = BSP_INDICATE_IDLE;
    err_code = app_timer_start(m_app_leds_tmr, APP_TIMER_TICKS(100), NULL);
    break;
//    if (led_status[LED_INDICATE_SENT_OK][LED_INDICATE_SENT_ERROR_COLOR_ID]) {
//      set_led(LED_INDICATE_SENT_OK, LED_INDICATE_SENT_ERROR_COLOR, 0);
//      m_stable_state = BSP_INDICATE_IDLE;
//    } else {
//      set_led(LED_INDICATE_SENT_OK, LED_INDICATE_SENT_ERROR_COLOR, LED_BRIGHTNESS);
//      m_stable_state = indicate;
//      err_code = app_timer_start(m_app_leds_tmr, APP_TIMER_TICKS(SEND_ERROR_INTERVAL), NULL);
//    }

    break;
    //
    //        case BSP_INDICATE_RCV_OK:
    //            // when receving shortly invert LED_1
    //            m_leds_clear = true;
    //            bsp_board_led_invert(BSP_LED_INDICATE_RCV_OK);
    //            err_code = app_timer_start(m_app_leds_tmr, APP_TIMER_TICKS(RCV_OK_INTERVAL), NULL);
    //            break;
    //
    //        case BSP_INDICATE_RCV_ERROR:
    //            // on receving error invert LED_1 for long time
    //            m_leds_clear = true;
    //            bsp_board_led_invert(BSP_LED_INDICATE_RCV_ERROR);
    //            err_code = app_timer_start(m_app_leds_tmr, APP_TIMER_TICKS(RCV_ERROR_INTERVAL), NULL);
    //            break;
    //
    //        case BSP_INDICATE_FATAL_ERROR:
    //            // on fatal error turn on all leds
    //            bsp_board_leds_on();
    //            m_stable_state = indicate;
    //            break;
    //
    //        case BSP_INDICATE_ALERT_0:
    //        case BSP_INDICATE_ALERT_1:
    //        case BSP_INDICATE_ALERT_2:
    //        case BSP_INDICATE_ALERT_3:
    //        case BSP_INDICATE_ALERT_OFF:
    //            err_code   = app_timer_stop(m_app_alert_tmr);
    //            next_delay = (uint32_t)BSP_INDICATE_ALERT_OFF - (uint32_t)indicate;
    //
    //            // a little trick to find out that if it did not fall through ALERT_OFF
    //            if (next_delay && (err_code == NRF_SUCCESS))
    //            {
    //                if (next_delay > 1)
    //                {
    //                    err_code = app_timer_start(m_app_alert_tmr,
    //                                               APP_TIMER_TICKS(((uint16_t)next_delay * ALERT_INTERVAL)),
    //                                               NULL);
    //                }
    //                bsp_board_led_on(BSP_LED_ALERT);
    //                m_alert_on = true;
    //            }
    //            else
    //            {
    //                bsp_board_led_off(BSP_LED_ALERT);
    //                m_alert_on = false;
    //
    //            }
    //            break;
    //
    //        case BSP_INDICATE_USER_STATE_OFF:
    //            leds_off();
    //            m_stable_state = indicate;
    //            break;
    //
  case BSP_INDICATE_USER_STATE_0: // BOOT
    on(LED_RED);
    on(LED_GREEN);
    on(LED_BLUE);

    m_stable_state = BSP_INDICATE_IDLE;
    err_code = app_timer_start(m_app_leds_tmr, APP_TIMER_TICKS(100), NULL);
    break;
  case BSP_INDICATE_USER_STATE_1: // TX
//    set_led(LED_INDICATE_TX, LED_INDICATE_TX_COLOR, LED_BRIGHTNESS);
    
    on(LED_BLUE);
    m_stable_state = BSP_INDICATE_IDLE;
    err_code = app_timer_start(m_app_leds_tmr, APP_TIMER_TICKS(100), NULL);
    break;
  case BSP_INDICATE_USER_STATE_2: // measure
//    set_led(LED_INDICATE_PEAK, LED_INDICATE_PEAK_COLOR, LED_BRIGHTNESS);
    on(LED_GREEN);
    m_stable_state = BSP_INDICATE_IDLE;
    err_code = app_timer_start(m_app_leds_tmr, APP_TIMER_TICKS(50), NULL);
    break;
    //
    //        case BSP_INDICATE_USER_STATE_3:
    //
    //        case BSP_INDICATE_USER_STATE_ON:
    //            bsp_board_leds_on();
    //            m_stable_state = indicate;f
    //            break;

  default:
    NRF_LOG_INFO("app_indication_set state %d", indicate);
    break;
  }

  return err_code;
}

static const double AFILTER_Acoef[] = {1.0, -2.12979364760736134, 0.42996125885751674, 1.62132698199721426, -0.96669962900852902, 0.00121015844426781, 0.04400300696788968};
static const double AFILTER_Bcoef[] = {0.169994948147430, 0.280415310498794, -1.120574766348363, 0.131562559965936, 0.974153561246036, -0.282740857326553, -0.152810756202003};
static double AFILTER_conditions[] = {0, 0, 0, 0, 0, 0};


int32_t a_filter(double input) {
  double output = input * AFILTER_Bcoef[0] + AFILTER_conditions[0];
  for (int j = 0; j < 5; j++) {
    AFILTER_conditions[j] = input * AFILTER_Bcoef[j + 1] - output * AFILTER_Acoef[j + 1] + AFILTER_conditions[j + 1];
  }
  AFILTER_conditions[5] = input * AFILTER_Bcoef[6] - output * AFILTER_Acoef[6];

  return (int32_t) output;
}

void transmit_ble_data () {
  transmitting = true;
  NRF_LOG_DEBUG("transmit_ble_data %d", ble_data_size);
  uint8_t txt_data[200];
  uint32_t error = 0;
  int i = 0;
  for (int i = 0; i<ble_data_size; i++) {
    sprintf(txt_data, "%d,%d,%d,%d;", ble_data[i].type, ble_data[i].time, ble_data[i].cdBSPL, ble_data_size - i);
    error = send_data_to_ble(txt_data,strlen(txt_data));
    NRF_LOG_DEBUG("BLE TX: %d of %d '%s' error %d", i, ble_data_size, txt_data, error);


    if (error != NRF_SUCCESS) {
      if (i > 0) {
        for (int j = i; j < ble_data_size; j++) {
          memcpy((uint8_t*) &ble_data[j-i], (uint8_t*) &ble_data[j], sizeof(sound_data_element_t));
        }
        ble_data_size -= i;
      }
      
      app_indication_set(BSP_INDICATE_SEND_ERROR);
      return;
    }
  }

  if (error == NRF_SUCCESS)
    ble_data_size = 0;

  
  NRF_LOG_DEBUG("transmit_ble_data clear  %d", ble_data_size);
  app_indication_set(BSP_INDICATE_SENT_OK);
  transmitting = false;
}



void process_sound_data(int16_t data[I2S_BUFFER_SIZE]) {
  static double sumsquare = 0.0;
  static int total_samples = 0;

  static double acc_min = 0;
  static uint64_t acc_min_count = 0;
  static double temp_sum, temp_sum2,temp_sum_avg = 0.0;

  uint8_t txt_data[200];

  parse_sound = false;

  //uint32_t new_ts = app_timer_cnt_get() / APP_TIMER_CLOCK_FREQ;

  if (time_stamp - prev_stamp >= 60) {
    if (acc_min_count > 0) {
      double leq_min =  MIC_OFFSET_DB + MIC_REF_DB + 20 * log10(sqrt(acc_min / acc_min_count) / MIC_REF_AMPL);


      if (ble_data_size >= MAX_BLE_DATA) {
        for (int i = BLE_DATA_FREE; i < MAX_BLE_DATA; i++) {
          memcpy((uint8_t*) &ble_data[i-BLE_DATA_FREE], (uint8_t*) &ble_data[i], sizeof(sound_data_element_t));
        }
        ble_data_size = MAX_BLE_DATA - BLE_DATA_FREE;
      }

      if (ble_data_size < MAX_BLE_DATA) {
        ble_data[ble_data_size].type = 1;
        ble_data[ble_data_size].time = time_stamp;
        ble_data[ble_data_size].cdBSPL = (uint16_t)(leq_min * 100);
        NRF_LOG_INFO("Add Data 1 %d, lea: %d", ble_data[ble_data_size].time, ble_data[ble_data_size].cdBSPL);
        ble_data_size++;
        transmitting = false;
      }

      acc_min = 0;
      acc_min_count = 0;
      prev_stamp = time_stamp;
    }
  }

  prev_avg = 0.0;
  for (int i = 0; i < 10; i++) {
    prev_avg += data[0];
  }

  prev_avg /= 10;

  for (int i = 0; i < I2S_BUFFER_SIZE; i++)
  {
    prev_avg = 0.999 * prev_avg + 0.001 * data[i];

    double corrected = data[i]  - prev_avg;
    double filtered = a_filter(corrected);
    sumsquare += filtered * filtered;

//    sprintf(txt_data, "%d,%d,%d,%d,%d,%d\n", i, data[i], (int) prev_avg, (int) corrected, (int) filtered, (int) sumsquare);
//    NRF_LOG_RAW_INFO("%s", txt_data);
  }
 
  total_samples +=  I2S_BUFFER_SIZE;

  if (total_samples >= SHORT_SAMPLE) 
  {
    double soundLevel = sumsquare / total_samples;
    double leq =  MIC_OFFSET_DB + MIC_REF_DB + 20 * log10(sqrt(sumsquare / total_samples) / MIC_REF_AMPL);
    acc_min += sumsquare / total_samples;
    acc_min_count++;

    NRF_LOG_INFO("LEQ %d, AVG: %d",(int)leq, (int) prev_avg);
    //NRF_LOG_INFO("(%d, %d,  %d) -> sqrt(%d / %d) / %d",  (int) temp_sum, (int) temp_sum2, (int) temp_sum_avg, (int) sumsquare, total_samples, MIC_REF_AMPL);

    stop_sound_measurement();

    //app_indication_set(BSP_INDICATE_USER_STATE_2);

//    sprintf(txt_data, "0,%d,%d,0;", (int) (new_ts), (uint16_t)(leq * 100));
//    send_data_to_ble(txt_data,strlen(txt_data));

    if (ble_data_size < MAX_BLE_DATA_THRESHOLD) {
      ble_data[ble_data_size].type = 0;
      ble_data[ble_data_size].time = time_stamp;
      ble_data[ble_data_size].cdBSPL = (uint16_t)(leq * 100);
      NRF_LOG_INFO("Add Data 0 %d, lea: %d",ble_data[ble_data_size].time, ble_data[ble_data_size].cdBSPL);
      ble_data_size++;
      transmitting = false;
    }

    sumsquare = 0.0;
    total_samples = 0;
    temp_sum = 0.0;
    temp_sum2 = 0.0;
    temp_sum_avg = 0.0;

    soundbuffer_position = -SKIP_BUFFERS;
    return;
  }
}


/*********PCM***********/

static void pdm_event_handler(nrfx_pdm_evt_t const * const p_evt) {
  uint32_t    err_code = NRF_SUCCESS;
  //NRF_LOG_DEBUG("pdm_event_handler buffer_reauest %d  buffer %d vs %d, error %d", p_evt->buffer_requested, p_evt->buffer_released, soundbuffer[buffer_index], p_evt->error);


  if (!p_evt->buffer_requested)
  {
      return;
  }

  if (!sampling_running) 
  {
    return;
  }

  buffer_index = (buffer_index + 1) % 3;
  err_code = nrfx_pdm_buffer_set(soundbuffer[buffer_index], SOUND_BUFFER_SIZE);
  if (err_code != NRF_SUCCESS) {
    NRF_LOG_DEBUG("Buffer cannot be set or is already set.");
  }

  if (!p_evt->buffer_released)  return;

  if (soundbuffer_position < 0) { // skip first buffer since sensor needs to start
    soundbuffer_position++;
    //NRF_LOG_DEBUG("pdm_event_handler neglect buffers");
    return;
  } 

  process_sound_data(p_evt->buffer_released);
}

/**@brief Application main function.
 */

int main(void) {
  uint32_t err_code;
  ble_state = 255;
  sleep_status = 1; // 0 can go to sleep,  1 keep awake

  // Initialize.
  log_init();

  NRF_LOG_RAW_INFO("\r\nTatySound\r\n");
  NRF_LOG_FLUSH();

  timers_init();
  power_management_init();
  init_buttons();

  nrf_gpio_pin_dir_set(LED_GREEN,    NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(LED_BLUE,    NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(LED_RED,    NRF_GPIO_PIN_DIR_OUTPUT);

  app_indication_set(BSP_INDICATE_USER_STATE_0);
  
  NRF_LOG_INFO("\n Audio Init\n");	
  pdm_cfg.gain_l = NRF_PDM_GAIN_MINIMUM;
  pdm_cfg.gain_r = NRF_PDM_GAIN_MINIMUM;

  err_code = ble_stack_init(&app_indication_set); 
  APP_ERROR_CHECK(err_code);
  err_code = gap_params_init(); 
  APP_ERROR_CHECK(err_code);
  err_code = gatt_init(); 
  APP_ERROR_CHECK(err_code);
  err_code = services_init(&nus_data_handler); 
  APP_ERROR_CHECK(err_code);
  err_code = advertising_init(); 
  APP_ERROR_CHECK(err_code);
  err_code = conn_params_init(); 
  APP_ERROR_CHECK(err_code);

  NRF_LOG_INFO("\n Start:PDM --> PCM\n");
  err_code =  start_sound_measurement(); 
  APP_ERROR_CHECK(err_code);

  NRF_LOG_INFO("app start!\n");
  err_code =  slow_advertising_start(); 
  APP_ERROR_CHECK(err_code);

  app_timer_start(m_sound_tmr, APP_TIMER_TICKS(SOUND_TIMER_INTERVAL), NULL);


  // Enter main loop.
  for (;;) {
    while (NRF_LOG_PROCESS());

//    if (parse_sound) {
//      NRF_LOG_DEBUG("parse_sound");
//      process_sound_data();
//    }

    if (!transmitting && !sampling_running && ble_data_size > 0) {
      transmit_ble_data();
    }

    idle_state_handle();
  }
}

/**
 * @}
 */