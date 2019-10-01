/**
  ******************************************************************************
  * @file    OTA_btl.h
  * @author  AMS - VMA RF Application team
  * @version V1.0.0
  * @date    30-November-2015
  * @brief   BlueNRG-1 OTA Bootloader header file
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BTL_H
#define __BTL_H

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/
#ifdef DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/* Exported define -----------------------------------------------------------*/

/* BlueNRG-1 Flash memory layout for OTA */

/** @brief Flash erase basic unit */
#define FLASH_PAGE_SIZE 2048 
#ifndef RESET_MANAGER_FLASH_BASE_ADDRESS 
/** @brief  Reset manager base address */
#define RESET_MANAGER_FLASH_BASE_ADDRESS        (0x10040000) 
#endif 

/** @brief Reset manager size: it defines the lower application base address (and
  * the OTA Service Manger base address) 
  */
#define RESET_MANAGER_FLASH_BASE_OFFSET         (0x800)   

/** @brief OTA Service Manager size: address offset for BLE application using OTA Service Manager */
#define OTA_BASIC_APPLICATION_BASE_OFFSET       (0x10000)   

/** @brief Address offset for BLE application with OTA service based on Higher Flash Location */
#define HIGHER_APPLICATION_BASE_OFFSET          (0x13800)    

/** @brief Numbers of sectors for Lower & Higher Application */
#define OTA_LOWER_HIGHER_APPLICATION_AVAILABLE_SECTORS_NUMBERS   38 /* 38 * 2048 = 76K */

/** @brief OTA Reset manager page numbers start and numbers of pages): don't change them */
#define OTA_RESET_MANAGER_PAGE_NUMBER_START      0
#define OTA_RESET_MANAGER_PAGE_NUMBERS           (RESET_MANAGER_FLASH_BASE_OFFSET/FLASH_PAGE_SIZE)

/** @brief OTA Lower application  page numbers start, end, start address and end boundary: don't change them */
#define OTA_LOWER_APPLICATION_PAGE_NUMBER_START  (OTA_RESET_MANAGER_PAGE_NUMBER_START + OTA_RESET_MANAGER_PAGE_NUMBERS) //1
#define OTA_LOWER_APPLICATION_PAGE_NUMBER_END    (OTA_LOWER_HIGHER_APPLICATION_AVAILABLE_SECTORS_NUMBERS) //38
#define APP_LOWER_ADDRESS                        (RESET_MANAGER_FLASH_BASE_ADDRESS + RESET_MANAGER_FLASH_BASE_OFFSET)
#define APP_LOWER_ADDRESS_END                    APP_LOWER_ADDRESS + (OTA_LOWER_HIGHER_APPLICATION_AVAILABLE_SECTORS_NUMBERS*FLASH_PAGE_SIZE) 

/** @brief OTA Higher application  page numbers start, end, start address and end boundary: don't change them */
#define OTA_HIGHER_APPLICATION_PAGE_NUMBER_START (OTA_LOWER_APPLICATION_PAGE_NUMBER_END + 1) //39: 0x13800 Higher application starts at sector 39 and ends at sector 76 
#define OTA_HIGHER_APPLICATION_PAGE_NUMBER_END   (OTA_LOWER_HIGHER_APPLICATION_AVAILABLE_SECTORS_NUMBERS*2) //76  
#define APP_HIGHER_ADDRESS                       (RESET_MANAGER_FLASH_BASE_ADDRESS + HIGHER_APPLICATION_BASE_OFFSET) 
#define APP_HIGHER_ADDRESS_END                   APP_HIGHER_ADDRESS + (((OTA_HIGHER_APPLICATION_PAGE_NUMBER_END - OTA_HIGHER_APPLICATION_PAGE_NUMBER_START)+1)*FLASH_PAGE_SIZE) 

/** @brief OTA Service Manager application address,page numbers start, end: don't change them */
#define APP_OTA_SERVICE_ADDRESS                  (RESET_MANAGER_FLASH_BASE_ADDRESS) 
#define APP_WITH_OTA_SERVICE_PAGE_NUMBER_START   (OTA_BASIC_APPLICATION_BASE_OFFSET)/FLASH_PAGE_SIZE //29
#define APP_WITH_OTA_SERVICE_PAGE_NUMBER_END     (OTA_HIGHER_APPLICATION_PAGE_NUMBER_END + 1) //77


/** @brief OTA application with OTA Service manager address: don't change them */
#define APP_WITH_OTA_SERVICE_ADDRESS             (RESET_MANAGER_FLASH_BASE_ADDRESS + OTA_BASIC_APPLICATION_BASE_OFFSET)

