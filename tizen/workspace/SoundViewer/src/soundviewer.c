#include "soundviewer.h"
#include <math.h>
#include <privacy_privilege_manager.h>

#include <recorder.h>
#include <storage.h>

// Define the sample rate for recording audio
#define SAMPLE_RATE 44100
#define SAMPLE_TYPE AUDIO_SAMPLE_TYPE_U8
#define RECORDING_SEC 10
#define MIN_RECORDING_INTERVAL  0.1
#define MIN_BUFFER_SIZE 19200
#define BUFLEN 512
#define MAX_2BYTES_SIGNED 32767
#define MIN_2BYTES_SIGNED (-32768)

static FILE *g_fp_w = NULL;  /* File used for asynchronous audio recording */
static char *g_sounds_directory = NULL;
static char g_audio_io_file_path[BUFLEN];
// Initialize the audio input device
static audio_in_h input;
// Initialize the audio output device
static audio_out_h output;
static sound_stream_info_h g_stream_info_h = NULL;

static bool recording = false;
static bool data_initialized = false;
static bool requesting_permission = false;

static void *g_buffer = NULL;  /* Buffer used for audio recording/playback */
static int g_buffer_size;  /* Size of the buffer used for audio recording/playback */


static Evas_Object *button;
static Evas_Object *volume;
typedef struct appdata {
	Evas_Object *win;
	Evas_Object *conform;
	Evas_Object *label;
	Evas_Object *box;
	Evas_Object *nf;
} appdata_s;



