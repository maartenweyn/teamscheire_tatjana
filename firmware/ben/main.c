
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

#include "i2c.h"
#include "lp55231.h"
#include "nrf_delay.h"
#include "nrf_drv_twi.h"

#include "ble_app.h"
#include "config.h"
#include "lsm303_agr.h"
#include "battery.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

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
APP_TIMER_DEF(m_acc_tmr);

static volatile uint8_t led_status[8][3] = {0};
static uint8_t led_driver_status[3] = {0};
static uint8_t light_brightness = 50;
static uint8_t front_light_status = 0;

static uint8_t ble_acc_list[4 + ACC_LIST_SIZE * 6] = {0};
static Type3Axis16bit_U *acc_list = (Type3Axis16bit_U *)&ble_acc_list[4];
static uint16_t *timestamp = (uint16_t *)&ble_acc_list[2];
static int16_t acc_list_pointer = 0;

static uint8_t front_light_brightness_changed = 0;
static uint8_t button_pushed = 0;
static double totalBenZ = 0;

uint32_t app_indication_set(bsp_indication_t indicate);
static void app_leds_timer_handler(void *p_context);
static void app_alert_timer_handler(void *p_context);
static void acc_timer_handler(void *p_context);
void set_led(uint8_t led_nr, uint8_t color, uint8_t brightness);
void set_front_light(uint8_t status);
static void send_data_to_ble(uint8_t *data, uint16_t length);
void set_all_leds_color(uint8_t color, uint8_t brightness);

uint8_t sleep_status = 0; // 0 can go to sleep,  1 keep awake
extern uint8_t ble_state;

static const uint8_t led_addresses[8][3][2] = {
    {{LED1RED_ID, LED1RED_ADDR}, {LED1GREEN_ID, LED1GREEN_ADDR}, {LED1BLUE_ID, LED1BLUE_ADDR}},
    {{LED2RED_ID, LED2RED_ADDR}, {LED2GREEN_ID, LED2GREEN_ADDR}, {LED2BLUE_ID, LED2BLUE_ADDR}},
    {{LED3RED_ID, LED3RED_ADDR}, {LED3GREEN_ID, LED3GREEN_ADDR}, {LED3BLUE_ID, LED3BLUE_ADDR}},
    {{LED4RED_ID, LED4RED_ADDR}, {LED4GREEN_ID, LED4GREEN_ADDR}, {LED4BLUE_ID, LED4BLUE_ADDR}},
    {{LED5RED_ID, LED5RED_ADDR}, {LED5GREEN_ID, LED5GREEN_ADDR}, {LED5BLUE_ID, LED5BLUE_ADDR}},
    {{LED6RED_ID, LED6RED_ADDR}, {LED6GREEN_ID, LED6GREEN_ADDR}, {LED6BLUE_ID, LED6BLUE_ADDR}},
    {{LED7RED_ID, LED7RED_ADDR}, {LED7GREEN_ID, LED7GREEN_ADDR}, {LED7BLUE_ID, LED7BLUE_ADDR}},
    {{LED8RED_ID, LED8RED_ADDR}, {LED8GREEN_ID, LED8GREEN_ADDR}, {LED8BLUE_ID, LED8BLUE_ADDR}}};

static const uint8_t led_driver_addresses[3] = {ADDRESS0, ADDRESS1, ADDRESS2};

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

    NRF_LOG_DEBUG("Received data from BLE NUS.");
    NRF_LOG_HEXDUMP_DEBUG(p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);
  }
}

/* BUTTONS */

void in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
  //nrf_drv_gpiote_out_toggle(PIN_OUT);
  int32_t pin_state = nrf_gpio_pin_read(pin);
  NRF_LOG_INFO("in_pin_handler %d - %d / %d", pin, action, pin_state);

   if (pin_state == 0) {
      set_all_leds_color(GREEN, 50);
    } else {
      set_all_leds_color(GREEN, 0);
    }
}

ret_code_t init_buttons() {
  ret_code_t err_code;

  err_code = nrfx_gpiote_init();
  APP_ERROR_CHECK(err_code);

  nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
  in_config.pull = NRF_GPIO_PIN_PULLUP;

  //define CONFIG_NFCT_PINS_AS_GPIOS in the preporcessor symbols of the app
  err_code = nrf_drv_gpiote_in_init(BUTTON1, &in_config, in_pin_handler);
  APP_ERROR_CHECK(err_code);

  nrf_drv_gpiote_in_event_enable(BUTTON1, true);

  return err_code;
}

