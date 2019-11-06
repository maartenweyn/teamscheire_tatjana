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
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "nrf_assert.h"
#include "nrf_error.h"
#include "nrf_gpio.h"
#include "nrf_sdm.h"

#include "drv_audio.h"
#include "nrf_drv_pdm.h"
#include "config.h"

//#ifdef CONFIG_AUDIO_ENABLED

static volatile drv_audio_frame_t       m_audio_frame[CONFIG_AUDIO_FRAME_BUFFERS];
static int16_t                          m_pdm_buff[2][CONFIG_AUDIO_FRAME_SIZE_SAMPLES];

uint16_t				m_frame_index = 0;
uint16_t                          	m_next_frame_index = 0;
static app_sched_event_handler_t        m_audio_frame_handler = NULL;


/**@brief Search for free frame index, start from value stored in m_next_frame_index
 *
 * @retval - valid number is 0..CONFIG_AUDIO_FRAME_BUFFERS-1, CONFIG_AUDIO_FRAME_BUFFERS indicates, that there is no free buffer available
 */
static uint16_t drv_audio_find_free_frame_index(void)
{
    uint16_t frame_index;
    uint16_t i;

    frame_index = m_next_frame_index;
    for (i = 0; i < CONFIG_AUDIO_FRAME_BUFFERS; i++)
    {
        if (m_audio_frame[frame_index].buffer_free_flag)
        {
            return frame_index;
        }
        frame_index = (frame_index + 1) % CONFIG_AUDIO_FRAME_BUFFERS;
    }
    return CONFIG_AUDIO_FRAME_BUFFERS;
}

/**@brief Called from scheduler context when audio samples are ready for compression.
 *
 * @details Routine process and compresses the audio sample buffer.
 *
 * @param[in] p_event_data : Used to get index to audio buffer ready for compression.
 *
 * @param[in] event_size : Not used.
 */
static void drv_audio_buffer_handler(void * p_event_data, uint16_t event_size)
{
  drv_audio_frame_t *p_frame;
  uint16_t frame_index;
  int16_t *p_buffer;

  ASSERT(event_size == sizeof(p_buffer));
  p_buffer = *(void **)(p_event_data);

  //nrf_gpio_pin_toggle(DBG_PDM_AUDIO_BUFFER);
		
  frame_index = drv_audio_find_free_frame_index();
  m_frame_index = frame_index;

  if (frame_index != CONFIG_AUDIO_FRAME_BUFFERS)
  {
    p_frame = (drv_audio_frame_t *)&m_audio_frame[frame_index];
    p_frame->buffer_free_flag = false;

    p_frame->buffer_size = CONFIG_AUDIO_FRAME_SIZE_BYTES;

    memcpy(p_frame->buffer, p_buffer, p_frame->buffer_size);

    // Make subsequent index preferred for next frame.
    m_next_frame_index = (frame_index + 1) % CONFIG_AUDIO_FRAME_BUFFERS;

    app_sched_event_put(&p_frame, sizeof(drv_audio_frame_t *), m_audio_frame_handler);
     //DBG_PIO_CLEAR(CONFIG_IO_DBG_PCM);
  }
  else
  {
      // Invalid index means all buffers are claimed and not released.
      // Don't clear CONFIG_IO_DBG_PCM - this will make debugging pulse wider than expected and easy to spot on the logic analyzer.
  #if (DBG_FREE_FRAME_ASSERTS)
      APP_ERROR_CHECK_BOOL(false);
  #endif
  }
}

uint32_t drv_audio_transmission_enable(void)
{
  unsigned int i;
  m_next_frame_index = 0;
  for (i = 0; i < CONFIG_AUDIO_FRAME_BUFFERS; i++ )
  {
    m_audio_frame[i].buffer_free_flag = true;
  }
  return nrf_drv_pdm_start();
}

uint32_t drv_audio_transmission_disable(void)
{
    return nrf_drv_pdm_stop();
}

static void drv_audio_pdm_event_handler(uint32_t *p_buffer, uint16_t length)
{
	  //uint16_t frame_index;
	
    ASSERT(length == CONFIG_AUDIO_FRAME_SIZE_SAMPLES);
	
		//nrf_gpio_pin_toggle(DBG_PDM_HANDLER_PIN);

#if 0//CONFIG_DEBUG_ENABLED
	
    if ((int16_t *)p_buffer == m_pdm_buff[0])
    {
        nrf_gpio_pin_set(DBG_PDM_HANDLER_PIN);
    }
    else if ((int16_t *)p_buffer == m_pdm_buff[1])
    {
        nrf_gpio_pin_clear(DBG_PDM_HANDLER_PIN);
    }
    else
    {
        APP_ERROR_CHECK_BOOL(false);  // assert error for unsupported events
    }
#endif /* CONFIG_DEBUG_ENABLED */
	
   APP_ERROR_CHECK(app_sched_event_put(&p_buffer, sizeof(p_buffer), drv_audio_buffer_handler));

}

uint32_t audio_init(app_sched_event_handler_t audio_frame_handler)
{
    nrfx_pdm_config_t pdm_cfg = NRFX_PDM_DEFAULT_CONFIG(CONFIG_IO_PDM_CLK, CONFIG_IO_PDM_DATA);


    return nrf_drv_pdm_init(&pdm_cfg, drv_audio_pdm_event_handler);
}

//#endif /* CONFIG_AUDIO_ENABLED */
