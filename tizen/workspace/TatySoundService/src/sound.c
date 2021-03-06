/*
 * sound.c
 *
 *  Created on: May 27, 2019
 *      Author: maartenweyn
 */

#include "sound.h"
#include "tatysoundservice.h"

#include <stdlib.h>
#include <recorder.h>
#include <Ecore.h>
#include <app_event.h>



static audio_in_h input;
static sound_stream_info_h g_stream_info_h = NULL;
static void *g_buffer = NULL;  /* Buffer used for audio recording/playback */
static int g_buffer_size;  /* Size of the buffer used for audio recording/playback */

static double cumulativeSoundLevel = 0.0;
static int cumulativeSoundCounter = 0;
static double start_ts = 0.0;

static double last_day_ts[24*60] = {0.0, };
static double last_day_SoundLevel[24*60] = {0.0, };
static int last_day_index = 0;


static const double AFILTER_Acoef[] = {1.0, -4.0195761811158315, 6.1894064429206921, -4.4531989035441155,
                    1.4208429496218764, -0.14182547383030436,
                    0.0043511772334950787};
static const double AFILTER_Bcoef[] = {0.2557411252042574, -0.51148225040851436,
                    -0.25574112520425807, 1.0229645008170318,
                    -0.25574112520425918, -0.51148225040851414,
                    0.25574112520425729};

static double AFILTER_conditions[] = {0, 0, 0, 0, 0, 0};

static void synchronous_recording(void *data, Ecore_Thread *thread);
static void synchronous_recording_ended(void *data, Ecore_Thread *thread);

/// general

void sound_init() {

}

/// DB filter

double a_filter(double input) {
	double output = input * AFILTER_Bcoef[0] + AFILTER_conditions[0];
	for (int j = 0; j < 5; j++) {
		AFILTER_conditions[j] = input * AFILTER_Bcoef[j + 1] - output * AFILTER_Acoef[j + 1] + AFILTER_conditions[j + 1];
	}
	AFILTER_conditions[5] = input * AFILTER_Bcoef[6] - output * AFILTER_Acoef[6];

	return output;
}

double correctdB(double input) {
	double corrected = ((input - CORR_X0) * ((CORR_REF1 - CORR_REF0) / (CORR_X1 - CORR_X0))) + CORR_REF0;
	return corrected;
}

/// AUDIO


static void focus_callback_audioio(sound_stream_info_h stream_info,
                            sound_stream_focus_mask_e focus_mask,
                            sound_stream_focus_state_e focus_state,
                            sound_stream_focus_change_reason_e reason,
                            int sound_behavior,
                            const char *extra_info,
                            void *user_data)
{
	PRINT_MSG("\n*** FOCUS callback is called  ***\n");
	PRINT_MSG(" -changed focus sate(%d)\n", focus_state);
	PRINT_MSG("(0:released, 1:acquired)\n");
	PRINT_MSG(" - extra information\n");
	PRINT_MSG("(%s)\n", extra_info);
	return;
}

bool audio_device_init(void)
{
    int error_code = SOUND_MANAGER_ERROR_NONE;
	static sound_stream_type_e sound_stream_type = SOUND_STREAM_TYPE_MEDIA;

	/* Create the sound stream information handle and registers the focus state changed callback function. */
	error_code = sound_manager_create_stream_information(sound_stream_type, focus_callback_audioio, NULL, &g_stream_info_h);
	if (SOUND_MANAGER_ERROR_NONE != error_code) {
		dlog_print(DLOG_ERROR, LOG_TAG, "sound_manager_create_stream_information() failed! Error code = 0x%x", error_code);
		return false;
	} else {
		dlog_print(DLOG_INFO, LOG_TAG, "sound_manager_create_stream_information() ok!");
	}

	error_code = sound_manager_set_focus_reacquisition(g_stream_info_h, false);
	if (SOUND_MANAGER_ERROR_NONE != error_code) {
		dlog_print(DLOG_ERROR, LOG_TAG, "sound_manager_set_focus_reacquisition() failed! Error code = 0x%x", error_code);
		return false;
	} else {
			dlog_print(DLOG_INFO, LOG_TAG, "sound_manager_set_focus_reacquisition() ok!");
	}

    /* Audio input device initialization. */
	error_code = AUDIO_IO_ERROR_NONE;
    error_code = audio_in_create(SAMPLE_RATE, AUDIO_CHANNEL_MONO, SAMPLE_TYPE, &input);
    if (error_code != AUDIO_IO_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "audio_in_create failed! Error code = 0x%x", error_code);
		return false;
    }

    /*
     * Buffer size for 1 sec is equals to  sample rate * num of channel * num of bytes per sample
     * Buffer size for mentioned RECORDING_SEC
     */
#if	SAMPLE_TYPE == AUDIO_SAMPLE_TYPE_S16_LE
    		g_buffer_size = SAMPLE_RATE * 2 * 1 * RECORDING_SEC;
#else
    		g_buffer_size = SAMPLE_RATE * 1 * 1 * RECORDING_SEC;
#endif

    /* Allocate the memory for the buffer used for recording/playback. */
    g_buffer = malloc(g_buffer_size);

    return true;
}

