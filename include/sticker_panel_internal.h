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

#ifndef __TIZEN_STICKER_PANEL_INTERNAL_H__
#define __TIZEN_STICKER_PANEL_INTERNAL_H__

#include <app_control.h>
#include <Elementary.h>
#include <sqlite3.h>
#include "sticker_panel.h"
#include "conf.h"

#define DATA_KEY_STICKER_PANEL "__dkap__"
#define DATA_KEY_STICKER_PANEL_INFO "__dkapi__"
#define DATA_KEY_PAGE "__dkpg__"
#define DATA_KEY_UG "__dkug__"
#define DEFAULT_ICON "/usr/share/sticker-panel/images/sticker_tab_latest.png"
#define RECENT_ID "__sticker_panel_recent_tabbar__"
#define TAB_THUMBNAIL_DIR_PATH "@Tab_Thumbnail"
#define FILE_LEN 1024



typedef enum {
	STICKER_PANEL_STATE_HIDE,
	STICKER_PANEL_STATE_HALF,
	STICKER_PANEL_STATE_FULL,
} sticker_panel_state_e;



struct _sticker_panel {
	/* Innate features */
	Evas_Object *win;
	Evas_Object *conformant;

	/* Acquired features */
	sqlite3 *db;
	Evas_Object *sticker_panel_rect;
	Evas_Object *ui_manager;
	Evas_Object *current_tab;
	Evas_Object *recent_grid;
	Evas_Object *toolbar[CATEGORY_COUNT];
	Evas_Object *scroller[CATEGORY_COUNT];
	int cur_page_no;

	Eina_List *group_icon_info_list;
	Ecore_Animator *animator;
	Ecore_Animator *toolbar_animator;

	sticker_panel_state_e sticker_panel_port_state;
	sticker_panel_state_e sticker_panel_land_state;
	sticker_panel_state_e sticker_panel_state;
	sticker_panel_result_cb result_cb;
	void *result_data;

	Evas_Coord transit_width;
	Evas_Coord transit_height;

	Eina_Bool is_delete;
	Eina_Bool rotate;
	Eina_Bool flick;
	int view_mode;
	int current_category;
};
typedef struct _sticker_panel sticker_panel_s;



extern void _sticker_panel_del(sticker_panel_h sticker_panel);



#endif // __TIZEN_STICKER_PANEL_INTERNAL_H__

