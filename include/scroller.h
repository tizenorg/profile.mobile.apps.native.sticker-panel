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

#ifndef __TIZEN_STICKER_PANEL_SCROLLER_H__
#define __TIZEN_STICKER_PANEL_SCROLLER_H__

#include <Elementary.h>

typedef enum {
	SCROLLER_EVENT_TYPE_INVALID = 0,
	SCROLLER_EVENT_TYPE_SCROLL,
	SCROLLER_EVENT_TYPE_MAX,
} scroller_event_type_e;

extern Evas_Object *_scroller_create(Evas_Object *ui_manager, sticker_panel_h sticker_panel);
extern void _scroller_destroy(Evas_Object *scroller);

extern void _scroller_append_page(Evas_Object *scroller, Evas_Object *page);
extern void _scroller_remove_page(Evas_Object *scroller, Evas_Object *page);
extern void _scroller_reorder_page(Evas_Object *scroller, Eina_List *group_icon_info_list);

extern void _scroller_bring_in_page(Evas_Object *scroller, Evas_Object *page, int *cur_page_no);

extern int _scroller_is_scrolling(Evas_Object *scroller);
extern void _scroller_resize(Evas_Object *scroller, int width, int height);

extern int _scroller_register_event_cb(Evas_Object *scroller, int event_type, void (*event_cb)(Evas_Object *scroller, int event_type, void *event_info, void *data), void *data);
extern int _scroller_unregister_event_cb(Evas_Object *scroller, int event_type, void (*event_cb)(Evas_Object *scroller, int event_type, void *event_info, void *data));

#endif /* __TIZEN_STICKER_PANEL_SCROLLER_H__ */
