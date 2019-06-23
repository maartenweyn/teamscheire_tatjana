#include "tatysoundviewer.h"
#include "view.h"

#include <app_control.h>
#include <device/power.h>
#include <system_info.h>


#include <bundle.h>
#include <app_event.h>

typedef struct appdata {
	Evas_Object *win;
	Evas_Object *conform;
	Evas_Object *list;
	Evas_Object *box;
	Evas_Object *nf;
} appdata_s;

//static Ecore_Timer *update_timer;
//static bool getting_data = false;
//
//static Evas_Object *startStopButton;
//static Evas_Object *getDataButton;
//static Evas_Object *dataLabel;

static bool serviceLaunced = false;

static void launch_service();
static void stop_service();


void control_service()
 {
	if (serviceLaunced)
	{
		stop_service();
	} else {
		launch_service();
	}
 }

static void new_data_event_cb(const char *event_name, bundle *event_data, void *user_data)
{
    dlog_print(DLOG_INFO, LOG_TAG, "user_event_cb: %s: %d\n", event_name, bundle_get_count(event_data));
    char *raw = NULL;
    char *leq = NULL;

    if (bundle_get_str(event_data, "raw", &raw) == BUNDLE_ERROR_NONE) {
    		dlog_print(DLOG_INFO, LOG_TAG, "user_event_cb: raw: %s\n", raw);
    		//raw = atoi(value);
    }

    if (bundle_get_str(event_data, "leq", &leq) == BUNDLE_ERROR_NONE) {
    		dlog_print(DLOG_INFO, LOG_TAG, "user_event_cb: corrected leq: %s\n", leq);
    		//leq = atoi(value);
    }

    if (bundle_get_count(event_data) > 2) {
    		dlog_print(DLOG_INFO, LOG_TAG, "user_event_cb: MORE DATA");
        char *leq_min = NULL;
        	char *leq_hour = NULL;
        	char *leq_8hours = NULL;
        char *leq_day = NULL;
        char *network_string = NULL;
        int network_status = 0;

		if (bundle_get_str(event_data, "leg_min", &leq_min) == BUNDLE_ERROR_NONE) {
				dlog_print(DLOG_INFO, LOG_TAG, "user_event_cb: leg_min: %s\n", leq_min);
		}

		if (bundle_get_str(event_data, "leq_hour", &leq_hour) == BUNDLE_ERROR_NONE) {
				dlog_print(DLOG_INFO, LOG_TAG, "user_event_cb: leq_hour: %s\n", leq_hour);
		}
		if (bundle_get_str(event_data, "leq_8hours", &leq_8hours) == BUNDLE_ERROR_NONE) {
				dlog_print(DLOG_INFO, LOG_TAG, "user_event_cb: leq_8hours: %s\n", leq_8hours);
		}
		if (bundle_get_str(event_data, "leq_day", &leq_day) == BUNDLE_ERROR_NONE) {
				dlog_print(DLOG_INFO, LOG_TAG, "user_event_cb: leq_day: %s\n", leq_day);
		}
		if (bundle_get_str(event_data, "network", &network_string) == BUNDLE_ERROR_NONE) {
				dlog_print(DLOG_INFO, LOG_TAG, "user_event_cb: leq_day: %s\n", network_string);
				network_status = atoi(network_string);
		}
		updates_values(leq_min, leq_hour, leq_8hours, leq_day, network_status);
    }

    updates_current_values(raw, leq);

    return;
}


