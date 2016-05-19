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

#ifndef __TIZEN_STICKER_PANEL_SETTING_H__
#define __TIZEN_STICKER_PANEL_SETTING_H__

typedef enum setting_view_type {
	SETTING_VIEW_NONE = 0,
	SETTING_VIEW_REORDER,
	SETTING_VIEW_DOWNLOAD,
	SETTING_VIEW_DELETE,
	SETTING_VIEW_MAX,
} setting_view_type_e;

extern Evas_Object *_setting_create(Evas_Object *parent, sticker_panel_h sticker_panel);
extern void _setting_destroy(Evas_Object *setting);

#endif /* __TIZEN_STICKER_PANEL_SETTING_H__ */
