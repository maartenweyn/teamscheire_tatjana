/**
 ******************************************************************************
 * @file    peripheral_mngr_app.c
 * @author  Central Labs
 * @version V 1.0.0
 * @date    May-2017
 * @brief   This file contains definitions for the application manager.
 *******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
 *
 * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *        http://www.st.com/software_license_agreement_liberty_v2
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ********************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "peripheral_mngr_app.h"
#include "steval_bluemic1.h"  

/** @addtogroup BLUEMIC_1_APP BLUEMIC_1_APP
 * @{
 */

/** @defgroup BLUEMIC_1_APP_MNGR BLUEMIC_1_APP_MNGR
 * @{
 */

/** @defgroup APP_MNGR_Private_Defines APP_MNGR_Private_Defines
 * @{
 */

#define NAME_WEAR 'B', 'V', 'B', 'N', 'R', 'G', '1'
   
#define LED_TOGGLE_ADVERTISEMENT                        10000
#define LED_TOGGLE_READY                                3000
#define LED_TOGGLE_INERTIAL                             800
#define LED_TOGGLE_BLUEVOICE                            800

#define APP_READY                                       (0x00)
#define APP_BLUEVOICE_ENABLE                            (0x01)
#define APP_INERTIAL_ENABLE                             (0x02)

/**
 * @}
 */
 
/** @defgroup APP_MNGR_Private_Variables APP_MNGR_Private_Variables
 * @{
 */
volatile uint16_t ServiceHandle;
volatile uint16_t conn_handle;
volatile uint8_t APP_PER_enabled = APP_READY;

uint8_t PERIPHERAL_BDADDR[] = { 0x55, 0x11, 0x07, 0x01, 0x16, 0xE2 }; 
static uint32_t led_toggle_count = 0; /*!< Variable used to handle led toggling.*/

/* BlueVoice service UUID*/ 
static const uint8_t bluemic1_service_uuid[16] =
{
  0x1b,0xc5,0xd5,0xa5,0x02,0x00,0xb4,0x9a,0xe1,0x11,0x01,0x00,0x00,0x00,0x00,0x00
};

/**
 * @}
 */

/** @defgroup APP_MNGR_Exported_Variables APP_MNGR_Exported_Variables
 * @{
 */
volatile uint8_t APP_PER_state = APP_STATUS_ADVERTISEMENT;
volatile uint8_t AccGryro_DataReady = 0;
/**
 * @}
 */

/** @defgroup APP_MNGR_Private_Functions APP_MNGR_Private_Functions
 * @{
 */

/**
 * @brief  BlueNRG-1 Initialization.
 * @param  None.
 * @retval APP_Status: APP_SUCCESS if the configuration is ok, APP_ERROR otherwise.
 */
APP_Status PER_APP_Init_BLE(void)
{
  uint8_t ret = 0;
  uint16_t service_handle, dev_name_char_handle, appearance_char_handle;
  
  ret = aci_hal_write_config_data(CONFIG_DATA_PUBADDR_OFFSET, CONFIG_DATA_PUBADDR_LEN, PERIPHERAL_BDADDR);
  
  if (ret != BLE_STATUS_SUCCESS)
  {
    return APP_ERROR;
  }
  
  aci_hal_set_tx_power_level(1, 4);
  
  /* GATT Init */
  ret = aci_gatt_init();
  if (ret != BLE_STATUS_SUCCESS) 
  { 
    return APP_ERROR;
  }
 
  /* GAP Init */
  ret = aci_gap_init(GAP_PERIPHERAL_ROLE, 0, 0x07, &service_handle, &dev_name_char_handle, &appearance_char_handle);
  if (ret != BLE_STATUS_SUCCESS) 
  {
    return APP_ERROR;
  }

  /* Set auth requirement*/
  aci_gap_set_authentication_requirement(MITM_PROTECTION_REQUIRED, OOB_AUTH_DATA_ABSENT, NULL, 7, 16, USE_FIXED_PIN_FOR_PAIRING, 123456, BONDING);
                               
  return APP_SUCCESS;
}

/**
 * @brief  BLE service and characteristics initialization.
 * @param  None.
 * @retval APP_Status: APP_SUCCESS if the configuration is ok, APP_ERROR otherwise.
 */
