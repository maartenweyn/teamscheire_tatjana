#include "soundviewer.h"
#include <math.h>
#include <privacy_privilege_manager.h>

#include <recorder.h>
#include <storage.h>
#include <http.h>

#include <curl/curl.h>
#include <net_connection.h>

// Define the sample rate for recording audio
#define SAMPLE_RATE 44100
//#define SAMPLE_TYPE AUDIO_SAMPLE_TYPE_U8
#define SAMPLE_TYPE AUDIO_SAMPLE_TYPE_S16_LE
#define RECORDING_SEC 10
#define MIN_RECORDING_INTERVAL  1
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
static bool permissions_initialized = false;
static int currentLeq = 0;
static int correctedLeq = 0;
static int corr_avg_leq = 0;
static int avg_leq = 0;

static void *g_buffer = NULL;  /* Buffer used for audio recording/playback */
static int g_buffer_size;  /* Size of the buffer used for audio recording/playback */

static const double AFILTER_Acoef[] = {1.0, -4.0195761811158315, 6.1894064429206921, -4.4531989035441155,
                    1.4208429496218764, -0.14182547383030436,
                    0.0043511772334950787};
static const double AFILTER_Bcoef[] = {0.2557411252042574, -0.51148225040851436,
                    -0.25574112520425807, 1.0229645008170318,
                    -0.25574112520425918, -0.51148225040851414,
                    0.25574112520425729};

static double AFILTER_conditions[] = {0, 0, 0, 0, 0, 0};

static const double x0 = 55.0; // watch
static const double x1 = 82.0; // watch
static const double ref0 = 20.0; //ref
static const double ref1 = 76.0; //ref

static int tb_response = 0;
static int min = 200;
static int max = 0;


static Evas_Object *button;
static Evas_Object *volume;
typedef struct appdata {
	Evas_Object *win;
	Evas_Object *conform;
	Evas_Object *label;
	Evas_Object *box;
	Evas_Object *nf;
} appdata_s;

struct MemoryStruct {
	char *memory;
	size_t size;
};


struct MemoryStruct chunk;

static void app_check_and_request_permissions();



static size_t
read_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;

	dlog_print(DLOG_INFO, LOG_TAG, "read_callback size: %d", realsize);
	dlog_print(DLOG_INFO, LOG_TAG, "read_callback %s", (char*) contents);

	return realsize;
}


static int post_to_thingsboard() {
	/* Initialize CURL */
	    CURL *curlHandler = curl_easy_init();

	    connection_h connection;

	    int conn_err;
	    conn_err = connection_create(&connection);
	    if (conn_err != CONNECTION_ERROR_NONE) {
	        /* Error handling */
	        return 1;
	    }

	    char *proxy_address;
	    conn_err = connection_get_proxy(connection, CONNECTION_ADDRESS_FAMILY_IPV4, &proxy_address);

	    if(curlHandler) {
	    		if (conn_err == CONNECTION_ERROR_NONE && proxy_address)
				curl_easy_setopt(curlHandler, CURLOPT_PROXY, proxy_address);

	      /* Set CURL parameters */
	      //curl_easy_setopt(curlHandler, CURLOPT_URL, "http://apidev.accuweather.com/currentconditions/v1/28143.json?language=en&apikey=hoArfRosT1215");
	      curl_easy_setopt(curlHandler, CURLOPT_URL, "http://demo.thingsboard.io/api/v1/ObCtJ5ttQ8U9tToxcQvD/telemetry");

	      struct curl_slist *list = NULL;
	      list = curl_slist_append(list, "Content-Type: application/json");
	      curl_easy_setopt(curlHandler, CURLOPT_HTTPHEADER, list);
	      /* Now specify the POST data */
	      char json[100];
	      double ts = ecore_time_unix_get();
	      dlog_print(DLOG_INFO, LOG_TAG, "ts %f  %.0f", ts, ts * 1000);
	      snprintf(json, sizeof(json), "{\"ts\":%.0f, \"values\":{\"leq\":%d, \"cleq\":%d}}", ts * 1000, avg_leq, corr_avg_leq);
	      dlog_print(DLOG_INFO, LOG_TAG, "json %s", json);
	      curl_easy_setopt(curlHandler, CURLOPT_POSTFIELDS, json);
	      curl_easy_setopt(curlHandler, CURLOPT_READFUNCTION, read_callback);
	      curl_easy_setopt(curlHandler, CURLOPT_READDATA, (void *)&chunk);
	      curl_easy_setopt(curlHandler, CURLOPT_VERBOSE, 1L);
	      //curl_easy_setopt(curl, CURLOPT_POST, 1L);
	      //curl_easy_setopt(curlHandler, CURLOPT_CUSTOMREQUEST, "GET");
	      //curl_easy_setopt(curlHandler, CURLOPT_POSTFIELDS,  jObj);
	      /* send all data to this function  */
	      //curl_easy_setopt(curlHandler, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	      /* we pass the 'chunk' struct to the callback function */
	      //curl_easy_setopt(curlHandler, CURLOPT_WRITEDATA, (void *)&chunk);

	      /* Perform the request */
	      CURLcode res = curl_easy_perform(curlHandler);

	      /* Check for errors */
	      if(res != CURLE_OK)
	        fprintf(stderr, "CURL failed: %s\n",
	                curl_easy_strerror(res));

	      /* Clean up */
	      curl_easy_cleanup(curlHandler);
	      free(chunk.memory);
	      connection_destroy(connection);
	      //free(jObj);

	      if(res == CURLE_OK)
	    	  	  return 0;
	      else {
	    	  	  return 3;
		}
	  }

	    return 2;
}


