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
#include "conformant.h"

Evas_Object *_conformant_create(Evas_Object *caller_conformant, Eina_Bool show_indicator)
{
	Evas_Object *conformant = NULL;

	retv_if(!caller_conformant, NULL);

	conformant = elm_conformant_add(caller_conformant);
	retv_if(!conformant, NULL);

	evas_object_size_hint_weight_set(conformant, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(caller_conformant, conformant);

	if (show_indicator) {
		_D("Show indicator");
		elm_object_signal_emit(conformant, "elm,state,indicator,overlap", "elm");
	} else {
		_D("Hide indicator");
	}

	return conformant;
}

void _conformant_destroy(Evas_Object *conformant)
{
	ret_if(!conformant);

	evas_object_del(conformant);
}
