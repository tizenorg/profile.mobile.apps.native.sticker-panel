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
#include <tizen.h>

#include "sticker_panel.h"
#include "sticker_panel_internal.h"
#include "conf.h"
#include "grid.h"
#include "group_icon_info.h"
#include "icon_info.h"
#include "log.h"
#include "page.h"
#include "scroller.h"
#include "setting.h"
#include "toolbar.h"

#define TRANSIT_DURATION 0.5f



static struct {
	Elm_Transit *transit;
	sticker_panel_state_e sticker_panel_state;
} gesture_info_s = {
	.transit = NULL,
	.sticker_panel_state = STICKER_PANEL_STATE_HIDE,
};

struct _effect_factor {
	Evas_Coord from_h;
	Evas_Coord to_h;
};
typedef struct _effect_factor effect_factor_s;



static Elm_Transit_Effect *__create_effect_factor(Evas_Coord from_h, Evas_Coord to_h)
{
	effect_factor_s *effect_factor = calloc(1, sizeof(effect_factor_s));

	retv_if(!effect_factor, NULL);

	effect_factor->from_h = from_h;
	effect_factor->to_h = to_h;

	return effect_factor;
}



static void __between_effect_factor(Elm_Transit_Effect *effect, Elm_Transit *transit, double progress)
{
	effect_factor_s *effect_factor = effect;
	const Eina_List *objs = NULL;
	const Eina_List *l = NULL;
	Evas_Object *obj = NULL;
	Evas_Coord h = 0;

	ret_if(!effect);

	objs = elm_transit_objects_get(transit);
	ret_if(!objs);

	h = (Evas_Coord) ((effect_factor->from_h * (1.0 - progress)) + (effect_factor->to_h * progress));

	EINA_LIST_FOREACH(objs, l, obj) {
		evas_object_size_hint_min_set(obj, -1, h);
		evas_object_size_hint_max_set(obj, -1, h);
	}
}



static void __destroy_effect_factor(Elm_Transit_Effect *effect, Elm_Transit *transit)
{
	free(effect);
}



static void __click_setting_icon_cb(void *user_data, Evas_Object *obj, void *event_info)
{
	sticker_panel_h sticker_panel = NULL;
	Elm_Object_Item *item = NULL;

	sticker_panel = user_data;
	ret_if(!sticker_panel);

	ret_if(!obj);

	if (!_setting_create(sticker_panel->conformant, sticker_panel)) {
		_E("Failed to create setting layout");
		return;
	}

	item = elm_toolbar_selected_item_get(obj);
	goto_if(!item, out);

	elm_toolbar_item_selected_set(item, EINA_FALSE);

out:
	elm_toolbar_item_selected_set(sticker_panel->current_tab, EINA_TRUE);
}



static Eina_Bool __animator_cb(void *data)
{
	group_icon_info_s *group_icon_info = NULL;
	sticker_panel_h sticker_panel = data;
	Elm_Object_Item *setting_item = NULL;
	Evas_Object *grid = NULL;

	const Eina_List *lm = NULL;
	icon_info_s *icon_info = NULL;
	static int i = 1;

	goto_if(!sticker_panel, out);
	goto_if(!sticker_panel->group_icon_info_list, out);

	if (i >= eina_list_count(sticker_panel->group_icon_info_list)) {
		/* UI Feature : Setting is only on the first category */
		setting_item = _toolbar_append_setting_item(sticker_panel->toolbar[0], __click_setting_icon_cb, sticker_panel);
		if (!setting_item) {
			_E("cannot append a tabbar setting item");
		}

		_D("List is done to reload");
		goto out;
	}

	group_icon_info = eina_list_nth(sticker_panel->group_icon_info_list, i);
	if (!group_icon_info->list) {
		group_icon_info->list = _icon_info_list_create_preset_package(group_icon_info->id);
		if (!group_icon_info->list)
			goto renew;
	}

	grid = elm_object_part_content_get(group_icon_info->item_view, "content");
	goto_if(!grid, renew);

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

out:
	sticker_panel->toolbar_animator = NULL;
	return ECORE_CALLBACK_CANCEL;
}



