/*
 * view.h
 *
 *  Created on: May 28, 2019
 *      Author: maartenweyn
 */

#ifndef VIEW_H_
#define VIEW_H_

#include <Elementary.h>
#include <efl_extension.h>

Eina_Bool view_create(void);
Evas_Object *view_create_win(const char *pkg_name);
Evas_Object *view_create_layout_for_win(Evas_Object *win, const char *file_path, const char *group_name);

void updates_values(int current, int corrected, int avg_min, int avg_hour, int avg_8hour, int avg_day);
void set_running_status(int running);

#endif /* VIEW_H_ */
