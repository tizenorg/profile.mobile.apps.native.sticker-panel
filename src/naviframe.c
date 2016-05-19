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
#include "conf.h"
#include "ui_manager.h"
#include "log.h"
#include "naviframe.h"


Evas_Object *_naviframe_create(Evas_Object *conformant)
{
	Evas_Object *navi = NULL;

	retv_if(!conformant, NULL);

	_D("Create sticker panel naviframe");

	navi = elm_naviframe_add(conformant);
	retv_if(!navi, NULL);

	evas_object_size_hint_weight_set(navi, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_content_set(conformant, navi);

	evas_object_show(navi);

	return navi;
}

void _naviframe_destroy(Evas_Object *navi)
{
	Evas_Object *btn = NULL;

	_D("Delete sticker panel naviframe");

	ret_if(!navi);

	btn = elm_object_part_content_unset(navi, "prev_btn");
	if (btn) {
		evas_object_del(btn);
		btn = NULL;
	}

	btn = elm_object_part_content_unset(navi, "next_btn");
	if (btn) {
		evas_object_del(btn);
		btn = NULL;
	}

	evas_object_del(navi);
}

void _naviframe_set_title(Evas_Object *navi, const char *title)
{
	ret_if(!navi);
	ret_if(!title);

	elm_object_part_text_set(navi, "default", title);
}

Evas_Object *_naviframe_button_create(Evas_Object *navi, sticker_panel_navi_btn_type_e btn_type, const char *text, btn_clicked_cb clicked_cb, void *data)
{
	Evas_Object *btn = NULL;
	Evas_Object *old_btn = NULL;

	retv_if(!navi, NULL);
	retv_if(!text, NULL);
	retv_if(!clicked_cb, NULL);

	btn = elm_button_add(navi);
	retv_if(!btn, NULL);

	elm_object_text_set(btn, text);

	switch (btn_type) {
	case STICKER_PANEL_NAVI_BTN_LEFT:
		old_btn = elm_object_part_content_unset(navi, "prev_btn");
		if (old_btn) {
			_D("Delete old button(left)");
			evas_object_del(old_btn);
			old_btn = NULL;
		}

		elm_object_part_content_set(navi, "prev_btn", btn);
		break;
	case STICKER_PANEL_NAVI_BTN_RIGHT:
		old_btn = elm_object_part_content_unset(navi, "next_btn");
		if (old_btn) {
			_D("Delete old button(right)");
			evas_object_del(old_btn);
			old_btn = NULL;
		}

		elm_object_part_content_set(navi, "next_btn", btn);
		break;
	default:
		_E("Failed to create naviframe button(type : %d", btn_type);
		if (btn) {
			evas_object_del(btn);
			btn = NULL;
		}
		return NULL;
	}

	evas_object_smart_callback_add(btn, "clicked", clicked_cb, &data);

	evas_object_show(btn);

	return btn;
}

void _naviframe_button_destroy(Evas_Object *navi, sticker_panel_navi_btn_type_e btn_type)
{
	Evas_Object *btn = NULL;

	ret_if(!navi);

	switch (btn_type) {
	case STICKER_PANEL_NAVI_BTN_LEFT:
		btn = elm_object_part_content_unset(navi, "prev_btn");
		if (btn) {
			_D("Delete left button");
			evas_object_del(btn);
			btn = NULL;
		}
		break;
	case STICKER_PANEL_NAVI_BTN_RIGHT:
		btn = elm_object_part_content_unset(navi, "next_btn");
		if (btn) {
			_D("Delete right button");
			evas_object_del(btn);
			btn = NULL;
		}
		break;
	default:
		_E("Failed to delete naviframe button(type : %d)", btn_type);
		return;
	}
}

void _naviframe_show(Evas_Object *navi)
{
	ret_if(!navi);

	evas_object_show(navi);
}

void _naviframe_hide(Evas_Object *navi)
{
	ret_if(!navi);

	evas_object_hide(navi);
}
