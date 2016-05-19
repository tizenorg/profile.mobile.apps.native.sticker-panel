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

#ifndef __TIZEN_STICKER_PANEL_ICON_INFO_H__
#define __TIZEN_STICKER_PANEL_ICON_INFO_H__

typedef enum {
	ICON_INFO_TYPE_NONE = 0,
	ICON_INFO_TYPE_DIRECTORY = 1,
	ICON_INFO_TYPE_FILE,
	ICON_INFO_TYPE_ID,
	ICON_INFO_TYPE_MAX,
} icon_info_type_e;

typedef struct _icon_info_s {
	/* Primary key */
	char *dir_path;
	int type;

	/* Innate features */
	char *keyword;
	char *thumbnail_file;
	int repeat;
	int interval;
	int play_type;
	int th_frame;
	int width;
	int height;
	Eina_List *icon_info_image_list;
	Eina_Bool animation;

	/* Acquired features */
	Elm_Object_Item *tabbar_item;
	Evas_Object *icon;
} icon_info_s;

typedef struct _icon_info_image_s {
	char *file;
	int frame_order;
	int diff_time;
} icon_info_image_s;

extern icon_info_s *_icon_info_create(const char *id, int type);
extern icon_info_s *_icon_info_create_thumbnail(const char *thumbnail_file);
extern void _icon_info_destroy(icon_info_s *icon_info);

extern Eina_List *_icon_info_list_create_preset_package(const char *dir_path);
extern Eina_List *_icon_info_list_create_user_defined(const char *dir_path);
extern Eina_List *_icon_info_list_create_recent_package(sqlite3 *db);
extern void _icon_info_list_destroy(Eina_List *list);

#endif /* __TIZEN_STICKER_PANEL_ICON_INFO_H__ */
