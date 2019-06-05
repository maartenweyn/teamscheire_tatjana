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



#endif /* VIEW_H_ */
