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
#include "log.h"
#include "page.h"



static void __resize_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Object *page = obj;

	int x, y, w, h;

	ret_if(!page);

	evas_object_geometry_get(page, &x, &y, &w, &h);
	_D("page(%s), resize(%d, %d, %d, %d)", data, x, y, w, h);
}



#define PAGE_EDJE_FILE EDJEDIR"/page.edj"
Evas_Object *_page_create(Evas_Object *scroller, int width, int height)
{
	Evas_Object *page = NULL;
	Evas_Object *page_bg = NULL;
	Evas *e = NULL;

	page = elm_layout_add(scroller);
	retv_if(!page, NULL);
	elm_layout_file_set(page, PAGE_EDJE_FILE, "page");

	evas_object_size_hint_weight_set(page, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(page, EVAS_HINT_FILL, EVAS_HINT_FILL);

	e = evas_object_evas_get(scroller);
	goto_if(!e, ERROR);

	page_bg = evas_object_rectangle_add(e);
	goto_if(!page_bg, ERROR);
	evas_object_size_hint_weight_set(page_bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(page_bg, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_min_set(page_bg, width, height);
	evas_object_color_set(page_bg, 0, 0, 0, 0);
	evas_object_repeat_events_set(page_bg, EINA_TRUE);
	evas_object_show(page_bg);
	elm_object_part_content_set(page, "bg", page_bg);

	evas_object_event_callback_add(page, EVAS_CALLBACK_RESIZE, __resize_cb, "PAGE");

	evas_object_show(page);
	return page;

ERROR:
	_page_destroy(page);
	return NULL;
}



void _page_destroy(Evas_Object *page)
{
	Evas_Object *bg = NULL;

	ret_if(!page);

	evas_object_event_callback_del(page, EVAS_CALLBACK_RESIZE, __resize_cb);

	bg = elm_object_part_content_unset(page, "bg");
	if (bg) {
		evas_object_del(bg);
	}

	evas_object_del(page);
}



void _page_resize(Evas_Object *page, int width, int height)
{
	Evas_Object *bg = NULL;

	ret_if(!page);

	bg = elm_object_part_content_get(page, "bg");
	ret_if(!bg);

	evas_object_size_hint_min_set(bg, width, height);
	evas_object_size_hint_max_set(bg, width, height);
}
