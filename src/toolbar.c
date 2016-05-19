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

#include <Elementary.h>

#include "sticker_panel.h"
#include "sticker_panel_internal.h"
#include "icon_info.h"
#include "group_icon_info.h"
#include "conf.h"
#include "log.h"
#include "toolbar.h"

const char *const PRIVATE_DATA_KEY_EVENT_CALLBACK_LIST = "pdkec";
const char *const PRIVATE_DATA_TOOLBAR_ITEM = "pdti";
const char *const SETTING_ICON_IMG_PATH = "/usr/share/sticker-panel/images/sticker_tab_settings.png";



struct _event_cb {
	int event_type;
	void (*event_cb)(Evas_Object *toolbar, int event_type, void *event_info, void *user_data);
	void *user_data;
};
typedef struct _event_cb event_cb_s;



static void __resize_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Object *toolbar = obj;

	int x, y, w, h;

	ret_if(!toolbar);

	evas_object_geometry_get(toolbar, &x, &y, &w, &h);
	_D("toolbar resize(%d, %d, %d, %d)", x, y, w, h);
}



Evas_Object *_toolbar_create(Evas_Object *ui_manager, sticker_panel_h sticker_panel)
{
	Evas_Object *toolbar = NULL;

	retv_if(!ui_manager, NULL);
	retv_if(!sticker_panel, NULL);

	toolbar = elm_toolbar_add(ui_manager);
	goto_if(!toolbar, ERROR);

	/* This will expand the transverse(horizontal) length of items according to the length of toolbar */
	elm_toolbar_transverse_expanded_set(toolbar, EINA_TRUE);
	elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_SCROLL);
	elm_toolbar_homogeneous_set(toolbar, EINA_FALSE);
	elm_toolbar_select_mode_set(toolbar, ELM_OBJECT_SELECT_MODE_ALWAYS);

	evas_object_size_hint_weight_set(toolbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(toolbar);
	evas_object_data_set(toolbar, DATA_KEY_STICKER_PANEL_INFO, sticker_panel);

	evas_object_event_callback_add(toolbar, EVAS_CALLBACK_RESIZE, __resize_cb, NULL);

	return toolbar;

ERROR:
	_toolbar_destroy(toolbar);
	return NULL;
}



void _toolbar_destroy(Evas_Object *toolbar)
{
	sticker_panel_h sticker_panel = NULL;

	ret_if(!toolbar);

	sticker_panel = evas_object_data_del(toolbar, DATA_KEY_STICKER_PANEL_INFO);
	ret_if(!sticker_panel);

	evas_object_event_callback_del(toolbar, EVAS_CALLBACK_RESIZE, __resize_cb);
	evas_object_del(toolbar);
}



static void __view_changed_cb(void *user_data, Evas_Object *obj, void *event_info)
{
	Evas_Object *toolbar = obj;
	Evas_Object *item_view = user_data;
	Eina_List *event_cb_list = NULL;
	const Eina_List *l = NULL;
	const Eina_List *ln = NULL;
	event_cb_s *event_cb_info = NULL;

	ret_if(!toolbar);
	ret_if(!item_view);

	event_cb_list = evas_object_data_get(toolbar, PRIVATE_DATA_KEY_EVENT_CALLBACK_LIST);
	ret_if(!event_cb_list);

	EINA_LIST_FOREACH_SAFE(event_cb_list, l, ln, event_cb_info) {
		if (TOOLBAR_EVENT_TYPE_CHANGE_TAB == event_cb_info->event_type) {
			if (event_cb_info->event_cb) {
				event_cb_info->event_cb(toolbar, TOOLBAR_EVENT_TYPE_CHANGE_TAB, item_view, event_cb_info->user_data);
			}
		}
	}
}



Elm_Object_Item *_toolbar_append_item(Evas_Object *toolbar, const char *icon_path, Evas_Object *item_view)
{
	Elm_Object_Item *tab_item = NULL;

	retv_if(!toolbar, NULL);
	retv_if(!icon_path, NULL);

	tab_item = elm_toolbar_item_append(toolbar, icon_path, NULL, __view_changed_cb, item_view);
	retv_if(!tab_item, NULL);
	evas_object_data_set(item_view, PRIVATE_DATA_TOOLBAR_ITEM, tab_item);

	return tab_item;
}