/** @brief  OTA application validity tags: don't change them.
  */
#define OTA_NO_OPERATION                                 (0x11)
#define OTA_APP_SWITCH_OP_CODE_NO_OPERATION              (0xb0014211)
#define OTA_APP_SWITCH_OP_CODE_GO_TO_LOWER_APP           (OTA_APP_SWITCH_OP_CODE_NO_OPERATION + (OTA_NO_OPERATION*2)) //0xb0014233
#define OTA_APP_SWITCH_OP_CODE_GO_TO_HIGHER_APP          (OTA_APP_SWITCH_OP_CODE_NO_OPERATION + (OTA_NO_OPERATION*3)) //0xb0014244
#define OTA_APP_SWITCH_OP_CODE_GO_TO_OTA_SERVICE_MANAGER (OTA_APP_SWITCH_OP_CODE_NO_OPERATION + (OTA_NO_OPERATION*4)) 
#define OTA_APP_SWITCH_OP_CODE_GO_TO_NEW_APP             (OTA_APP_SWITCH_OP_CODE_NO_OPERATION + (OTA_NO_OPERATION*5)) 

/** @brief  Compiler/Linker options to be used on project preprocessor options based on 
            specific application scenario: don't change them */

/** @brief Options for Higher Application with OTA:
  *        Compiler option: ST_OTA_HIGHER_APPLICATION=1
  *        Linker Option:   ST_OTA_HIGHER_APPLICATION=1
  */
#if ST_OTA_HIGHER_APPLICATION /* linker option */
    /* BLE Application with OTA Service, based at higher FLASH address */

    #define ST_OTA_FIRMWARE_UPGRADE_SUPPORT 1 
    #define APPLICATION_BASE_ADDRESS APP_HIGHER_ADDRESS

    /** @brief  Free space x lower application starts from  Reset Manager size: 0x800 (sector 1) to 0x100537ff (end of sector 38) --> 38 sectors: 76KB */
    #define OTA_FREE_SPACE_RANGE {0x10,0x04,0x08,0x00,0x10,0x05,0x37,0xff}  //Flash start: 0x10040800; Flash end: 0x100537ff 

    #define OTA_OP_CODE OTA_APP_SWITCH_OP_CODE_GO_TO_LOWER_APP // Go to Lower App when OTA completes with success

/** @brief Options for Lower Application with OTA:
  *        Compiler option: ST_OTA_LOWER_APPLICATION=1
  *        Linker Option:   ST_OTA_LOWER_APPLICATION=1
  */
#elif ST_OTA_LOWER_APPLICATION  /* linker option */
    /* BLE Application with OTA Service, based at lower FLASH address */

    #define ST_OTA_FIRMWARE_UPGRADE_SUPPORT 1
    #define APPLICATION_BASE_ADDRESS APP_LOWER_ADDRESS

    /** @brief  Free space for higher application starts from  Higher Application default base: 0x0x10053800 (sector 39) to 0x100667ff (end of sector 76) --> 38 sectors: 76KB */
    #define OTA_FREE_SPACE_RANGE  {0x10,0x05,0x38,0x00,0x10,0x06,0x67,0xFF} //Flash start: 0x10053800; Flash end: 0x100667ff

    #define OTA_OP_CODE OTA_APP_SWITCH_OP_CODE_GO_TO_HIGHER_APP// Go to Higher App when OTA completes with success   

/** @brief Options for OTA Service Manager:
  *        Compiler option: ST_OTA_SERVICE_MANAGER_APPLICATION=1
*        Linker option: MEMORY_FLASH_APP_SIZE=0xF800
  */
#elif ST_OTA_SERVICE_MANAGER_APPLICATION
    /* OTA Service manager, based at  FLASH base address */
    #define APPLICATION_BASE_ADDRESS APP_OTA_SERVICE_ADDRESS

    /** @brief  Free space x application starts from  OTA_ServiceManager_Size:  0x10050000 (sector 32)  to 0x10066FFF (end of sector 77) --> 46 sectors: 92KB */  
    #define OTA_FREE_SPACE_RANGE  {0x10,0x05,0x00,0x00,0x10,0x06,0x6F,0xFF}; //Flash start: 0x10050000; Flash end: 0x10066FFF; 
   
    #define OTA_OP_CODE OTA_APP_SWITCH_OP_CODE_GO_TO_NEW_APP 

