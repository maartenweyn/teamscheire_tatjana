/*
 * view.c
 *
 *  Created on: May 28, 2019
 *      Author: maartenweyn
 */

#include "view.h"
#include "tatysoundviewer.h"

static struct view_info {
	Evas_Object *win;
	Evas_Object *layout_setup;
	Evas_Object *layout_run;
	int current_leq;
} s_info = {
	.win = NULL,
	.layout_setup = NULL,
	.layout_run = NULL,
	.current_leq = 0,
};

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

	//s_info.layout_setup = view_create_layout_for_win(s_info.win, _create_resource_path(TIMER_SETUP_EDJ), "grp");
	//elm_layout_signal_callback_add(s_info.layout_setup, SIGNAL_PART_SELECTED, PART_HOUR, _selected_time_display_part_cb, NULL);
	//elm_layout_signal_callback_add(s_info.layout_setup, SIGNAL_PART_SELECTED, PART_MINUTE, _selected_time_display_part_cb, NULL);
	//elm_layout_signal_callback_add(s_info.layout_setup, SIGNAL_PART_SELECTED, PART_SECOND, _selected_time_display_part_cb, NULL);
	//elm_layout_signal_callback_add(s_info.layout_setup, SIGNAL_RESET_TIMER, "", _reset_timer_cb, NULL);

	//s_info.layout_run = view_create_layout_for_win(s_info.win, _create_resource_path(TIMER_RUN_EDJ), GRP_RUN);
	//elm_layout_signal_callback_add(s_info.layout_run, SIGNAL_SET_TIMER, "", _set_time_cb, NULL);

	//_init_rotary();

	evas_object_show(s_info.layout_setup);

	/* Show window after main view is set up */
	evas_object_show(s_info.win);

	return EINA_TRUE;
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

