/**
 ******************************************************************************
 * @file    steval_bluemic1.c
 * @author  Central Lab
 * @version V1.0.0
 * @date    16-May-2017
 * @brief   This file provides low level functionalities for STEVAL-BLUEMIC-1
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
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
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/

#include "steval_bluemic1.h"

/** @defgroup BSP BSP
 * @{
 */

/** @defgroup BLUEMIC1_LOW_LEVEL BLUEMIC1_LOW_LEVEL
 * @{
 */

/** @defgroup BLUEMIC1_LOW_LEVEL_Private_Defines BLUEMIC1_LOW_LEVEL_Private_Defines
* @{
*/  

/**
 * @brief  I2C DR address
 */
#define I2C_TX_DR_BASE_ADDR                (I2C2_BASE + 0x10)

#define I2C_RX_DR_BASE_ADDR                (I2C2_BASE + 0x18)
   
/**
 * @}
 */

/** @defgroup BLUEMIC1_LOW_LEVEL_Private_Variables BLUEMIC1_LOW_LEVEL_Private_Variables
 * @{
 */

/**
 * @brief I2C buffers used for DMA application
 */
uint8_t i2c_buffer_tx[I2C_BUFF_SIZE];
uint8_t i2c_buffer_rx[I2C_BUFF_SIZE];

/**
 * @brief  I2C flag set at end of transaction
 */
volatile FlagStatus i2c_eot = SET;

const uint16_t BUTTON_PIN[BUTTONn] = {PUSH_BUTTON1_PIN}; 
const uint16_t LED_PIN[LEDn] = {BLUE_MIC1_LED1_PIN, BLUE_MIC1_LED2_PIN}; 

/**
 * @}
 */

void StevalBlueMic1_I2CDmaInit(uint32_t baudrate);
void StevalBlueMic1_I2CDmaRead(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr, uint8_t NumByteToRead);
void StevalBlueMic1_I2CDmaWrite(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr, uint8_t NumByteToWrite);
void StevalBlueMic1_I2CIrqInit(uint32_t baudrate);
void StevalBlueMic1_I2CIrqRead(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr, uint8_t NumByteToRead);
void StevalBlueMic1_I2CIrqWrite(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr, uint8_t NumByteToWrite);
void StevalBlueMic1_I2CInit(uint32_t baudrate);
DrvStatusTypeDef StevalBlueMic1_I2CRead(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr, uint8_t NumByteToRead);
DrvStatusTypeDef StevalBlueMic1_I2CWrite(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr, uint8_t NumByteToWrite);


/** @defgroup BLUEMIC1_LOW_LEVEL_Exported_Functions BLUEMIC1_LOW_LEVEL_Exported_Functions
  * @{
  */

/**
  * @brief  Configures Button GPIO and EXTI Line.
  * @param  Button: Specifies the Button to be configured. This parameter should be: BUTTON_KEY
  * @param  ButtonMode: Specifies Button mode.
  *         This parameter can be one of following parameters:   
  *         @arg BUTTON_MODE_GPIO: Button will be used as simple IO 
  *         @arg BUTTON_MODE_EXTI: Button will be connected to EXTI line with interrupt generation capability  
  */
void BSP_PB_Init(Button_TypeDef Button, ButtonMode_TypeDef ButtonMode)
{  
  GPIO_InitType GPIO_InitStructure;
   
  if(ButtonMode == BUTTON_MODE_GPIO)
  {
    /* Enables the BUTTON Clock */
    SysCtrl_PeripheralClockCmd(CLOCK_PERIPH_GPIO, ENABLE);

    /* Configures Button pin as input */
    GPIO_InitStructure.GPIO_Pin = BUTTON_PIN[Button];
    GPIO_InitStructure.GPIO_Mode = GPIO_Input;
    GPIO_InitStructure.GPIO_Pull = DISABLE;
    GPIO_InitStructure.GPIO_HighPwr = DISABLE;
    GPIO_Init(&GPIO_InitStructure);
  }
  if(ButtonMode == BUTTON_MODE_EXTI)
  {
    GPIO_EXTIConfigType GPIO_EXTIStructure;
    NVIC_InitType NVIC_InitStructure;

    /* Enables the BUTTON Clock */
    SysCtrl_PeripheralClockCmd(CLOCK_PERIPH_GPIO, ENABLE);

    /* Configures Button pin as input */
    GPIO_InitStructure.GPIO_Pin = BUTTON_PIN[Button];
    GPIO_InitStructure.GPIO_Mode = GPIO_Input;
    GPIO_InitStructure.GPIO_Pull = DISABLE;
    GPIO_InitStructure.GPIO_HighPwr = DISABLE;
    GPIO_Init(&GPIO_InitStructure);
    
    /* Set the GPIO interrupt priority and enable it */
    NVIC_InitStructure.NVIC_IRQChannel = GPIO_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = LOW_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    /* Configures EXTI line */
    GPIO_EXTIStructure.GPIO_Pin = BUTTON_PIN[Button];
    GPIO_EXTIStructure.GPIO_IrqSense = GPIO_IrqSense_Edge;
    GPIO_EXTIStructure.GPIO_Event = GPIO_Event_High;
    GPIO_EXTIConfig(&GPIO_EXTIStructure);

    /* Clear pending interrupt */
    GPIO_ClearITPendingBit(BUTTON_PIN[Button]);
    
    /* Enable the interrupt */
    GPIO_EXTICmd(BUTTON_PIN[Button], ENABLE);
  }
}