static void __del_transit_cb(void *data, Elm_Transit *transit)
{
	sticker_panel_h sticker_panel = data;

	gesture_info_s.transit = NULL;

	if (gesture_info_s.sticker_panel_state == STICKER_PANEL_STATE_HIDE) {
		elm_object_signal_emit(sticker_panel->conformant, "elm,state,attach_panel,hide,finished", "");

		if (sticker_panel->is_delete)
			_sticker_panel_del(sticker_panel);
	} else {
		elm_object_signal_emit(sticker_panel->conformant, "elm,state,attach_panel,show,finished", "");
		sticker_panel->toolbar_animator = ecore_animator_add(__animator_cb, sticker_panel);
		ret_if(!sticker_panel->toolbar_animator);
	}
}



static void __set_transit(sticker_panel_h sticker_panel, Evas_Coord from_h, Evas_Coord to_h, double duration)
{
	Elm_Transit_Effect *effect_factor = NULL;

	if (gesture_info_s.transit) {
		_E("Transit is already activating");
		return;
	}

	if (gesture_info_s.sticker_panel_state == STICKER_PANEL_STATE_HIDE) {
		elm_object_signal_emit(sticker_panel->conformant, "elm,state,attach_panel,hide,started", "");
	} else {
		elm_object_signal_emit(sticker_panel->conformant, "elm,state,attach_panel,show,started", "");
	}

	gesture_info_s.transit = elm_transit_add();
	ret_if(!gesture_info_s.transit);

	elm_transit_object_add(gesture_info_s.transit, sticker_panel->sticker_panel_rect);
	elm_transit_tween_mode_set(gesture_info_s.transit, ELM_TRANSIT_TWEEN_MODE_DECELERATE);
	elm_transit_smooth_set(gesture_info_s.transit, EINA_FALSE);
	elm_transit_duration_set(gesture_info_s.transit, duration);

	effect_factor = __create_effect_factor(from_h, to_h);
	if (!effect_factor) {
		_E("cannot create an effect factor");
		elm_transit_del(gesture_info_s.transit);
		gesture_info_s.transit = NULL;
		return;
	}
	elm_transit_effect_add(gesture_info_s.transit, __between_effect_factor, effect_factor, __destroy_effect_factor);
	elm_transit_del_cb_set(gesture_info_s.transit, __del_transit_cb, sticker_panel);
	elm_transit_go(gesture_info_s.transit);
}



void _gesture_show(sticker_panel_h sticker_panel)
{
	gesture_info_s.sticker_panel_state = STICKER_PANEL_STATE_HALF;

	if (sticker_panel->rotate) {
		sticker_panel->sticker_panel_land_state = STICKER_PANEL_STATE_FULL;
		__set_transit(sticker_panel, 0, sticker_panel->transit_height, TRANSIT_DURATION);
		elm_object_signal_emit(sticker_panel->conformant, "elm,state,attach_panel,show,full", "");
	} else {
		sticker_panel->sticker_panel_port_state = STICKER_PANEL_STATE_HALF;
		__set_transit(sticker_panel, 0, sticker_panel->transit_height, TRANSIT_DURATION);
	}
}



void _gesture_hide(sticker_panel_h sticker_panel)
{
	if (sticker_panel->rotate) {
		sticker_panel->sticker_panel_land_state = STICKER_PANEL_STATE_HIDE;
		elm_object_signal_emit(sticker_panel->conformant, "elm,state,attach_panel,show,half", "");
	} else {
		if (STICKER_PANEL_STATE_FULL == sticker_panel->sticker_panel_port_state) {
			elm_object_signal_emit(sticker_panel->conformant, "elm,state,attach_panel,show,half", "");
		}
		sticker_panel->sticker_panel_port_state = STICKER_PANEL_STATE_HIDE;
	}
	gesture_info_s.sticker_panel_state = STICKER_PANEL_STATE_HIDE;
	__set_transit(sticker_panel, sticker_panel->transit_height, 0, TRANSIT_DURATION);
}
