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

#include <app_control.h>
#include <Elementary.h>
#include <bundle.h>

#include "sticker_panel.h"
#include "sticker_panel_internal.h"
#include "group_icon_info.h"
#include "icon_info.h"
#include "grid.h"
#include "log.h"
#include "db.h"
#include "conf.h"



const char *const PRIVATE_DATA_KEY_GRID_INFO = "pdk_gi";
const char *const PRIVATE_DATA_KEY_ICON_INFO = "pdk_ii";
const char *const ITEM_EDJE_FILE = EDJEDIR"/item.edj";



struct _grid_info_s {
	Elm_Gengrid_Item_Class *gic;
	int item_width;
	int item_height;
	int icon_width;
	int icon_height;
};
typedef struct _grid_info_s grid_info_s;



static Evas_Object *__add_icon(Evas_Object *parent, const char *file, int item_width, int item_height, int icon_width, int icon_height)
{
	Evas *e = NULL;
	Evas_Object *item = NULL;
	Evas_Object *bg = NULL;
	Evas_Object *icon = NULL;

	e = evas_object_evas_get(parent);
	retv_if(!e, NULL);

	item = elm_layout_add(parent);
	retv_if(!item, NULL);
	elm_layout_file_set(item, ITEM_EDJE_FILE, "item");
	evas_object_size_hint_weight_set(item, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(item, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(item);

	bg = evas_object_rectangle_add(e);
	goto_if(!bg, error);
	evas_object_size_hint_min_set(bg, item_width, item_height);
	evas_object_color_set(bg, 0, 0, 0, 0);
	evas_object_show(bg);
	elm_object_part_content_set(item, "bg", bg);

	if (access(file, R_OK) != 0) {
		_E("Failed to access an icon(%s)", file);
		file = DEFAULT_ICON;
	}

	icon = elm_icon_add(parent);
	goto_if(!icon, error);
	goto_if(elm_image_file_set(icon, file, NULL) == EINA_FALSE, error);
	elm_image_preload_disabled_set(icon, EINA_TRUE);
	elm_image_smooth_set(icon, EINA_TRUE);
	elm_image_no_scale_set(icon, EINA_FALSE);
	evas_object_size_hint_min_set(icon, icon_width, icon_height);
	evas_object_show(icon);
	elm_object_part_content_set(item, "icon", icon);

	return item;

error:
	if (icon)
		evas_object_del(icon);

	if (bg)
		evas_object_del(bg);

	if (item)
		evas_object_del(item);

	return NULL;
}



static Evas_Object *__content_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *grid = obj;
	grid_info_s *grid_info = NULL;

	retv_if(!data, NULL);

	grid_info = evas_object_data_get(grid, PRIVATE_DATA_KEY_GRID_INFO);
	retv_if(!grid_info, NULL);

	if (!strcmp(part, "elm.swallow.icon")) {
		return __add_icon(obj
				, data
				, grid_info->item_width
				, grid_info->item_height
				, grid_info->icon_width
				, grid_info->icon_height);
	} else if (!strcmp(part, "selected")) {
	}

	return NULL;
}



static void __del(void *data, Evas_Object *obj)
{
	ret_if(NULL == data);
}



static void __refresh_recent_grid(Evas_Object *grid, const char *id, int type, sticker_panel_h sticker_panel)
{
	Elm_Object_Item *recent_item = NULL;
	icon_info_s *icon_info = NULL;
	int ret = STICKER_PANEL_ERROR_NONE;

	ret_if(!grid);
	ret_if(!id);
	ret_if(!sticker_panel);

	ret = _db_touch_recent_icon(sticker_panel->db, id, type);
	ret_if(ret != STICKER_PANEL_ERROR_NONE);

	recent_item = _grid_get_item(grid, id, type);
	if (recent_item) {
		icon_info = evas_object_data_get(recent_item, PRIVATE_DATA_KEY_ICON_INFO);
		_grid_remove_item(grid, recent_item);
	} else {
		group_icon_info_s *group_icon_info = _group_icon_info_list_get(sticker_panel->group_icon_info_list, RECENT_ID);
		ret_if(!group_icon_info);

		icon_info = _icon_info_create(id, type);
		ret_if(!icon_info);
		group_icon_info->list = eina_list_prepend(group_icon_info->list, icon_info);
	}

	recent_item = _grid_prepend_item(grid, sticker_panel, icon_info);
	if (!recent_item)
		_E("cannot prepend an item into the recent tab");
}



static void __select_item_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *selected_item = NULL;
	bundle *b = NULL;
	sticker_panel_h sticker_panel = data;
	icon_info_s *icon_info = NULL;
	char *result_type = NULL;

	ret_if(!obj);
	ret_if(!sticker_panel);
	ret_if(!sticker_panel->result_cb);

	selected_item = elm_gengrid_selected_item_get(obj);
	ret_if(!selected_item);
	elm_gengrid_item_selected_set(selected_item, EINA_FALSE);

	icon_info = evas_object_data_get(selected_item, PRIVATE_DATA_KEY_ICON_INFO);
	ret_if(!icon_info);

	_D("Selected an item : %s",  icon_info->thumbnail_file);

	switch (icon_info->type) {
	case ICON_INFO_TYPE_DIRECTORY:
		result_type = STICKER_PANEL_RESULT_TYPE_DIRECTORY;
		break;
	case ICON_INFO_TYPE_FILE:
		result_type = STICKER_PANEL_RESULT_TYPE_FILE;
		break;
	default:
		_E("Failed to set result type");
		return;
	}

	b = bundle_create();
	ret_if(!b);

	if (BUNDLE_ERROR_NONE != bundle_add_str(b, STICKER_PANEL_RESULT_TYPE, result_type))
		_E("Failed to add bundle(type)");
	if (BUNDLE_ERROR_NONE != bundle_add_str(b, STICKER_PANEL_RESULT_VALUE, icon_info->dir_path))
		_E("Failed to add bundle(path)");

	sticker_panel->result_cb(sticker_panel, b, sticker_panel->result_data);
	bundle_free(b);

	__refresh_recent_grid(sticker_panel->recent_grid
			, icon_info->dir_path
			, icon_info->type
			, sticker_panel);
}



