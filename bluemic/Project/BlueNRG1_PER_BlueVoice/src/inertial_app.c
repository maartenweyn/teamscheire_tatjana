/**
 ******************************************************************************
 * @file    inertial_app.c
 * @author  Central Labs
 * @version V 1.0.0
 * @date    May-2017
 * @brief   This file contains definitions for Inertial application.
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
#include "inertial_app.h"
#include "BlueNRG1_conf.h"
#include "sleep.h"  
#include "ble_const.h" 
#include "steval_bluemic1.h"
#include "steval_bluemic1_accelero.h"
#include "steval_bluemic1_gyro.h"

/** @addtogroup BLUEMIC_1_APP BLUEMIC_1_APP
 * @{
 */

/** @defgroup BLUEMIC_1_INERTIAL_APP BLUEMIC_1_INERTIAL_APP
 * @{
 */

/** @defgroup INERTIAL_APP_Private_Types INERTIAL_APP_Private_Types
 * @{
 */

/** 
 * @brief Structure containing accelerometer or gyroscope value for each axis.
 */
typedef struct {
  int32_t AXIS_X;
  int32_t AXIS_Y;
  int32_t AXIS_Z;
} AxesRaw_t;

/**
  * @}
  */

/** @defgroup INERTIAL_APP_Private_Variables INERTIAL_APP_Private_Variables
 * @{
 */
void *ACCELERO_handle = NULL;
void *GYRO_handle = NULL;
/**
  * @}
  */

/** @defgroup INERTIAL_APP_Private_Constants INERTIAL_APP_Private_Constants
 * @{
 */   
static const uint8_t acc_gyr_char_uuid[16] =
{
  0x1b,0xc5,0xd5,0xa5,0x02,0x00,0x36,0xac,0xe1,0x11,0x01,0x00,0x00,0x00,0xE0,0x00
};
/**
  * @}
  */

/** @defgroup INERTIAL_APP_Exported_Variables INERTIAL_APP_Exported_Variables
 * @{
 */  
volatile uint16_t AccGyroHandle = 0;
/**
  * @}
  */

INERTIAL_APP_Status AccGyro_Update(void);


/** @defgroup INERTIAL_APP_Exported_Functions INERTIAL_APP_Exported_Functions
 * @{
 */  

/**
 * @brief  This function is called to add Inertial characteristics.
 * @param  service_handle: handle of the service
 * @retval INERTIAL_APP_Status: INERTIAL_APP_SUCCESS if the configuration is ok, INERTIAL_APP_ERROR otherwise.
 */
INERTIAL_APP_Status INERTIAL_APP_add_char(uint16_t service_handle)
{      
  uint8_t ret = aci_gatt_add_char(service_handle,
                            UUID_TYPE_128, (Char_UUID_t *) acc_gyr_char_uuid,
                            2+3*3*2, CHAR_PROP_NOTIFY, ATTR_PERMISSION_NONE, GATT_DONT_NOTIFY_EVENTS, 16, 1,
                            (uint16_t*)&AccGyroHandle);
                        
  if (ret != BLE_STATUS_SUCCESS)
  {
    return INERTIAL_APP_ERROR;
  }

  return INERTIAL_APP_SUCCESS;
}

/**
 * @brief  This function is called to initialize Inertial application.
 * @param  None
 * @retval INERTIAL_APP_Status: INERTIAL_APP_SUCCESS if the configuration is ok, INERTIAL_APP_ERROR otherwise.
 */
INERTIAL_APP_Status INERTIAL_APP_Init(void)
{
  if(BSP_ACCELERO_Init(LSM6DSL_X_0, &ACCELERO_handle) == COMPONENT_ERROR)
  {
    return INERTIAL_APP_ERROR;
  }
  BSP_ACCELERO_Sensor_Enable( ACCELERO_handle );
  
  if(BSP_GYRO_Init(LSM6DSL_G_0, &GYRO_handle) == COMPONENT_ERROR)
  {
    return INERTIAL_APP_ERROR;
  }
  BSP_ACCELERO_Sensor_Enable( GYRO_handle );
  
  return INERTIAL_APP_SUCCESS;
}

/**
  * @brief  Timer Configuration.
  * @param  None
  * @retval None
  */
