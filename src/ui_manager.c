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
#include <sys/types.h>
#include <dirent.h>

#include "sticker_panel.h"
#include "sticker_panel_internal.h"
#include "icon_info.h"
#include "group_icon_info.h"
#include "conf.h"
#include "log.h"
#include "page.h"
#include "scroller.h"
#include "toolbar.h"
#include "grid.h"
#include "setting.h"

#define FILE_LAYOUT_EDJ EDJEDIR"/layout.edj"
#define GROUP_LAYOUT "layout"



static void __resize_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Object *ui_manager = obj;
	Evas_Coord x = 0;
	Evas_Coord y = 0;
	Evas_Coord w = 0;
	Evas_Coord h = 0;

	ret_if(!ui_manager);

	evas_object_geometry_get(ui_manager, &x, &y, &w, &h);
	_D("ui_manager resize(%d, %d, %d, %d)", x, y, w, h);
}



static void __change_tab_cb(Evas_Object *toolbar, int event_type, void *event_info, void *data)
{
	sticker_panel_h sticker_panel = data;
	Evas_Object *page = event_info;

	ret_if(!sticker_panel);
	ret_if(!page);

	_scroller_bring_in_page(sticker_panel->scroller[sticker_panel->current_category], page, &sticker_panel->cur_page_no);
}



static void __scroll_cb(Evas_Object *scroller, int event_type, void *event_info, void *data)
{
	sticker_panel_h sticker_panel = data;
	group_icon_info_s *group_icon_info = NULL;
	const Eina_List *l = NULL;
	int index = (int) event_info;

	ret_if(!sticker_panel);

	EINA_LIST_FOREACH(sticker_panel->group_icon_info_list, l, group_icon_info) {
		if (group_icon_info->category != sticker_panel->current_category)
			continue;

		if (group_icon_info->ordering == index) {
			_toolbar_bring_in(sticker_panel->toolbar[group_icon_info->category]
					, group_icon_info->toolbar_item);
			return;
		}
	}

	_E("cannot find ordering[%d] in the list", index);
}



static void __destroy_scroller_pages(Evas_Object *scroller)
{
	Evas_Object *box = NULL;
	Evas_Object *page = NULL;
	Eina_List *list = NULL;

	ret_if(!scroller);

	box = elm_object_content_get(scroller);
	ret_if(!box);

	list = elm_box_children_get(box);
	ret_if(!list);

	EINA_LIST_FREE(list, page) {
		Evas_Object *grid = NULL;

		continue_if(!page);

		grid = elm_object_part_content_unset(page, "content");
		if (grid) {
			_grid_destroy(grid);
		}

		_page_destroy(page);
	}
}



static Eina_Bool __animator_cb(void *data)
{
	group_icon_info_s *group_icon_info = NULL;
	sticker_panel_h sticker_panel = data;
	Evas_Object *grid = NULL;

	const Eina_List *lm = NULL;
	icon_info_s *icon_info = NULL;
	static int i = 0;

	goto_if(!sticker_panel, out);
	goto_if(!sticker_panel->group_icon_info_list, out);

	group_icon_info = eina_list_nth(sticker_panel->group_icon_info_list, i);
	if (!group_icon_info)
		goto out;

	group_icon_info->item_view = _page_create(sticker_panel->scroller[group_icon_info->category]
					, sticker_panel->transit_width
					, (sticker_panel->transit_height - ELM_SCALE_SIZE(TOOLBAR_HEIGHT)));
	goto_if(!group_icon_info->item_view, out);
	_scroller_append_page(sticker_panel->scroller[group_icon_info->category], group_icon_info->item_view);

	group_icon_info->toolbar_item = _toolbar_append_item(sticker_panel->toolbar[group_icon_info->category]
			, group_icon_info->toolbar_icon_path
			, group_icon_info->item_view);
	goto_if(!group_icon_info->toolbar_item, error);

	grid = _grid_create(group_icon_info->item_view, sticker_panel);
	goto_if(!grid, error);
	elm_object_part_content_set(group_icon_info->item_view, "content", grid);

	if (group_icon_info->recent)
		sticker_panel->recent_grid = grid;

	if (!group_icon_info->list)
		goto renew;

	/* FIXME : We have an empty page for downloading stickers */
	EINA_LIST_FOREACH(group_icon_info->list, lm, icon_info) {
		if (icon_info->keyword && icon_info->thumbnail_file) {
			Elm_Object_Item *item = NULL;
			item = _grid_append_item(grid, sticker_panel, icon_info);
			goto_if(!item, renew);
		}
	}

renew:
	i++;
	return ECORE_CALLBACK_RENEW;

error:
	if (group_icon_info->toolbar_item) {
		_toolbar_remove_item(group_icon_info->toolbar_item);
		group_icon_info->toolbar_item = NULL;
	}

	_scroller_remove_page(sticker_panel->scroller[group_icon_info->category], group_icon_info->item_view);
	_page_destroy(group_icon_info->item_view);
	group_icon_info->item_view = NULL;

out:
	sticker_panel->animator = NULL;
	return ECORE_CALLBACK_CANCEL;
}