APP_Status PER_APP_Service_Init(void)
{
  /* service creation */
  tBleStatus ret = aci_gatt_add_service(UUID_TYPE_128,
                            (Service_UUID_t *) bluemic1_service_uuid, PRIMARY_SERVICE,  10,
                            (uint16_t*)&ServiceHandle); 
                               
  if (ret != BLE_STATUS_SUCCESS)
  {
    return APP_ERROR;
  }
  
  /* BlueVoice characteristics creation */  
  BV_APP_Status BV_status = BV_APP_add_char(ServiceHandle);
  
  if(BV_status != BV_APP_SUCCESS)
  {
    return APP_ERROR;
  }
  
  /* Inertial characteristics creation */  
  INERTIAL_APP_Status IN_status = INERTIAL_APP_add_char(ServiceHandle);
  
  if(IN_status != INERTIAL_APP_SUCCESS)
  {
    return APP_ERROR;
  }
  
  return APP_SUCCESS;
}

/**
 * @brief  This function is called from the server in order set advertisement mode.
 * @param  None
 * @retval APP_Status: APP_SUCCESS if the configuration is ok, APP_ERROR otherwise.
 */
APP_Status PER_APP_Advertise(void)
{
  uint8_t ret = 0;
  
  uint8_t local_name[] =
  {
    AD_TYPE_COMPLETE_LOCAL_NAME, NAME_WEAR
  };

  uint8_t manuf_data[20] = {
  2,0x0A,0x00,
  8,0x09,NAME_WEAR, /* Complete Name */
  7,0xFF,0x01,      /* SKD version */
         0x00,
         0x48,      /* AudioSync+AudioData */
         0xC0,      /* Acc+Gyr */
         0x00,
         0x00 
  };

  /* disable scan response */
  ret = hci_le_set_scan_response_data(0, NULL);

  ret = aci_gap_set_discoverable(ADV_IND, 0,0, PUBLIC_ADDR, NO_WHITE_LIST_USE,
                           sizeof(local_name), local_name, 0, NULL, 0, 0);

  /* Send Advertising data */
  ret = aci_gap_update_adv_data(20, manuf_data);

  if (ret != BLE_STATUS_SUCCESS)
  {
    return APP_ERROR;
  }
  
  return APP_SUCCESS;
}

/**
 * @brief  Process user input.
 * @param  None.
 * @retval None.
 */
void APP_Tick(void)
{
  switch (APP_PER_state)
  {
  case APP_STATUS_ADVERTISEMENT:
    {
      if (led_toggle_count++ >= LED_TOGGLE_ADVERTISEMENT)
      {
        led_toggle_count = 0;
        BSP_LED_Toggle(LED2);
      }
    }
    break;
  case APP_STATUS_CONNECTED:
    {   
      switch(APP_PER_enabled)
      {
      case APP_READY:
        {
          if (led_toggle_count++ >= LED_TOGGLE_READY)
          {
            led_toggle_count = 0;
            BSP_LED_Toggle(LED2);
          }
        }
        break;         
      case APP_BLUEVOICE_ENABLE:
        {
          if(audio_streaming_active)
          {
            if (led_toggle_count++ >= LED_TOGGLE_BLUEVOICE)
            {
              led_toggle_count = 0;
              BSP_LED_Toggle(LED2);
            }
            /* Updated Audio data */  
            BV_APP_DataUpdate();   
          }
          else
          {
            if (led_toggle_count++ >= LED_TOGGLE_BLUEVOICE*3)
            {
              led_toggle_count = 0;
              BSP_LED_Toggle(LED2);
            }
          }
        }
        break;
      case APP_INERTIAL_ENABLE:
        {
          if (led_toggle_count++ >= LED_TOGGLE_INERTIAL)
          {
            led_toggle_count = 0;
            BSP_LED_Toggle(LED2);         
          }      
          if(AccGryro_DataReady)
          {
            AccGryro_DataReady = 0;
            /* Updated Acc and Gyro data */  
            INERTIAL_APP_DataUpdate(ServiceHandle);
          }
        }
        break;
      default:
        {
        }
        break;
      }
    }
    break;  
  } 
}

/**
 * @brief  Error handler.
 * @param  None.
 * @retval None.
 */
void PER_APP_Error_Handler(void)
{
  while(1)
  {
  }
}    

/**
 * @}
 */

/** @defgroup APP_MNGR_BlueNRG_Events APP_MNGR_BlueNRG_Events
 * @{
 */

