/**
******************************************************************************
* @file    inertial_app.h
* @author  Central Labs
* @version V 1.0.0
* @date    May-2017
* @brief   Header for inertial_app.c module.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __INERTIAL_APP_H
#define __INERTIAL_APP_H

/* Includes ------------------------------------------------------------------*/

#include "bluenrg1_stack.h"

/** @addtogroup BLUEMIC_1_APP BLUEMIC_1_APP
 * @{
 */

/** @addtogroup BLUEMIC_1_INERTIAL_APP BLUEMIC_1_INERTIAL_APP
 * @{
 */

/** @defgroup INERTIAL_APP_Exported_Types INERTIAL_APP_Exported_Types
  * @{
  */

/**
 * @brief BlueVoice application status
 */
typedef enum 
{
  INERTIAL_APP_SUCCESS = 0x00,  /*!< IN_APP Success.*/
  INERTIAL_APP_ERROR = 0x10     /*!< IN_APP Error.*/
} INERTIAL_APP_Status;

/**
  * @}
  */

/* Exported variables --------------------------------------------------------*/
extern volatile uint16_t AccGyroHandle;

/* Exported functions ------------------------------------------------------- */

/** @defgroup INERTIAL_APP_Functions_prototype INERTIAL_APP_Functions_prototype
  * @{
  */
  
/**
 * @brief  This function is called to initialize Inertia application.
 * @param  None
 * @retval INERTIAL_APP_Status: INERTIAL_APP_SUCCESS if the configuration is ok, INERTIAL_APP_ERROR otherwise.
 */
INERTIAL_APP_Status INERTIAL_APP_Init(void);

/**
 * @brief  This function is called to add Inertial characteristics.
 * @param  service_handle: handle of the service
 * @retval INERTIAL_APP_Status: INERTIAL_APP_SUCCESS if the configuration is ok, INERTIAL_APP_ERROR otherwise.
 */
INERTIAL_APP_Status INERTIAL_APP_add_char(uint16_t service_handle);

/**
 * @brief  Update acceleration/Gryoscope characteristics value
 * @param  service_handle: handle of the service
 * @retval INERTIAL_APP_Status: INERTIAL_APP_SUCCESS if the configuration is ok, INERTIAL_APP_ERROR otherwise.
 */
INERTIAL_APP_Status INERTIAL_APP_DataUpdate(uint16_t service_handle);

/**
  * @brief  Timer Configuration.
  * @param  None
  * @retval None
  */
void MFT_Configuration(void);
    
/**
* @brief  Timer start
* @param  None
* @retval None
*/                                   
void INERTIAL_StartTimer(void);

/**
* @brief  Timer stop.
* @param  None
* @retval None
*/                               
void INERTIAL_StopTimer(void);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#endif /* __INERTIAL_APP_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
