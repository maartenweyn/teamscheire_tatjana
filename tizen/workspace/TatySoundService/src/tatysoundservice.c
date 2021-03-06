#include <tizen.h>
#include <service_app.h>
#include <privacy_privilege_manager.h>
#include <Ecore.h>
#include <system_info.h>
#include <app_event.h>

#include "tatysoundservice.h"
#include "sound.h"
#include "communication.h"

typedef struct appdata
{
	Ecore_Timer *timer1;
} appdata_s;

typedef struct
{
	int current_leq;
	int current_corrected_leq;
	int avg_leq;
	int avg_corrected_leq;
} leq_data_s;



static post_data_s postdata[POSTDATA_BUFFER_SIZE];
static int post_data_length	= 0;
static bool permissions_initialized = false;
static bool audio_initialized = false;
//static leq_data_s leq_data;

static void app_check_and_request_permissions();

void get_device_id(void)
{
    char *value;
    int ret;
    char token[20];

    ret = system_info_get_platform_string("http://tizen.org/system/tizenid", &value);
    if (ret != SYSTEM_INFO_ERROR_NONE) {
        /* Error handling */

        return;
    }

    memcpy(token, value, 20);

    for (char* p = token; (p = strchr(p, '\\')); ++p) {
		*p = '_';
	}

    for (char* p = token; (p = strchr(p, '/')); ++p) {
		*p = '_';
	}

    dlog_print(DLOG_INFO, LOG_TAG, "Tizen ID: %s", value);
    dlog_print(DLOG_INFO, LOG_TAG, "token: %s", token);

	snprintf(thingsboard_url, sizeof(thingsboard_url), HOST_URL, token);

    dlog_print(DLOG_INFO, LOG_TAG, "thingsboard_url: %s", thingsboard_url);

    //free(token); /* Release after use */
    free(value); /* Release after use */
}