//static recorder_h g_recorder;
static void __focus_callback_audioio(sound_stream_info_h stream_info,
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

/*
 * @brief Gets the ID of the internal storage.
 * @details It assigns the ID of the internal storage to the variable passed
 *          as the user data to the callback.
 *          This callback is called for every storage supported by the device.
 * @remarks This function matches the storage_device_supported_cb() signature
 *          defined in the storage-expand.h header file.
 *
 * @param storage_id  The unique ID of the detected storage
 * @param type        The type of the detected storage
 * @param state       The current state of the detected storage.
 *                    This argument is not used in this case.
 * @param path        The absolute path to the root directory of the detected
 *                    storage. This argument is not used in this case.
 * @param user_data   The user data passed via void pointer
 *
 * @return @c true to continue iterating over supported storages, @c false to
 *         stop the iteration.
 */
static bool _storage_cb(int storage_id, storage_type_e type,
                        storage_state_e state, const char *path,
                        void *user_data)
{
    if (STORAGE_TYPE_INTERNAL == type) {
        int *internal_storage_id = (int *) user_data;

        if (NULL != internal_storage_id)
            *internal_storage_id = storage_id;

        /* Internal storage found, stop the iteration. */
        return false;
    } else {
        /* Continue iterating over storages. */
        return true;
    }
}

/**
 * @brief Prepare path for the file.
 */
static void __prepare_path_to_file(void)
{
    int error_code = AUDIO_IO_ERROR_NONE;
    int internal_storage_id;
    static const char *g_audio_io_file_name = "pcm_w.raw";

    /* Get the path to the Sounds directory: */

    /* 1. Get internal storage id. */
    internal_storage_id = -1;

    error_code = storage_foreach_device_supported(_storage_cb,
            &internal_storage_id);
    CHECK_ERROR_AND_RETURN("storage_foreach_device_supported", error_code);

    /* 2. Get the path to the Sounds directory. */
    error_code = storage_get_directory(internal_storage_id,
            STORAGE_DIRECTORY_SOUNDS, &g_sounds_directory);
    CHECK_ERROR_AND_RETURN("storage_get_directory", error_code);

    /* Prepare a path to the file used for asynchronous playback. */
    snprintf(g_audio_io_file_path, BUFLEN, "%s/%s", g_sounds_directory,
            g_audio_io_file_name);

    dlog_print(DLOG_INFO, LOG_TAG, "g_audio_io_file_path %s", g_audio_io_file_path);
}

static void __audio_device_init(void)
{
    int error_code = AUDIO_IO_ERROR_NONE;

    /* Audio input device initialization. */
    error_code = audio_in_create(SAMPLE_RATE, AUDIO_CHANNEL_MONO, SAMPLE_TYPE, &input);
    CHECK_ERROR_AND_RETURN("audio_in_create", error_code);

//    /* Audio output device initialization. */
    error_code = audio_out_create_new(SAMPLE_RATE, AUDIO_CHANNEL_MONO,
            AUDIO_SAMPLE_TYPE_S16_LE, &output);
    CHECK_ERROR_AND_RETURN("audio_out_create", error_code);

    /*
     * Buffer size for 1 sec is equals to  sample rate * num of channel * num of bytes per sample
     * Buffer size for mentioned RECORDING_SEC
     */
    g_buffer_size = SAMPLE_RATE * 2 * 2 * RECORDING_SEC;

    /* Allocate the memory for the buffer used for recording/playback. */
    g_buffer = malloc(g_buffer_size);
}

void data_initialize(void)
{
    int error_code = SOUND_MANAGER_ERROR_NONE;
    static sound_stream_type_e sound_stream_type = SOUND_STREAM_TYPE_MEDIA;
    /*
     * If you need to initialize application data,
     * please use this function.
     */

    /* Create the sound stream information handle and registers the focus state changed callback function. */
    error_code = sound_manager_create_stream_information(sound_stream_type, __focus_callback_audioio, NULL, &g_stream_info_h);
    if (SOUND_MANAGER_ERROR_NONE != error_code)
        dlog_print(DLOG_ERROR, LOG_TAG, "sound_manager_create_stream_information() failed! Error code = 0x%x", error_code);
    else
    		dlog_print(DLOG_INFO, LOG_TAG, "sound_manager_create_stream_information() ok!");

    error_code = sound_manager_set_focus_reacquisition(g_stream_info_h, false);
    if (SOUND_MANAGER_ERROR_NONE != error_code)
        dlog_print(DLOG_ERROR, LOG_TAG, "sound_manager_set_focus_reacquisition() failed! Error code = 0x%x", error_code);
    else
    		dlog_print(DLOG_INFO, LOG_TAG, "sound_manager_set_focus_reacquisition() ok!");
    //g_current_state = NONE;

    /* Initialize audio device input and output handles. */
    __audio_device_init();

    /* Prepare path to file. */
    __prepare_path_to_file();

    data_initialized = true;
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

	//return;
	//elm_object_text_set(button, "recording");
    int read_length;
    //int loop_count ;
    int bytes_read;
    char *buffer_ptr = NULL;
    int error_code = SOUND_MANAGER_ERROR_NONE;

    /*
     * Buffer size for 1 sec is equals to  sample rate * num of channel * num of bytes per sample
     * Buffer size for mentioned RECORDING_SEC
     */
    g_buffer_size = SAMPLE_RATE * 2 * 2 * RECORDING_SEC;
    error_code = sound_manager_acquire_focus(g_stream_info_h, SOUND_STREAM_FOCUS_FOR_RECORDING,
                                             SOUND_BEHAVIOR_NONE, NULL);
    if (SOUND_MANAGER_ERROR_NONE != error_code)
        dlog_print(DLOG_ERROR, LOG_TAG, "sound_manager_acquire_focus() failed! Error code = 0x%x", error_code);
    CHECK_ERROR_AND_RETURN("sound_manager_acquire_focus", error_code);

    //g_current_state = SYNC_RECORD;

    /* Open a file, where the recorded data will be stored. */
    g_fp_w = fopen(g_audio_io_file_path, "w");

	if (g_fp_w) {
		PRINT_MSG("Recording stored in %s file.", g_audio_io_file_path);
	} else {
        dlog_print(DLOG_ERROR, LOG_TAG, "fopen error ");
		return;
	}

    /* Prepare audio input (starts the hardware recording process) */
    error_code = audio_in_prepare(input);
    CHECK_ERROR_AND_RETURN("audio_in_prepare", error_code);

    /* Record in small chunks. */
    //num_of_iterations = RECORDING_SEC / MIN_RECORDING_INTERVAL;
    read_length = 2 * SAMPLE_RATE * MIN_RECORDING_INTERVAL; //MIN_BUFFER_SIZE;
    buffer_ptr = (char *)g_buffer;

    //dlog_print(DLOG_INFO, LOG_TAG, "num_of_iterations %d", num_of_iterations);
    dlog_print(DLOG_INFO, LOG_TAG, "read_length %d", read_length);

    //recording = false;
    while (recording) {
    		bytes_read = audio_in_read(input, buffer_ptr, read_length);
    		if (bytes_read < 0) {
			DLOG_PRINT_ERROR("audio_in_read", bytes_read);
		} else {
			dlog_print(DLOG_INFO, LOG_TAG, "%d bytes have been recorded.", bytes_read);
			 /* Store the recorded part in the file. */
			 fwrite(g_buffer, sizeof(char), read_length, g_fp_w);
		}

    		uint64_t sum = 0;

    		uint8_t *index = (uint8_t*)g_buffer;
		while (index < (((uint8_t*)g_buffer) + read_length)) {
			/* Use the int16_t type, because it is 2 bytes long */
			int32_t value = *(int16_t*)index;
			value *= value;
			double fraction = value / (read_length / 2.0 );
			sum += (uint16_t) fraction;

			dlog_print(DLOG_DEBUG, LOG_TAG, "value %d -> %d",  *(int16_t*)index, (uint8_t) fraction);

			/* Go to the next sample */
			index += 2;
		}

		uint32_t mean = 20 * log(sqrt(sum));
		dlog_print(DLOG_INFO, LOG_TAG, "sum %d mean %i",sum,  mean);
//		char text[20];
//		snprintf(text, sizeof(text), "%i", mean);
//		elm_object_text_set(volume , text);
		recording = false;
    }

    /* Close the file used for recording. */
	error_code = fclose(g_fp_w);
	CHECK_ERROR("fclose", error_code);

    /* Stop the hardware recording process. */
    error_code = audio_in_unprepare(input);
    CHECK_ERROR("audio_in_unprepare", error_code);

}

/**
 * @brief Enables disabled buttons when the current synchronous recording is
 *        finished.
 * @details Executes after the thread with synchronous recording ends.
 *          It is safe to call EFL functions here because this function is
 *          called from the main thread.
 * @remarks This function matches the Ecore_Thread_Cb() signature defined in the
 *          Ecore_Common.h header file.
 *
 * @param data    The user data passed via void pointer This argument is not
 *                used in this case.
 * @param thread  @c NULL if the thread was cancelled, otherwise probably
 *                (missing documentation) a pointer to the new thread that is
 *                executing the blocking function. This argument is not used in
 *                this case.
 */
static void synchronous_recording_ended(void *data, Ecore_Thread *thread)
{
//	return;

    PRINT_MSG("Synchronous recording ended.");

//    elm_object_text_set(button, "recording done");
}

static void _button_click_cb(void *data, Evas_Object *button, void *ev)
{
	static int counter = 0;
	counter++;

//    char text[20];
//    snprintf(text, sizeof(text), "Clicked! %d times", counter);
//	elm_object_text_set(button, text);
	dlog_print(DLOG_DEBUG, LOG_TAG, "button clicked");

	if (!data_initialized)
		data_initialize();

	recording = !recording;

	if(recording) {
		ecore_thread_run(synchronous_recording, synchronous_recording_ended, NULL, NULL);
	}
}


static void
win_delete_request_cb(void *data, Evas_Object *obj, void *event_info)
{
	ui_app_exit();
}

static void
win_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = data;
	/* Let window go to hide state. */
	elm_win_lower(ad->win);
}