Evas_Object *_grid_create(Evas_Object *page, sticker_panel_h sticker_panel)
{
	Evas_Object *grid = NULL;
	grid_info_s *grid_info = NULL;

	int w = 0;
    int icon_width = 0;

	retv_if(!sticker_panel, NULL);

	grid_info = calloc(1, sizeof(grid_info_s));
	retv_if(!grid_info, NULL);

	grid = elm_gengrid_add(page);
	goto_if(!grid, ERROR);

	evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(grid, EVAS_HINT_FILL, EVAS_HINT_FILL);

	w = sticker_panel->transit_width / GRID_ITEM_PER_ROW;
	goto_if(!w, ERROR);

	icon_width = (int) w * GRID_ITEM_ICON_RATIO;
	goto_if(!icon_width, ERROR);

	elm_gengrid_item_size_set(grid, w, w);
	elm_gengrid_align_set(grid, 0.0, 0.0);
	elm_gengrid_horizontal_set(grid, EINA_FALSE);
	elm_gengrid_multi_select_set(grid, EINA_FALSE);

	grid_info->gic = elm_gengrid_item_class_new();
	goto_if(!grid_info->gic, ERROR);
	grid_info->gic->func.content_get = __content_get;
	grid_info->gic->func.del = __del;

	grid_info->item_width = w;
	grid_info->item_height = w;
	grid_info->icon_width = icon_width;
	grid_info->icon_height = icon_width;

	evas_object_data_set(grid, PRIVATE_DATA_KEY_GRID_INFO, grid_info);
	evas_object_show(grid);

	return grid;

ERROR:
	_grid_destroy(grid);
	free(grid_info);
	return NULL;
}



void _grid_destroy(Evas_Object *grid)
{
	Eina_List *list = NULL;
	Elm_Object_Item *item = NULL;
	grid_info_s *grid_info = NULL;

	ret_if(!grid);

	grid_info = evas_object_data_del(grid, PRIVATE_DATA_KEY_GRID_INFO);
	ret_if(!grid_info);

	list = elm_grid_children_get(grid);
	EINA_LIST_FREE(list, item) {
		evas_object_data_del(item, PRIVATE_DATA_KEY_ICON_INFO);
		elm_object_item_del(item);
	}

	evas_object_del(grid);
}



Elm_Object_Item *_grid_prepend_item(Evas_Object *grid, sticker_panel_h sticker_panel, icon_info_s *icon_info)
{
	grid_info_s *grid_info = NULL;
	Elm_Object_Item *item = NULL;

	retv_if(!grid, NULL);
	retv_if(!sticker_panel, NULL);
	retv_if(!icon_info, NULL);

	grid_info = evas_object_data_get(grid, PRIVATE_DATA_KEY_GRID_INFO);
	retv_if(!grid_info, NULL);
	retv_if(!grid_info->gic, NULL);

	item = elm_gengrid_item_prepend(grid, grid_info->gic, icon_info->thumbnail_file, __select_item_cb, sticker_panel);
	retv_if(!item, NULL);

	evas_object_data_set(item, PRIVATE_DATA_KEY_ICON_INFO, icon_info);

	return item;
}



Elm_Object_Item *_grid_append_item(Evas_Object *grid, sticker_panel_h sticker_panel, icon_info_s *icon_info)
{
	grid_info_s *grid_info = NULL;
	Elm_Object_Item *item = NULL;

	retv_if(!grid, NULL);
	retv_if(!sticker_panel, NULL);
	retv_if(!icon_info, NULL);

	grid_info = evas_object_data_get(grid, PRIVATE_DATA_KEY_GRID_INFO);
	retv_if(!grid_info, NULL);
	retv_if(!grid_info->gic, NULL);

	item = elm_gengrid_item_append(grid, grid_info->gic, icon_info->thumbnail_file, __select_item_cb, sticker_panel);
	retv_if(!item, NULL);

	evas_object_data_set(item, PRIVATE_DATA_KEY_ICON_INFO, icon_info);

	return item;
}



void _grid_remove_item(Evas_Object *grid, Elm_Object_Item *item)
{
	ret_if(!grid);
	ret_if(!item);

	evas_object_data_del(item, PRIVATE_DATA_KEY_ICON_INFO);

	elm_object_item_del(item);
}



Elm_Object_Item *_grid_get_item(Evas_Object *grid, const char *id, int type)
{
	Elm_Object_Item *item = NULL;

	retv_if(!grid, NULL);
	retv_if(!id, NULL);

	item = elm_gengrid_first_item_get(grid);
	retv_if(!item, NULL);

	while (item) {
		icon_info_s *icon_info = evas_object_data_get(item, PRIVATE_DATA_KEY_ICON_INFO);
		continue_if(!icon_info);
		continue_if(!icon_info->dir_path);

		if (!strcmp(icon_info->dir_path, id)
				&& icon_info->type == type) {
			return item;
		}

		item = elm_gengrid_item_next_get(item);
	}

	return NULL;
}



int _grid_count_item(Evas_Object *grid)
{
	int count = 0;

	retv_if(!grid, 0);

	count = elm_gengrid_items_count(grid);

	return count;
}
