
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



typedef struct {
  uint16_t time;
  uint16_t cdBSPL;
} sound_data_element_t;

typedef struct {
  uint8_t type;
  uint16_t length;;
  sound_data_element_t data[MAX_BLE_DATA];
} sound_data_t;

static sound_data_t ble_data = {
      .type = 1, 
      .length = 0,
      .data[0].time = 0};

static volatile uint8_t led_status[8][3] = {0};
static uint8_t led_driver_status[3] = {0};
static uint8_t light_brightness = 50;
static uint8_t front_light_status = 0;


static uint16_t time_stamp = 0;
static int16_t acc_list_pointer = 0;

static uint8_t front_light_brightness_changed = 0;
static uint8_t button_pushed = 0;

uint32_t app_indication_set(bsp_indication_t indicate);
static void app_leds_timer_handler(void *p_context);
static void app_alert_timer_handler(void *p_context);
static void sound_timer_handler(void *p_context);
void set_led(uint8_t led_nr, uint8_t color, uint8_t brightness);
void set_front_light(uint8_t status);
static void send_data_to_ble(uint8_t *data, uint16_t length);
void set_all_leds_color(uint8_t color, uint8_t brightness);

uint8_t sleep_status = 0; // 0 can go to sleep,  1 keep awake
extern uint8_t ble_state;

                                                                           
//static uint32_t m_buffer_rx[];
//static int32_t soundbuffer[SHORT_SAMPLE];
static int32_t soundbuffer[I2S_BUFFER_SIZE];
static volatile bool i2s_running = false;

APP_PWM_INSTANCE(PWM1,1);                                                                                  // Create the instance "PWM1" using TIMER0.
APP_PWM_INSTANCE(PWM2,2);     


static volatile int soundbuffer_position = -1;
static double prev_avg = 0;
static volatile bool parse_sound = false;

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

static volatile uint8_t     pwm_ready = 0;                                                                       // A PWM ready status



void start_sound_measurement() {
  pwm_ready = 0;

  NRF_LOG_DEBUG("Starting pwm");
  
  app_pwm_enable(&PWM2);
  app_pwm_enable(&PWM1);

  app_pwm_channel_duty_set(&PWM2, 0, 50);
  app_pwm_channel_duty_set(&PWM1, 0, 50);  // Set at 50% duty cycle for square wave


  //while(pwm_ready < 2) {}

  NRF_LOG_DEBUG("Starting I2S");
   nrf_drv_i2s_buffers_t const initial_buffers = {
      .p_tx_buffer = NULL,
      .p_rx_buffer = soundbuffer,
  };
  // Initialize I2S data callback buffer

  uint32_t err_code = nrf_drv_i2s_start(&initial_buffers, I2S_BUFFER_SIZE, 0);

  if (err_code != 0) {
    NRF_LOG_INFO("cannot nrf_drv_i2s_startr\n");
    return;
  }    

  i2s_running = true;

}