void measure_sound() {
	//dlog_print(DLOG_DEBUG, LOG_TAG, "%s", __func__);

	ecore_thread_run(synchronous_recording, synchronous_recording_ended, NULL, NULL);
}

/**
 * @brief Records the audio synchronously.
 * @details Executes in a different thread, because it launches synchronous
 *          recording which is blocking.
 * @remarks This function matches the Ecore_Thread_Cb() signature defined in the
 *          Ecore_Common.h header file.
 *
 * @param data    The user data passed via void pointer. This argument is not
 *                used in this case.
 * @param thread  @c NULL if the thread was cancelled, otherwise probably
 *                (missing documentation) a pointer to the new thread that is
 *                executing the blocking function. This argument is not used in
 *                this case.
 */
static void synchronous_recording(void *data, Ecore_Thread *thread)
{
	//dlog_print(DLOG_DEBUG, LOG_TAG, "%s", __func__);

    int read_length;
    //int loop_count ;
    int bytes_read;
    char *buffer_ptr = NULL;
    int error_code = SOUND_MANAGER_ERROR_NONE;

    /*
     * Buffer size for 1 sec is equals to  sample rate * num of channel * num of bytes per sample
     * Buffer size for mentioned RECORDING_SEC
     */
#if	SAMPLE_TYPE == AUDIO_SAMPLE_TYPE_S16_LE
    		g_buffer_size = SAMPLE_RATE * 2 * 1 * RECORDING_SEC;
#else
    		g_buffer_size = SAMPLE_RATE * 1 * 1 * RECORDING_SEC;
#endif

    error_code = sound_manager_acquire_focus(g_stream_info_h, SOUND_STREAM_FOCUS_FOR_RECORDING, SOUND_BEHAVIOR_NONE, NULL);
    if (SOUND_MANAGER_ERROR_NONE != error_code)
        dlog_print(DLOG_ERROR, LOG_TAG, "sound_manager_acquire_focus() failed! Error code = 0x%x", error_code);
    CHECK_ERROR_AND_RETURN("sound_manager_acquire_focus", error_code);

    /* Prepare audio input (starts the hardware recording process) */
    error_code = audio_in_prepare(input);
    CHECK_ERROR_AND_RETURN("audio_in_prepare", error_code);

    /* Record in small chunks. */
    //num_of_iterations = RECORDING_SEC / MIN_RECORDING_INTERVAL;
     //MIN_BUFFER_SIZE;
#if	SAMPLE_TYPE == AUDIO_SAMPLE_TYPE_S16_LE
    		read_length = 2 * SAMPLE_RATE * RECORDING_SEC;
#else
    		read_length = 1 * SAMPLE_RATE * RECORDING_SEC;
#endif

    buffer_ptr = (char *)g_buffer;

    //dlog_print(DLOG_INFO, LOG_TAG, "num_of_iterations %d", num_of_iterations);
    //dlog_print(DLOG_INFO, LOG_TAG, "read_length %d", read_length);

    if (start_ts == 0.0)
    		start_ts = ecore_time_unix_get();

    	bytes_read = audio_in_read(input, buffer_ptr, read_length);
	if (bytes_read < 0) {
		DLOG_PRINT_ERROR("audio_in_read", bytes_read);
	} else {
		dlog_print(DLOG_INFO, LOG_TAG, "%d bytes have been recorded.", bytes_read);
	}

	double sumsquare = 0;

	uint8_t *index = (uint8_t*)g_buffer;
	while (index < (((uint8_t*)g_buffer) + read_length)) {
#if	SAMPLE_TYPE == AUDIO_SAMPLE_TYPE_S16_LE
		int16_t ivalue = *(int16_t*)index;
		double fraction = 0.0;
		if (ivalue < 0)
			fraction = ivalue / 32768.0;
		else
			fraction = ivalue / 32767.0;

#else
		uint8_t value = *(uint8_t*)index;
		int16_t ivalue = value - 128;
		double fraction = 0.0;
		if (ivalue < 0)
			fraction = ivalue / 128.0;
		else
			fraction = ivalue / 127.0;

		//dlog_print(DLOG_DEBUG, LOG_TAG, "value %d -> %d",  value,  ivalue;
#endif
		double filtered = a_filter(fraction);
		sumsquare += filtered * filtered;
		index++;
	}

	double soundLevel = sumsquare / read_length;
	cumulativeSoundLevel += soundLevel;
	cumulativeSoundCounter++;


	double ts = ecore_time_unix_get();
	double Leq = ((10.0 * log10(soundLevel)) + 93.97940008672037609572522210551);
	double correctedLeq = correctdB(Leq);
	//push_current_values(ts, currentLeq, correctedLeq);

	bundle *event_data = NULL;
	event_data = bundle_create();
	int ret = EVENT_ERROR_NONE;

	char rawString[5], leqString[5];
	snprintf(rawString, sizeof(rawString), "%.1f", Leq);
	snprintf(leqString, sizeof(leqString), "%.1f", correctedLeq);
	ret = bundle_add_str(event_data, "leq", leqString);
	ret = bundle_add_str(event_data, "raw", rawString);

	if (ts - start_ts >= MIN_RECORDING_INTERVAL) {
		double avg_sound_level = cumulativeSoundLevel / cumulativeSoundCounter;
		double leq = ((10.0 * log10(avg_sound_level)) + 93.97940008672037609572522210551);
		double corr_avg_leqmin = correctdB(leq);

		char sound_level_min[10], avg_min[5];
		snprintf(avg_min, sizeof(avg_min), "%.1f", corr_avg_leqmin);
		snprintf(sound_level_min, sizeof(sound_level_min), "%.4f", avg_sound_level);
		ret = bundle_add_str(event_data, "soundlevel", sound_level_min);
		ret = bundle_add_str(event_data, "leg_min", avg_min);

		if (last_day_index == 60*24) {
			//rotate
			for (int i = 0; i < last_day_index-1; i++)
			{
				last_day_ts[i] = last_day_ts[i+1];
				last_day_SoundLevel[i] = last_day_SoundLevel[i+1];
			}
			last_day_index--;
		}
		last_day_ts[last_day_index] = start_ts;
		last_day_SoundLevel[last_day_index] = avg_sound_level;

		int last_hour_counter = last_day_index < 60 ? last_day_index : 60;
		double sum_last_hour = 0;
		for (int i = last_day_index - last_hour_counter; i < last_day_index; i++) {
			sum_last_hour += last_day_SoundLevel[i];
		}
		double last_hour = last_hour_counter > 0 ? sum_last_hour/ last_hour_counter : avg_sound_level;
		double leq_last_hour = correctdB(((10.0 * log10(last_hour)) + 93.97940008672037609572522210551));

		int last_8hour_counter = last_day_index < 8 * 60 ? last_day_index : 8 * 60;
		int last_8hour_start = last_8hour_counter < 60 ? last_8hour_counter : 60;
		double sum_last_8hour = sum_last_hour;
		for (int i = last_day_index - last_8hour_counter + last_8hour_start; i < last_day_index; i++) {
			sum_last_8hour += last_day_SoundLevel[i];
		}

		double last_8hour = last_8hour_counter > 0 ? sum_last_8hour / last_8hour_counter : avg_sound_level;
		double leq_last_8hour = correctdB(((10.0 * log10(last_8hour)) + 93.97940008672037609572522210551));

		int last_day_counter = last_day_index < 24 * 60 ? last_day_index : 24 * 60;
		int last_day_start = last_day_counter < 8 * 60 ? last_day_counter : 8 * 60;
		double sum_last_day = sum_last_8hour;
		for (int i = last_day_index - last_day_counter + last_day_start; i < last_day_index; i++) {
			sum_last_day += last_day_SoundLevel[i];
		}
		double last_day = last_day_counter > 0 ? sum_last_day / last_day_counter : avg_sound_level;
		double leq_last_day = correctdB(((10.0 * log10(last_day)) + 93.97940008672037609572522210551));

		char leqString_hour[5], leqString_8hour[5], leqString_day[5];
		snprintf(leqString_hour, sizeof(leqString_hour), "%.1f", leq_last_hour);
		snprintf(leqString_8hour, sizeof(leqString_8hour), "%.1f", leq_last_8hour);
		snprintf(leqString_day, sizeof(leqString_day), "%.1f", leq_last_day);
		ret = bundle_add_str(event_data, "leq_hour", leqString_hour);
		ret = bundle_add_str(event_data, "leq_8hours", leqString_8hour);
		ret = bundle_add_str(event_data, "leq_day", leqString_day);

		cumulativeSoundLevel = 0;
		cumulativeSoundCounter = 0;
		last_day_index++;
		start_ts += MIN_RECORDING_INTERVAL;

		if (push_average_values(ts, last_day_index-1, avg_sound_level, corr_avg_leqmin, leq_last_hour, leq_last_8hour, leq_last_day) == 0)
			bundle_add_str(event_data, "network", "1");
		else
			bundle_add_str(event_data, "network", "0");
	}

	dlog_print(DLOG_INFO, LOG_TAG, "event_publish_app_event");

	ret = event_publish_app_event("event.be.wesdec.tatysoundservice.new_data_event", event_data);
	if (ret != EVENT_ERROR_NONE)
		dlog_print(DLOG_ERROR, LOG_TAG, "event_publish_app_event err: [%d]", ret);

	ret = bundle_free(event_data);

    /* Stop the hardware recording process. */
    error_code = audio_in_unprepare(input);
    CHECK_ERROR("audio_in_unprepare", error_code);

    error_code = sound_manager_release_focus(g_stream_info_h, SOUND_STREAM_FOCUS_FOR_RECORDING, SOUND_BEHAVIOR_NONE, "record(Release)");
	if (SOUND_MANAGER_ERROR_NONE != error_code)
		dlog_print(DLOG_ERROR, LOG_TAG, "sound_manager_release_focus() failed! Error code = 0x%x", error_code);

	return;
}

static void synchronous_recording_ended(void *data, Ecore_Thread *thread)
{
    PRINT_MSG("Synchronous recording ended.");
}