void app_request_response_cb(ppm_call_cause_e cause, ppm_request_result_e result, const char *privilege, void *user_data)
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
static void app_check_and_request_mediapermissions() {
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
static void app_check_and_request_permissions() {
	dlog_print(DLOG_DEBUG, LOG_TAG, "%s", __func__);

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

//void push_current_values(double ts, int leq, int corrected) {
//	dlog_print(DLOG_INFO, LOG_TAG, "Current %d / %d (%0.3f)", leq, corrected, ts);
//	leq_data.current_leq = leq;
//	leq_data.current_corrected_leq = corrected;
//}

uint8_t push_average_values(double ts, int id, double sound_level, double leq_min, double leq_hour, double leq_8hours, double leq_day) {
	dlog_print(DLOG_INFO, LOG_TAG, "%.3f %d (%.4f) %.1f %.1f %.1f %.1f", ts, id, sound_level, leq_min, leq_hour, leq_8hours, leq_day);
	postdata[post_data_length].ts = ts;
	postdata[post_data_length].id = id;
	postdata[post_data_length].sound_level = sound_level;
	postdata[post_data_length].leq_min = leq_min;
	postdata[post_data_length].leq_hour = leq_hour;
	postdata[post_data_length].leq_8hours = leq_8hours;
	postdata[post_data_length].leq_day = leq_day;
	postdata[post_data_length].response = 0;
	uint8_t response = post_to_thingsboard(postdata, post_data_length+1);
	if (response == 0) {
			post_data_length = 0;
	} else {
		postdata[post_data_length].response = 1;
		if (post_data_length > 0)
			postdata[post_data_length].response += postdata[post_data_length-1].response;

		post_data_length++;
	}

	return response;
}

static Eina_Bool init_recording(void *data)
{
	appdata_s* ad = (appdata_s*)data;
   dlog_print(DLOG_INFO, LOG_TAG, "Timer expired at %0.3f.", ecore_time_get());

   if (!permissions_initialized) {
   		app_check_and_request_permissions();
   		ad->timer1 = ecore_timer_add(1, init_recording, ad);
   		return ECORE_CALLBACK_CANCEL;
   }

   if (!audio_initialized) {
	   audio_initialized = audio_device_init();
	   if (!audio_initialized) {
		   ad->timer1 = ecore_timer_add(1, init_recording, ad);
		   return ECORE_CALLBACK_CANCEL;
	   } else {
		   measure_sound();
		   ad->timer1 = ecore_timer_add(RECORDING_INTERVAL, init_recording, ad);
		   return ECORE_CALLBACK_CANCEL;
	   }
   }

   measure_sound();

   return ECORE_CALLBACK_RENEW;
}
//
//void
// sample_cb(const char *key, const int type, const bundle_keyval_t *kv, void *user_data)
// {
//     void *basic_val = NULL;
//     size_t basic_size = 0;
//     void **array_val = NULL;
//     int array_len = 0;
//     size_t *array_elem_size = NULL;
//
//     dlog_print(DLOG_INFO, LOG_TAG, "Key:%s, Type:%d\n", key, type);
//     if (bundle_keyval_type_is_array(kv)) {
//         bundle_keyval_get_array_val(kv, &array_val, &array_len, &array_elem_size);
//         // Do something
//     }
//     else {
//         bundle_keyval_get_basic_val(kv, &basic_val, &basic_size);
//         dlog_print(DLOG_INFO, LOG_TAG, "Value:%s\n", basic_val);
//     }
// }


//static void user_event_cb(const char *event_name, bundle *event_data, void *user_data)
//{
//    dlog_print(DLOG_INFO, LOG_TAG, "user_event_cb: %s: %d\n", event_name, bundle_get_count(event_data));
//    bundle_foreach(event_data, sample_cb, NULL);
//    return;
//}

bool service_app_create(void *data)
{
	appdata_s* ad = (appdata_s*)data;
	dlog_print(DLOG_DEBUG, LOG_TAG, "%s", __func__);

	ad->timer1 = ecore_timer_add(1, init_recording, ad);

	get_device_id();

//	event_handler_h event_handler;
//	int ret = EVENT_ERROR_NONE;
//
//	ret = event_add_event_handler("event.arq901aCcl.tatysoundservice.new_data_event", user_event_cb, "new_data_event", &event_handler);
//
//	if (ret != EVENT_ERROR_NONE)
//	    dlog_print(DLOG_ERROR, LOG_TAG, "event_add_event_handler err: [%d]", ret);

    return true;
}

void service_app_terminate(void *data)
{
	dlog_print(DLOG_DEBUG, LOG_TAG, "%s", __func__);
	service_app_exit();
    return;
}

void service_app_control(app_control_h app_control, void *data)
{
	dlog_print(DLOG_DEBUG, LOG_TAG, "%s", __func__);
	char *caller_id = NULL, *action_value = NULL;
	if ((app_control_get_caller(app_control, &caller_id) == APP_CONTROL_ERROR_NONE)
		&& (app_control_get_extra_data(app_control, "service_action", &action_value) == APP_CONTROL_ERROR_NONE))
	{
		if((caller_id != NULL) && (action_value != NULL)
			&& (!strncmp(caller_id, MYSERVICELAUNCHER_APP_ID, STRNCMP_LIMIT)))
		{
			if (!strncmp(action_value,"stop", STRNCMP_LIMIT)) {
				dlog_print(DLOG_INFO, LOG_TAG, "Stopping MyService!");
				free(caller_id);
				free(action_value);
				service_app_exit();
				return;
//			} else if (!strncmp(action_value,"update", STRNCMP_LIMIT)) {
//				dlog_print(DLOG_INFO, LOG_TAG, "Requesting data update");
//				free(caller_id);
//				free(action_value);
//
//				app_control_h reply;
//				app_control_create(&reply);
//				//app_control_add_extra_data(reply, "leq", "0,0,0,0");
//
//				char leq[40];
//			    snprintf(leq, sizeof(leq), "%d,%d,%d,%d", leq_data.current_leq, leq_data.current_corrected_leq, leq_data.avg_leq, leq_data.avg_corrected_leq);
//				app_control_add_extra_data(reply, "leq", leq);
//
//				if (app_control_reply_to_launch_request(reply, app_control, APP_CONTROL_RESULT_SUCCEEDED) == APP_CONTROL_ERROR_NONE)
//				{
//					dlog_print(DLOG_INFO, LOG_TAG, "Update reply send");
//				} else {
//					dlog_print(DLOG_ERROR, LOG_TAG, "Update reply send failed");
//				}
//				app_control_destroy(reply);
//				return;
			}  else {
				dlog_print(DLOG_INFO, LOG_TAG, "Unsupported action %s! Doing nothing...", action_value);
				free(caller_id);
				free(action_value);
				caller_id = NULL;
				action_value = NULL;
			}
		} else {
			dlog_print(DLOG_INFO, LOG_TAG, "Unsupported action fro %s! Doing nothing...", caller_id);
			free(caller_id);
			free(action_value);
			caller_id = NULL;
			action_value = NULL;
		}

	}
}

static void
service_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LANGUAGE_CHANGED*/
	return;
}

static void
service_app_region_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void
service_app_low_battery(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_BATTERY*/
	dlog_print(DLOG_DEBUG, LOG_TAG, "%s", __func__);
}

static void
service_app_low_memory(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_MEMORY*/
	dlog_print(DLOG_DEBUG, LOG_TAG, "%s", __func__);
}

int main(int argc, char* argv[])
{
	appdata_s ad = {0,};

	service_app_lifecycle_callback_s event_callback;
	app_event_handler_h handlers[5] = {NULL, };

	event_callback.create = service_app_create;
	event_callback.terminate = service_app_terminate;
	event_callback.app_control = service_app_control;

	service_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, service_app_low_battery, &ad);
	service_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, service_app_low_memory, &ad);
	service_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, service_app_lang_changed, &ad);
	service_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, service_app_region_changed, &ad);

	return service_app_main(argc, argv, &event_callback, &ad);
}