void stop_sound_measurement() {
    app_pwm_disable(&PWM1);
    app_pwm_disable(&PWM2);

    nrf_drv_i2s_stop();

    i2s_running = false;
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

  UNUSED_PARAMETER(p_context);

  NRF_LOG_INFO("sound_timer_handler i2s: %d", i2s_running);

  if (!i2s_running) {
    start_sound_measurement();
  } else {
    app_indication_set(BSP_INDICATE_SEND_ERROR);
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
  case BSP_INDICATE_USER_STATE_2: // measure
    set_led(LED_INDICATE_PEAK, LED_INDICATE_PEAK_COLOR, LED_BRIGHTNESS);
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

void process_sound_data() {
  static double sumsquare = 0.0;
  static int total_samples = 0;

  static double acc_min = 0;
  static uint64_t acc_min_count = 0;
  static double temp_sum, temp_sum2,temp_sum_avg = 0.0;

  uint8_t data[50];

  parse_sound = false;

  uint32_t new_ts = app_timer_cnt_get() / APP_TIMER_CLOCK_FREQ;

  if (new_ts - time_stamp >= 60) {
    if (acc_min_count > 0) {
      double leq_min =  MIC_OFFSET_DB + MIC_REF_DB + 20 * log10(sqrt(acc_min / acc_min_count) / MIC_REF_AMPL);

      sprintf(data, "1,%d,%d,0;", (int) (time_stamp), (uint16_t)(leq_min * 100));
      send_data_to_ble(data,strlen(data));
      NRF_LOG_DEBUG(data);

      acc_min = 0;
      acc_min_count = 0;
      time_stamp = new_ts;
    }
  }

  
  NRF_LOG_DEBUG("process_sound_data from %d to %d", 0, soundbuffer_position);

  prev_avg = 0.0;
  int64_t sum = 0.0;
  for (int i = 0; i < I2S_BUFFER_SIZE ; i++) {
    soundbuffer[i] = soundbuffer[i] >> 6;

    prev_avg = 0.999 * prev_avg + 0.001 * soundbuffer[i];
    sum += soundbuffer[i];
  }

  int32_t avg = (int)((sum + 0.5) /I2S_BUFFER_SIZE);
  NRF_LOG_INFO("%d/%d %d, AVG: %d",(int) sum, I2S_BUFFER_SIZE,  avg, (int) prev_avg);


  for (int i = 0; i < I2S_BUFFER_SIZE ; i++) {
  
    temp_sum += soundbuffer[i];
    soundbuffer[i] -= avg; //(int) (prev_avg + 0.5);
    temp_sum2 += soundbuffer[i];
    double filtered = a_filter(soundbuffer[i]);
    sumsquare += filtered * filtered;
    temp_sum_avg += filtered;
  }


  total_samples +=  I2S_BUFFER_SIZE;

  if (total_samples >= SHORT_SAMPLE) {
    double soundLevel = sumsquare / total_samples;
    double leq =  MIC_OFFSET_DB + MIC_REF_DB + 20 * log10(sqrt(sumsquare / total_samples) / MIC_REF_AMPL);
    acc_min += sumsquare / total_samples;
    acc_min_count++;

    NRF_LOG_INFO("LEQ %d, AVG: %d",(int)leq, (int) prev_avg);
    NRF_LOG_INFO("(%d, %d,  %d) -> sqrt(%d / %d) / %d",  (int) temp_sum, (int) temp_sum2, (int) temp_sum_avg, (int) sumsquare, total_samples, MIC_REF_AMPL);

    stop_sound_measurement();

    app_indication_set(BSP_INDICATE_USER_STATE_2);

    ble_data.length = 1;
    ble_data.data[0].time++;
    ble_data.data[0].cdBSPL = (uint16_t)(leq * 100);

    sprintf(data, "0,%d,%d,0;", (int) (new_ts), (uint16_t)(leq * 100));
    send_data_to_ble(data,strlen(data));

    sumsquare = 0.0;
    total_samples = 0;
    temp_sum = 0.0;
    temp_sum2 = 0.0;
    temp_sum_avg = 0.0;

    soundbuffer_position = -1;
    return;
  }



  soundbuffer_position = -1;

  start_sound_measurement();
  
}

static void i2s_data_handler(nrf_drv_i2s_buffers_t const * buffers, uint32_t  status)
{
    //NRF_LOG_DEBUG("i2s_data_handler status %d\n", status);

    ASSERT(buffers);

    if (!(status & NRFX_I2S_STATUS_NEXT_BUFFERS_NEEDED))
    {
        return;
    }

    if (!buffers->p_rx_buffer) {
          nrf_drv_i2s_buffers_t const next_buffers = {
            .p_rx_buffer = soundbuffer,
            .p_tx_buffer = NULL,
        };
        APP_ERROR_CHECK(nrf_drv_i2s_next_buffers_set(&next_buffers));
    } else {
        if (soundbuffer_position == -1) { // skip first buffer since sensor needs to start
          APP_ERROR_CHECK(nrf_drv_i2s_next_buffers_set(buffers));
          soundbuffer_position++;
          return;
        } 

//        if ((soundbuffer_position >= 0) && (soundbuffer_position <= 5)) { 
//          NRF_LOG_DEBUG("i2s_data_handler memcpy %d", soundbuffer_position*I2S_BUFFER_SIZE);
//          //memcpy(&soundbuffer[soundbuffer_position*I2S_BUFFER_SIZE], buffers->p_rx_buffer, 4*I2S_BUFFER_SIZE);
//        }

        stop_sound_measurement();
        process_sound_data();
        return;

//        soundbuffer_position++;
//
//        if (soundbuffer_position == 1) { 
//          stop_sound_measurement();
//         
//          parse_sound = true;
//          return;
//        } 
//
//        APP_ERROR_CHECK(nrf_drv_i2s_next_buffers_set(buffers));

    }

   // NRF_LOG_FLUSH();

}



//static void i2s_data_handler(uint32_t const * p_data_received,
//                         uint32_t       * p_data_to_send,
//                         uint16_t         number_of_words)
//{
//    // Non-NULL value in 'p_data_received' indicates that a new portion of
//    // data has been received and should be processed.
//    if (p_data_received != NULL)
//    {
//      //  check_rx_data(p_data_received, number_of_words);
//    }
//
//    NRF_LOG_INFO("i2s_data_handler %d\n", number_of_words);
//
//    // Non-NULL value in 'p_data_to_send' indicates that the driver needs
//    // a new portion of data to send. Nothing done here; RX only...
//    if (p_data_to_send != NULL)
//    {
//    }
//}




void pwm_ready_callback(uint32_t pwm_id) 
{
	pwm_ready++;
}

/**@brief Application main function.
 */
 

 int main(void) {
  uint32_t err_code;
  ble_state = 255;
  sleep_status = 0; // 0 can go to sleep,  1 keep awake

  // Initialize.
  log_init();

  NRF_LOG_RAW_INFO("\r\nTatySound\r\n");
  NRF_LOG_FLUSH();

  timers_init();
  power_management_init();
  init_buttons();

  NRF_LOG_INFO("init i2c\n");
  i2c_init();

  NRF_LOG_INFO("boot_LP55231\n");
  boot_LP55231();

  err_code = lsm303agr_init();
  if (err_code != 0) {
    NRF_LOG_INFO("cannot init accelerometer\n");
  }  
  NRF_LOG_FLUSH();

  nrf_drv_i2s_config_t config = NRF_DRV_I2S_DEFAULT_CONFIG;
  err_code = nrf_drv_i2s_init(&config, i2s_data_handler);
  APP_ERROR_CHECK(err_code);

  app_pwm_config_t pwm1_cfg = APP_PWM_DEFAULT_CONFIG_1CH(CLOCK_PERIOD, 19);                           // SCK; pick a convenient gpio pin
  //pwm1_cfg.pin_polarity[0] = APP_PWM_POLARITY_ACTIVE_HIGH; 
  app_pwm_config_t pwm2_cfg = APP_PWM_DEFAULT_CONFIG_1CH(CLOCK_PERIOD*64, 26);                          // LRCK; pick a convenient gpio pin. LRCK period = 64X SCK period
  pwm2_cfg.pin_polarity[0] = APP_PWM_POLARITY_ACTIVE_HIGH;

  // Initialize and enable PWM's
  err_code = app_pwm_ticks_init(&PWM1,&pwm1_cfg,pwm_ready_callback);
  APP_ERROR_CHECK(err_code);
 err_code = app_pwm_ticks_init(&PWM2,&pwm2_cfg,pwm_ready_callback);
 APP_ERROR_CHECK(err_code);


  start_sound_measurement();
                                                         

  time_stamp = app_timer_cnt_get() / APP_TIMER_CLOCK_FREQ;


  app_indication_set(BSP_INDICATE_USER_STATE_0);

  ble_stack_init(&app_indication_set);
  gap_params_init();
  gatt_init();
  services_init(&nus_data_handler);
  advertising_init();
  conn_params_init();



  //init_battery_monitor();

  NRF_LOG_INFO("app start!\n");
  //advertising_start();

 // app_timer_start(m_acc_tmr, APP_TIMER_TICKS(ACC_TIMER_INTERVAL), NULL);
  app_timer_start(m_sound_tmr, APP_TIMER_TICKS(SOUND_TIMER_INTERVAL), NULL);


  // Enter main loop.
  for (;;) {
    while (NRF_LOG_PROCESS());

//    if (parse_sound) {
//      NRF_LOG_DEBUG("parse_sound");
//      process_sound_data();
//    }

    idle_state_handle();
  }
}

/**
 * @}
 */