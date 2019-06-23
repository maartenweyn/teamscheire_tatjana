/*
 * view.c
 *
 *  Created on: May 28, 2019
 *      Author: maartenweyn
 */

#include "view.h"
#include "tatysoundviewer.h"

#include "view_defines.h"

#define MAIN_EDJ "/edje/main.edj"


struct sound_val_s {
	char raw[5];
	char leq[5];
	char leq_min[5];
	char leq_hour[5];
	char leq_8hours[5];
	char leq_day[5];
};

typedef struct sound_val_s sound_val_t;

static struct view_info {
	Evas_Object *win;
	Evas_Object *layout_setup;
	Evas_Object *layout_run;
	sound_val_t sound_values;
	int network_status;
} s_info = {
	.win = NULL,
	.layout_setup = NULL,
	.layout_run = NULL,
	.sound_values = {"25.0", "25.0","25.0","25.0","25.0","25.0"},
	.network_status = 0
};

static const char *_create_resource_path(const char *file_name);
static void _set_selected_part_displayed_value(Evas_Object *current_layout, char *part, char* value);
static void _set_displayed_sound_value(Evas_Object *layout);


/**
 * @brief Create Essential Object window and layout
 */
Eina_Bool view_create(void)
{
	/* Create window */
	s_info.win = view_create_win(PACKAGE);
	if (s_info.win == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "failed to create a window.");
		return EINA_FALSE;
	}

	s_info.layout_setup = view_create_layout_for_win(s_info.win, _create_resource_path(MAIN_EDJ), GRP_SETUP);
	elm_layout_signal_callback_add(s_info.layout_setup, START_CLICKED, "", control_service, NULL);



	evas_object_show(s_info.layout_setup);

	/* Show window after main view is set up */
	evas_object_show(s_info.win);

	return EINA_TRUE;
}


void updates_current_values(char* raw, char* leq) {
	strcpy(s_info.sound_values.raw, raw);
	strcpy(s_info.sound_values.leq, leq);

	_set_displayed_sound_value(s_info.layout_setup);
}
void updates_values(char* leq_min, char* leq_hour, char* leq_8hours, char* leq_day, int network_status) {
	strcpy(s_info.sound_values.leq_min, leq_min);
	strcpy(s_info.sound_values.leq_hour, leq_hour);
	strcpy(s_info.sound_values.leq_8hours, leq_8hours);
	strcpy(s_info.sound_values.leq_day, leq_day);
	s_info.network_status = network_status;
}

void set_token(char* token_id) {

	Eina_Stringshare *txt = NULL;

	if (!s_info.layout_setup) {
		dlog_print(DLOG_ERROR, LOG_TAG, "[%s:%d] s_info.layout == NULL", __FILE__, __LINE__);
		return;
	}

	txt = eina_stringshare_printf("%s", token_id);
	elm_layout_text_set(s_info.layout_setup, PART_DEVID, txt);
	eina_stringshare_del(txt);
}

void set_running_status(int running) {
	int msg_id = MSG_ID_SET_RUNNING_STATUS;
	Edje_Message_Int_Set *msg = NULL;

	if (!s_info.layout_setup) {
		dlog_print(DLOG_ERROR, LOG_TAG, "[%s:%d] s_info.layout == NULL", __FILE__, __LINE__);
		return;
	}

	if (running == 0)
		msg_id = MSG_ID_SET_NOT_RUNNING_STATUS;

	msg = calloc(1, sizeof(Edje_Message_Int_Set));
	if (!msg) {
		dlog_print(DLOG_ERROR, LOG_TAG, "[%s:%d] msg == NULL", __FILE__, __LINE__);
		return;
	}

	msg->count = 1;

	edje_object_message_send(elm_layout_edje_get(s_info.layout_setup), EDJE_MESSAGE_INT_SET, msg_id, msg);
	free(msg);
}

/**
 * @brief Creates path to the given resource file by concatenation of the basic resource path and the given file_name.
 * @param[in] file_name File name or path relative to the resource directory.
 * @return The absolute path to the resource with given file_name is returned.
 */
static const char *_create_resource_path(const char *file_name)
{
	static char res_path_buff[PATH_MAX] = {0,};
	char *res_path = NULL;

	res_path = app_get_resource_path();
	if (res_path == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "[%s:%d] failed to get resource path.", __FILE__, __LINE__);
		return NULL;
	}

	snprintf(res_path_buff, PATH_MAX, "%s%s", res_path, file_name);
	free(res_path);

	return &res_path_buff[0];
}

