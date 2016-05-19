/*
 * Samsung API
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <app.h>
#include <app_control.h>
#include <Elementary.h>
#include <efl_extension.h>
#include <bundle.h>

#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

#include "sticker_panel.h"
#include "sticker_icon.h"
#include "sample.h"
#include "log.h"

const char *const KEY_BACK = "XF86Back";
const char *const LAYOUT = "/usr/share/sticker-panel/sample/sample.edj";


static struct {
	Evas_Object *win;
	Evas_Object *layout;
	Evas_Object *bg;
	Evas_Object *conformant;
	Evas_Object *icon;
	sticker_panel_h sticker_panel;

	int root_w;
	int root_h;
} sample_info = {
	.win = NULL,
	.layout = NULL,
	.bg = NULL,
	.conformant = NULL,
	.icon = NULL,
	.sticker_panel = NULL,

	.root_w = 0,
	.root_h = 0,
};



static void _rotate_cb(void *data, Evas_Object *obj, void *event)
{
	ret_if(!obj);

	int angle = 0;

	angle = elm_win_rotation_get(obj);
	_D("Angle is %d degree", angle);

	switch (angle) {
	case 0:
	case 180:
		evas_object_size_hint_min_set(obj, sample_info.root_w, sample_info.root_h);
		evas_object_resize(obj, sample_info.root_w, sample_info.root_h);
		evas_object_move(obj, 0, 0);
		break;
	case 90:
	case 270:
		evas_object_size_hint_min_set(obj, sample_info.root_h, sample_info.root_w);
		evas_object_resize(obj, sample_info.root_h, sample_info.root_w);
		evas_object_move(obj, 0, 0);
		break;
	default:
		_E("cannot reach here");
	}
}



static void _win_back_key_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (sample_info.sticker_panel) {
		sticker_panel_hide(sample_info.sticker_panel);
		sticker_panel_destroy(sample_info.sticker_panel);
		sample_info.sticker_panel = NULL;
	} else {
		ui_app_exit();
	}
}



static void _bg_up_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	if (sample_info.sticker_panel) {
		sticker_panel_hide(sample_info.sticker_panel);
		sticker_panel_destroy(sample_info.sticker_panel);
		sample_info.sticker_panel = NULL;
	}
}


static int _get_result_type(const char *result_type)
{
	int type = 0;

	retv_if(!result_type, -1);

	if (!strcmp(result_type, STICKER_PANEL_RESULT_TYPE_DIRECTORY)) {
		type = ICON_INFO_TYPE_DIRECTORY;
	} else if (!strcmp(result_type, STICKER_PANEL_RESULT_TYPE_FILE)) {
		type = ICON_INFO_TYPE_FILE;
	} else {
		_E("Failed to get result type(%s)", result_type);
		return -1;
	}

	_D("result type(%s) : %d", result_type, type);

	return type;
}



static void _result_cb(sticker_panel_h sticker_panel, bundle *b, void *user_data)
{
	Evas_Object *new_icon = NULL;
	Evas_Object *old_icon = NULL;
	char *result_type = NULL;
	char *result_path = NULL;
	int type = 0;
	int ret = BUNDLE_ERROR_NONE;

	ret_if(!b);

	ret = bundle_get_str(b, STICKER_PANEL_RESULT_TYPE, &result_type);
	if (ret != BUNDLE_ERROR_NONE) {
		_E("Failed to get bundle(type) : %d", ret);
		return;
	} else if (!result_type) {
		_E("result_type is NULL");
		return;
	}

	type = _get_result_type(result_type);
	if (type == -1) {
		_E("Failed to get result type");
		return;
	}

	ret = bundle_get_str(b, STICKER_PANEL_RESULT_VALUE, &result_path);
	if (ret != BUNDLE_ERROR_NONE) {
		_E("Failed to get bundle(path) : %d", ret);
	} else if (!result_path) {
		_E("result_path is NULL");
		return;
	}

	_D("bundle type(%s), path(%s)", result_type, result_path);

	old_icon = elm_object_part_content_unset(sample_info.layout, "sw.icon");
	if (old_icon) {
		sticker_icon_destroy(old_icon);
	}

	new_icon = sticker_icon_create(sample_info.layout, type, result_path);
	ret_if(!new_icon);
	elm_object_part_content_set(sample_info.layout, "sw.icon", new_icon);
}



static void _click_sticker_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	sticker_panel_h sticker_panel = NULL;
	int ret = STICKER_PANEL_ERROR_NONE;

	if (sample_info.sticker_panel) {
		_D("sticker panel already existed");
		sticker_panel_show(sample_info.sticker_panel);
		return;
	}

	ret = sticker_panel_create(sample_info.conformant, &sticker_panel);
	ret_if(STICKER_PANEL_ERROR_NONE != ret);

	sticker_panel_set_result_cb(sticker_panel, _result_cb, NULL);
	sticker_panel_show(sticker_panel);

	sample_info.sticker_panel = sticker_panel;
}



static Evas_Object *_create_button(Evas_Object *layout)
{
	Evas_Object *button = NULL;

	retv_if(!layout, NULL);

	button = elm_button_add(layout);
	retv_if(!button, NULL);

	elm_object_text_set(button, "sticker");
	elm_object_part_content_set(layout, "button", button);
	evas_object_size_hint_weight_set(button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(button);

	evas_object_smart_callback_add(button, "clicked", _click_sticker_button_cb, NULL);

	return button;
}



static void _destroy_button(Evas_Object *layout)
{
	Evas_Object *button = NULL;

	button = elm_object_part_content_unset(layout, "button");
	if (!button) {
		return;
	}

	evas_object_smart_callback_del(button, "clicked", _click_sticker_button_cb);
	evas_object_del(button);
}



static void _destroy_layout(void)
{
	ret_if(!sample_info.layout);

	_destroy_button(sample_info.layout);

	elm_object_signal_callback_del(sample_info.layout, "bg,up", "bg", _bg_up_cb);

	if (sample_info.conformant) {
		elm_object_content_unset(sample_info.conformant);
	}
	evas_object_del(sample_info.layout);
	sample_info.layout = NULL;
}



static Evas_Object *_create_layout(Evas_Object *parent)
{
	Evas_Object *layout = NULL;
	Eina_Bool ret = EINA_FALSE;

	retv_if(!parent, NULL);

	layout = elm_layout_add(parent);
	goto_if(!layout, ERROR);

	ret = elm_layout_file_set(layout, LAYOUT, "layout");
	goto_if(EINA_FALSE == ret, ERROR);

	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(layout);
	elm_object_signal_callback_add(layout, "bg,up", "bg", _bg_up_cb, NULL);

	goto_if(!_create_button(layout), ERROR);

	elm_object_content_set(parent, layout);

	return layout;


ERROR:
	_destroy_layout();
	return NULL;
}



static Evas_Object *_create_bg(Evas_Object *parent)
{
	Evas_Object *bg = NULL;
	retv_if(!parent, NULL);

	bg = elm_bg_add(parent);
	retv_if(!bg, NULL);
	evas_object_size_hint_weight_set(bg,  EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(parent, bg);

	evas_object_show(bg);

	return bg;
}



static void _destroy_bg(void)
{
	ret_if(!sample_info.bg);
	evas_object_del(sample_info.bg);
	sample_info.bg = NULL;
}



static void _destroy_conformant(void)
{
	ret_if(!sample_info.conformant);
	evas_object_del(sample_info.conformant);
	sample_info.conformant = NULL;
}



static Evas_Object *_create_conformant(Evas_Object *parent)
{
	Evas_Object *conformant = NULL;
	retv_if(!parent, NULL);

	conformant = elm_conformant_add(parent);
	retv_if(!conformant, NULL);

	elm_win_indicator_mode_set(parent, ELM_WIN_INDICATOR_HIDE);
	evas_object_size_hint_weight_set(conformant,  EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(parent, conformant);
	elm_win_conformant_set(parent, EINA_TRUE);

	evas_object_show(conformant);

	return conformant;
}



static void __win_resize_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Object *win = obj;
	Evas_Coord x = 0;
	Evas_Coord y = 0;
	Evas_Coord w = 0;
	Evas_Coord h = 0;

	ret_if(!win);

	evas_object_geometry_get(win, &x, &y, &w, &h);
	_D("win resize(%d, %d, %d, %d)", x, y, w, h);
}



static void __conformant_resize_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Object *conformant = obj;
	Evas_Coord x = 0;
	Evas_Coord y = 0;
	Evas_Coord w = 0;
	Evas_Coord h = 0;

	ret_if(!conformant);

	evas_object_geometry_get(conformant, &x, &y, &w, &h);
	_D("conformant resize(%d, %d, %d, %d)", x, y, w, h);
}



static void __layout_resize_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Object *layout = obj;
	Evas_Coord x = 0;
	Evas_Coord y = 0;
	Evas_Coord w = 0;
	Evas_Coord h = 0;

	ret_if(!layout);

	evas_object_geometry_get(layout, &x, &y, &w, &h);
	_D("layout resize(%d, %d, %d, %d)", x, y, w, h);
}


static void _create_cb(void)
{
	Evas_Object *layout = NULL;
	Evas_Object *conformant = NULL;
	Evas_Object *bg = NULL;

	sample_info.win = elm_win_add(NULL, "Sticker Panel Sample", ELM_WIN_BASIC);
	ret_if(!sample_info.win);
	evas_object_event_callback_add(sample_info.win, EVAS_CALLBACK_RESIZE, __win_resize_cb, NULL);

	elm_app_base_scale_set(2.6);

	elm_win_alpha_set(sample_info.win, EINA_FALSE);
	elm_win_title_set(sample_info.win, "Sticker Panel");
	elm_win_borderless_set(sample_info.win, EINA_TRUE);
	elm_win_autodel_set(sample_info.win, EINA_TRUE);
	elm_win_raise(sample_info.win);

	evas_object_show(sample_info.win);

	elm_win_screen_size_get(sample_info.win, NULL, NULL, &sample_info.root_w, &sample_info.root_h);
	_D("screen size is (%d, %d)", sample_info.root_w, sample_info.root_h);


	if (elm_win_wm_rotation_supported_get(sample_info.win)) {
		int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(sample_info.win, rots, 4);
	}

	eext_object_event_callback_add(sample_info.win, EEXT_CALLBACK_BACK, _win_back_key_cb, NULL);
	evas_object_smart_callback_add(sample_info.win, "wm,rotation,changed", _rotate_cb, NULL);

	bg = _create_bg(sample_info.win);
	goto_if(!bg, ERROR);

	conformant = _create_conformant(sample_info.win);
	goto_if(!conformant, ERROR);
	evas_object_event_callback_add(conformant, EVAS_CALLBACK_RESIZE, __conformant_resize_cb, NULL);

	layout = _create_layout(conformant);
	goto_if(!layout, ERROR);
	evas_object_event_callback_add(layout, EVAS_CALLBACK_RESIZE, __layout_resize_cb, NULL);

	sample_info.layout = layout;
	sample_info.conformant = conformant;
	sample_info.bg = bg;

	return;

ERROR:
	_D("there is some error");
	if (conformant) {
		_destroy_conformant();
	}

	if (bg) {
		_destroy_bg();
	}

	if (sample_info.win) {
		evas_object_del(sample_info.win);
		eext_object_event_callback_del(sample_info.win, EEXT_CALLBACK_BACK, _win_back_key_cb);
		sample_info.win = NULL;
	}
}



static void _terminate_cb(void)
{
	_destroy_layout();
	_destroy_conformant();
	_destroy_bg();

	if (sample_info.win) {
		eext_object_event_callback_del(sample_info.win, EEXT_CALLBACK_BACK, _win_back_key_cb);
		evas_object_del(sample_info.win);
		sample_info.win = NULL;
	}
}



int main(int argc, char **argv)
{
	elm_init(argc, argv);
	_create_cb();
	elm_run();
	_terminate_cb();
	elm_shutdown();

	return 0;
}
