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
#include <isf_control.h>
#include <tizen.h>

#include "sticker_panel.h"
#include "sticker_panel_internal.h"
#include "conf.h"
#include "ui_manager.h"
#include "gesture.h"
#include "log.h"
#include "db.h"
#include "scroller.h"



const char *const STICKER_PANEL_DOMAIN = "share-panel";
const char *const DIRECTORY_INHOUSE = PANELDIR"/res/images";
const char *const DIRECTORY_3RD_PARTY = "/opt/"PANELDIR"/res/images";


static void __resize_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	int width = 0;
	int height = 0;

	evas_object_geometry_get(obj, NULL, NULL, &width, &height);
	_D("The rectangle of Sticker-panel (%d:%d)", width, height);
}



static Evas_Object *__create_sticker_panel_rect(Evas_Object *conformant, int height)
{
	Evas_Object *rect = NULL;

	retv_if(!conformant, NULL);

	_D("sticker panel rect create");

	rect = evas_object_rectangle_add(evas_object_evas_get(conformant));
	retv_if(!rect, NULL);

	evas_object_size_hint_weight_set(rect, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(rect, EVAS_HINT_FILL, 1.0);
	evas_object_size_hint_min_set(rect, 0, height);
	evas_object_color_set(rect, 0, 0, 0, 0);
	evas_object_show(rect);

	evas_object_event_callback_add(rect, EVAS_CALLBACK_RESIZE, __resize_cb, NULL);

	return rect;
}



static void __destroy_sticker_panel_rect(Evas_Object *rect)
{
	ret_if(!rect);
	evas_object_del(rect);
}



static void __rotate_cb(void *data, Evas_Object *obj, void *event)
{
	Evas_Object *win = obj;
	sticker_panel_h sticker_panel = data;

	int w, h;
	int angle = 0;
	int height = 0;
	int i = 0;
	int temp = 0;

	ret_if(!sticker_panel);
	ret_if(!sticker_panel->conformant);

	angle = elm_win_rotation_get(win);
	elm_win_screen_size_get(win, NULL, NULL, &w, &h);

	switch (angle) {
	case 90:
	case 270:
		temp = w;
		w = h;
		h = temp;
		break;
	case 0:
	case 180:
		break;
	default:
		_E("cannot reach here");
		break;
	}

	_D("Angle is %d degree, win size is %d, %d", angle, w, h);

	isf_control_get_recent_ime_geometry(NULL, NULL, NULL, &height);
	if (!height) {
		height = h;
	}

	_D("isf height : %d, toolbar : %d", height, ELM_SCALE_SIZE(TOOLBAR_HEIGHT));

	for (; i < CATEGORY_COUNT; i++) {
		_scroller_resize(sticker_panel->scroller[i], w, height - ELM_SCALE_SIZE(TOOLBAR_HEIGHT));
		elm_scroller_page_show(sticker_panel->scroller[i], sticker_panel->cur_page_no, 0);
	}
}



EXPORT_API int sticker_panel_create(Evas_Object *conformant, sticker_panel_h *sticker_panel)
{
	sticker_panel_h panel = NULL;
	Evas_Object *win = NULL;
	Evas_Object *old_panel = NULL;
	const char *type = NULL;
	int width = 0;
	int height = 0;
	int base_height = 0;
	int count = 0;

	retv_if(!sticker_panel, STICKER_PANEL_ERROR_INVALID_PARAMETER);
	retv_if(!conformant, STICKER_PANEL_ERROR_INVALID_PARAMETER);

	old_panel = elm_object_part_content_get(conformant, "elm.swallow.attach_panel");
	retv_if(old_panel, STICKER_PANEL_ERROR_ALREADY_EXISTS);

	type = elm_object_widget_type_get(conformant);
	retv_if(!type, STICKER_PANEL_ERROR_INVALID_PARAMETER);

	if (strcmp(type, "Elm_Conformant")) {
		_E("No conformant, %s", elm_object_widget_type_get(conformant));
		return STICKER_PANEL_ERROR_INVALID_PARAMETER;
	} else {
		Evas_Object *parent = conformant;
		do {
			const char *type = NULL;

			parent = elm_object_parent_widget_get(parent);
			break_if(!parent);

			type = elm_object_widget_type_get(parent);
			break_if(!type);
			if (!strcmp(type, "Elm_Win")) {
				win = parent;
				break;
			}
		} while (parent);
		retv_if(!win, STICKER_PANEL_ERROR_INVALID_PARAMETER);
	}

	bindtextdomain(STICKER_PANEL_DOMAIN, LOCALEDIR);

	panel = calloc(1, sizeof(sticker_panel_s));
	retv_if(!panel, STICKER_PANEL_ERROR_OUT_OF_MEMORY);
	panel->win = win;
	panel->conformant = conformant;

	panel->db = _db_open();
	goto_if(!panel->db, error);
	goto_if(_db_create_table(panel->db) != STICKER_PANEL_ERROR_NONE, error);
	goto_if(_db_count_group_icon(panel->db, NULL, &count) != STICKER_PANEL_ERROR_NONE, error);
	if (!count)
		goto_if(_db_initialize_group_icon(panel->db) != STICKER_PANEL_ERROR_NONE, error);

	evas_object_geometry_get(conformant, NULL, NULL, &width, &base_height);
	panel->transit_width = width;

	if (width > base_height) {
		base_height = width * BASE_TRANSIT_HEIGHT_REL;
	} else {
		base_height = base_height * BASE_TRANSIT_HEIGHT_REL;
	}

	isf_control_get_recent_ime_geometry(NULL, NULL, NULL, &height);
	if (!height || height < base_height) {
		_D("Fail to get the recent ime height");
		height = base_height;
	}
	panel->transit_height = height;

	panel->sticker_panel_rect = __create_sticker_panel_rect(conformant, height);
	goto_if(!panel->sticker_panel_rect, error);
	elm_object_part_content_set(conformant, "elm.swallow.attach_panel_base", panel->sticker_panel_rect);

	panel->ui_manager = _ui_manager_create(panel);
	goto_if(!panel->ui_manager, error);
	elm_object_part_content_set(conformant, "elm.swallow.attach_panel", panel->ui_manager);

	evas_object_smart_callback_add(panel->win, "wm,rotation,changed", __rotate_cb, panel);
	__rotate_cb(panel, panel->win, NULL);

	panel->is_delete = EINA_FALSE;
	*sticker_panel = panel;

	return STICKER_PANEL_ERROR_NONE;

error:
	if (panel->sticker_panel_rect) {
		elm_object_part_content_unset(conformant, "elm.swallow.attach_panel_base");
		__destroy_sticker_panel_rect(panel->sticker_panel_rect);
	}
	if (panel->db) {
		_db_close(panel->db);
	}
	free(panel);

	return STICKER_PANEL_ERROR_NOT_INITIALIZED;
}



void _sticker_panel_del(sticker_panel_h sticker_panel)
{
	ret_if(!sticker_panel);

	if (sticker_panel->animator) {
		ecore_animator_del(sticker_panel->animator);
		sticker_panel->animator = NULL;
	}

	if (sticker_panel->toolbar_animator) {
		ecore_animator_del(sticker_panel->toolbar_animator);
		sticker_panel->toolbar_animator = NULL;
	}

	elm_object_part_content_unset(sticker_panel->conformant, "elm.swallow.attach_panel_base");
	__destroy_sticker_panel_rect(sticker_panel->sticker_panel_rect);

	elm_object_part_content_unset(sticker_panel->conformant, "elm.swallow.attach_panel");
	_ui_manager_destroy(sticker_panel->ui_manager);

	evas_object_smart_callback_del(sticker_panel->win, "wm,rotation,changed", __rotate_cb);

	if (sticker_panel->db) {
		_db_close(sticker_panel->db);
	}

	free(sticker_panel);
}



EXPORT_API int sticker_panel_destroy(sticker_panel_h sticker_panel)
{
	retv_if(!sticker_panel, STICKER_PANEL_ERROR_INVALID_PARAMETER);

	if (EINA_TRUE == sticker_panel->is_delete) {
		_E("Sticker panel is already removed");
		return STICKER_PANEL_ERROR_ALREADY_REMOVED;
	}

	if (STICKER_PANEL_STATE_HIDE == sticker_panel->sticker_panel_state) {
		_sticker_panel_del(sticker_panel);
	} else {
		sticker_panel->is_delete = EINA_TRUE;
	}

	return STICKER_PANEL_ERROR_NONE;
}



EXPORT_API int sticker_panel_set_result_cb(sticker_panel_h sticker_panel, sticker_panel_result_cb result_cb, void *user_data)
{
	retv_if(!sticker_panel, STICKER_PANEL_ERROR_INVALID_PARAMETER);
	retv_if(!result_cb, STICKER_PANEL_ERROR_INVALID_PARAMETER);

	if (EINA_TRUE == sticker_panel->is_delete) {
		_E("Sticker panel is already removed");
		return STICKER_PANEL_ERROR_ALREADY_REMOVED;
	}

	sticker_panel->result_cb = result_cb;
	sticker_panel->result_data = user_data;

	return STICKER_PANEL_ERROR_NONE;
}



EXPORT_API int sticker_panel_unset_result_cb(sticker_panel_h sticker_panel)
{
	retv_if(!sticker_panel, STICKER_PANEL_ERROR_INVALID_PARAMETER);

	if (EINA_TRUE == sticker_panel->is_delete) {
		_E("Sticker panel is already removed");
		return STICKER_PANEL_ERROR_ALREADY_REMOVED;
	}

	sticker_panel->result_cb = NULL;
	sticker_panel->result_data = NULL;

	return STICKER_PANEL_ERROR_NONE;
}


EXPORT_API int sticker_panel_set_view_mode(sticker_panel_h sticker_panel, sticker_panel_view_mode_e view_mode)
{
	retv_if(!sticker_panel, STICKER_PANEL_ERROR_INVALID_PARAMETER);
	retv_if(view_mode < STICKER_PANEL_VIEW_MODE_HALF, STICKER_PANEL_ERROR_INVALID_PARAMETER);
	retv_if(view_mode > STICKER_PANEL_VIEW_MODE_FULL, STICKER_PANEL_ERROR_INVALID_PARAMETER);

	if (EINA_TRUE == sticker_panel->is_delete) {
		_E("Sticker panel is already removed");
		return STICKER_PANEL_ERROR_ALREADY_REMOVED;
	}

	sticker_panel->view_mode = view_mode;

	return STICKER_PANEL_ERROR_NONE;
}

EXPORT_API int sticker_panel_show(sticker_panel_h sticker_panel)
{
	retv_if(!sticker_panel, STICKER_PANEL_ERROR_INVALID_PARAMETER);
	retv_if(!sticker_panel->ui_manager, STICKER_PANEL_ERROR_INVALID_PARAMETER);

	if (EINA_TRUE == sticker_panel->is_delete) {
		_E("Sticker panel is already removed");
		return STICKER_PANEL_ERROR_ALREADY_REMOVED;
	}

	evas_object_show(sticker_panel->ui_manager);
	_gesture_show(sticker_panel);

	return STICKER_PANEL_ERROR_NONE;
}



EXPORT_API int sticker_panel_hide(sticker_panel_h sticker_panel)
{
	retv_if(!sticker_panel, STICKER_PANEL_ERROR_INVALID_PARAMETER);
	retv_if(!sticker_panel->ui_manager, STICKER_PANEL_ERROR_INVALID_PARAMETER);

	if (EINA_TRUE == sticker_panel->is_delete) {
		_E("Sticker panel is already removed");
		return STICKER_PANEL_ERROR_ALREADY_REMOVED;
	}

	_gesture_hide(sticker_panel);

	return STICKER_PANEL_ERROR_NONE;
}