void MFT_Configuration(void)
{
  NVIC_InitType NVIC_InitStructure;
  MFT_InitType timer_init;
  
  SysCtrl_PeripheralClockCmd(CLOCK_PERIPH_MTFX1, ENABLE);
  
  MFT_StructInit(&timer_init);
  
  timer_init.MFT_Mode = MFT_MODE_1;
  timer_init.MFT_Prescaler = 160-1;      /* 5 us clock */
  
  /* MFT1 configuration */
  timer_init.MFT_Clock2 = MFT_PRESCALED_CLK;
  MFT_Init(MFT1, &timer_init);
  /* Set the counter at 30 ms */
  MFT_SetCounter2(MFT1, 6000);
  
  /* Enable MFT1 Interrupt 1 */
  NVIC_InitStructure.NVIC_IRQChannel = MFT1B_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = LOW_PRIORITY;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /** Enable the MFT interrupt */
  MFT_EnableIT(MFT1, MFT_IT_TND, ENABLE);
}

/**
* @brief  Timer start
* @param  None
* @retval None
*/                                
void INERTIAL_StartTimer(void)
{
  /* Set the counter at 30 ms */
  MFT_SetCounter2(MFT1, 6000);
  MFT_Cmd(MFT1, ENABLE);
}

/**
* @brief  Timer stop.
* @param  None
* @retval None
*/                                
void INERTIAL_StopTimer(void)
{
  MFT_Cmd(MFT1, DISABLE);
}

extern uint32_t lSystickCounter;
/**
 * @brief  Update acceleration/Gryoscope characteristics value
 * @param  service_handle: handle of the service
 * @retval INERTIAL_APP_Status: INERTIAL_APP_SUCCESS if the configuration is ok, INERTIAL_APP_ERROR otherwise.
 */
INERTIAL_APP_Status INERTIAL_APP_DataUpdate(uint16_t service_handle)
{  
  SensorAxes_t ACC_Value;
  SensorAxes_t GYR_Value;
  uint8_t status = 0;
  static uint32_t timestamp = 0;
  timestamp += 3;
  uint8_t buff[2+3*2*2];
  
  if(BSP_ACCELERO_IsInitialized(ACCELERO_handle, &status) == COMPONENT_OK && status == 1)
  {  
    if (BSP_ACCELERO_Get_Axes(ACCELERO_handle, &ACC_Value) != COMPONENT_OK) 
    {
      return INERTIAL_APP_ERROR;
    }
            
    buff[0] = (uint8_t)timestamp;
    buff[1] = (uint8_t)(timestamp>>8);

    buff[2] = (uint8_t)(ACC_Value.AXIS_X);
    buff[3] = (uint8_t)(ACC_Value.AXIS_X>>8);
    buff[4] = (uint8_t)(ACC_Value.AXIS_Y);
    buff[5] = (uint8_t)(ACC_Value.AXIS_Y>>8);
    buff[6] = (uint8_t)(ACC_Value.AXIS_Z);
    buff[7] = (uint8_t)(ACC_Value.AXIS_Z>>8);
  
    if (BSP_GYRO_Get_Axes(GYRO_handle, &GYR_Value) != COMPONENT_OK)
    {
      return INERTIAL_APP_ERROR;
    }
    
    GYR_Value.AXIS_X/=100;
    GYR_Value.AXIS_Y/=100;
    GYR_Value.AXIS_Z/=100;

    buff[8] = (uint8_t)(GYR_Value.AXIS_X);
    buff[9] = (uint8_t)(GYR_Value.AXIS_X>>8);
    buff[10] = (uint8_t)(GYR_Value.AXIS_Y);
    buff[11] = (uint8_t)(GYR_Value.AXIS_Y>>8);
    buff[12] = (uint8_t)(GYR_Value.AXIS_Z);
    buff[13] = (uint8_t)(GYR_Value.AXIS_Z>>8);
    
    if(aci_gatt_update_char_value(service_handle, AccGyroHandle, 0,
                                      14, buff)==BLE_STATUS_INSUFFICIENT_RESOURCES)
    {
      return INERTIAL_APP_ERROR;
    }
    return INERTIAL_APP_SUCCESS;	
  }
  return INERTIAL_APP_ERROR;	
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