/**
  * @brief  Returns the selected Button state.
  * @param  Button: Specifies the Button to be checked.
  *                 This parameter should be: BUTTON_USER 
  * @retval The Button GPIO pin value.
  */
uint32_t BSP_PB_GetState(Button_TypeDef Button)
{
  if(GPIO_ReadBit(BUTTON_PIN[Button]))
    return 1;
  else
    return 0;
}

/**
 * @brief  Get the pending bit state.
 * @param  Button: Specifies the Button to be checked.
 *                 This parameter should be: BUTTON_USER 
 * @retval This parameter can be: 1 or 0.
 */
uint32_t BSP_PB_GetITPendingBit(Button_TypeDef Button)
{
  return GPIO_GetITPendingBit(BUTTON_PIN[Button]);
}

/**
 * @brief  Clear the pending bit state.
 * @param  Button: Specifies the Button ID.
 *                 This parameter should be: BUTTON_USER 
 * @retval None
 */
void BSP_PB_ClearITPendingBit(Button_TypeDef Button)
{
  GPIO_ClearITPendingBit(BUTTON_PIN[Button]);
}

/**
 * @brief  Configures LEDs.
 * @param  Led Specifies the Led to be configured.
 *         This parameter can be one of following parameters:
 *         @arg LED1
 *         @arg LED2
 * @retval None.
 */
void BSP_LED_Init(Led_TypeDef Led)
{
  GPIO_InitType GPIO_InitStructure;
  
  /* Enable the GPIO Clock */
  SysCtrl_PeripheralClockCmd(CLOCK_PERIPH_GPIO, ENABLE);
  
  /* Configure the LED pin */
  GPIO_InitStructure.GPIO_Pin = LED_PIN[Led];
  GPIO_InitStructure.GPIO_Mode = GPIO_Output;
  GPIO_InitStructure.GPIO_Pull = ENABLE;
  GPIO_InitStructure.GPIO_HighPwr = ENABLE;
  GPIO_Init(&GPIO_InitStructure);
}

/**
  * @brief  Turns selected LED On.
  * @param  Led: Specifies the Led to be set On. 
  *         This parameter can be one of following parameters:
  *         @arg LED1
  *         @arg LED2
  */
void BSP_LED_On(Led_TypeDef Led)
{
  GPIO_WriteBit(LED_PIN[Led], Bit_SET);
}

/**
  * @brief  Turns selected LED Off.
  * @param  Led: Specifies the Led to be set Off. 
  *         This parameter can be one of following parameters:
  *         @arg LED1
  *         @arg LED2
  */
void BSP_LED_Off(Led_TypeDef Led)
{
  GPIO_WriteBit(LED_PIN[Led], Bit_RESET);
}

/**
  * @brief  Toggles the selected LED.
  * @param  Led: Specifies the Led to be toggled. 
  *         This parameter can be one of following parameters:
  *         @arg LED1
  *         @arg LED2
  */
void BSP_LED_Toggle(Led_TypeDef Led)
{
  GPIO_ToggleBits(LED_PIN[Led]);
}


/**
 * @brief  Configures sensor I2C interface.
 * @param  None
 * @retval None
 */
void Sensor_IO_Init(void)
{
#ifdef BLUENRG1_I2C_DMA
  StevalBlueMic1_I2CDmaInit(LSM6DS3_I2C_FREQUENCY);
#else
#ifdef BLUENRG1_I2C_IRQ 
  StevalBlueMic1_I2CIrqInit(LSM6DS3_I2C_FREQUENCY);
#else /* BLUENRG1_I2C_POLL */
  StevalBlueMic1_I2CInit(LSM6DS3_I2C_FREQUENCY);
#endif
#endif
}


/**
 * @brief  Writes a buffer to the sensor
 * @param  handle instance handle
 * @param  WriteAddr specifies the internal sensor address register to be written to
 * @param  pBuffer pointer to data buffer
 * @param  nBytesToWrite number of bytes to be written
 * @retval 0 in case of success
 * @retval 1 in case of failure
 */
