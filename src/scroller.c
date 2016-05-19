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
#include "group_icon_info.h"
#include "conf.h"
#include "log.h"
#include "page.h"
#include "scroller.h"

#define PRIVATE_DATA_KEY_SCROLLER_IS_SCROLLING "p_is_sc"
#define PRIVATE_DATA_KEY_SCROLLER_DRAG_START "p_dg_st"
#define PRIVATE_DATA_KEY_EVENT_CALLBACK_LIST "pdkec"



struct _event_cb {
	int event_type;
	void (*event_cb)(Evas_Object *scroller, int event_type, void *event_info, void *user_data);
	void *user_data;
};
typedef struct _event_cb event_cb_s;



int _scroller_is_scrolling(Evas_Object *scroller)
{
	retv_if(!scroller, 0);
	return (int)evas_object_data_get(scroller, PRIVATE_DATA_KEY_SCROLLER_IS_SCROLLING);
}



static void __anim_start_cb(void *data, Evas_Object *scroller, void *event_info)
{
	_D("start the scroller(%p) animation", scroller);
	evas_object_data_set(scroller, PRIVATE_DATA_KEY_SCROLLER_IS_SCROLLING, (void *)1);
}



static void __anim_stop_cb(void *data, Evas_Object *scroller, void *event_info)
{
	_D("stop the scroller(%p) animation", scroller);
	evas_object_data_del(scroller, PRIVATE_DATA_KEY_SCROLLER_IS_SCROLLING);
	evas_object_data_del(scroller, PRIVATE_DATA_KEY_SCROLLER_DRAG_START);
}



static void __drag_start_cb(void *data, Evas_Object *scroller, void *event_info)
{
	_D("start to drag the scroller(%p)", scroller);
	evas_object_data_set(scroller, PRIVATE_DATA_KEY_SCROLLER_DRAG_START, (void *)1);
}



static void __drag_stop_cb(void *data, Evas_Object *scroller, void *event_info)
{
	_D("stop to drag the scroller(%p)", scroller);
}



static void __scroll_cb(void *data, Evas_Object *scroller, void *event_info)
{
	sticker_panel_h sticker_panel = data;
	Eina_List *event_cb_list = NULL;
	const Eina_List *l = NULL;
	const Eina_List *ln = NULL;
	event_cb_s *event_cb_info = NULL;

	int index = 0;
	int x = 0;
	int page_w = 0;

	ret_if(!sticker_panel);
	ret_if(!scroller);

	if (!evas_object_data_get(scroller, PRIVATE_DATA_KEY_SCROLLER_DRAG_START)) {
		return;
	}

	event_cb_list = evas_object_data_get(scroller, PRIVATE_DATA_KEY_EVENT_CALLBACK_LIST);
	ret_if(!event_cb_list);

	elm_scroller_region_get(scroller, &x, NULL, NULL, NULL);
	elm_scroller_page_size_get(scroller, &page_w, NULL);

	index = x / page_w;
	x = x % page_w;
	if (x > (page_w / 2)) {
		index++;
	}

	sticker_panel->cur_page_no = index;

	EINA_LIST_FOREACH_SAFE(event_cb_list, l, ln, event_cb_info) {
		if (SCROLLER_EVENT_TYPE_SCROLL == event_cb_info->event_type) {
			if (event_cb_info->event_cb) {
				event_cb_info->event_cb(scroller, SCROLLER_EVENT_TYPE_SCROLL, (void *) index, event_cb_info->user_data);
			}
		}
	}
}



static void __resize_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Object *scroller = obj;
	sticker_panel_h sticker_panel = data;

	int x, y, w, h;
	int screen_w = 0, screen_h = 0;

	ret_if(!scroller);
	ret_if(!sticker_panel);

	elm_win_screen_size_get(sticker_panel->win, NULL, NULL, &screen_w, &screen_h);

	evas_object_geometry_get(scroller, &x, &y, &w, &h);
	_D("scroller resize(%d, %d, %d, %d)", x, y, w, h);

	if (w == screen_w || w == screen_h) {
		_D("screen size(%d, %d)", screen_w, screen_h);
		elm_scroller_page_show(scroller, sticker_panel->cur_page_no, 0);
	}
}