static void _mouse_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	_D("mouse is down");
}



Evas_Object *_ui_manager_create(sticker_panel_h sticker_panel)
{
	Evas_Object *ui_manager = NULL;
	int ret = STICKER_PANEL_ERROR_NONE;
	int i = 0;

	retv_if(!sticker_panel, NULL);
	retv_if(!sticker_panel->conformant, NULL);

	ui_manager = elm_layout_add(sticker_panel->conformant);
	retv_if(!ui_manager, NULL);
	elm_layout_file_set(ui_manager, FILE_LAYOUT_EDJ, GROUP_LAYOUT);
	evas_object_size_hint_weight_set(ui_manager, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ui_manager, EVAS_HINT_FILL, 1.0);
	evas_object_event_callback_add(ui_manager, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb, NULL);
	evas_object_show(ui_manager);

	evas_object_data_set(ui_manager, DATA_KEY_STICKER_PANEL_INFO, sticker_panel);
	evas_object_event_callback_add(ui_manager, EVAS_CALLBACK_RESIZE, __resize_cb, NULL);

	for (; i < CATEGORY_COUNT; i++) {
		sticker_panel->toolbar[i] = _toolbar_create(ui_manager, sticker_panel);
		goto_if(!sticker_panel->toolbar[i], ERROR);

		ret = _toolbar_register_event_cb(sticker_panel->toolbar[i], TOOLBAR_EVENT_TYPE_CHANGE_TAB, __change_tab_cb, sticker_panel);
		goto_if(STICKER_PANEL_ERROR_NONE != ret, ERROR);

		sticker_panel->scroller[i] = _scroller_create(ui_manager, sticker_panel);
		goto_if(!sticker_panel->scroller[i], ERROR);

		ret = _scroller_register_event_cb(sticker_panel->scroller[i], SCROLLER_EVENT_TYPE_SCROLL, __scroll_cb, sticker_panel);
		goto_if(STICKER_PANEL_ERROR_NONE != ret, ERROR);
	}

	/* Initialize with the first category */
	elm_object_part_content_set(ui_manager, "toolbar", sticker_panel->toolbar[0]);
	elm_object_part_content_set(ui_manager, "scroller", sticker_panel->scroller[0]);

	sticker_panel->group_icon_info_list = _group_icon_info_list_create(sticker_panel->db);
	goto_if(!sticker_panel->group_icon_info_list, ERROR);

	sticker_panel->animator = ecore_animator_add(__animator_cb, sticker_panel);
	if (!sticker_panel->animator) {
		_E("cannot add an animator");
	}

	return ui_manager;

ERROR:
	elm_object_part_content_unset(ui_manager, "scroller");
	elm_object_part_content_unset(ui_manager, "toolbar");

	for (i = 0; i < CATEGORY_COUNT; i++) {
		if (sticker_panel->scroller[i]) {
			ret = _scroller_unregister_event_cb(sticker_panel->scroller[i], SCROLLER_EVENT_TYPE_SCROLL, __scroll_cb);
			if (STICKER_PANEL_ERROR_NONE != ret) {
				_E("cannot unregiter event_cb for scroller");
			}

			_scroller_destroy(sticker_panel->scroller[i]);
		}

		if (sticker_panel->toolbar[i]) {
			ret = _toolbar_unregister_event_cb(sticker_panel->toolbar[i], TOOLBAR_EVENT_TYPE_CHANGE_TAB, __change_tab_cb);
			if (STICKER_PANEL_ERROR_NONE != ret) {
				_E("cannot unregiter event_cb for toolbar");
			}

			_toolbar_destroy(sticker_panel->toolbar[i]);
		}
	}

	if (ui_manager) {
		evas_object_data_del(ui_manager, DATA_KEY_STICKER_PANEL_INFO);
		evas_object_del(ui_manager);
	}

	return NULL;
}