uint8_t Sensor_IO_Write(void *handle, uint8_t WriteAddr, uint8_t *pBuffer, uint16_t nBytesToWrite)
{
  DrvContextTypeDef *ctx = (DrvContextTypeDef *)handle;
  
#ifdef BLUENRG1_I2C_DMA
  return StevalBlueMic1_I2CDmaWrite(pBuffer, ctx->address, WriteAddr, nBytesToWrite);
#else
#ifdef BLUENRG1_I2C_IRQ 
  return StevalBlueMic1_I2CIrqWrite(pBuffer, ctx->address, WriteAddr, nBytesToWrite);
#else /* BLUENRG1_I2C_POLL */
  return StevalBlueMic1_I2CWrite(pBuffer, ctx->address, WriteAddr, nBytesToWrite);
#endif
#endif
}



/**
 * @brief  Reads a from the sensor to buffer
 * @param  handle instance handle
 * @param  ReadAddr specifies the internal sensor address register to be read from
 * @param  pBuffer pointer to data buffer
 * @param  nBytesToRead number of bytes to be read
 * @retval 0 in case of success
 * @retval 1 in case of failure
 */
uint8_t Sensor_IO_Read( void *handle, uint8_t ReadAddr, uint8_t *pBuffer, uint16_t nBytesToRead )
{
  DrvContextTypeDef *ctx = (DrvContextTypeDef *)handle;
  
#ifdef BLUENRG1_I2C_DMA
  return StevalBlueMic1_I2CDmaRead(pBuffer, ctx->address, ReadAddr, nBytesToRead);
#else
#ifdef BLUENRG1_I2C_IRQ 
  return StevalBlueMic1_I2CIrqRead(pBuffer, ctx->address, ReadAddr, nBytesToRead);
#else /* BLUENRG1_I2C_POLL */
  return StevalBlueMic1_I2CRead(pBuffer, ctx->address, ReadAddr, nBytesToRead);
#endif
#endif  
}


/**
 * @brief  Configures sensor interrupts interface for LSM6DSL sensor.
 * @param  None
 * @retval COMPONENT_OK in case of success
 * @retval COMPONENT_ERROR in case of failure
 */
DrvStatusTypeDef LSM6DSL_Sensor_IO_ITConfig( void )
{
  SysCtrl_PeripheralClockCmd(CLOCK_PERIPH_GPIO, ENABLE);
  GPIO_Init(&(GPIO_InitType){LSM6DS3_IRQ_1_PIN|LSM6DS3_IRQ_2_PIN, GPIO_Input, DISABLE, DISABLE});
  NVIC_Init(&(NVIC_InitType){GPIO_IRQn, LOW_PRIORITY, ENABLE});
  GPIO_EXTIConfig(&(GPIO_EXTIConfigType){LSM6DS3_IRQ_1_PIN, GPIO_IrqSense_Edge, GPIO_Event_Low});
  GPIO_EXTIConfig(&(GPIO_EXTIConfigType){LSM6DS3_IRQ_2_PIN, GPIO_IrqSense_Edge, GPIO_Event_Low});
  GPIO_ClearITPendingBit(LSM6DS3_IRQ_1_PIN);
  GPIO_ClearITPendingBit(LSM6DS3_IRQ_2_PIN);
  GPIO_EXTICmd(LSM6DS3_IRQ_1_PIN, ENABLE);
  GPIO_EXTICmd(LSM6DS3_IRQ_2_PIN, ENABLE);
  
  return COMPONENT_OK;
}

/**
 * @}
 */

/** @defgroup BLUEMIC1_LOW_LEVEL_Private_Functions BLUEMIC1_LOW_LEVEL_Private_Functions
* @{
*/ 

/**
 * @brief  Configures I2C interface
 * @param  baudrate I2C clock frequency
 * @retval None
 */
