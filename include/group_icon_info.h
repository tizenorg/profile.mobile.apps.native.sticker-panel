/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __TIZEN_STICKER_PANEL_GROUP_ICON_INFO_H__
#define __TIZEN_STICKER_PANEL_GROUP_ICON_INFO_H__

#include <Elementary.h>
#include <sqlite3.h>

struct _group_icon_info_s {
	/* innate features */
	char *id;
	char *name;
	char *repository;
	int ordering;
	int category;

	/* acquired features */
	Eina_List *list;
	Evas_Object *item_view;
	Elm_Object_Item *toolbar_item;
	char *toolbar_icon_path;
	int permutable;
	int initialized;
	int recent;
	int removable;
};
typedef struct _group_icon_info_s group_icon_info_s;

extern group_icon_info_s *_group_icon_info_create(const char *id, const char *name, const char *repository, int ordering, int category);
extern void _group_icon_info_destroy(group_icon_info_s *group_icon_info);

extern Eina_List *_group_icon_info_list_create(sqlite3 *db);
extern void _group_icon_info_list_destroy(Eina_List *group_icon_info_list);
extern Eina_List *_group_icon_info_list_sort(Eina_List *group_icon_info_list);
extern Eina_List *_group_icon_info_list_trim(Eina_List *group_icon_info_list, sqlite3 *db);
extern group_icon_info_s *_group_icon_info_list_get(Eina_List *group_icon_info_list, const char *id);

#endif /* __TIZEN_STICKER_PANEL_GROUP_ICON_INFO_H__ */
