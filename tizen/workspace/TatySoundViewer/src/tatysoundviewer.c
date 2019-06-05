#include "tatysoundviewer.h"
#include "view.h"

#include <app_control.h>
#include <device/power.h>

typedef struct appdata {
	Evas_Object *win;
	Evas_Object *conform;
	Evas_Object *list;
	Evas_Object *box;
	Evas_Object *nf;
} appdata_s;

static Ecore_Timer *update_timer;
static bool getting_data = false;

static Evas_Object *startStopButton;
static Evas_Object *getDataButton;
static Evas_Object *dataLabel;

static bool serviceLaunced = false;


static app_control_h app_update_control = NULL;

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
    		elm_object_text_set(startStopButton, "Stop Service");
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
			serviceLaunced = false;
			elm_object_text_set(startStopButton, "Start Service");
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

//	int ret;
//	char *value;
//	//ret = app_control_get_extra_data(app_update_control, "leq", &value);
//	ret = app_control_get_extra_data(app_update_control, APP_CONTROL_DATA_SELECTED, &value);
//	if (ret != APP_CONTROL_ERROR_NONE)
//	{
//	  dlog_print(DLOG_ERROR, LOG_TAG, "app_control_get_extra_data() is failed. err = %d", ret);
//	}
//	dlog_print(DLOG_DEBUG, LOG_TAG, "[value] %s", value);
//
//	if (app_control_destroy(app_update_control) == APP_CONTROL_ERROR_NONE)
//	{
//		dlog_print(DLOG_INFO, LOG_TAG, "App control destroyed.");
//	}
//}


static Eina_Bool get_data(void *data)
{
	dlog_print(DLOG_DEBUG, LOG_TAG, "get_data");

    if (app_control_create(&app_update_control)== APP_CONTROL_ERROR_NONE)
    {
    		int response = app_control_set_app_id(app_update_control, "be.wesdec.tatysoundservice");
    		if (response == APP_CONTROL_ERROR_NONE) {
    			response = app_control_add_extra_data(app_update_control, "service_action", "update");
    			if (response == APP_CONTROL_ERROR_NONE) {
    				response = app_control_send_launch_request(app_update_control, data_cb, NULL);
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


        if (app_control_destroy(app_update_control) == APP_CONTROL_ERROR_NONE)
        {
            dlog_print(DLOG_INFO, LOG_TAG, "App control destroyed.");
        }
        //ui_app_exit();
    }
    else
    {
        dlog_print(DLOG_ERROR, LOG_TAG, "App control creation failed!");
    }

    if (getting_data)
    		return ECORE_CALLBACK_RENEW;
    else
    		return ECORE_CALLBACK_CANCEL;
}

static void launch_service_cb(void *data, Evas_Object *obj, void *event_info)
 {
	if (serviceLaunced)
	{
		stop_service();
	} else {
     launch_service();
	}
 }

 static void get_data_cb(void *data, Evas_Object *obj, void *event_info)
 {
	 dlog_print(DLOG_DEBUG, LOG_TAG, "get_data_cb");

	 getting_data = !getting_data;

	 if (getting_data) {
		 update_timer = ecore_timer_add(UPDATE_INTERVAL, get_data, NULL);
	 }
 }

static void close_app_cb(void *data, Evas_Object *obj, void *event_info)
 {
     ui_app_exit();
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
	elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(ad->win, ELM_WIN_INDICATOR_OPAQUE);
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

	startStopButton = elm_button_add(ad->box);
	elm_object_text_set(startStopButton, "Start Service");
	//elm_object_style_set(button, "bottom");
	evas_object_size_hint_weight_set(startStopButton, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(startStopButton, EVAS_HINT_FILL, 0.5);
	elm_box_pack_end(ad->box, startStopButton);
	evas_object_show(startStopButton);
	evas_object_smart_callback_add(startStopButton, "clicked", launch_service_cb, NULL);

	getDataButton = elm_button_add(ad->box);
	elm_object_text_set(getDataButton, "get Data");
	//elm_object_style_set(button, "bottom");
	evas_object_size_hint_weight_set(getDataButton, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(getDataButton, EVAS_HINT_FILL, 0.5);
	elm_box_pack_end(ad->box, getDataButton);
	evas_object_show(getDataButton);
	evas_object_smart_callback_add(getDataButton, "clicked", get_data_cb, NULL);


	/* Show window after base gui is set up */
	evas_object_show(ad->win);
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

	//view_create();

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