static void
create_base_gui(appdata_s *ad)
{
	/* Window */
	/* Create and initialize elm_win.
	   elm_win is mandatory to manipulate window. */
	ad->win = elm_win_util_standard_add(PACKAGE, PACKAGE);
	elm_win_autodel_set(ad->win, EINA_TRUE);

	if (elm_win_wm_rotation_supported_get(ad->win)) {
		int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(ad->win, (const int *)(&rots), 4);
	}

	evas_object_smart_callback_add(ad->win, "delete,request", win_delete_request_cb, NULL);
	eext_object_event_callback_add(ad->win, EEXT_CALLBACK_BACK, win_back_cb, ad);

	/* Conformant */
	/* Create and initialize elm_conformant.
	   elm_conformant is mandatory for base gui to have proper size
	   when indicator or virtual keypad is visible. */
	ad->conform = elm_conformant_add(ad->win);
	//elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);
	//elm_win_indicator_opacity_set(ad->win, ELM_WIN_INDICATOR_OPAQUE);
	evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win, ad->conform);
	evas_object_show(ad->conform);

	/* Conformant */
	/* Create and initialize elm_conformant.
	   elm_conformant is mandatory for base gui to have proper size
	   when indicator or virtual keypad is visible. */
	ad->conform = elm_conformant_add(ad->win);
	//elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);
	//elm_win_indicator_opacity_set(ad->win, ELM_WIN_INDICATOR_OPAQUE);
	evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win, ad->conform);
	evas_object_show(ad->conform);

	/* Naviframe */
	ad->nf = elm_naviframe_add(ad->conform);
	evas_object_show(ad->nf);
	elm_naviframe_prev_btn_auto_pushed_set(ad->nf, EINA_TRUE);
	elm_object_content_set(ad->conform, ad->nf);

	/* Add a box and push it into the naviframe */
	ad->box = elm_box_add(ad->nf);
	evas_object_show(ad->box);
	elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, ad->box, NULL);

	ad->label = elm_label_add(ad->box);
	elm_object_text_set(ad->label, "<align=center>Team Scheire</align>");
	evas_object_size_hint_weight_set(ad->label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	//elm_object_content_set(ad->conform, ad->label);
	evas_object_size_hint_align_set(ad->label, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_min_set(ad->label, 100, 100);
	/* Show and add to box */
	evas_object_show(ad->label);
	elm_box_pack_end(ad->box, ad->label);

	volume = elm_label_add(ad->box);
	elm_object_text_set(volume, "Volume: ?");
	evas_object_size_hint_weight_set(volume, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	//elm_object_content_set(ad->conform, ad->label);
	evas_object_size_hint_align_set(volume, 0.5, 0.5);
	evas_object_size_hint_min_set(volume, 200, 100);
	/* Show and add to box */


	evas_object_show(volume);
	elm_box_pack_end(ad->box, volume);

	button = elm_button_add(ad->box);
	elm_object_text_set(button, "Click me");
	//elm_object_style_set(ad->button, "bottom");
	evas_object_size_hint_weight_set(button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(button, EVAS_HINT_FILL, 0.5);
	elm_box_pack_end(ad->box, button);
	evas_object_show(button);
	evas_object_smart_callback_add(button, "clicked", _button_click_cb, NULL);

	/* Show window after base gui is set up */
	evas_object_show(ad->win);

	dlog_print(DLOG_DEBUG, LOG_TAG, "gui ok");
}

void
app_request_response_cb(ppm_call_cause_e cause, ppm_request_result_e result,
                             const char *privilege, void *user_data)
{
    if (cause == PRIVACY_PRIVILEGE_MANAGER_CALL_CAUSE_ERROR) {
        /* Log and handle errors */
	    dlog_print(DLOG_ERROR, LOG_TAG, "app_request_response_cb: %s error code = %d", privilege, result);
	    requesting_permission = false;
        return;
    }

    switch (result) {
        case PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_ALLOW_FOREVER:
    		dlog_print(DLOG_INFO, LOG_TAG, "%s PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_ALLOW_FOREVER", privilege);
            /* Update UI and start accessing protected functionality */
            break;
        case PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_DENY_FOREVER:
    		dlog_print(DLOG_INFO, LOG_TAG, "%s PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_DENY_FOREVER", privilege);
            /* Show a message and terminate the application */
            break;
        case PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_DENY_ONCE:
    		dlog_print(DLOG_INFO, LOG_TAG, "%s PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_DENY_ONCE", privilege);
            /* Show a message with explanation */
            break;
    }

    requesting_permission = false;
}

// only tizen 5.0
//void
//app_request_multiple_response_cb(ppm_call_cause_e cause, ppm_request_result_e* results,
//                                 const char **privileges, size_t privileges_count, void *user_data)
//{
//    if (cause == PRIVACY_PRIVILEGE_MANAGER_CALL_CAUSE_ERROR) {
//        /* Log and handle errors */
//
//        return;
//    }
//    for (int it = 0; it < privileges_count; ++it) {
//        switch (results[it]) {
//            case PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_ALLOW_FOREVER:
//                /* Update UI and start accessing protected functionality */
//                break;
//            case PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_DENY_FOREVER:
//                /* Show a message and terminate the application */
//                break;
//            case PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_DENY_ONCE:
//                /* Show a message with explanation */
//                break;
//        }
//    }
//}

void
app_check_and_request_permissions()
{
//works only in tizen 5.0
//    ppm_check_result_e results[2];
//    const char* privileges [] = {"http://tizen.org/privilege/recorder",
//                                 "http://tizen.org/privilege/mediastorage"};
//    char* askable_privileges[2];
//    int askable_privileges_count = 0;
//
//    int ret = ppm_check_permissions(privileges, sizeof(privileges) / sizeof(privileges[0]), results);
//
//    if (ret == PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE) {
//        for (int it = 0; it < sizeof(privileges) / sizeof(privileges[0]); ++it)
//        {
//            switch (results[it]) {
//            case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ALLOW:
//                /* Update UI and start accessing protected functionality */
//                break;
//            case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_DENY:
//			   /* Show a message and terminate the application */
//			   break;
//            case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ASK:
//				askable_privileges[askable_privileges_count++] = privileges[it];
//				/* Log and handle errors */
//				break;
//			}
//		}
//		ret = ppm_request_permissions(askable_privileges, askable_privileges_count,
//									  app_request_multiple_response_cb, NULL);
//	} else {
//		/* ret != PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE */
//		/* Handle errors */
//	}


	ppm_check_result_e result;
	const char *privilege = "http://tizen.org/privilege/recorder";

	int ret = ppm_check_permission(privilege, &result);

	if (ret == PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE) {
		switch (result) {
		case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ALLOW:
			/* Update UI and start accessing protected functionality */
			dlog_print(DLOG_INFO, LOG_TAG, "recorder PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ALLOW");
			data_initialize();
			break;
		 case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_DENY:
			/* Show a message and terminate the application */
			 dlog_print(DLOG_INFO, LOG_TAG, "recorder PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_DENY");
			break;
		 case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ASK:
			 dlog_print(DLOG_INFO, LOG_TAG, "recorder PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ASK");
			 requesting_permission = true;
			 ret = ppm_request_permission(privilege, app_request_response_cb, NULL);
			 /* Log and handle errors */
			if (ret != PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE)
				dlog_print(DLOG_ERROR, LOG_TAG, "recorder ppm_request_permission: error code = %d", ret);
			else
				dlog_print(DLOG_INFO, LOG_TAG, "recorder ppm_request_permission ok");
			 break;
		}
	} else {
	 /* ret != PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE */
	 /* Handle errors */
	 dlog_print(DLOG_ERROR, LOG_TAG, "ppm_check_permission: error code = %d", ret);
	}

	while (requesting_permission) {}

	const char *privilege2 = "http://tizen.org/privilege/mediastorage";

	ret = ppm_check_permission(privilege2, &result);

	if (ret == PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE) {
		switch (result) {
		case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ALLOW:
			/* Update UI and start accessing protected functionality */
			dlog_print(DLOG_INFO, LOG_TAG, "recorder PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ALLOW");
			data_initialize();
			break;
		 case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_DENY:
			/* Show a message and terminate the application */
			 dlog_print(DLOG_INFO, LOG_TAG, "recorder PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_DENY");
			break;
		 case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ASK:
			 dlog_print(DLOG_INFO, LOG_TAG, "recorder PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ASK");
			 ret = ppm_request_permission(privilege, app_request_response_cb, NULL);
			 /* Log and handle errors */
			if (ret != PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE)
				dlog_print(DLOG_ERROR, LOG_TAG, "recorder ppm_request_permission: error code = %d", ret);
			else
				dlog_print(DLOG_INFO, LOG_TAG, "recorder ppm_request_permission ok");
			 break;
		}
	} else {
	 /* ret != PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE */
	 /* Handle errors */
	 dlog_print(DLOG_ERROR, LOG_TAG, "ppm_check_permission: error code = %d", ret);
	}

}

static bool
app_create(void *data)
{
	/* Hook to take necessary actions before main event loop starts
		Initialize UI resources and application's data
		If this function returns true, the main loop of application starts
		If this function returns false, the application is terminated */
	appdata_s *ad = data;

	create_base_gui(ad);

	return true;
}

static void
app_control(app_control_h app_control, void *data)
{
	/* Handle the launch request. */
}

static void
app_pause(void *data)
{
	/* Take necessary actions when application becomes invisible. */
}

static void
app_resume(void *data)
{
	/* Take necessary actions when application becomes visible. */
	ppm_check_result_e result;
	const char *privilege = "http://tizen.org/privilege/recorder";

	int ret = ppm_check_permission(privilege, &result);

	 if (ret == PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE) {
		switch (result) {
			case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ALLOW:
				/* Update UI and start accessing protected functionality */
				dlog_print(DLOG_INFO, LOG_TAG, "recorder PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ALLOW");
				data_initialize();
				break;
			 case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_DENY:
				/* Show a message and terminate the application */
				 dlog_print(DLOG_INFO, LOG_TAG, "recorder PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_DENY");
				break;
			 case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ASK:
				 dlog_print(DLOG_INFO, LOG_TAG, "recorder PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ASK");
				 ret = ppm_request_permission(privilege, app_request_response_cb, NULL);
				 /* Log and handle errors */
				if (ret != PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE)
					dlog_print(DLOG_ERROR, LOG_TAG, "recorder ppm_request_permission: error code = %d", ret);
				else
					dlog_print(DLOG_INFO, LOG_TAG, "recorder ppm_request_permission ok");
				 break;
		 }
	 }
	 else {
		 /* ret != PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE */
		 /* Handle errors */
		 dlog_print(DLOG_ERROR, LOG_TAG, "ppm_check_permission: error code = %d", ret);
	 }
}

static void
app_terminate(void *data)
{
	/* Release all resources. */
	if (g_buffer !=  NULL)
	        free(g_buffer);

	    /* Stop the hardware recording process. */
	    audio_in_unprepare(input);

	    /* Unset the callback function used for asynchronous recording process. */
	    audio_in_unset_stream_cb(input);

	    /* Deinitialize audio input device. */
	    audio_in_destroy(input);

	    /* Stop the hardware playback process. */
	    audio_out_unprepare(output);

	    /* Unset the callback function used for asynchronous playback process. */
	    audio_out_unset_stream_cb(output);

	    /* Deinitialize audio output device. */
	    audio_out_destroy(output);

	    /* Free the Sounds directory path. */
	    free(g_sounds_directory);
}

static void
ui_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LANGUAGE_CHANGED*/
	char *locale = NULL;
	system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);
	elm_language_set(locale);
	free(locale);
	return;
}

static void
ui_app_orient_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_DEVICE_ORIENTATION_CHANGED*/
	return;
}

static void
ui_app_region_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void
ui_app_low_battery(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_BATTERY*/
}

static void
ui_app_low_memory(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_MEMORY*/
}

int
main(int argc, char *argv[])
{
	appdata_s ad = {0,};
	int ret = 0;

	ui_app_lifecycle_callback_s event_callback = {0,};
	app_event_handler_h handlers[5] = {NULL, };

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_control;

	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, ui_app_low_battery, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, ui_app_low_memory, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, ui_app_orient_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, ui_app_lang_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, ui_app_region_changed, &ad);

	ret = ui_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "app_main() is failed. err = %d", ret);
	}

	return ret;
}