/**
 * @brief Make a basic window named package
 * @param[in] pkg_name Name of the window
 */
Evas_Object *view_create_win(const char *pkg_name)
{
	Evas_Object *win = NULL;

	/*
	 * Window
	 * Create and initialize elm_win.
	 * elm_win is mandatory to manipulate window
	 */
	win = elm_win_util_standard_add(pkg_name, pkg_name);
	elm_win_conformant_set(win, EINA_TRUE);
	elm_win_autodel_set(win, EINA_TRUE);

	/* Rotation setting */
//	if (elm_win_wm_rotation_supported_get(win)) {
//		int rots[4] = { 0, 90, 180, 270 };
//		elm_win_wm_rotation_available_rotations_set(win, (const int *)(&rots), 4);
//	}

	//evas_object_smart_callback_add(win, "delete,request", _win_delete_request_cb, NULL);

	return win;
}

/**
 * @brief Internal function which creates a layout object and adds it to the main window.
 * @param[in] win The main window.
 * @param[in] file_path Path to the layout file.
 * @param[in] group_name The group name to be set to the created layout.
 * @return The newly created layout object or NULL if the function fails.
 */
Evas_Object *view_create_layout_for_win(Evas_Object *win, const char *file_path, const char *group_name)
{
	Evas_Object *layout = NULL;

	if (win == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "[%s:%d] parent is NULL.", __FILE__, __LINE__);
		return NULL;
	}

	layout = elm_layout_add(win);
	if (layout == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "[%s:%d] layout is NULL.", __FILE__, __LINE__);
		return NULL;
	}

	elm_layout_file_set(layout, file_path, group_name);
	//elm_layout_signal_callback_add(layout, SIGNAL_VIEW_CHANGE, "", _switch_view_cb, NULL);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(win, layout);
	//eext_object_event_callback_add(layout, EEXT_CALLBACK_BACK, _layout_back_cb, NULL);

	return layout;
}

static void _set_selected_part_displayed_value(Evas_Object *current_layout, char *part, char * value)
{
	Eina_Stringshare *txt = NULL;

	if (!s_info.layout_setup) {
		dlog_print(DLOG_ERROR, LOG_TAG, "[%s:%d] s_info.layout == NULL", __FILE__, __LINE__);
		return;
	}

	txt = eina_stringshare_printf("%s", value);
	elm_layout_text_set(current_layout, part, txt);
	eina_stringshare_del(txt);
}

static void _set_displayed_sound_value(Evas_Object *layout)
{
	_set_selected_part_displayed_value(layout, PART_RAW, s_info.sound_values.raw);
	_set_selected_part_displayed_value(layout, PART_LEQ, s_info.sound_values.leq);
	_set_selected_part_displayed_value(layout, PART_LEQ_MIN, s_info.sound_values.leq_min);
	_set_selected_part_displayed_value(layout, PART_LEQ_HOUR, s_info.sound_values.leq_hour);
	_set_selected_part_displayed_value(layout, PART_LEQ_8HOUR, s_info.sound_values.leq_8hours);
	_set_selected_part_displayed_value(layout, PART_LEQ_DAY, s_info.sound_values.leq_day);

	if (s_info.network_status)
		_set_selected_part_displayed_value(layout, PART_NETWORK, "connected");
	else
		_set_selected_part_displayed_value(layout, PART_NETWORK, "no connection");

	int msg_id = MSG_ID_SET_VALUES;
	Edje_Message_Int_Set *msg = NULL;

	if (!s_info.layout_setup) {
		dlog_print(DLOG_ERROR, LOG_TAG, "[%s:%d] s_info.layout == NULL", __FILE__, __LINE__);
		return;
	}

	msg = calloc(1, sizeof(Edje_Message_Int_Set) + (sizeof(int) * 5));
	if (!msg) {
		dlog_print(DLOG_ERROR, LOG_TAG, "[%s:%d] msg == NULL", __FILE__, __LINE__);
		return;
	}

	msg->count = 6;
	msg->val[0] = atoi(s_info.sound_values.raw);
	msg->val[1] = atoi(s_info.sound_values.leq);
	msg->val[2] = atoi(s_info.sound_values.leq_min);
	msg->val[3] = atoi(s_info.sound_values.leq_hour);
	msg->val[4] = atoi(s_info.sound_values.leq_8hours);
	msg->val[5] = atoi(s_info.sound_values.leq_day);

	edje_object_message_send(elm_layout_edje_get(s_info.layout_setup), EDJE_MESSAGE_INT_SET, msg_id, msg);
	free(msg);
}