Evas_Object *_scroller_create(Evas_Object *ui_manager, sticker_panel_h sticker_panel)
{
	Evas_Object *box = NULL;
	Evas_Object *scroller = NULL;
	int width = 0;

	retv_if(!ui_manager, NULL);
	retv_if(!sticker_panel, NULL);

	scroller = elm_scroller_add(ui_manager);
	retv_if(!scroller, NULL);

	elm_scroller_bounce_set(scroller, EINA_TRUE, EINA_FALSE);
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
	elm_scroller_page_scroll_limit_set(scroller, 1, 1);
	elm_scroller_content_min_limit(scroller, EINA_FALSE, EINA_TRUE);
	elm_scroller_single_direction_set(scroller, ELM_SCROLLER_SINGLE_DIRECTION_HARD);

	elm_scroller_page_size_set(scroller, sticker_panel->transit_width, sticker_panel->transit_height - ELM_SCALE_SIZE(TOOLBAR_HEIGHT));
	evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_min_set(scroller, width, 0);

	elm_object_style_set(scroller, "effect");
	evas_object_show(scroller);
	elm_object_scroll_lock_y_set(scroller, EINA_TRUE);

	evas_object_smart_callback_add(scroller, "scroll,anim,start", __anim_start_cb, NULL);
	evas_object_smart_callback_add(scroller, "scroll,anim,stop", __anim_stop_cb, NULL);
	evas_object_smart_callback_add(scroller, "scroll,drag,start", __drag_start_cb, NULL);
	evas_object_smart_callback_add(scroller, "scroll,drag,stop", __drag_stop_cb, NULL);
	evas_object_smart_callback_add(scroller, "scroll", __scroll_cb, sticker_panel);
	evas_object_event_callback_add(scroller, EVAS_CALLBACK_RESIZE, __resize_cb, sticker_panel);

	box = elm_box_add(scroller);
	goto_if(!box, ERROR);

	elm_box_horizontal_set(box, EINA_TRUE);
	elm_box_align_set(box, 0.5, 0.0);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(box);

	elm_object_content_set(scroller, box);
	sticker_panel->cur_page_no = 0;

	return scroller;

ERROR:
	if (scroller) {
		evas_object_del(scroller);
	}
	return NULL;
}



void _scroller_destroy(Evas_Object *scroller)
{
	Evas_Object *box = NULL;
	ret_if(!scroller);

	evas_object_event_callback_del(scroller, EVAS_CALLBACK_RESIZE, __resize_cb);
	box = elm_object_content_unset(scroller);
	if (box) {
		evas_object_del(box);
	}

	evas_object_del(scroller);
}



void _scroller_append_page(Evas_Object *scroller, Evas_Object *page)
{
	Evas_Object *box = NULL;

	box = elm_object_content_get(scroller);
	ret_if(!box);

	elm_box_pack_end(box, page);
}



void _scroller_remove_page(Evas_Object *scroller, Evas_Object *page)
{
	Evas_Object *box = NULL;
	Evas_Object *tmp = NULL;
	Eina_List *list = NULL;

	int is_page_exist = 0;

	box = elm_object_content_get(scroller);
	ret_if(!box);

	list = elm_box_children_get(box);
	EINA_LIST_FREE(list, tmp) {
		continue_if(!tmp);
		if (page == tmp) {
			is_page_exist = 1;
		}
	}

	if (!is_page_exist) {
		_D("No page to remove");
		return;
	}

	elm_box_unpack(box, page);
}



void _scroller_reorder_page(Evas_Object *scroller, Eina_List *group_icon_info_list)
{
	Eina_List *l = NULL;
	group_icon_info_s *group_icon_info = NULL;

	ret_if(!scroller);
	ret_if(!group_icon_info_list);

	EINA_LIST_FOREACH(group_icon_info_list, l, group_icon_info) {
		if (!group_icon_info) {
			continue;
		}

		if (group_icon_info->permutable == 0) {
			continue;
		}

		_scroller_remove_page(scroller, group_icon_info->item_view);
		_scroller_append_page(scroller, group_icon_info->item_view);
	}
}