Elm_Object_Item *_toolbar_append_setting_item(Evas_Object *toolbar, void (*cb_func)(void *user_data, Evas_Object *obj, void *event_info), sticker_panel_h sticker_panel)
{
	Elm_Object_Item *tab_item = NULL;

	retv_if(!toolbar, NULL);
	retv_if(!sticker_panel, NULL);

	tab_item = elm_toolbar_item_append(toolbar, SETTING_ICON_IMG_PATH, NULL, cb_func, sticker_panel);
	retv_if(!tab_item, NULL);

	return tab_item;
}



void _toolbar_remove_setting_item(Evas_Object *toolbar)
{
	Elm_Object_Item *setting_item = NULL;

	ret_if(!toolbar);

	setting_item = elm_toolbar_last_item_get(toolbar);
	ret_if(!setting_item);

	elm_object_item_del(setting_item);
}



void _toolbar_remove_item_by_item_view(Evas_Object *toolbar, Evas_Object *item_view)
{
	Elm_Object_Item *tabbar_item = NULL;

	ret_if(!toolbar);
	ret_if(!item_view);

	evas_object_data_del(item_view, PRIVATE_DATA_TOOLBAR_ITEM);

	tabbar_item = _toolbar_get_item(toolbar, item_view);
	if (tabbar_item) {
		elm_object_item_del(tabbar_item);
	}

	/* This data has to be kept because of _toolbar_get_item() */
	evas_object_data_del(item_view, PRIVATE_DATA_TOOLBAR_ITEM);

	/* FIXME : If the selected item is removed, we have to activate the current */
}



void _toolbar_remove_item(Elm_Object_Item *item)
{
	ret_if(!item);

	elm_object_item_del(item);
}



void _toolbar_remove_all_item(Evas_Object *toolbar)
{
	Elm_Object_Item *item = NULL;

	ret_if(!toolbar);

	item = elm_toolbar_first_item_get(toolbar);
	while (item) {
		elm_object_item_del(item);
		item = elm_toolbar_item_next_get(item);
	}
}



Elm_Object_Item *_toolbar_get_item(Evas_Object *toolbar, Evas_Object *item_view)
{
	Elm_Object_Item *tabbar_item = NULL;

	retv_if(!toolbar, NULL);
	retv_if(!item_view, NULL);

	tabbar_item = evas_object_data_get(item_view, PRIVATE_DATA_TOOLBAR_ITEM);
	retv_if(!tabbar_item, NULL);

	/* FIXME : check the tabbar is in the toolbar */

	return tabbar_item;
}



Elm_Object_Item *_toolbar_insert_item_before(Evas_Object *toolbar, const char *icon_path, Evas_Object *item_view, Elm_Object_Item *before)
{
	Elm_Object_Item *tab_item = NULL;
	retv_if(!toolbar, NULL);
	retv_if(!icon_path, NULL);
	retv_if(!item_view, NULL);
	retv_if(!before, NULL);

	tab_item = elm_toolbar_item_insert_before(toolbar, before, icon_path, NULL, __view_changed_cb, item_view);
	retv_if(!tab_item, NULL);
	evas_object_data_set(item_view, PRIVATE_DATA_TOOLBAR_ITEM, tab_item);

	return tab_item;
}



void _toolbar_reorder_item(Evas_Object *toolbar, Eina_List *group_icon_info_list)
{
	Elm_Object_Item *last_item = NULL;
	Eina_List *l = NULL;
	group_icon_info_s *group_icon_info = NULL;

	ret_if(!toolbar);
	ret_if(!group_icon_info_list);

	last_item = elm_toolbar_last_item_get(toolbar);
	ret_if(!last_item);

	EINA_LIST_FOREACH(group_icon_info_list, l, group_icon_info) {
		icon_info_s *icon_info = NULL;

		if (!group_icon_info) {
			continue;
		}

		if (group_icon_info->permutable == 0) {
			continue;
		}

		if (group_icon_info->toolbar_item) {
			_toolbar_remove_item(group_icon_info->toolbar_item);
			group_icon_info->toolbar_item = NULL;
		}

		icon_info = eina_list_nth(group_icon_info->list, 0);
		if (!icon_info) {
			_E("Failed to get icon info");
			continue;
		}

		group_icon_info->toolbar_item = _toolbar_insert_item_before(toolbar, icon_info->thumbnail_file, group_icon_info->item_view, last_item);
		if (!group_icon_info->toolbar_item) {
			_E("Failed to insert toolbar item");
			continue;
		}

	}

}