static void launch_service()
{
	static app_control_h app_control = NULL;
	int response = app_control_create(&app_control);

    if (response == APP_CONTROL_ERROR_NONE)
    {
    		response = app_control_set_app_id(app_control, "be.wesdec.tatysoundservice");

        if (response == APP_CONTROL_ERROR_NONE)
        {
        		response = app_control_send_launch_request(app_control, NULL, NULL) ;
            if (response == APP_CONTROL_ERROR_NONE)
			{
				dlog_print(DLOG_INFO, LOG_TAG, "App launch request sent!");
			}
			else
			{
				dlog_print(DLOG_ERROR, LOG_TAG, "App launch request sending failed! %d", response);
			}
		} else {
			dlog_print(DLOG_ERROR, LOG_TAG, "App Control set app id failed! %d", response);
		}
        if (app_control_destroy(app_control) == APP_CONTROL_ERROR_NONE)
        {
            dlog_print(DLOG_INFO, LOG_TAG, "App control destroyed.");
        }
        // We exit our launcher app, there is no point in keeping it open.
        //ui_app_exit();
    }
    else
    {
        dlog_print(DLOG_ERROR, LOG_TAG, "App control creation failed!");
    }

    if (response == APP_CONTROL_ERROR_NONE) {
    		serviceLaunced = true;
    		set_running_status(1);
    }
}

static void stop_service()
{
	static app_control_h app_control = NULL;

	int response = app_control_create(&app_control);

    if (response == APP_CONTROL_ERROR_NONE)
    {
    		int response = app_control_set_app_id(app_control, "be.wesdec.tatysoundservice");
    		if (response == APP_CONTROL_ERROR_NONE) {
    			response = app_control_add_extra_data(app_control, "service_action", "stop");
    			if (response == APP_CONTROL_ERROR_NONE) {
    				response = app_control_send_launch_request(app_control, NULL, NULL);
    				if (response == APP_CONTROL_ERROR_NONE) {
    					dlog_print(DLOG_INFO, LOG_TAG, "App launch request sent!");
    				} else {
    					dlog_print(DLOG_ERROR, LOG_TAG, "App launch request sending failed!");
    				}
			} else {
				dlog_print(DLOG_ERROR, LOG_TAG, "App Control add extra data failed! %d", response);
			}
    		} else {
    			dlog_print(DLOG_ERROR, LOG_TAG, "App Control set app id failed! %d", response);
    		}


        if (app_control_destroy(app_control) == APP_CONTROL_ERROR_NONE)
        {
            dlog_print(DLOG_INFO, LOG_TAG, "App control destroyed.");
        }
        //ui_app_exit();
    }
    else
    {
        dlog_print(DLOG_ERROR, LOG_TAG, "App control creation failed!");
    }

    if (response == APP_CONTROL_ERROR_NONE) {
			serviceLaunced = 0;
			set_running_status(0);
	}
}

void data_cb(app_control_h request, app_control_h reply, app_control_result_e result, void *user_data) {
	char *value;

	if (result == APP_CONTROL_RESULT_SUCCEEDED) {
		if (app_control_get_extra_data(reply, "leq", &value) == APP_CONTROL_ERROR_NONE)
		{
			dlog_print(DLOG_INFO, LOG_TAG, "data_cb Succeeded: value(%s)", value);
		}
		else
		{
			dlog_print(DLOG_ERROR, LOG_TAG, "data_cb Failed");
		}
	} else {
		dlog_print(DLOG_ERROR, LOG_TAG, "data_cb APP_CONTROL_RESULT_FAILED.");
	}
}

void get_device_id(void)
{
    char *value;
    int ret;
    char token[21];

    ret = system_info_get_platform_string("http://tizen.org/system/tizenid", &value);
    if (ret != SYSTEM_INFO_ERROR_NONE) {
        /* Error handling */

        return;
    }

    memcpy(token, value, 20);
    token[20] = '\0';
    set_token(token);

    free(value); /* Release after use */
}

static bool
app_create(void *data)
{
	/* Hook to take necessary actions before main event loop starts
		Initialize UI resources and application's data
		If this function returns true, the main loop of application starts
		If this function returns false, the application is terminated */
	//appdata_s *ad = data;
	//create_base_gui(ad);

	view_create();

	event_handler_h event_handler;
	int ret = EVENT_ERROR_NONE;

	ret = event_add_event_handler("event.be.wesdec.tatysoundservice.new_data_event", new_data_event_cb, "new_data_event", &event_handler);

	if (ret != EVENT_ERROR_NONE)
	    dlog_print(DLOG_ERROR, LOG_TAG, "event_add_event_handler err: [%d]", ret);

	get_device_id();

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
}

static void
app_terminate(void *data)
{
	/* Release all resources. */
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