/* LEDS */

void check_led_driver(uint8_t id, uint8_t status) {
  if (led_driver_status[id] == status)
    return;

  if (status) {
    NRF_LOG_INFO("enabling driver %d", id);
    lp55231_enable(led_driver_addresses[id]);
    led_driver_status[id] = 1;
  } else {
    uint8_t l, c;
    for (l = 0; l < 8; l++) {
      for (c = 0; c < 3; c++) {
        if (led_status[l][c] > 0) {
          if (led_addresses[l][c][1] == id) {
            //led driver still needs to be on so do not disable
            return;
          }
        }
      }
    }

    lp55231_disable(led_driver_addresses[id]);
    led_driver_status[id] = 0;

    NRF_LOG_INFO("dissabling driver %d", id);
  }
}

void boot_LP55231(void) {

  lp55231_enable(ADDRESS0);
  lp55231_enable(ADDRESS1);
  lp55231_enable(ADDRESS2);

  leds_set_all(0);

  lp55231_disable(ADDRESS0);
  lp55231_disable(ADDRESS1);
  lp55231_disable(ADDRESS2);
}

void set_led(uint8_t led_nr, uint8_t color, uint8_t brightness) {
  NRF_LOG_INFO("set_led %d %d to %d", led_nr, color, brightness);

  if (color & RED) {
    led_status[led_nr][0] = brightness;
    check_led_driver(led_addresses[led_nr][0][1], brightness > 0);
    lp55231_setBrightness(led_addresses[led_nr][0][0], brightness, led_driver_addresses[led_addresses[led_nr][0][1]]);
  }
  if (color & GREEN) {
    led_status[led_nr][1] = brightness;
    check_led_driver(led_addresses[led_nr][1][1], brightness > 0);
    lp55231_setBrightness(led_addresses[led_nr][1][0], brightness, led_driver_addresses[led_addresses[led_nr][1][1]]);
  }
  if (color & BLUE) {
    led_status[led_nr][2] = brightness;
    check_led_driver(led_addresses[led_nr][2][1], brightness > 0);
    lp55231_setBrightness(led_addresses[led_nr][2][0], brightness, led_driver_addresses[led_addresses[led_nr][2][1]]);
  }
}

void set_front_light(uint8_t status) {
  uint8_t brightness = status == 0 ? 0 : light_brightness;

  set_led(1, WHITE, brightness);
  set_led(2, WHITE, brightness);
  set_led(3, WHITE, brightness);
  set_led(4, WHITE, brightness);
  set_led(5, WHITE, brightness);
  set_led(6, WHITE, brightness);
  set_led(7, WHITE, brightness);
}

void set_all_leds_color(uint8_t color, uint8_t brightness) {
  leds_set_all(0);
  set_led(0, color, brightness);
  set_led(1, color, brightness);
  set_led(2, color, brightness);
  set_led(3, color, brightness);
  set_led(4, color, brightness);
  set_led(5, color, brightness);
  set_led(6, color, brightness);
  set_led(7, color, brightness);
}