void StevalBlueMic1_I2CInit(uint32_t baudrate)
{   
  GPIO_InitType GPIO_InitStructure;
  I2C_InitType I2C_InitStruct;
    
  /* Enable I2C and GPIO clocks */
  if(BLUE_MIC1_I2C == I2C2) 
  {
    SysCtrl_PeripheralClockCmd(CLOCK_PERIPH_GPIO | CLOCK_PERIPH_I2C2, ENABLE);
  }
  else 
  {
    SysCtrl_PeripheralClockCmd(CLOCK_PERIPH_GPIO | CLOCK_PERIPH_I2C1, ENABLE);
  }
      
  /* Configure I2C pins */
  GPIO_InitStructure.GPIO_Pin = BLUE_MIC1_I2C_CLK_PIN ;
  GPIO_InitStructure.GPIO_Mode = BLUE_MIC1_I2C_CLK_MODE;
  GPIO_InitStructure.GPIO_Pull = DISABLE;
  GPIO_InitStructure.GPIO_HighPwr = DISABLE;
  GPIO_Init(&GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = BLUE_MIC1_I2C_DATA_PIN;
  GPIO_InitStructure.GPIO_Mode = BLUE_MIC1_I2C_DATA_MODE;  
  GPIO_Init(&GPIO_InitStructure);
      
  /* Configure I2C in master mode */
  I2C_StructInit(&I2C_InitStruct);
  I2C_InitStruct.I2C_OperatingMode = I2C_OperatingMode_Master;
  I2C_InitStruct.I2C_ClockSpeed = baudrate;
  I2C_Init((I2C_Type*)BLUE_MIC1_I2C, &I2C_InitStruct);
  
  /* Clear all I2C pending interrupts */
  I2C_ClearITPendingBit((I2C_Type*)BLUE_MIC1_I2C, I2C_IT_MSK);
}

/**
 * @brief  I2C function to write registers of a slave device
 * @param  pBuffer buffer contains data to write
 * @param  DeviceAddr the I2C slave address
 * @param  RegisterAddr register address
 * @param  NumByteToRead number of byte to write
 * @retval SUCCESS in case of success, an error code otherwise
 */
DrvStatusTypeDef StevalBlueMic1_I2CWrite(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr, uint8_t NumByteToWrite)
{
  I2C_TransactionType t;
  
  /* Write the slave address */
  t.Operation = I2C_Operation_Write;
  t.Address = DeviceAddr;
  t.StartByte = I2C_StartByte_Disable;
  t.AddressType = I2C_AddressType_7Bit;
  t.StopCondition = I2C_StopCondition_Enable;
  t.Length = 1+NumByteToWrite;
  
  /* Flush the slave address */
  I2C_FlushTx((I2C_Type*)BLUE_MIC1_I2C);
  while (I2C_WaitFlushTx((I2C_Type*)BLUE_MIC1_I2C) == I2C_OP_ONGOING);
  
  /* Begin transaction */
  I2C_BeginTransaction((I2C_Type*)BLUE_MIC1_I2C, &t);

  /* Fill TX FIFO with data to write */
  I2C_FillTxFIFO((I2C_Type*)BLUE_MIC1_I2C, RegisterAddr);

  for(uint8_t i=0; i<NumByteToWrite;i++) {
    I2C_FillTxFIFO((I2C_Type*)BLUE_MIC1_I2C, pBuffer[i]);
  }
  
  /* Wait loop */
  do 
  {
    if(I2C_GetStatus((I2C_Type*)BLUE_MIC1_I2C) == I2C_OP_ABORTED)
      return COMPONENT_ERROR;
   
  } while (I2C_GetITStatus((I2C_Type*)BLUE_MIC1_I2C, I2C_IT_MTD) == RESET);
    
  /* Clear pending bits */
  I2C_ClearITPendingBit((I2C_Type*)BLUE_MIC1_I2C, I2C_IT_MTD | I2C_IT_MTDWS);

  return COMPONENT_OK;
}

/**
 * @brief  I2C function to read registers from a slave device
 * @param  pBuffer buffer to retrieve data from a slave
 * @param  DeviceAddr the I2C slave address
 * @param  RegisterAddr register address
 * @param  NumByteToRead number of byte to read
 * @retval SUCCESS in case of success, an error code otherwise
 */
DrvStatusTypeDef StevalBlueMic1_I2CRead(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr, uint8_t NumByteToRead)
{
  I2C_TransactionType t;
  
  /* Write the slave address */
  t.Operation = I2C_Operation_Write;
  t.Address = DeviceAddr;
  t.StartByte = I2C_StartByte_Disable;
  t.AddressType = I2C_AddressType_7Bit;
  t.StopCondition = I2C_StopCondition_Disable;
  t.Length = 1;  
  
  /* Flush the slave address */
  I2C_FlushTx((I2C_Type*)BLUE_MIC1_I2C);
  while (I2C_WaitFlushTx((I2C_Type*)BLUE_MIC1_I2C) == I2C_OP_ONGOING);
    
  /* Begin transaction */
  I2C_BeginTransaction((I2C_Type*)BLUE_MIC1_I2C, &t);

  /* Fill TX FIFO with data to write */
  I2C_FillTxFIFO((I2C_Type*)BLUE_MIC1_I2C, RegisterAddr);

  /* Wait loop */
  do 
  {
    if(I2C_GetStatus((I2C_Type*)BLUE_MIC1_I2C) == I2C_OP_ABORTED)
      return COMPONENT_ERROR;
    
  } while (I2C_GetITStatus((I2C_Type*)BLUE_MIC1_I2C, I2C_IT_MTDWS) == RESET);
  
  /* Clear pending bits */
  I2C_ClearITPendingBit((I2C_Type*)BLUE_MIC1_I2C, I2C_IT_MTDWS);
  
  /* read data */
  t.Operation = I2C_Operation_Read;
  t.Address = DeviceAddr;
  t.StartByte = I2C_StartByte_Disable;
  t.AddressType = I2C_AddressType_7Bit;
  t.StopCondition = I2C_StopCondition_Enable;
  t.Length = NumByteToRead;  
  I2C_BeginTransaction((I2C_Type*)BLUE_MIC1_I2C, &t);
  
  /* Wait loop */
  do 
  {
    if(I2C_OP_ABORTED == I2C_GetStatus((I2C_Type*)BLUE_MIC1_I2C))
      return COMPONENT_ERROR;
    
  } while (RESET == I2C_GetITStatus((I2C_Type*)BLUE_MIC1_I2C, I2C_IT_MTD));
  
  /* Clear pending bits */
  I2C_ClearITPendingBit((I2C_Type*)BLUE_MIC1_I2C, I2C_IT_MTD | I2C_IT_MTDWS);
  
  /* Get data from RX FIFO */
  while(NumByteToRead--) 
  {
    *pBuffer = I2C_ReceiveData((I2C_Type*)BLUE_MIC1_I2C);
    pBuffer ++;
  }
  
  return COMPONENT_OK;
}

/**
 * @brief  Configures I2C interface and the I2C_TX/I2C_RX DMA channels
 * @param  baudrate I2C clock frequency
 * @retval None
 */
void StevalBlueMic1_I2CDmaInit(uint32_t baudrate)
{   
  GPIO_InitType GPIO_InitStructure;
  I2C_InitType I2C_InitStruct;
  DMA_InitType DMA_InitStructure;
  NVIC_InitType NVIC_InitStructure;
    
  /* Enable I2C and GPIO clocks */
  if(BLUE_MIC1_I2C == I2C2) {
    SysCtrl_PeripheralClockCmd(CLOCK_PERIPH_GPIO | CLOCK_PERIPH_I2C2 | CLOCK_PERIPH_DMA, ENABLE);
  }
  else {
    SysCtrl_PeripheralClockCmd(CLOCK_PERIPH_GPIO | CLOCK_PERIPH_I2C1 | CLOCK_PERIPH_DMA, ENABLE);
  }
      
  /* Configure I2C pins */
  GPIO_InitStructure.GPIO_Pin = BLUE_MIC1_I2C_CLK_PIN ;
  GPIO_InitStructure.GPIO_Mode = BLUE_MIC1_I2C_CLK_MODE;
  GPIO_InitStructure.GPIO_Pull = DISABLE;
  GPIO_InitStructure.GPIO_HighPwr = DISABLE;
  GPIO_Init(&GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = BLUE_MIC1_I2C_DATA_PIN;
  GPIO_InitStructure.GPIO_Mode = BLUE_MIC1_I2C_DATA_MODE;  
  GPIO_Init(&GPIO_InitStructure);
      
  /* Configure I2C in master mode */
  I2C_StructInit(&I2C_InitStruct);
  I2C_InitStruct.I2C_OperatingMode = I2C_OperatingMode_Master;
  I2C_InitStruct.I2C_ClockSpeed = baudrate;
  I2C_Init((I2C_Type*)BLUE_MIC1_I2C, &I2C_InitStruct);
  
  /* Clear all I2C pending interrupts */
  I2C_ClearITPendingBit((I2C_Type*)BLUE_MIC1_I2C, I2C_IT_MSK);
  
  /* Configure DMA I2C TX channel */
  DMA_InitStructure.DMA_PeripheralBaseAddr = I2C_TX_DR_BASE_ADDR;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)i2c_buffer_tx;  
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
  DMA_InitStructure.DMA_BufferSize = (uint32_t)I2C_BUFF_SIZE;  
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;    
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;  
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  
  DMA_Init((DMA_CH_Type*)DMA_CH_I2C_TX, &DMA_InitStructure);
  
  /* Configure DMA I2C RX channel */
  DMA_InitStructure.DMA_PeripheralBaseAddr = I2C_RX_DR_BASE_ADDR;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)i2c_buffer_rx;  
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = (uint32_t)I2C_BUFF_SIZE;  
  DMA_Init((DMA_CH_Type*)DMA_CH_I2C_RX, &DMA_InitStructure);
  
  /* Enable DMA_CH_I2C_RX Transfer Complete interrupt */
  DMA_FlagConfig((DMA_CH_Type*)DMA_CH_I2C_RX, DMA_FLAG_TC, ENABLE);
  DMA_FlagConfig((DMA_CH_Type*)DMA_CH_I2C_TX, DMA_FLAG_TC, ENABLE);
  
  /* Enable the DMA Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = DMA_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = HIGH_PRIORITY;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/**
 * @brief  I2C function to write registers of a slave device
 * @param  pBuffer buffer contains data to write
 * @param  DeviceAddr the I2C slave address
 * @param  RegisterAddr register address
 * @param  NumByteToRead number of byte to write
 * @retval None
 */
void StevalBlueMic1_I2CDmaWrite(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr, uint8_t NumByteToWrite)
{
  I2C_TransactionType t;

  /* Write the slave address */
  t.Operation = I2C_Operation_Write;
  t.Address = DeviceAddr;
  t.StartByte = I2C_StartByte_Disable;
  t.AddressType = I2C_AddressType_7Bit;
  t.StopCondition = I2C_StopCondition_Enable;
  t.Length = 1 + NumByteToWrite;

  /* Fill TX FIFO with data to write */
  i2c_buffer_tx[0] = RegisterAddr;
  for(uint8_t i=0; i<NumByteToWrite;i++) 
  {
    i2c_buffer_tx[i+1] = pBuffer[i];
  }

  while(i2c_eot==RESET);
  i2c_eot = RESET;
  
  /* Clear pending bits */
  I2C_ClearITPendingBit((I2C_Type*)BLUE_MIC1_I2C, I2C_IT_MTDWS);
  
  /* Flush the slave address */
  I2C_FlushTx((I2C_Type*)BLUE_MIC1_I2C);
  while (I2C_WaitFlushTx((I2C_Type*)BLUE_MIC1_I2C) == I2C_OP_ONGOING);

  /* Enable I2C_TX/I2C_RX DMA requests */
  I2C_DMACmd((I2C_Type*)BLUE_MIC1_I2C, I2C_DMAReq_Tx, ENABLE);

  /* Reset DMA_CH_UART_TX remaining bytes register */
  ((DMA_CH_Type*)DMA_CH_I2C_TX)->CNDTR = NumByteToWrite + 1;

  /* Begin transaction */
  I2C_BeginTransaction((I2C_Type*)BLUE_MIC1_I2C, &t);
  
  /* DMA_CH_UART_TX enable */
  ((DMA_CH_Type*)DMA_CH_I2C_TX)->CCR_b.EN = SET;

  while (I2C_GetITStatus((I2C_Type*)BLUE_MIC1_I2C, I2C_IT_MTDWS) == RESET);
}


/**
 * @brief  I2C function to read registers from a slave device
 * @param  pBuffer buffer to retrieve data from a slave
 * @param  DeviceAddr the I2C slave address
 * @param  RegisterAddr register address
 * @param  NumByteToRead number of byte to read
 * @retval None
 */
void StevalBlueMic1_I2CDmaRead(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr, uint8_t NumByteToRead)
{
  I2C_TransactionType t;
  
  /* Write the slave address */
  t.Operation = I2C_Operation_Write;
  t.Address = DeviceAddr;
  t.StartByte = I2C_StartByte_Disable;
  t.AddressType = I2C_AddressType_7Bit;
  t.StopCondition = I2C_StopCondition_Disable;
  t.Length = 1;  
  
  /* Fill TX FIFO with data to write */
  i2c_buffer_tx[0] = RegisterAddr;

  while(i2c_eot==RESET);
  i2c_eot = RESET;
  
  /* Clear pending bits */
  I2C_ClearITPendingBit((I2C_Type*)BLUE_MIC1_I2C, I2C_IT_MTDWS);
  
  /* Enable I2C_TX/I2C_RX DMA requests */
  I2C_DMACmd((I2C_Type*)BLUE_MIC1_I2C, I2C_DMAReq_Tx, ENABLE);

  /* Reset DMA_CH_UART_TX remaining bytes register */
  ((DMA_CH_Type*)DMA_CH_I2C_TX)->CNDTR = 1;

  /* Flush the slave address */
  I2C_FlushTx((I2C_Type*)BLUE_MIC1_I2C);
  while (I2C_WaitFlushTx((I2C_Type*)BLUE_MIC1_I2C) == I2C_OP_ONGOING);
  
  /* Begin transaction */
  I2C_BeginTransaction((I2C_Type*)BLUE_MIC1_I2C, &t);

  /* DMA_CH_UART_TX enable */
  ((DMA_CH_Type*)DMA_CH_I2C_TX)->CCR_b.EN = SET;
  
  while(i2c_eot==RESET);
  i2c_eot = RESET;
  
  while (I2C_GetITStatus((I2C_Type*)BLUE_MIC1_I2C, I2C_IT_MTDWS) == RESET);
  
  /* Clear pending bits */
  I2C_ClearITPendingBit((I2C_Type*)BLUE_MIC1_I2C, I2C_IT_MTDWS);
 
  /* read data */
  t.Operation = I2C_Operation_Read;
  t.Address = DeviceAddr;
  t.StartByte = I2C_StartByte_Disable;
  t.AddressType = I2C_AddressType_7Bit;
  t.StopCondition = I2C_StopCondition_Enable;
  t.Length = NumByteToRead;

  /* DMA RX channel request enable */
  I2C_DMACmd((I2C_Type*)BLUE_MIC1_I2C, I2C_DMAReq_Rx, ENABLE);

  /* Reset DMA_CH_UART_TX remaining bytes register */
  ((DMA_CH_Type*)DMA_CH_I2C_RX)->CNDTR = NumByteToRead;
  
  /* DMA_CH_UART_TX enable */
  ((DMA_CH_Type*)DMA_CH_I2C_RX)->CCR_b.EN = SET;
  
  I2C_BeginTransaction((I2C_Type*)BLUE_MIC1_I2C, &t);
  
  while(i2c_eot==RESET);
  
  /* Get data from RX FIFO */
  for(uint8_t i=0;i<NumByteToRead;i++) 
  {
    pBuffer[i] = i2c_buffer_rx[i];
  }
}

/**
 * @brief  I2C DMA ISR callback function
 * @param  None
 * @retval None
 */
void DmaI2CHandlerCallback(void)
{
  /* Check DMA_CH_I2C_RX_IT_TC */
  if(DMA_GetFlagStatus(DMA_CH_I2C_RX_IT_TC)) 
  {
    /* Clear pending bit */
    DMA_ClearFlag(DMA_CH_I2C_RX_IT_TC);
        
    /* DMA_I2C_TX/RX disable */
    ((DMA_CH_Type*)DMA_CH_I2C_RX)->CCR_b.EN = RESET;

    /* Set the i2c_eot flag */
    i2c_eot = SET;
  }

  /* Check DMA_CH_I2C_TX_IT_TC */
  if(DMA_GetFlagStatus(DMA_CH_I2C_TX_IT_TC)) 
  {
    /* Clear pending bit */
    DMA_ClearFlag(DMA_CH_I2C_TX_IT_TC);
        
    /* DMA_I2C_TX/RX disable */
    ((DMA_CH_Type*)DMA_CH_I2C_TX)->CCR_b.EN = RESET;

    /* Set the i2c_eot flag */
    i2c_eot = SET;
  }
}

/**
 * @brief  Configures I2C interface
 * @param  baudrate I2C clock frequency
 * @retval None
 */
void StevalBlueMic1_I2CIrqInit(uint32_t baudrate)
{   
  GPIO_InitType GPIO_InitStructure;
  I2C_InitType I2C_InitStruct;
  NVIC_InitType NVIC_InitStructure;
    
  /* Enable I2C and GPIO clocks */
  if(BLUE_MIC1_I2C == I2C2) 
  {
    SysCtrl_PeripheralClockCmd(CLOCK_PERIPH_GPIO | CLOCK_PERIPH_I2C2, ENABLE);
  }
  else 
  {
    SysCtrl_PeripheralClockCmd(CLOCK_PERIPH_GPIO | CLOCK_PERIPH_I2C1, ENABLE);
  }
      
  /* Configure I2C pins */
  GPIO_InitStructure.GPIO_Pin = BLUE_MIC1_I2C_CLK_PIN ;
  GPIO_InitStructure.GPIO_Mode = BLUE_MIC1_I2C_CLK_MODE;
  GPIO_InitStructure.GPIO_Pull = DISABLE;
  GPIO_InitStructure.GPIO_HighPwr = DISABLE;
  GPIO_Init(&GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = BLUE_MIC1_I2C_DATA_PIN;
  GPIO_InitStructure.GPIO_Mode = BLUE_MIC1_I2C_DATA_MODE;  
  GPIO_Init(&GPIO_InitStructure);
      
  /* Configure I2C in master mode */
  I2C_StructInit(&I2C_InitStruct);
  I2C_InitStruct.I2C_OperatingMode = I2C_OperatingMode_Master;
  I2C_InitStruct.I2C_ClockSpeed = baudrate;
  I2C_Init((I2C_Type*)BLUE_MIC1_I2C, &I2C_InitStruct);
  
  /* Configure the interrupt source */
  I2C_ITConfig((I2C_Type*)BLUE_MIC1_I2C, I2C_IT_MTDWS , ENABLE);
  
  /* Enable the DMA Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = I2C2_IRQn;             
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = MED_PRIORITY;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Clear all I2C pending interrupts */
  I2C_ClearITPendingBit((I2C_Type*)BLUE_MIC1_I2C, I2C_IT_MSK);
}

/**
 * @brief  I2C function to write registers of a slave device
 * @param  pBuffer buffer contains data to write
 * @param  DeviceAddr the I2C slave address
 * @param  RegisterAddr register address
 * @param  NumByteToRead number of byte to write
 * @retval None
 */
void StevalBlueMic1_I2CIrqWrite(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr, uint8_t NumByteToWrite)
{
  I2C_TransactionType t;
  
  /* Write the slave address */
  t.Operation = I2C_Operation_Write;
  t.Address = DeviceAddr;
  t.StartByte = I2C_StartByte_Disable;
  t.AddressType = I2C_AddressType_7Bit;
  t.StopCondition = I2C_StopCondition_Enable;
  t.Length = 1+NumByteToWrite;
  
  /* Flush the slave address */
  I2C_FlushTx((I2C_Type*)BLUE_MIC1_I2C);
  while (I2C_WaitFlushTx((I2C_Type*)BLUE_MIC1_I2C) == I2C_OP_ONGOING);
  
  /* Begin transaction */
  i2c_eot = RESET;
  I2C_BeginTransaction((I2C_Type*)BLUE_MIC1_I2C, &t);

  /* Fill TX FIFO with data to write */
  I2C_FillTxFIFO((I2C_Type*)BLUE_MIC1_I2C, RegisterAddr);

  for(uint8_t i=0; i<NumByteToWrite;i++) 
  {
    I2C_FillTxFIFO((I2C_Type*)BLUE_MIC1_I2C, pBuffer[i]);
  }
  
  /* Wait loop */
  while(i2c_eot == RESET);
}

/**
 * @brief  I2C function to read registers from a slave device
 * @param  pBuffer buffer to retrieve data from a slave
 * @param  DeviceAddr the I2C slave address
 * @param  RegisterAddr register address
 * @param  NumByteToRead number of byte to read
 * @retval None
 */
void StevalBlueMic1_I2CIrqRead(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr, uint8_t NumByteToRead)
{
  I2C_TransactionType t;
  
  /* Write the slave address */
  t.Operation = I2C_Operation_Write;
  t.Address = DeviceAddr;
  t.StartByte = I2C_StartByte_Disable;
  t.AddressType = I2C_AddressType_7Bit;
  t.StopCondition = I2C_StopCondition_Disable;
  t.Length = 1;  
  
  /* Flush the slave address */
  I2C_FlushTx((I2C_Type*)BLUE_MIC1_I2C);
  while (I2C_WaitFlushTx((I2C_Type*)BLUE_MIC1_I2C) == I2C_OP_ONGOING);
  
  /* Begin transaction */
  i2c_eot = RESET;  
  I2C_BeginTransaction((I2C_Type*)BLUE_MIC1_I2C, &t);

  /* Fill TX FIFO with data to write */
  I2C_FillTxFIFO((I2C_Type*)BLUE_MIC1_I2C, RegisterAddr);
  
  /* Wait loop */
  while(i2c_eot == RESET);
  
  /* read data */
  t.Operation = I2C_Operation_Read;
  t.Address = DeviceAddr;
  t.StartByte = I2C_StartByte_Disable;
  t.AddressType = I2C_AddressType_7Bit;
  t.StopCondition = I2C_StopCondition_Enable;
  t.Length = NumByteToRead; 
  
  /* Begin transaction */
  i2c_eot = RESET;  
  I2C_BeginTransaction((I2C_Type*)BLUE_MIC1_I2C, &t);
  
  /* Wait loop */
  while(i2c_eot == RESET);
  
  /* Get data from RX FIFO */
  while(NumByteToRead--) 
  {
    *pBuffer = I2C_ReceiveData((I2C_Type*)BLUE_MIC1_I2C);
    pBuffer ++;
  }
}

/**
 * @brief  I2C DMA ISR callback function
 * @param  None
 * @retval None
 */
void IrqI2CHandlerCallback(void)
{
  /* Check DMA_CH_I2C_RX_IT_TC */
  if(I2C_GetITStatus((I2C_Type*)BLUE_MIC1_I2C, I2C_IT_MTDWS))
  {
    I2C_ClearITPendingBit((I2C_Type*)BLUE_MIC1_I2C, I2C_IT_MTD | I2C_IT_MTDWS);
    
    /* Set the i2c_eot flag */
    i2c_eot = SET;    
  }
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
