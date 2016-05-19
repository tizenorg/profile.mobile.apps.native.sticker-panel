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

#ifndef __TIZEN_STICKER_PANEL_NAVIFRAME_H__
#define __TIZEN_STICKER_PANEL_NAVIFRAME_H__

typedef enum sticker_panel_navi_btn_type {
	STICKER_PANEL_NAVI_BTN_RIGHT = 1,
	STICKER_PANEL_NAVI_BTN_LEFT = 2,
	STICKER_PANEL_NAVI_BTN_MAX,
} sticker_panel_navi_btn_type_e;

typedef void (*btn_clicked_cb)(void *data, Evas_Object *obj, void *event_info);

Evas_Object *_naviframe_create(Evas_Object *conformant);
void _naviframe_destroy(Evas_Object *navi);

void _naviframe_set_title(Evas_Object *navi, const char *title);
Evas_Object *_naviframe_button_create(Evas_Object *navi, sticker_panel_navi_btn_type_e btn_type, const char *text, btn_clicked_cb clicked_cb, void *data);
void _naviframe_button_destroy(Evas_Object *navi, sticker_panel_navi_btn_type_e btn_type);

void _naviframe_show(Evas_Object *navi);
void _naviframe_hide(Evas_Object *navi);

#endif /* __TIZEN_STICKER_PANEL_NAVIFRAME_H__ */