void _scroller_bring_in_page(Evas_Object *scroller, Evas_Object *page, int *cur_page_no)
{
	Evas_Object *box = NULL;
	Evas_Object *tmp = NULL;
	Eina_List *list = NULL;
	int index = 0;
	int exist = 1;

	ret_if(!scroller);
	ret_if(!page);

	box = elm_object_content_get(scroller);
	ret_if(!box);

	list = elm_box_children_get(box);
	ret_if(!list);

	EINA_LIST_FREE(list, tmp) {
		continue_if(!tmp);
		if (page == tmp) {
			exist = 0;
		}
		index += exist;
	}

	if (exist) {
		index = 0;
	}

	*cur_page_no = index;

	elm_scroller_page_bring_in(scroller, index, 0);
}



void _scroller_resize(Evas_Object *scroller, int width, int height)
{
	Evas_Object *box = NULL;
	Evas_Object *page = NULL;
	Eina_List *list = NULL;

	ret_if(!scroller);

	elm_scroller_page_size_set(scroller, width, height);

	box = elm_object_content_get(scroller);
	ret_if(!box);

	list = elm_box_children_get(box);
	ret_if(!list);

	EINA_LIST_FREE(list, page) {
		_page_resize(page, width, height);
	}
}



int _scroller_register_event_cb(Evas_Object *scroller, int event_type, void (*event_cb)(Evas_Object *scroller, int event_type, void *event_info, void *user_data), void *user_data)
{
	Eina_List *event_cb_list = NULL;
	event_cb_s *event_cb_info = NULL;

	retv_if(!scroller, STICKER_PANEL_ERROR_INVALID_PARAMETER);
	retv_if(event_type <= SCROLLER_EVENT_TYPE_INVALID, STICKER_PANEL_ERROR_INVALID_PARAMETER);
	retv_if(event_type >= SCROLLER_EVENT_TYPE_MAX, STICKER_PANEL_ERROR_INVALID_PARAMETER);
	retv_if(!event_cb, STICKER_PANEL_ERROR_INVALID_PARAMETER);

	event_cb_info = calloc(1, sizeof(event_cb_s));
	retv_if(!event_cb_info, STICKER_PANEL_ERROR_OUT_OF_MEMORY);

	event_cb_info->event_type = event_type;
	event_cb_info->event_cb = event_cb;
	event_cb_info->user_data = user_data;

	event_cb_list = evas_object_data_get(scroller, PRIVATE_DATA_KEY_EVENT_CALLBACK_LIST);
	event_cb_list = eina_list_append(event_cb_list, event_cb_info);
	evas_object_data_set(scroller, PRIVATE_DATA_KEY_EVENT_CALLBACK_LIST, event_cb_list);

	return STICKER_PANEL_ERROR_NONE;
}



int _scroller_unregister_event_cb(Evas_Object *scroller, int event_type, void (*event_cb)(Evas_Object *scroller, int event_type, void *event_info, void *user_data))
{
	Eina_List *event_cb_list = NULL;
	const Eina_List *l = NULL;
	const Eina_List *ln = NULL;
	event_cb_s *event_cb_info = NULL;

	retv_if(!scroller, STICKER_PANEL_ERROR_INVALID_PARAMETER);
	retv_if(event_type <= SCROLLER_EVENT_TYPE_INVALID, STICKER_PANEL_ERROR_INVALID_PARAMETER);
	retv_if(event_type >= SCROLLER_EVENT_TYPE_MAX, STICKER_PANEL_ERROR_INVALID_PARAMETER);
	retv_if(!event_cb, STICKER_PANEL_ERROR_INVALID_PARAMETER);

	event_cb_list = evas_object_data_get(scroller, PRIVATE_DATA_KEY_EVENT_CALLBACK_LIST);
	retv_if(!event_cb_list, STICKER_PANEL_ERROR_NOT_INITIALIZED);

	EINA_LIST_FOREACH_SAFE(event_cb_list, l, ln, event_cb_info) {
		if (event_cb_info->event_type == event_type
			&& event_cb_info->event_cb == event_cb) {
			event_cb_list = eina_list_remove(event_cb_list, event_cb_info);
			break;
		}
	}

	evas_object_data_set(scroller, PRIVATE_DATA_KEY_EVENT_CALLBACK_LIST, event_cb_list);

	return STICKER_PANEL_ERROR_NONE;
}
