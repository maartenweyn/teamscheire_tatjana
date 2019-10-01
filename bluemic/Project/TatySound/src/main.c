
/******************** (C) COPYRIGHT 2015 STMicroelectronics ********************
* File Name          : main.c 
* Author             : Central Labs
* Version            : V1.0.0
* Date               : May-2017
* Description        : Example code for STEVAL-BLUEMIC-1 
********************************************************************************
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
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "peripheral_mngr_app.h"
#include "BlueVoice_config.h"
#include "steval_bluemic1.h"
#include "steval_bluemic1_audio_in.h"
#include "sleep.h"
#include "clock.h"

/* Private variables ---------------------------------------------------------*/ 
volatile uint32_t lSystickCounter=0;

/* Private function prototypes -----------------------------------------------*/
void DelayMs(volatile uint32_t lTimeMs);
void Error_Handler(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{
  /* System initialization function */
  SystemInit();
 
  /* SysTick initialization 1ms */  
  Clock_Init(); 
  
  /* Led Init */
  BSP_LED_Init(LED1); /* Activity led */
  BSP_LED_Init(LED2); /* Activity led */
      
  /* BUTTON_1 initialization for start/stop audio streaming */
  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);

  /* BlueNRG-1 stack init */
  uint8_t ret = BlueNRG_Stack_Initialization(&BlueNRG_Stack_Init_params);
  if (ret != BLE_STATUS_SUCCESS)
  {   
    PER_APP_Error_Handler();
  }
  
  /* BLE Initialization */
  ret = PER_APP_Init_BLE();
  if(ret != APP_SUCCESS)
  {
    PER_APP_Error_Handler();
  }
  
  /* BlueVoice profile Initialization */
  ret = BV_APP_profile_init(AUDIO_SAMPLING_FREQUENCY);  
  if(ret != APP_SUCCESS)
  {
    PER_APP_Error_Handler();
  }
   
  /* Inertial sensors Initialization */
  INERTIAL_APP_Init();
  
  /* BLE Service Initialization*/
  ret = PER_APP_Service_Init();
  if(ret != APP_SUCCESS)
  {
    PER_APP_Error_Handler();
  }
  
  BSP_AUDIO_IN_Init(AUDIO_SAMPLING_FREQUENCY);

  /* MFT configuration for INERTIAL app */
  MFT_Configuration();
  
  /*Set module in advertise mode*/
  ret = PER_APP_Advertise(); 
  if(ret != BV_APP_SUCCESS)
  {
    PER_APP_Error_Handler();
  }
  
  /* Infinite loop */
  while(1) 
  { 
    /* BLE Stack Tick */
    BTLE_StackTick();

    /* Application Tick */
    APP_Tick();
  }
}

/**
* @brief  Delay function in ms.
* @param  lTimeMs time in ms
* @retval None
*/
void DelayMs(volatile uint32_t lTimeMs)
{
  uint32_t nWaitPeriod = ~lSystickCounter;
  
  if(nWaitPeriod<lTimeMs)
  {
    while( lSystickCounter != 0xFFFFFFFF);
    nWaitPeriod = lTimeMs-nWaitPeriod;
  }
  else
    nWaitPeriod = lTimeMs+ ~nWaitPeriod;
  
  while( lSystickCounter != nWaitPeriod ) ;

}


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}

#endif


/******************* (C) COPYRIGHT 2015 STMicroelectronics *****END OF FILE****/