/*******************************************************************************
 * Function Name  : hci_le_connection_complete_event.
 * Description    : This event indicates that a new connection has been created.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void hci_le_connection_complete_event(uint8_t Status,
                                      uint16_t Connection_Handle,
                                      uint8_t Role,
                                      uint8_t Peer_Address_Type,
                                      uint8_t Peer_Address[6],
                                      uint16_t Conn_Interval,
                                      uint16_t Conn_Latency,
                                      uint16_t Supervision_Timeout,
                                      uint8_t Master_Clock_Accuracy)

{ 
  /* Connection completed */
  BluevoiceADPCM_BNRG1_ConnectionComplete_CB(Connection_Handle);
   
  BSP_LED_On(LED1);

  conn_handle = Connection_Handle;
  int ret = aci_l2cap_connection_parameter_update_req(conn_handle,
                                                9 /* interval_min*/,
                                                9 /* interval_max */,
                                                0   /* slave_latency */,
                                                400 /*timeout_multiplier*/);
  
  
  /* In order to use an iOS device as receiver, with the ST BlueMS app, please substitute the following function with the previous. */
  /* With iOS only the 8kHz (as audio sampling frequency) version is available */
//  int ret = aci_l2cap_connection_parameter_update_req(conn_handle,
//                                                8 /* interval_min*/,
//                                                17 /* interval_max */,
//                                                0   /* slave_latency */,
//                                                400 /*timeout_multiplier*/);
  
  /* In order to use an Android device version 4 as receiver, with audio sampling frequancy @16kHz, */
  /* please substitute the following function with the previous. */
//  int ret = aci_l2cap_connection_parameter_update_req(conn_handle,
//                                                8 /* interval_min*/,
//                                                8 /* interval_max */,
//                                                0   /* slave_latency */,
//                                                400 /*timeout_multiplier*/);
  
  if (ret != BLE_STATUS_SUCCESS)
  {
    while (1);
  }
  
  APP_PER_state = APP_STATUS_CONNECTED;
    
}/* end hci_le_connection_complete_event() */


/*******************************************************************************
 * Function Name  : hci_disconnection_complete_event_isr.
 * Description    : This event occurs when a connection is terminated.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void hci_disconnection_complete_event(uint8_t Status,
                                          uint16_t Connection_Handle,
                                          uint8_t Reason)
{
  if(audio_streaming_active)
    BV_APP_StartStop_ctrl();
  audio_streaming_active = 0;
  
  /* Make the device connectable again. */
  BluevoiceADPCM_BNRG1_DisconnectionComplete_CB();
  APP_PER_state = APP_STATUS_ADVERTISEMENT;
  
  BSP_LED_Off(LED1);
  
  /*Set module in advertise mode*/
  APP_Status status = PER_APP_Advertise(); 
  if(status != APP_SUCCESS)
  {
    PER_APP_Error_Handler();
  }

}/* end hci_disconnection_complete_event_isr() */


/*******************************************************************************
 * Function Name  : aci_gatt_attribute_modified_event.
 * Description    : This event occurs when an attribute is modified.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void aci_gatt_attribute_modified_event(uint16_t Connection_Handle,
                                       uint16_t Attr_Handle,
                                       uint16_t Offset,
                                       uint8_t Attr_Data_Length,
                                       uint8_t Attr_Data[])
{
  if((Attr_Handle == tx_handle.CharAudioHandle+2) || (Attr_Handle == tx_handle.CharAudioSyncHandle+2))
  {
    if(Attr_Data[0] == 0x01)
    {   
      if(!audio_streaming_active)
      {
        BV_APP_StartStop_ctrl();
      }
      
      APP_PER_enabled = APP_BLUEVOICE_ENABLE;
      BluevoiceADPCM_BNRG1_AttributeModified_CB(Attr_Handle, Attr_Data_Length, Attr_Data);
    }
    else if(Attr_Data[0] == 0x00)
    {
      if(APP_PER_enabled == APP_BLUEVOICE_ENABLE)
      {
        APP_PER_enabled = APP_READY;
        if(audio_streaming_active)
        {
          BV_APP_StartStop_ctrl();
        }
      }
    }
  }     
  if(Attr_Handle == AccGyroHandle+2)
  {
    if(Attr_Data[0] == 0x01)
    {
      APP_PER_enabled = APP_INERTIAL_ENABLE;
      INERTIAL_StartTimer();
    }
    else if(Attr_Data[0] == 0x00)
    {
      if(APP_PER_enabled == APP_INERTIAL_ENABLE)
      {
        APP_PER_enabled = APP_READY;
      }
      INERTIAL_StopTimer();
    }
  } 
}

/*******************************************************************************
 * Function Name  : aci_gatt_tx_pool_available_event.
 * Description    : This event occurs when a TX pool available is received.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void aci_gatt_tx_pool_available_event(uint16_t Connection_Handle,
                                      uint16_t Available_Buffers)
{       
  /* It allows to notify when at least 2 GATT TX buffers are available */
  tx_buffer_full = 0;
} 
/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