void _toolbar_bring_in(Evas_Object *toolbar, Elm_Object_Item *tabbar_item)
{
	Elm_Object_Item *selected_item = NULL;

	ret_if(!toolbar);
	ret_if(!tabbar_item);

	selected_item = elm_toolbar_selected_item_get(toolbar);
	if (selected_item && selected_item == tabbar_item) {
		return;
	}

	elm_toolbar_item_selected_set(tabbar_item, EINA_TRUE);
}



unsigned int _toolbar_count_item(Evas_Object *toolbar)
{
	retv_if(!toolbar, 0);

	return elm_toolbar_items_count(toolbar);
}



int _toolbar_register_event_cb(Evas_Object *toolbar, int event_type, void (*event_cb)(Evas_Object *toolbar, int event_type, void *event_info, void *user_data), void *user_data)
{
	Eina_List *event_cb_list = NULL;
	event_cb_s *event_cb_info = NULL;

	retv_if(!toolbar, STICKER_PANEL_ERROR_INVALID_PARAMETER);
	retv_if(event_type <= TOOLBAR_EVENT_TYPE_INVALID, STICKER_PANEL_ERROR_INVALID_PARAMETER);
	retv_if(event_type >= TOOLBAR_EVENT_TYPE_MAX, STICKER_PANEL_ERROR_INVALID_PARAMETER);
	retv_if(!event_cb, STICKER_PANEL_ERROR_INVALID_PARAMETER);

	event_cb_info = calloc(1, sizeof(event_cb_s));
	retv_if(!event_cb_info, STICKER_PANEL_ERROR_OUT_OF_MEMORY);

	event_cb_info->event_type = event_type;
	event_cb_info->event_cb = event_cb;
	event_cb_info->user_data = user_data;

	event_cb_list = evas_object_data_get(toolbar, PRIVATE_DATA_KEY_EVENT_CALLBACK_LIST);
	event_cb_list = eina_list_append(event_cb_list, event_cb_info);
	evas_object_data_set(toolbar, PRIVATE_DATA_KEY_EVENT_CALLBACK_LIST, event_cb_list);

	return STICKER_PANEL_ERROR_NONE;
}



int _toolbar_unregister_event_cb(Evas_Object *toolbar, int event_type, void (*event_cb)(Evas_Object *toolbar, int event_type, void *event_info, void *user_data))
{
	Eina_List *event_cb_list = NULL;
	const Eina_List *l = NULL;
	const Eina_List *ln = NULL;
	event_cb_s *event_cb_info = NULL;

	retv_if(!toolbar, STICKER_PANEL_ERROR_INVALID_PARAMETER);
	retv_if(event_type <= TOOLBAR_EVENT_TYPE_INVALID, STICKER_PANEL_ERROR_INVALID_PARAMETER);
	retv_if(event_type >= TOOLBAR_EVENT_TYPE_MAX, STICKER_PANEL_ERROR_INVALID_PARAMETER);
	retv_if(!event_cb, STICKER_PANEL_ERROR_INVALID_PARAMETER);

	event_cb_list = evas_object_data_get(toolbar, PRIVATE_DATA_KEY_EVENT_CALLBACK_LIST);
	retv_if(!event_cb_list, STICKER_PANEL_ERROR_NOT_INITIALIZED);

	EINA_LIST_FOREACH_SAFE(event_cb_list, l, ln, event_cb_info) {
		if (event_cb_info->event_type == event_type
			&& event_cb_info->event_cb == event_cb) {
			event_cb_list = eina_list_remove(event_cb_list, event_cb_info);
			break;
		}
	}

	evas_object_data_set(toolbar, PRIVATE_DATA_KEY_EVENT_CALLBACK_LIST, event_cb_list);

	return STICKER_PANEL_ERROR_NONE;
}