void _ui_manager_destroy(Evas_Object *ui_manager)
{
	sticker_panel_h sticker_panel = NULL;
	int ret = STICKER_PANEL_ERROR_NONE;
	int i = 0;

	ret_if(!ui_manager);

	sticker_panel = evas_object_data_del(ui_manager, DATA_KEY_STICKER_PANEL_INFO);
	ret_if(!sticker_panel);

	if (sticker_panel->animator) {
		ecore_animator_del(sticker_panel->animator);
	}

	if (sticker_panel->group_icon_info_list) {
		_group_icon_info_list_destroy(sticker_panel->group_icon_info_list);
	}

	elm_object_part_content_unset(ui_manager, "scroller");
	elm_object_part_content_unset(ui_manager, "toolbar");

	for (; i < CATEGORY_COUNT; i++) {
		if (sticker_panel->scroller[i]) {
			ret = _scroller_unregister_event_cb(sticker_panel->scroller[i], SCROLLER_EVENT_TYPE_SCROLL, __scroll_cb);
			if (STICKER_PANEL_ERROR_NONE != ret) {
				_E("cannot unregiter event_cb for scroller");
			}

			__destroy_scroller_pages(sticker_panel->scroller[i]);
			_scroller_destroy(sticker_panel->scroller[i]);
		}

		if (sticker_panel->toolbar[i]) {
			ret = _toolbar_unregister_event_cb(sticker_panel->toolbar[i], TOOLBAR_EVENT_TYPE_CHANGE_TAB, __change_tab_cb);
			if (STICKER_PANEL_ERROR_NONE != ret) {
				_E("cannot unregiter event_cb for toolbar[i]");
			}

			_toolbar_remove_all_item(sticker_panel->toolbar[i]);
			_toolbar_destroy(sticker_panel->toolbar[i]);
		}
	}

	if (sticker_panel->ui_manager) {
		evas_object_event_callback_del(ui_manager, EVAS_CALLBACK_RESIZE, __resize_cb);
		evas_object_del(ui_manager);
	}
}



void _ui_manager_show_category(Evas_Object *ui_manager, sticker_panel_h sticker_panel, int category)
{
	Evas_Object *scroller = NULL;
	Evas_Object *toolbar = NULL;

	ret_if(!ui_manager);
	ret_if(!sticker_panel);
	ret_if(!sticker_panel->scroller[category]);
	ret_if(!sticker_panel->toolbar[category]);

	if (sticker_panel->current_category == category)
		return;
	else
		sticker_panel->current_category = category;

	scroller = elm_object_part_content_unset(ui_manager, "scroller");
	if (scroller)
		evas_object_hide(scroller);

	toolbar = elm_object_part_content_unset(ui_manager, "toolbar");
	if (toolbar)
		evas_object_hide(toolbar);

	evas_object_show(sticker_panel->scroller[category]);
	elm_object_part_content_set(ui_manager, "scroller", sticker_panel->scroller[category]);

	evas_object_show(sticker_panel->toolbar[category]);
	elm_object_part_content_set(ui_manager, "toolbar", sticker_panel->toolbar[category]);
}
