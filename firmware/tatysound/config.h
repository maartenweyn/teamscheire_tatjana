#include <ble_types.h>

#include <app_timer.h>

#define ODR 200

#define ACC_TIMER_INTERVAL              1000/ODR                       /**< interval (msecs). */

#define ACC_LIST_SIZE 40 // restricted by max size of ble

#define I2S_BUFFER_SIZE     2000
#define SHORT_SAMPLE        16000
#define SKIP_BUFFERS        1
#define SAMPLE_RATE   16000//15650
#define CLOCK_PERIOD        16000000/(64*SAMPLE_RATE) 
#define MAX_BLE_DATA  1440
#define BLE_DATA_FREE  60 //if buffer full free 1 hour
#define MAX_BLE_DATA_THRESHOLD  0 //50

#define SOUND_TIMER_INTERVAL_SEC  10
#define SOUND_TIMER_INTERVAL  SOUND_TIMER_INTERVAL_SEC*1000

//PCM 
#define CONFIG_IO_PDM_CLK		23
#define CONFIG_IO_PDM_DATA		16
//#define CONFIG_IO_PDM_DATA		26

#define CONFIG_AUDIO_PDM_GAIN 		0x38
#define CONFIG_AUDIO_FRAME_SIZE_BYTES   64
#define CONFIG_AUDIO_FRAME_SIZE_SAMPLES 64
#define CONFIG_PDM_FRAME_SIZE_SAMPLES	64
#define CONFIG_AUDIO_FRAME_BUFFERS 	16

#define SOUND_BUFFER_SIZE     2000

// ADUIO PROCESSING
#define MIC_OFFSET_DB     3.0103    // Default offset (sine-wave RMS vs. dBFS)
#define MIC_REF_DB        94.0      // dB(SPL)
  #define MIC_REF_AMPL      1642 //Amplitude at 94dB(SPL) (-26dBFS from datasheet, i.e. (2^15-1)*10^(-26/20) )
//#define MIC_REF_AMPL      6569 //Amplitude at 94dB(SPL) (-26dBFS from datasheet, i.e. (2^17-1)*10^(-26/20) )
//#define MIC_REF_AMPL  0.050118723362727

// BLE

#define APP_BLE_CONN_CFG_TAG            1                                           /**< A tag identifying the SoftDevice BLE configuration. */
#define APP_BLE_OBSERVER_PRIO           3                                           /**< Application's BLE observer priority. You shouldn't need to modify this value. */

#define DEVICE_NAME                     "TatySound"                                    /**< Name of device. Will be included in the advertising data. */
#define NUS_SERVICE_UUID_TYPE           BLE_UUID_TYPE_VENDOR_BEGIN                  /**< UUID type for the Nordic UART Service (vendor specific). */

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(20, UNIT_1_25_MS)             /**< Minimum acceptable connection interval (20 ms), Connection interval uses 1.25 ms units. */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(75, UNIT_1_25_MS)             /**< Maximum acceptable connection interval (75 ms), Connection interval uses 1.25 ms units. */
#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds), Supervision Timeout uses 10 ms units. */
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)                       /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)                      /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

#define APP_BLE_OBSERVER_PRIO           3                                           /**< Application's BLE observer priority. You shouldn't need to modify this value. */

#define APP_ADV_INTERVAL                64                                          /**< The advertising interval (in units of 0.625 ms. This value corresponds to 40 ms). */
#define APP_ADV_DURATION                10//18000                                       /**< The advertising duration (180 seconds) in units of 10 milliseconds. */

#define SLOW_APP_ADV_INTERVAL           1600   //1000 ms * 1.6
#define SLOW_APP_ADV_TIMEOUT_IN_SECONDS 0


// Buttons
#define BUTTON1 3

// LEDS

#define LED1  5
#define LED2  8
#define LED3  6

#define LED_RED LED1
#define LED_GREEN LED2
#define LED_BLUE LED3

#define LED_INDICATE_SENT_OK               0
#define LED_INDICATE_SEND_ERROR            0
#define LED_INDICATE_RCV_OK                0
#define LED_INDICATE_RCV_ERROR             0
#define LED_INDICATE_CONNECTED             LED_BLUE
#define LED_INDICATE_BONDING               0
#define LED_INDICATE_ADVERTISING_DIRECTED  0
#define LED_INDICATE_ADVERTISING_SLOW      0
#define LED_INDICATE_ADVERTISING_WHITELIST 0
#define LED_INDICATE_ADVERTISING  0

#define LED_INDICATE_TX  0


#define RED 1
#define GREEN 2
#define BLUE  4
#define WHITE 7

#define RED_ID   0
#define GREEN_ID 1
#define BLUE_ID  2


#define LED_BRIGHTNESS  10


#define ADVERTISING_LED_ON_INTERVAL             100
#define ADVERTISING_LED_OFF_INTERVAL            4900

#define CONNECTED_LED_ON_INTERVAL               100
#define CONNECTED_LED_OFF_INTERVAL              3000

#define ADVERTISING_DIRECTED_LED_ON_INTERVAL   200
#define ADVERTISING_DIRECTED_LED_OFF_INTERVAL  200

#define ADVERTISING_WHITELIST_LED_ON_INTERVAL  200
#define ADVERTISING_WHITELIST_LED_OFF_INTERVAL 800

#define ADVERTISING_SLOW_LED_ON_INTERVAL       400
#define ADVERTISING_SLOW_LED_OFF_INTERVAL      4000

#define BONDING_INTERVAL                       100

#define SENT_OK_INTERVAL                       100
#define SEND_ERROR_INTERVAL                    500

#define RCV_OK_INTERVAL                        100
#define RCV_ERROR_INTERVAL                     500

#define ALERT_INTERVAL                         200

#define LIGHT_BUTTON_CHANGE_INTERVAL            1000
#define BUTTON_TIMEOUT_INTERVAL                 10000
#define BUTTON_ACK_FLASH_INTERVAL               500

extern uint8_t sleep_status;