// DB filter

static double a_filter(double input) {
	double output = input * AFILTER_Bcoef[0] + AFILTER_conditions[0];
	for (int j = 0; j < 5; j++) {
		AFILTER_conditions[j] = input * AFILTER_Bcoef[j + 1] - output * AFILTER_Acoef[j + 1] + AFILTER_conditions[j + 1];
	}
	AFILTER_conditions[5] = input * AFILTER_Bcoef[6] - output * AFILTER_Acoef[6];

	return output;
}

static int correctdB(double input) {
	double corrected = ((input - x0) * ((ref1 - ref0) / (x1 - x0))) + ref0;
	return (int) corrected;
}

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
    //error_code = audio_out_create_new(SAMPLE_RATE, AUDIO_CHANNEL_MONO,AUDIO_SAMPLE_TYPE_S16_LE, &output);
    //error_code = audio_out_create_new(SAMPLE_RATE, AUDIO_CHANNEL_MONO, SAMPLE_RATE, &output);
    //CHECK_ERROR_AND_RETURN("audio_out_create", error_code);

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

static void* setvolume_async(void *data)
{
	int16_t* leq = data;
	char text[20];
	snprintf(text, sizeof(text), "%d-%d-%d: %d dB - %d dB %d",min, *leq, max, correctedLeq, corr_avg_leq, tb_response);
	dlog_print(DLOG_DEBUG, LOG_TAG, "setvolume_async() %d", *leq);
	elm_object_text_set(volume , text);

	return NULL;
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
	static double cumulativeSoundLevel = 0.0;
	static int cumulativeSoundCounter = 0;
	static double start_ts = 0.0;

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
#if	SAMPLE_TYPE == AUDIO_SAMPLE_TYPE_S16_LE
    		g_buffer_size = SAMPLE_RATE * 2 * 1 * RECORDING_SEC;
#else
    		g_buffer_size = SAMPLE_RATE * 1 * 1 * RECORDING_SEC;
#endif

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
     //MIN_BUFFER_SIZE;
#if	SAMPLE_TYPE == AUDIO_SAMPLE_TYPE_S16_LE
    		read_length = 2 * SAMPLE_RATE * MIN_RECORDING_INTERVAL;
#else
    		read_length = 1 * SAMPLE_RATE * MIN_RECORDING_INTERVAL;
#endif

    buffer_ptr = (char *)g_buffer;

    //dlog_print(DLOG_INFO, LOG_TAG, "num_of_iterations %d", num_of_iterations);
    dlog_print(DLOG_INFO, LOG_TAG, "read_length %d", read_length);

    //recording = false;
    start_ts = ecore_time_unix_get();
    while (recording) {
    		bytes_read = audio_in_read(input, buffer_ptr, read_length);
    		if (bytes_read < 0) {
			DLOG_PRINT_ERROR("audio_in_read", bytes_read);
		} else {
			dlog_print(DLOG_INFO, LOG_TAG, "%d bytes have been recorded.", bytes_read);
			 /* Store the recorded part in the file. */
			 fwrite(g_buffer, sizeof(char), read_length, g_fp_w);
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


			//dlog_print(DLOG_DEBUG, LOG_TAG, "%d -> %.4f -> %.4f",  ivalue, fraction, filtered);

			/* Go to the next sample */
			index += 1;
		}

		double soundLevel = sumsquare / read_length;
		cumulativeSoundLevel += soundLevel;
		cumulativeSoundCounter++;

		currentLeq = (int16_t) ((10.0 * log10(soundLevel)) + 93.97940008672037609572522210551);
		correctedLeq = correctdB(currentLeq);
		dlog_print(DLOG_INFO, LOG_TAG, "Leq %d / %d", currentLeq, correctedLeq);

		if (currentLeq < min) min = currentLeq;
		if (currentLeq > max) max = currentLeq;

		if (ecore_time_unix_get() - start_ts >= 10.0) {
			avg_leq = (int16_t) ((10.0 * log10(cumulativeSoundLevel / cumulativeSoundCounter)) + 93.97940008672037609572522210551);
			corr_avg_leq = correctdB(avg_leq);
			tb_response =  post_to_thingsboard();
			cumulativeSoundLevel = 0;
			cumulativeSoundCounter = 0;
			start_ts = ecore_time_unix_get();
		}

		ecore_main_loop_thread_safe_call_sync(setvolume_async, &currentLeq);

    }

    /* Close the file used for recording. */
	error_code = fclose(g_fp_w);
	CHECK_ERROR("fclose", error_code);

    /* Stop the hardware recording process. */
    error_code = audio_in_unprepare(input);
    CHECK_ERROR("audio_in_unprepare", error_code);

    error_code = sound_manager_release_focus(g_stream_info_h, SOUND_STREAM_FOCUS_FOR_RECORDING, SOUND_BEHAVIOR_NONE, "record(Release)");
	if (SOUND_MANAGER_ERROR_NONE != error_code)
		dlog_print(DLOG_ERROR, LOG_TAG, "sound_manager_release_focus() failed! Error code = 0x%x", error_code);

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
	dlog_print(DLOG_DEBUG, LOG_TAG, "button clicked");


	//init_http();
	//get_http();
	//deinit_http();
	//test_curl();

	//return;

	if (!permissions_initialized) {
		app_check_and_request_permissions();
		return;
	}

	if (!data_initialized)
		data_initialize();

	recording = !recording;

	if(recording) {
		elm_object_text_set(button, "Recording");
		ecore_thread_run(synchronous_recording, synchronous_recording_ended, NULL, NULL);
	} else {
		elm_object_text_set(button, "Record");
		char text[20];
		snprintf(text, sizeof(text), " Volume: %d dB", currentLeq);
		elm_object_text_set(volume , text);
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
	elm_object_text_set(button, "Record");
	elm_object_style_set(button, "bottom");
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
	    return;
    }

    switch (result) {
        case PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_ALLOW_FOREVER:
    		dlog_print(DLOG_INFO, LOG_TAG, "%s PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_ALLOW_FOREVER", privilege);
        if (strcmp(privilege, "http://tizen.org/privilege/mediastorage") == 0) {
			permissions_initialized = true;
        }
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
app_check_and_request_mediapermissions() {
	ppm_check_result_e result;
	const char *privilege = "http://tizen.org/privilege/mediastorage";
	int ret = ppm_check_permission(privilege, &result);

	if (ret == PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE) {
		switch (result) {
		case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ALLOW:
			/* Update UI and start accessing protected functionality */
			dlog_print(DLOG_INFO, LOG_TAG, "recorder PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ALLOW");
			permissions_initialized = true;
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
			app_check_and_request_mediapermissions();
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
	app_check_and_request_permissions();
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
