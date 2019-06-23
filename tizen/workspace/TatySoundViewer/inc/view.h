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

void updates_current_values(char* raw, char* leq);
void updates_values(char* leq_min, char* leq_hour, char* leq_8hours, char* leq_day, int network_status);
void set_running_status(int running);
void set_token(char* token_id);

#endif /* VIEW_H_ */