static void send_data_to_ble(uint8_t *data, uint16_t length) {
  NRF_LOG_INFO("send_data_to_ble");

  uint32_t err_code;

  do {
    err_code = send_data(data, length);
    if ((err_code != NRF_ERROR_INVALID_STATE) && (err_code != NRF_ERROR_BUSY) &&
        (err_code != NRF_ERROR_NOT_FOUND)) {
      if (err_code != NRF_SUCCESS) {
        NRF_LOG_INFO("send_data error %d\n", err_code);
      }
    }
  } while (err_code == NRF_ERROR_BUSY);
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

  err_code = app_timer_create(&m_acc_tmr, APP_TIMER_MODE_REPEATED, acc_timer_handler);
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

static void process_acc(Type3Axis16bit_U acc) {
  static uint16_t no_motion_counter = 0;
  static uint16_t no_motion_seconds = 0;
  static int32_t sum[3];
  static Type3Axis16bit_U mean;
  static Type3Axis16bit_U corrected;
  static bool calibrated = false;
  static double r[3][3];
  static double filtered = 0;
  static int16_t acc_buffer[PEAK_DETECTION_LAG] = {0};
  static bool acc_buffer_filled = false;
  static uint16_t acc_buffer_pointer = 0;
  static int16_t acc_buffer_mean = 0;
  static int16_t acc_buffer_std = 0;

  // Detection a moment of no motion
  uint32_t msqr = (acc.i16bit[0] * acc.i16bit[0]) + (acc.i16bit[1] * acc.i16bit[1]) + (acc.i16bit[2] * acc.i16bit[2]);

  if (abs(msqr) < NO_MOTION_THRESHOLD) {
    no_motion_counter++;
    sum[0] += acc.i16bit[0];
    sum[1] += acc.i16bit[1];
    sum[2] += acc.i16bit[2];

    if ((no_motion_counter % ODR) == 0) {
      no_motion_seconds++;

      if (no_motion_seconds > 60) {
        sleep_mode_enter();
      }
    }

  } else {
    sum[0] = 0;
    sum[1] = 0;
    sum[2] = 0;
    no_motion_counter = 0;
    no_motion_seconds = 0;
  }

  if (no_motion_counter > NO_MOTION_COUNTER_THRESHOLD) {
    mean.i16bit[0] = sum[0] >> NO_MOTION_SUM_TO_MEAN_SHIFT;
    mean.i16bit[1] = sum[1] >> NO_MOTION_SUM_TO_MEAN_SHIFT;
    mean.i16bit[2] = sum[2] >> NO_MOTION_SUM_TO_MEAN_SHIFT;

    NRF_LOG_INFO("no motion (%d) - mean: %d,%d,%d", no_motion_counter, mean.i16bit[0], mean.i16bit[1], mean.i16bit[2]);

    // Calculate rotation matrix R - todo avoid double
    double tempZ = sqrt((mean.i16bit[0] * mean.i16bit[0]) + (mean.i16bit[1] * mean.i16bit[1]) + (mean.i16bit[2] * mean.i16bit[2]));
    NRF_LOG_INFO("tempZ %d", (tempZ * 1000));

    r[2][0] = -mean.i16bit[0] / tempZ;
    r[2][1] = -mean.i16bit[1] / tempZ;
    r[2][2] = -mean.i16bit[2] / tempZ;

    /*
    We only need to rotate the Z axis
    double tempY = sqrt((r[2][2] * r[2][2]) + (r[2][1] * r[2][1]));
    NRF_LOG_INFO("tempY %d", ((int16_t)(tempY * 1000)));
    r[1][0] = 0;
    r[1][1] = r[2][2] / tempY;
    r[1][2] = -r[2][1] / tempY;

    r[0][0] = (r[1][1] * r[2][2]) - (r[1][2] * r[2][1]);
    r[0][1] = (r[1][2] * r[2][0]) - (r[1][0] * r[2][2]);
    r[0][2] = (r[1][0] * r[2][1]) - (r[1][1] * r[2][0]);



    NRF_LOG_INFO("Rx: %d,%d,%d", r[0][0] * 1000, r[0][1] * 1000, r[0][2] * 1000);
    NRF_LOG_INFO("Ry: %d,%d,%d", r[1][0] * 1000, r[1][1] * 1000, r[1][2] * 1000);
    */

    calibrated = true;

    send_data_to_ble("calibrated", 10);
    set_led(3, BLUE, LED_BRIGHTNESS);

    //set_all_leds_color(GREEN, 50);

    NRF_LOG_INFO("Rx: %d,%d,%d", r[2][0] * 1000, r[2][1] * 1000, r[2][2] * 1000);

    // transmit mean and R

    uint32_t ts = app_timer_cnt_get();
    uint32_t tstmsec = ts / APP_TIMER_TICKS(1);
    uint16_t timestamp = tstmsec & 0xFFFF;

    //uint8_t data_buffer[4 + 6 + 12];
    //data_buffer[0] = 1; // send mean and r
//    int32_t rint[3] = { r[2][0] * 1000, r[2][1] * 1000, r[2][2] * 1000};
//
//    memcpy(&data_buffer[2], (uint8_t *)&timestamp, 2);
//    memcpy(&data_buffer[4], mean.u8bit, 6);
//    memcpy(&data_buffer[4 + 6], (uint8_t *)rint, 12);
//    send_data_to_ble(data_buffer, 4 + 6 + 12);

    sum[0] = 0;
    sum[1] = 0;
    sum[2] = 0;
    no_motion_counter = 0;
  }

  NRF_LOG_INFO("%d,%d,%d -> %d (%d) - %d", acc.i16bit[0], acc.i16bit[1], acc.i16bit[2], msqr, no_motion_counter, calibrated);

  // Correcting based on rotation matrix
  if (calibrated) {
    // We only need the Z axis
    //corrected.i16bit[0]  = r[0][0] * acc.i16bit[0] +r[0][1] * acc.i16bit[1] + r[0][2] * acc.i16bit[2];
    //corrected.i16bit[1]  = r[1][0] * acc.i16bit[0] +r[1][1] * acc.i16bit[1] + r[1][2] * acc.i16bit[2];
    corrected.i16bit[2] = (r[2][0] * acc.i16bit[0] + r[2][1] * acc.i16bit[1] + r[2][2] * acc.i16bit[2]) + 1000;

    filtered = HF_FILTER_APHA * filtered + (1 - HF_FILTER_APHA) * corrected.i16bit[2];

    NRF_LOG_INFO("corrected %d,%d,%d (%d)", corrected.i16bit[0], corrected.i16bit[1], corrected.i16bit[2], filtered);

    int16_t prev_value = acc_buffer[acc_buffer_pointer];
    acc_buffer[acc_buffer_pointer++] = filtered;
    if (acc_buffer_pointer == PEAK_DETECTION_LAG) {
      acc_buffer_filled = true;
      acc_buffer_pointer = 0;
    }

    if (acc_buffer_filled) {
      // Calculate mean and std

      
      double filtered_sum = 0;
      double filtered_sq_sum = 0;
      // fast way but rounding errors


      //acc_buffer_mean = ((acc_buffer_mean * PEAK_DETECTION_LAG) - prev_value + filtered) / PEAK_DETECTION_LAG;

      int16_t i = 1;

      for (i = 0; i < PEAK_DETECTION_LAG; i++) {
        filtered_sum += acc_buffer[i];
        filtered_sq_sum += acc_buffer[i] * acc_buffer[i];
      }

      acc_buffer_mean = filtered_sum / PEAK_DETECTION_LAG;
      acc_buffer_std = sqrt((filtered_sq_sum / PEAK_DETECTION_LAG) - (acc_buffer_mean * acc_buffer_mean));

      // detect peaks
      static uint8_t prev_peak_id = 0;
      static double prev_peak = 0;

      if (acc_buffer_std > 1000)
      {

      }
      
      NRF_LOG_INFO("Peak ? %d - %d (%d) > %d * %d (%d)", filtered, acc_buffer_mean, filtered - acc_buffer_mean, PEAK_DETECTION_THRESHOLD, acc_buffer_std, PEAK_DETECTION_THRESHOLD * acc_buffer_std);
      
      if (((filtered - acc_buffer_mean) > PEAK_DETECTION_THRESHOLD * acc_buffer_std)  && ((filtered - acc_buffer_mean) > PEAK_MIN_DETECTION_THRESHOLD))
      {
        NRF_LOG_INFO("yes. prev_peak_id <= PEAK_DETECTION_DIFFERENCE -> %d <  %d", prev_peak_id, PEAK_DETECTION_DIFFERENCE);
        // if previous peak is very close to new one
        //if (prev_peak_id <= PEAK_DETECTION_DIFFERENCE) {
          if (prev_peak < filtered) {
            // neglect previous peek
            prev_peak_id = 0;
            prev_peak = filtered;
          } else {
            // else neglect current peak
          }
            
      }

      if (prev_peak > 0) prev_peak_id++;

      NRF_LOG_INFO("peak_id %d, peak %d", prev_peak_id, prev_peak);  

      if (prev_peak_id > PEAK_DETECTION_DIFFERENCE)
      {
      // peak is far enough of the previous one
          // add previous peak to total measurement
          NRF_LOG_INFO("peak far enough : peak_id %d, peak %d", prev_peak_id, prev_peak);
          totalBenZ += (prev_peak / 1000);

          NRF_LOG_INFO("totalBenZ %d", totalBenZ);

          uint32_t ts = app_timer_cnt_get();
          uint32_t tstmsec = ts / APP_TIMER_TICKS(1);
          uint16_t timestamp = tstmsec & 0xFFFF;

          send_data_to_ble("peak", 4);
          app_indication_set(BSP_INDICATE_USER_STATE_2);

//          uint8_t data_buffertotalBenZ[4 + 4];
//          data_buffertotalBenZ[0] = 2; // send totalBenZ
//          memcpy(&data_buffertotalBenZ[2], (uint8_t *)&timestamp, 2);
//          memcpy(&data_buffertotalBenZ[4], (uint8_t *)totalBenZ, 4);
//          send_data_to_ble(data_buffertotalBenZ, 4 + 4);

          prev_peak_id = 0;
          prev_peak = 0;
      }
    }
  }
}

/**@brief Handle events from alert timer.
 *
 * @param[in]   p_context   parameter registered in timer start function.
 */

static void acc_timer_handler(void *p_context) {
  UNUSED_PARAMETER(p_context);

  //NRF_LOG_INFO("acc_timer_handler");

  int32_t acc_data[3];
  lsm303agr_acc_get_acceleration(acc_data);

  uint8_t str[64];
  uint8_t str_lenght = sprintf(str, "%d,%d,%d\n", acc_data[0], acc_data[1], acc_data[2]);
  //NRF_LOG_INFO("%d,%d,%d", acc_data[0], acc_data[1], acc_data[2]);

  Type3Axis16bit_U acc = {acc_data[0], acc_data[1], acc_data[2]};
  process_acc(acc);

//  acc_list[acc_list_pointer].i16bit[0] = acc_data[0];
//  acc_list[acc_list_pointer].i16bit[1] = acc_data[1];
//  acc_list[acc_list_pointer].i16bit[2] = acc_data[2];
//
//  process_acc(acc_list[acc_list_pointer]);
//
//  if (acc_list_pointer == 0) {
//    uint32_t ts = app_timer_cnt_get();
//    uint32_t tstmsec = ts / APP_TIMER_TICKS(1);
//    *timestamp = tstmsec & 0xFFFF;
//    NRF_LOG_INFO("%d -> %d -> %d", ts, tstmsec, *timestamp);
//  }
//  acc_list_pointer++;

  //if (acc_list_pointer == ACC_LIST_SIZE) {
  //  NRF_LOG_INFO("Transmitting 4 + (3 x 2 x %d) values", acc_list_pointer);
//    send_data_to_ble((void *)ble_acc_list, (acc_list_pointer * 6) + 4);
//    acc_list_pointer = 0;
//
//    app_indication_set(BSP_INDICATE_USER_STATE_1);
//  }
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
    set_led(0, WHITE, 0);
    set_led(LED_INDICATE_PEAK, WHITE, 0);
    m_stable_state = indicate;
    break;

  case BSP_INDICATE_SCANNING:
  case BSP_INDICATE_ADVERTISING:
    // in advertising blink LED_0
    if (led_status[LED_INDICATE_ADVERTISING][LED_INDICATE_ADVERTISING_COLOR_ID]) {
      set_led(LED_INDICATE_ADVERTISING, LED_INDICATE_ADVERTISING_COLOR, 0);
      next_delay = indicate ==
                           BSP_INDICATE_ADVERTISING
                       ? ADVERTISING_LED_OFF_INTERVAL
                       : ADVERTISING_SLOW_LED_OFF_INTERVAL;
    } else {
      set_led(LED_INDICATE_ADVERTISING, LED_INDICATE_ADVERTISING_COLOR, LED_BRIGHTNESS);
      next_delay = indicate ==
                           BSP_INDICATE_ADVERTISING
                       ? ADVERTISING_LED_ON_INTERVAL
                       : ADVERTISING_SLOW_LED_ON_INTERVAL;
    }

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
    if (led_status[LED_INDICATE_CONNECTED][LED_INDICATE_ADVERTISING_COLOR_ID])
      set_led(LED_INDICATE_ADVERTISING, LED_INDICATE_ADVERTISING_COLOR, 0);

    if (led_status[LED_INDICATE_CONNECTED][LED_INDICATE_CONNECTED_COLOR_ID]) {
      set_led(LED_INDICATE_CONNECTED, LED_INDICATE_CONNECTED_COLOR, 0);
      next_delay = indicate ==
                           BSP_INDICATE_CONNECTED
                       ? CONNECTED_LED_OFF_INTERVAL
                       : CONNECTED_LED_OFF_INTERVAL;
    } else {
      set_led(LED_INDICATE_CONNECTED, LED_INDICATE_CONNECTED_COLOR, LED_BRIGHTNESS);
      next_delay = indicate ==
                           BSP_INDICATE_CONNECTED
                       ? CONNECTED_LED_ON_INTERVAL
                       : CONNECTED_LED_ON_INTERVAL;
    }

    m_stable_state = indicate;
    err_code = app_timer_start(m_app_leds_tmr, APP_TIMER_TICKS(next_delay), NULL);
    break;

  case BSP_INDICATE_SENT_OK:
    if (led_status[LED_INDICATE_SENT_OK][LED_INDICATE_SENT_OK_COLOR_ID]) {
      set_led(LED_INDICATE_SENT_OK, LED_INDICATE_SENT_OK_COLOR, 0);
      m_stable_state = BSP_INDICATE_IDLE;
    } else {
      set_led(LED_INDICATE_SENT_OK, LED_INDICATE_SENT_OK_COLOR, LED_BRIGHTNESS);
      m_stable_state = indicate;
      err_code = app_timer_start(m_app_leds_tmr, APP_TIMER_TICKS(SENT_OK_INTERVAL), NULL);
    }
    break;

  case BSP_INDICATE_SEND_ERROR:

    if (led_status[LED_INDICATE_SENT_OK][LED_INDICATE_SENT_ERROR_COLOR_ID]) {
      set_led(LED_INDICATE_SENT_OK, LED_INDICATE_SENT_ERROR_COLOR, 0);
      m_stable_state = BSP_INDICATE_IDLE;
    } else {
      set_led(LED_INDICATE_SENT_OK, LED_INDICATE_SENT_ERROR_COLOR, LED_BRIGHTNESS);
      m_stable_state = indicate;
      err_code = app_timer_start(m_app_leds_tmr, APP_TIMER_TICKS(SEND_ERROR_INTERVAL), NULL);
    }

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
#ifdef DEBUG_NRF
    set_led(LED_INDICATE_ADVERTISING, LED_INDICATE_BOOT_DEBUG, LED_BRIGHTNESS);
#else
    set_led(LED_INDICATE_ADVERTISING, LED_INDICATE_BOOT, LED_BRIGHTNESS);
#endif
    m_stable_state = BSP_INDICATE_IDLE;
    err_code = app_timer_start(m_app_leds_tmr, APP_TIMER_TICKS(100), NULL);
    break;
  case BSP_INDICATE_USER_STATE_1: // TX
    set_led(LED_INDICATE_TX, LED_INDICATE_TX_COLOR, LED_BRIGHTNESS);
    m_stable_state = BSP_INDICATE_IDLE;
    err_code = app_timer_start(m_app_leds_tmr, APP_TIMER_TICKS(100), NULL);
    break;
  case BSP_INDICATE_USER_STATE_2: // peak
    set_led(LED_INDICATE_PEAK, LED_INDICATE_PEAK_COLOR, LED_BRIGHTNESS);
    m_stable_state = BSP_INDICATE_IDLE;
    err_code = app_timer_start(m_app_leds_tmr, APP_TIMER_TICKS(100), NULL);
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

/**@brief Application main function.
 */


int main(void) {
  uint32_t err_code;
  ble_state = 255;
  sleep_status = 0; // 0 can go to sleep,  1 keep awake

  // Initialize.
  log_init();

  NRF_LOG_RAW_INFO("\r\nCME_Light_Button\r\n");
  NRF_LOG_FLUSH();

  timers_init();
  power_management_init();
  init_buttons();

  NRF_LOG_INFO("init i2c\n");
  i2c_init();

  NRF_LOG_INFO("boot_LP55231\n");
  boot_LP55231();

  err_code = lsm303agr_init();
  if (err_code == 0) {
    NRF_LOG_INFO("cannot init accelerometer\n");
  } else {  
    NRF_LOG_INFO("configuring accelerometer\n");
    lsm303agr_acc_set_low_power_mode(false, true);
    lsm303agr_acc_set_x_odr(ODR);
    lsm303agr_acc_set_x_fs(LSM303AGR_ACC_FS_8G);

    ble_acc_list[1] = ACC_TIMER_INTERVAL;

    app_indication_set(BSP_INDICATE_USER_STATE_0);

    ble_stack_init(&app_indication_set);
    gap_params_init();
    gatt_init();
    services_init(&nus_data_handler);
    advertising_init();
    conn_params_init();



    //init_battery_monitor();

    NRF_LOG_INFO("app start!\n");
    advertising_start();

    app_timer_start(m_acc_tmr, APP_TIMER_TICKS(ACC_TIMER_INTERVAL), NULL);
  }

  // Enter main loop.
  for (;;) {
    while (NRF_LOG_PROCESS())
      ;
    idle_state_handle();
  }
}

/**
 * @}
 */