/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/**
 * @defgroup DRV_AUDIO Audio driver
 * @{
 * @ingroup MOD_AUDIO
 * @brief Audio top level driver
 *
 * @details
 */
#ifndef __DRV_AUDIO_H__
#define __DRV_AUDIO_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdbool.h>

#include "app_scheduler.h"
#include "app_error.h"

#include "config.h"

typedef struct
{
    uint8_t buffer[CONFIG_AUDIO_FRAME_SIZE_BYTES];
    int     buffer_size;
    bool    buffer_free_flag;
} drv_audio_frame_t;

/**@brief Enable audio transmission.
 *
 * @return
 * @retval NRF_SUCCESS
 * @retval NRF_ERROR_INTERNAL
 */
uint32_t drv_audio_transmission_enable(void);

/**@brief Disable audio transmission.
 *
 * @return
 * @retval NRF_SUCCESS
 * @retval NRF_ERROR_INTERNAL
 */
uint32_t drv_audio_transmission_disable(void);

/**@brief Initialization.
 *
 * @param[in] adpcm_frame_handler callback function to be scheduled when a new audio_adpcm frame is ready.
 * @return
 * @retval NRF_SUCCESS
 * @retval NRF_ERROR_INVALID_PARAM
 * @retval NRF_ERROR_INTERNAL
 */
uint32_t drv_audio_init(app_sched_event_handler_t  adpcm_frame_handler);

#endif /* __DRV_AUDIO_H__ */
/** @} */