/** @brief Options for Application which can use OTA Service Manager:
  *        Compiler option: ST_USE_OTA_SERVICE_MANAGER_APPLICATION=1
  *        Linker Option:   ST_USE_OTA_SERVICE_MANAGER_APPLICATION=1 
  */
#elif ST_USE_OTA_SERVICE_MANAGER_APPLICATION /* linker option */
    /* BLE Application (with no OTA service) which uses the OTA Service Manager */
    /* BLE application is based at new higher FLASH address */
    #define APPLICATION_BASE_ADDRESS APP_WITH_OTA_SERVICE_ADDRESS
    
    #define OTA_FREE_SPACE_RANGE {0,0,0,0,0,0} //Not used in this context TBR

    #define OTA_OP_CODE OTA_APP_SWITCH_OP_CODE_GO_TO_OTA_SERVICE_MANAGER //Not used in this context TBR

#else 
/* Nothing to do: no OTA Service is supported; No OTA Service Manager is used */
#endif 

/* ************************************************************************************************ */

/** @brief OTA Service Manager defines values magic location in RAM: don't change it */
#define OTA_SERVICE_MANAGER_RAM_LOCATION (0x20000004)   
/** @brief OTA Service Manager utility to set the magic location value: don't change it */
#define OTA_SET_SERVICE_MANAGER_RAM_LOCATION (*(uint32_t *)(OTA_SERVICE_MANAGER_RAM_LOCATION))

/** @brief  Vector table entry used to register OTA application validity tag*/
#define OTA_TAG_VECTOR_TABLE_ENTRY_INDEX  (4)
/** @brief  Address offset for vector table entry used to register OTA application validity tag */
#define OTA_TAG_VECTOR_TABLE_ENTRY_OFFSET (OTA_TAG_VECTOR_TABLE_ENTRY_INDEX * 4)

/** @brief Application flag values to register application
  * validity on OTA_TAG_VECTOR_TABLE_ENTRY_INDEX as consequence of an OTA upgrade:
  */

/** @brief OTA in progress tag: it is sets on vector table during OTA upgrade */
#define OTA_IN_PROGRESS_TAG      (0xFFFFFFFF)

/** @brief  OTA  invalid, old tag: it tags old application as invalid/old 
  * (OTA upgrade done and jump to new application) 
  */
#define OTA_INVALID_OLD_TAG      (0x00000000)

/** @brief  OTA valid tag: it tags new application as valid 
  * (It is done after a OTA upgrade process is completed with success:  
  * as consequence of a SW reset to OTA Reset Manager) */
#define OTA_VALID_TAG            (0xAA5555AA)

/** @brief  OTA Service Manager valid tag: It tags OTA Service manager as valid
  */
#define OTA_SERVICE_MANAGER_TAG  (0xAABBCCDD)

/* Exported Variables  --------------------------------------------------------*/
/** @brief OTA Service UUID */
extern uint8_t BTLServiceUUID4Scan[];

/* Exported Functions  --------------------------------------------------------*/

/** 
 * @brief This function handles the OTA bootloader updgrade. 
 * It is called on the aci_gatt_attribute_modified_event() callback context for handling the
 * the specific characteristic wirte coming from the OTA Client.
 * 
 * @param attr_handle Handle of the OTA attribute that was modified.
 * @param data_length Length of att_data in octets
 * @param att_data    The modified value
 *
 * @retval None
 */
void OTA_Write_Request_CB(uint16_t attr_handle,
                          uint8_t data_length,
                          uint8_t *att_data);

/**
 * @brief  Add the 'OTABootloader' service.
 * @param  None.
 * @retval Value indicating success or error code.
 *
 * @note The API code could be subject to change in future releases.
 */
uint8_t OTA_Add_Btl_Service(void);

/**
 * @brief  It returns the OTA upgrade fw status
 * @param  None
 * @retval 1 if OTA upgrade session has been completed; 0 otherwise
 *
 * @note The API code could be subject to change in future releases.
 */
uint8_t OTA_Tick(void);

/**
 * @brief  It jumps to the new upgraded application
 * @param  None
 * @retval None
 *
 * @note The API code could be subject to change in future releases.
 */
void OTA_Jump_To_New_Application(void);

/**
 * @brief  It jumps to the OTA Service Manager application
 * @param  None
 * @retval None
 *
 * @note The API code could be subject to change in future releases.
 */
void OTA_Jump_To_Service_Manager_Application(void);

#endif /* __BTL_H */

/******************* (C) COPYRIGHT 2015 STMicroelectronics *****END OF FILE****/
