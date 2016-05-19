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

#ifndef __TIZEN_STICKER_PANEL_UI_MANAGER_H__
#define __TIZEN_STICKER_PANEL_UI_MANAGER_H__

#include <Elementary.h>

extern Evas_Object *_ui_manager_create(sticker_panel_h sticker_panel);
extern void _ui_manager_destroy(Evas_Object *ui_manager);

extern void _ui_manager_show_category(Evas_Object *ui_manager, sticker_panel_h sticker_panel, int category);

#endif /* __TIZEN_STICKER_PANEL_UI_MANAGER_H__ */
