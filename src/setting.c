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
#include <efl_extension.h>

#include "sticker_panel.h"
#include "sticker_panel_internal.h"
#include "setting.h"
#include "conf.h"
#include "log.h"
#include "group_icon_info.h"
#include "db.h"
#include "icon_info.h"
#include "toolbar.h"
#include "scroller.h"
#include "page.h"

#define BUF_SIZE_1024 1024

const char *const PRIVATE_DATA_KEY_SETTING_INFO = "__dksi__";
const char *const PRIVATE_DATA_KEY_SELECT_ALL_ITEM = "__dksai__";
const char *const PRIVATE_DATA_KEY_GROUP_ICON_INFO = "__dkgii__";
const char *const KEY_NAME_MENU = "XF86Menu";
const char *const KEY_NAME_BACK = "XF86Back";
const char *const FILE_LAYOUT_EDJ = EDJEDIR"/setting.edj";
const char *const GROUP_LAYOUT = "layout";
//@TODO: need to change string
const char *const STRING_SETTING = "Setting";
const char *const STRING_REORDER = "Reorder";
const char *const STRING_DELETE = "Delete";
const char *const STRING_CANCEL = "Cancel";
const char *const STRING_DONE = "Done";


struct _setting_info_s {
	sqlite3 *db;
	Eina_List *group_icon_info_list;
	Evas_Object *navi;
	Evas_Object *toolbar;
	Evas_Object *scroller;
	Evas_Object *genlist;
	Evas_Object *ctx_popup;
	Ecore_Event_Handler *key_press;
	Ecore_Event_Handler *key_release;
	Eina_Bool show_ctx_popup;
	setting_view_type_e view_type;
};
typedef struct _setting_info_s setting_info_s;



static char *__get_label_cb(void *data, Evas_Object *obj, const char *part)
{
	group_icon_info_s *group_icon_info = NULL;
	setting_info_s *setting_info = NULL;
	char *ret_str = NULL;
	int order = (int) data;

	retv_if(!obj, NULL);
	retv_if(!part, NULL);

	setting_info = evas_object_data_get(obj, PRIVATE_DATA_KEY_SETTING_INFO);
	retv_if(!setting_info, NULL);

	if (order == -1 && setting_info->view_type == SETTING_VIEW_DELETE) {
		if (!strcmp(part, "elm.text.main.left")) {
			//@TODO: need to change string
			ret_str = strdup("Select all");
			if (!ret_str) {
				_E("Failed to duplicate string");
				return NULL;
			}

			return ret_str;
		}
		return NULL;
	}

	if (!strcmp(part, "elm.text.main") || !strcmp(part, "elm.text.main.left")) {
		icon_info_s *icon_info = NULL;

		group_icon_info = eina_list_nth(setting_info->group_icon_info_list, order);
		goto_if(!group_icon_info, error);

		icon_info = eina_list_nth(group_icon_info->list, 0);
		goto_if(!icon_info, error);
		goto_if(!icon_info->thumbnail_file, error);

		ret_str = strdup(group_icon_info->name);
		if (!ret_str) {
			_E("Failed to duplicate string");
			return NULL;
		}

		return ret_str;
	}

error:

	return NULL;
}



static void __update_navi_title_with_count(Evas_Object *genlist, setting_info_s *setting_info)
{
	Elm_Object_Item *navi_item = NULL;
	Elm_Object_Item *genlist_item = NULL;
	Evas_Object *check = NULL;
	Eina_Bool check_on = EINA_FALSE;
	char title[BUF_SIZE_1024] = { 0, };
	int max_count = 0;
	int count = 0;

	ret_if(!genlist);
	ret_if(!setting_info);

	max_count = elm_genlist_items_count(genlist);

	/* Exclude 'Select all' item */
	max_count--;

	navi_item = elm_naviframe_top_item_get(setting_info->navi);
	ret_if(!navi_item);

	genlist_item = elm_genlist_first_item_get(genlist);
	ret_if(!genlist_item);

	/* Exclude 'Select all' item  */
	genlist_item = elm_genlist_item_next_get(genlist_item);

	while (genlist_item) {
		check = elm_object_item_part_content_get(genlist_item, "elm.icon.right");
		ret_if(!check);

		check_on = elm_check_state_get(check);
		if (check_on) {
			count++;
		}

		genlist_item = elm_genlist_item_next_get(genlist_item);
	}

	//@TODO: need to change string
	snprintf(title, sizeof(title), "%d %s", count, "selected");

	elm_object_item_part_text_set(navi_item, "default", title);

	if (count == 0) {
		genlist_item = elm_genlist_first_item_get(genlist);
		ret_if(!genlist_item);

		check = elm_object_item_part_content_get(genlist_item, "elm.icon.right");
		ret_if(!check);

		elm_check_state_set(check, EINA_FALSE);
	} else if (count == max_count) {
		genlist_item = elm_genlist_first_item_get(genlist);
		ret_if(!genlist_item);

		check = elm_object_item_part_content_get(genlist_item, "elm.icon.right");
		ret_if(!check);

		elm_check_state_set(check, EINA_TRUE);
	}
}



static void __change_check_state_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *genlist = NULL;
	Evas_Object *check = NULL;
	Elm_Object_Item *genlist_item = NULL;
	Eina_Bool is_select_all = EINA_FALSE;
	Eina_Bool check_on = EINA_FALSE;
	setting_info_s *setting_info = NULL;

	genlist = data;
	ret_if(!genlist);

	ret_if(!obj);

	setting_info = evas_object_data_get(genlist, PRIVATE_DATA_KEY_SETTING_INFO);
	ret_if(!setting_info);

	is_select_all = (Eina_Bool)(intptr_t) evas_object_data_get(obj, PRIVATE_DATA_KEY_SELECT_ALL_ITEM);
	goto_if(!is_select_all, out);

	genlist_item = elm_genlist_first_item_get(genlist);
	ret_if(!genlist_item);

	check = elm_object_item_part_content_get(genlist_item, "elm.icon.right");
	ret_if(!check);

	check_on = elm_check_state_get(check);

	genlist_item = elm_genlist_item_next_get(genlist_item);

	while (genlist_item) {
		check = elm_object_item_part_content_get(genlist_item, "elm.icon.right");
		ret_if(!check);

		if (check_on) {
			elm_check_state_set(check, EINA_TRUE);
		} else {
			elm_check_state_set(check, EINA_FALSE);
		}

		genlist_item = elm_genlist_item_next_get(genlist_item);
	}

out:
	/* update naviframe title */
	__update_navi_title_with_count(genlist, setting_info);
}



static Evas_Object *__create_delete_check(Evas_Object *parent, Eina_Bool is_select_all)
{
	Evas_Object *check = NULL;

	retv_if(!parent, NULL);

	check = elm_check_add(parent);
	retv_if(!check, NULL);

	elm_object_style_set(check, "default");
	evas_object_propagate_events_set(check, EINA_FALSE);
	evas_object_show(check);

	evas_object_data_set(check, PRIVATE_DATA_KEY_SELECT_ALL_ITEM, (void *)(intptr_t) is_select_all);

	evas_object_smart_callback_add(check, "changed", __change_check_state_cb, parent);

	return check;
}



static void __destroy_delete_check(Evas_Object *check)
{
	ret_if(!check);

	evas_object_data_del(check, PRIVATE_DATA_KEY_SELECT_ALL_ITEM);
	evas_object_smart_callback_del(check, "changed", __change_check_state_cb);
	evas_object_del(check);
}


static Evas_Object *__create_genlist_left_icon(Evas_Object *parent, int order, setting_info_s *setting_info)
{
	Evas_Object *content = NULL;
	Evas_Object *icon = NULL;
	icon_info_s *icon_info = NULL;
	group_icon_info_s *group_icon_info = NULL;

	retv_if(!parent, NULL);
	retv_if(!setting_info, NULL);

	content = elm_layout_add(parent);
	retv_if(!content, NULL);

	elm_layout_theme_set(content, "layout", "list/C/type.3", "default");

	icon = elm_image_add(content);
	goto_if(!icon, error);

	group_icon_info = eina_list_nth(setting_info->group_icon_info_list, order);
	goto_if(!group_icon_info, error);
	goto_if(!group_icon_info->list, error);

	icon_info = eina_list_nth(group_icon_info->list, 0);
	goto_if(!icon_info, error);
	goto_if(!icon_info->thumbnail_file, error);

	if (!elm_image_file_set(icon, icon_info->thumbnail_file, NULL)) {
		_E("Failed to set icon");
		goto error;
	}

	elm_layout_content_set(content, "elm.swallow.content", icon);

	evas_object_show(icon);
	evas_object_show(content);

	return content;

error:
	if (icon) {
		evas_object_del(icon);
	}

	if (content) {
		evas_object_del(content);
	}

	return NULL;
}



static void __destroy_genlist_left_icon(Evas_Object *left_icon)
{
	Evas_Object *icon = NULL;

	ret_if(!left_icon);

	icon = elm_layout_content_get(left_icon, "elm.swallow.content");
	if (icon) {
		evas_object_del(icon);
	}

	evas_object_del(left_icon);
}



static Evas_Object *__create_reorder_icon(Evas_Object *parent)
{
	Evas_Object *icon = NULL;

	retv_if(!parent, NULL);

	icon = elm_button_add(parent);
	retv_if(!icon, NULL);

	elm_object_style_set(icon, "icon_reorder");
	evas_object_show(icon);

	return icon;
}



static void __destroy_reorder_icon(Evas_Object *icon)
{
	ret_if(!icon);

	evas_object_del(icon);
}



static Evas_Object *__get_content_cb(void *data, Evas_Object *obj, const char *part)
{
	setting_info_s *setting_info = NULL;

	int order = (int) data;

	retv_if(!obj, NULL);
	retv_if(!part, NULL);

	setting_info = evas_object_data_get(obj, PRIVATE_DATA_KEY_SETTING_INFO);
	retv_if(!setting_info, NULL);

	if (order == -1 && setting_info->view_type == SETTING_VIEW_DELETE) {
		if (!strcmp(part, "elm.icon.right")) {
			Evas_Object *check = NULL;

			check = __create_delete_check(obj, EINA_TRUE);
			retv_if(!check, NULL);

			return check;
		}
		return NULL;
	}

	if (!strcmp(part, "elm.icon.1")) {
		Evas_Object *left_icon = NULL;

		left_icon = __create_genlist_left_icon(obj, order, setting_info);
		retv_if(!left_icon, NULL);

		return left_icon;
	} else if (!strcmp(part, "elm.icon.right")) {
		Evas_Object *right_btn = NULL;
		group_icon_info_s *group_icon_info = NULL;

		switch(setting_info->view_type) {
		case SETTING_VIEW_REORDER:
			right_btn = __create_reorder_icon(obj);
			retv_if(!right_btn, NULL);
			break;
		case SETTING_VIEW_DELETE:
			group_icon_info = eina_list_nth(setting_info->group_icon_info_list, order);
			if (!group_icon_info) {
				_E("Failed to get group icon info");
				return NULL;
			}

			if (group_icon_info->removable == 0) {
				_E("This item is not removable");
				return NULL;
			}

			right_btn = __create_delete_check(obj, EINA_FALSE);
			retv_if(!right_btn, NULL);
			break;
		default:
			_E("view type error : %d", setting_info->view_type);
			return NULL;
		}

		return right_btn;
	}

	return NULL;
}



static void __select_item_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *selected_item = NULL;

	setting_info_s *setting_info = data;
	ret_if(!data);

	ret_if(!obj);

	selected_item = elm_genlist_selected_item_get(obj);
	ret_if(!selected_item);

	elm_genlist_item_selected_set(selected_item, EINA_FALSE);

	if (setting_info->view_type == SETTING_VIEW_DELETE) {
		Evas_Object *check = NULL;
		Eina_Bool check_on = EINA_FALSE;

		check = elm_object_item_part_content_get(selected_item, "elm.icon.right");
		ret_if(!check);

		check_on = elm_check_state_get(check);

		if (check_on) {
			elm_check_state_set(check, EINA_FALSE);
		} else {
			elm_check_state_set(check, EINA_TRUE);
		}

		/* update naviframe title */
		__update_navi_title_with_count(obj, setting_info);
	}

	return;
}



static void __select_all_item_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *check = NULL;
	Eina_Bool check_on = EINA_FALSE;
	Elm_Object_Item *genlist_item = NULL;

	setting_info_s *setting_info = data;
	ret_if(!data);

	ret_if(!obj);

	genlist_item = elm_genlist_selected_item_get(obj);
	ret_if(!genlist_item);

	elm_genlist_item_selected_set(genlist_item, EINA_FALSE);

	check = elm_object_item_part_content_get(genlist_item, "elm.icon.right");
	ret_if(!check);

	check_on = elm_check_state_get(check);

	while (genlist_item) {
		check = elm_object_item_part_content_get(genlist_item, "elm.icon.right");
		ret_if(!check);

		if (check_on) {
			elm_check_state_set(check, EINA_FALSE);
		} else {
			elm_check_state_set(check, EINA_TRUE);
		}

		genlist_item = elm_genlist_item_next_get(genlist_item);
	}

	/* update naviframe title */
	__update_navi_title_with_count(obj, setting_info);

	return;
}



static void __destroy_genlist(Evas_Object *genlist)
{
	Evas_Object *left_icon = NULL;
	Evas_Object *right_icon = NULL;
	Elm_Object_Item *genlist_item = NULL;
	Elm_Object_Item *genlist_next_item = NULL;
	setting_info_s *setting_info = NULL;

	ret_if(!genlist);

	setting_info = evas_object_data_del(genlist, PRIVATE_DATA_KEY_SETTING_INFO);
	ret_if(!setting_info);

	genlist_item = elm_genlist_first_item_get(genlist);
	ret_if(!genlist_item);

	while (genlist_item) {
		left_icon = elm_object_item_part_content_get(genlist_item, "elm.icon.1");
		if (left_icon) {
			__destroy_genlist_left_icon(left_icon);
		}

		right_icon = elm_object_item_part_content_get(genlist_item, "elm.icon.right");
		goto_if(!right_icon, out);

		if (setting_info->view_type == SETTING_VIEW_REORDER) {
			__destroy_reorder_icon(right_icon);
		} else if (setting_info->view_type == SETTING_VIEW_DELETE) {
			__destroy_delete_check(right_icon);
		}

out:
		evas_object_data_del(genlist_item, PRIVATE_DATA_KEY_GROUP_ICON_INFO);
		genlist_next_item = elm_genlist_item_next_get(genlist_item);
		elm_object_item_del(genlist_item);
		genlist_item = genlist_next_item;
	}

	evas_object_del(genlist);
}



static void __append_genlist_item(Evas_Object *genlist, setting_view_type_e view_type, setting_info_s *setting_info)
{
	Elm_Genlist_Item_Class *itc = NULL;
	int i = 0;
	int count = 0;

	ret_if(!genlist);
	ret_if(!setting_info);

	count = eina_list_count(setting_info->group_icon_info_list);
	_D("group icon count : %d", count);
	ret_if(count == 0);

	itc = elm_genlist_item_class_new();
	if (!itc) {
		_E("Failed to create item class for genlist");
		__destroy_genlist(genlist);
		return;
	}

	itc->item_style = "1line";
	itc->func.text_get = __get_label_cb;
	itc->func.content_get = __get_content_cb;
	itc->func.state_get = NULL;
	itc->func.del = NULL;

	for (i = 0; i < count; i++) {
		Elm_Object_Item *item = NULL;
		group_icon_info_s *group_icon_info = eina_list_nth(setting_info->group_icon_info_list, i);
		if (!group_icon_info) {
			continue;
		}

		if (group_icon_info->permutable == 0) {
			continue;
		}

		item = elm_genlist_item_append(genlist, itc, (void *) i, NULL, ELM_GENLIST_ITEM_NONE, __select_item_cb, setting_info);
		if (!item) {
			continue;
		}

		evas_object_data_set(item, PRIVATE_DATA_KEY_GROUP_ICON_INFO, group_icon_info);
	}

	if (view_type == SETTING_VIEW_DELETE) {
		elm_genlist_item_prepend(genlist, itc, (void *) -1, NULL, ELM_GENLIST_ITEM_NONE, __select_all_item_cb, setting_info);
	}

	elm_genlist_item_class_free(itc);
}



static Evas_Object *__create_genlist(Evas_Object *parent, setting_info_s *setting_info, setting_view_type_e view_type)
{
	Evas_Object *genlist = NULL;

	retv_if(!parent, NULL);
	retv_if(!setting_info, NULL);

	genlist = elm_genlist_add(parent);
	retv_if(!genlist, NULL);

	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	elm_object_style_set(genlist, "dialogue");

	evas_object_data_set(genlist, PRIVATE_DATA_KEY_SETTING_INFO, (void *) setting_info);

	evas_object_show(genlist);

	__append_genlist_item(genlist, view_type, setting_info);

	if (view_type == SETTING_VIEW_REORDER) {
		elm_genlist_reorder_mode_set(genlist, EINA_TRUE);
	}

	return genlist;
}



static void __update_genlist(setting_info_s *setting_info)
{
	ret_if(!setting_info);

	elm_genlist_clear(setting_info->genlist);
	__append_genlist_item(setting_info->genlist, setting_info->view_type, setting_info);
}


static Evas_Object *__create_layout(Evas_Object *parent)
{
	Evas_Object *layout = NULL;
	int w = 0;
	int h = 0;

	retv_if(!parent, NULL);

	layout = elm_layout_add(parent);
	retv_if(!layout, NULL);

	if (!elm_layout_file_set(layout, FILE_LAYOUT_EDJ, GROUP_LAYOUT)) {
		_E("Failed to set edje file");
		evas_object_del(layout);
		return NULL;
	}

	evas_object_geometry_get(parent, NULL, NULL, &w, &h);
	_D("conformant size : %dx%d", w, h);

	evas_object_move(layout, 0, 0);
	evas_object_resize(layout, w, h);

	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, 1.0);

	evas_object_show(layout);

	return layout;
}



static void __destroy_layout(Evas_Object *layout)
{
	ret_if(!layout);

	evas_object_del(layout);
}




static Evas_Object *__create_navi_next_btn(Evas_Object *navi, const char *text)
{
	Evas_Object *next_btn = NULL;

	retv_if(!navi, NULL);
	retv_if(!text, NULL);

	next_btn = elm_button_add(navi);
	retv_if(!next_btn, NULL);

	elm_object_style_set(next_btn, "naviframe/title_right");
	elm_object_text_set(next_btn, text);

	evas_object_show(next_btn);

	return next_btn;
}



static Evas_Object *__create_navi_prev_btn(Evas_Object *navi)
{
	Evas_Object *prev_btn = NULL;

	retv_if(!navi, NULL);

	prev_btn = elm_button_add(navi);
	retv_if(!prev_btn, NULL);

	elm_object_style_set(prev_btn, "naviframe/back_btn/default");

	evas_object_show(prev_btn);

	return prev_btn;
}



static void __destroy_navi_btn(Evas_Object *btn)
{
	ret_if(!btn);

	evas_object_del(btn);
}



static void __click_navi_prev_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *layout = data;
	ret_if(!layout);

	_setting_destroy(layout);

	return;
}



static Evas_Object *__create_navi(Evas_Object *parent, sticker_panel_h sticker_panel)
{
	Evas_Object *navi = NULL;
	Evas_Object *prev_btn = NULL;
	Evas_Object *genlist = NULL;
	Elm_Object_Item *navi_item = NULL;
	setting_info_s *setting_info = NULL;

	retv_if(!parent, NULL);
	retv_if(!sticker_panel, NULL);

	setting_info = evas_object_data_get(parent, PRIVATE_DATA_KEY_SETTING_INFO);
	retv_if(!setting_info, NULL);

	navi = elm_naviframe_add(parent);
	retv_if(!navi, NULL);

	evas_object_size_hint_weight_set(navi, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_naviframe_content_preserve_on_pop_set(navi, EINA_TRUE);

	evas_object_show(navi);

	prev_btn = __create_navi_prev_btn(navi);
	goto_if(!prev_btn, error);

	setting_info->group_icon_info_list = sticker_panel->group_icon_info_list;

	genlist = __create_genlist(navi, setting_info, SETTING_VIEW_NONE);
	goto_if(!genlist, error);

	setting_info->genlist = genlist;

	navi_item = elm_naviframe_item_push(navi, STRING_SETTING, prev_btn, NULL, genlist, NULL);
	goto_if(!navi_item, error);

	setting_info->db = sticker_panel->db;

	evas_object_smart_callback_add(prev_btn, "clicked", __click_navi_prev_btn_cb, parent);

	return navi;

error:
	if (genlist) {
		__destroy_genlist(genlist);
	}

	if (prev_btn) {
		__destroy_navi_btn(prev_btn);
	}

	if (navi) {
		evas_object_del(navi);
	}

	return NULL;
}



static void __destroy_navi(Evas_Object *navi)
{
	Evas_Object *prev_btn = NULL;
	Evas_Object *next_btn = NULL;
	Evas_Object *genlist = NULL;

	ret_if(!navi);

	prev_btn = elm_object_part_content_unset(navi, "elm.swallow.prev_btn");
	__destroy_navi_btn(prev_btn);

	next_btn = elm_object_part_content_unset(navi, "elm.swallow.next_btn");
	__destroy_navi_btn(next_btn);

	genlist = elm_naviframe_item_pop(navi);
	__destroy_genlist(genlist);

	evas_object_del(navi);
}



static void __show_ctx_popup(setting_info_s *setting_info)
{
	ret_if(!setting_info);

	evas_object_show(setting_info->ctx_popup);

	setting_info->show_ctx_popup = EINA_TRUE;
}



static void __hide_ctx_popup(setting_info_s *setting_info)
{
	ret_if(!setting_info);

	elm_ctxpopup_dismiss(setting_info->ctx_popup);

	setting_info->show_ctx_popup = EINA_FALSE;
}



static void __destroy_ctx_popup(setting_info_s *setting_info)
{
	ret_if(!setting_info);

	evas_object_del(setting_info->ctx_popup);

	setting_info->ctx_popup = NULL;
	setting_info->show_ctx_popup = EINA_FALSE;
}



static void __click_prev_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *genlist = NULL;
	setting_info_s *setting_info = data;

	ret_if(!setting_info);
	ret_if(!obj);

	genlist = elm_naviframe_item_pop(setting_info->navi);
	ret_if(!genlist);

	__destroy_genlist(genlist);

	setting_info->view_type = SETTING_VIEW_NONE;
}



static void __update_toolbar_and_scroller(setting_info_s *setting_info)
{
	ret_if(!setting_info);

	_toolbar_reorder_item(setting_info->toolbar, setting_info->group_icon_info_list);
	_scroller_reorder_page(setting_info->scroller, setting_info->group_icon_info_list);
}




static void __reorder_group_icon_info_from_genlist(Evas_Object *genlist, setting_info_s *setting_info)
{
	Elm_Object_Item *item = NULL;
	group_icon_info_s *group_icon_info = NULL;

	ret_if(!genlist);
	ret_if(!setting_info);

	item = elm_genlist_first_item_get(genlist);
	ret_if(!item);

	while (item) {
		int index = 0;
		index = elm_genlist_item_index_get(item);

		group_icon_info = evas_object_data_get(item, PRIVATE_DATA_KEY_GROUP_ICON_INFO);
		goto_if(!group_icon_info, out);

		group_icon_info->ordering = index;

out:
		item= elm_genlist_item_next_get(item);
	}
}



static void __delete_group_icon_info_from_genlist(Evas_Object *genlist, setting_info_s *setting_info)
{
	Elm_Object_Item *item = NULL;
	group_icon_info_s *group_icon_info = NULL;

	ret_if(!genlist);
	ret_if(!setting_info);

	item = elm_genlist_first_item_get(genlist);
	ret_if(!item);

	item = elm_genlist_item_next_get(item);

	while (item) {
		Evas_Object *check = NULL;
		Eina_Bool check_on = EINA_FALSE;
		Eina_Bool ret = EINA_FALSE;

		group_icon_info = evas_object_data_get(item, PRIVATE_DATA_KEY_GROUP_ICON_INFO);
		goto_if(!group_icon_info, out);

		if (group_icon_info->removable == 0) {
			_E("This item is not removable");
			goto out;
		}

		check = elm_object_item_part_content_get(item, "elm.icon.right");
		goto_if(!check, out);

		check_on = elm_check_state_get(check);
		if (check_on) {
			_toolbar_remove_item(group_icon_info->toolbar_item);
			_scroller_remove_page(setting_info->scroller, group_icon_info->item_view);
			_page_destroy(group_icon_info->item_view);
			setting_info->group_icon_info_list = eina_list_remove(setting_info->group_icon_info_list, group_icon_info);

			ret = ecore_file_recursive_rm(group_icon_info->id);
			if (!ret) {
				_E("Failed to remove group icon file");
			}

			_group_icon_info_destroy(group_icon_info);
		}

out:
		item = elm_genlist_item_next_get(item);
	}
}



static void __click_next_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *genlist = NULL;
	setting_info_s *setting_info = data;

	ret_if(!setting_info);
	ret_if(!setting_info->group_icon_info_list);
	ret_if(!setting_info->db);
	ret_if(!obj);

	genlist = elm_naviframe_item_pop(setting_info->navi);
	ret_if(!genlist);

	switch(setting_info->view_type) {
	case SETTING_VIEW_REORDER:
		__reorder_group_icon_info_from_genlist(genlist, setting_info);
		break;
	case SETTING_VIEW_DELETE:
		__delete_group_icon_info_from_genlist(genlist, setting_info);
		break;
	default:
		_E("view type error(%d)", setting_info->view_type);
		return;
	}

	_group_icon_info_list_sort(setting_info->group_icon_info_list);
	_group_icon_info_list_trim(setting_info->group_icon_info_list, setting_info->db);

	__destroy_genlist(genlist);

	setting_info->view_type = SETTING_VIEW_NONE;

	/* update genlist */
	__update_genlist(setting_info);

	/* update toolbar & scroller */
	__update_toolbar_and_scroller(setting_info);
}



//@TODO: If it is not necessary, remove this func.
static Eina_Bool __naviframe_item_pop_cb(void *data, Elm_Object_Item *it)
{
	setting_info_s *setting_info = data;
	retv_if(!setting_info, EINA_FALSE);

	return EINA_TRUE;
}



static void __click_ctx_popup_item(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *next_btn = NULL;
	Evas_Object *prev_btn = NULL;
	Evas_Object *genlist = NULL;
	Elm_Object_Item *navi_item = NULL;
	setting_info_s *setting_info = NULL;
	setting_view_type_e view_type = (setting_view_type_e) data;

	ret_if(!obj);

	setting_info = evas_object_data_get(obj, PRIVATE_DATA_KEY_SETTING_INFO);
	ret_if(!setting_info);

	setting_info->view_type = view_type;

	genlist = __create_genlist(setting_info->navi, setting_info, view_type);
	ret_if(!genlist);

	prev_btn = __create_navi_prev_btn(setting_info->navi);
	goto_if(!prev_btn, error);

	evas_object_smart_callback_add(prev_btn, "clicked", __click_prev_btn_cb, setting_info);

	switch(view_type) {
	case SETTING_VIEW_REORDER:
		_D("Reorder");

		next_btn = __create_navi_next_btn(setting_info->navi, STRING_DONE);
		goto_if(!next_btn, error);

		navi_item = elm_naviframe_item_push(setting_info->navi, STRING_REORDER, prev_btn, NULL, genlist, NULL);
		goto_if(!navi_item, error);
		break;
	case SETTING_VIEW_DELETE:
		_D("Delete");

		next_btn = __create_navi_next_btn(setting_info->navi, STRING_DELETE);
		goto_if(!next_btn, error);

		navi_item = elm_naviframe_item_push(setting_info->navi, STRING_DELETE, prev_btn, NULL, genlist, NULL);
		goto_if(!navi_item, error);
		break;
	default:
		_E("view type error : %d", view_type);
		goto error;
	}

	evas_object_smart_callback_add(next_btn, "clicked", __click_next_btn_cb, setting_info);
	elm_object_item_part_content_set(navi_item, "title_right_btn", next_btn);
	elm_naviframe_item_pop_cb_set(navi_item, __naviframe_item_pop_cb, setting_info);

	__hide_ctx_popup(setting_info);

	return;

error:
	if (prev_btn) {
		__destroy_navi_btn(prev_btn);
	}

	if (next_btn) {
		__destroy_navi_btn(next_btn);
	}

	if (genlist) {
		__destroy_genlist(genlist);
	}
}



static Evas_Object *__create_ctx_popup(Evas_Object *parent)
{
	Evas_Object *ctx_popup = NULL;
	Elm_Object_Item *item = NULL;
	Evas_Coord x, y, w, h;
	setting_info_s *setting_info = NULL;

	retv_if(!parent, NULL);

	setting_info = evas_object_data_get(parent, PRIVATE_DATA_KEY_SETTING_INFO);
	retv_if(!setting_info, NULL);

	ctx_popup = elm_ctxpopup_add(parent);
	retv_if(!ctx_popup, NULL);

	evas_object_data_set(ctx_popup, PRIVATE_DATA_KEY_SETTING_INFO, (void *) setting_info);

	elm_object_style_set(ctx_popup, "more/default");
	elm_ctxpopup_auto_hide_disabled_set(ctx_popup, EINA_TRUE);
	eext_object_event_callback_add(ctx_popup, EEXT_CALLBACK_BACK, eext_ctxpopup_back_cb, NULL);
	eext_object_event_callback_add(ctx_popup, EEXT_CALLBACK_MORE, eext_ctxpopup_back_cb, NULL);

	//@TODO: need to change string
	item = elm_ctxpopup_item_append(ctx_popup, "Reorder", NULL, __click_ctx_popup_item, (void *) SETTING_VIEW_REORDER);
	if (!item) {
		_E("Failed to add ctx popup item : Reorder");
	}

	item = elm_ctxpopup_item_append(ctx_popup, "Delete", NULL, __click_ctx_popup_item, (void *) SETTING_VIEW_DELETE);
	if (!item) {
		_E("Failed to add ctx popup item : Delete");
	}

	elm_ctxpopup_direction_priority_set(ctx_popup, ELM_CTXPOPUP_DIRECTION_DOWN, ELM_CTXPOPUP_DIRECTION_DOWN, ELM_CTXPOPUP_DIRECTION_DOWN, ELM_CTXPOPUP_DIRECTION_DOWN);

	evas_object_geometry_get(parent, &x, &y, &w, &h);
	evas_object_resize(ctx_popup, w, ELM_SCALE_SIZE(120 * 3));
	evas_object_move(ctx_popup, ELM_SCALE_SIZE((x + w / 2)), ELM_SCALE_SIZE((y + h)));

	return ctx_popup;
}



static Eina_Bool __key_press_cb(void *data, int type, void *event)
{
	Ecore_Event_Key *ev = event;
	retv_if(!ev, ECORE_CALLBACK_CANCEL);

	_D("Key press : %s", ev->keyname);

	return ECORE_CALLBACK_CANCEL;
}



static Eina_Bool __key_release_cb(void *data, int type, void *event)
{
	Evas_Object *layout = data;
	Ecore_Event_Key *ev = event;
	setting_info_s *setting_info = NULL;

	retv_if(!layout, ECORE_CALLBACK_CANCEL);
	retv_if(!ev, ECORE_CALLBACK_CANCEL);

	layout = data;

	setting_info = evas_object_data_get(layout, PRIVATE_DATA_KEY_SETTING_INFO);
	retv_if(!setting_info, ECORE_CALLBACK_CANCEL);

	_D("Key release : %s", ev->keyname);

	if (!strcmp(ev->keyname, KEY_NAME_MENU)) {
		if (setting_info->view_type != SETTING_VIEW_NONE) {
			_E("setting view is not NONE");
			return ECORE_CALLBACK_CANCEL;
		}

		if (setting_info->show_ctx_popup) {
			__hide_ctx_popup(setting_info);
		} else {
			__show_ctx_popup(setting_info);
		}
	} else if (!strcmp(ev->keyname, KEY_NAME_BACK)) {
		//@TODO: add code for HW back key event
	}

	return ECORE_CALLBACK_CANCEL;
}



Evas_Object *_setting_create(Evas_Object *parent, sticker_panel_h sticker_panel)
{
	Evas_Object *layout = NULL;
	Evas_Object *navi = NULL;
	Evas_Object *ctx_popup = NULL;
	setting_info_s *setting_info = NULL;

	retv_if(!parent, NULL);
	retv_if(!sticker_panel, NULL);

	layout = __create_layout(parent);
	retv_if(!layout, NULL);

	setting_info = calloc(1, sizeof(setting_info_s));
	goto_if(!setting_info, error);

	evas_object_data_set(layout, PRIVATE_DATA_KEY_SETTING_INFO, setting_info);

	setting_info->toolbar = sticker_panel->toolbar[sticker_panel->current_category];
	setting_info->scroller = sticker_panel->scroller[sticker_panel->current_category];
	setting_info->group_icon_info_list = sticker_panel->group_icon_info_list;

	navi = __create_navi(layout, sticker_panel);
	goto_if(!navi, error);
	elm_object_part_content_set(layout, "naviframe", navi);

	setting_info->navi = navi;

	ctx_popup = __create_ctx_popup(layout);
	goto_if(!ctx_popup, error);

	setting_info->ctx_popup = ctx_popup;

	setting_info->key_press = ecore_event_handler_add(ECORE_EVENT_KEY_DOWN, __key_press_cb, NULL);
	if (!setting_info->key_press) {
		_E("Failed to add event handler : key press");
	}

	setting_info->key_release = ecore_event_handler_add(ECORE_EVENT_KEY_UP, __key_release_cb, layout);
	if (!setting_info->key_release) {
		_E("Failed to add event handler : key relase");
	}

	setting_info->view_type = SETTING_VIEW_NONE;

	return layout;

error:
	_setting_destroy(layout);

	return NULL;
}



void _setting_destroy(Evas_Object *setting)
{
	Evas_Object *navi = NULL;
	setting_info_s *setting_info = NULL;

	ret_if(!setting);

	navi = elm_object_part_content_unset(setting, "naviframe");
	ret_if(!navi);

	setting_info = evas_object_data_del(setting, PRIVATE_DATA_KEY_SETTING_INFO);

	if (setting_info) {
		if (setting_info->ctx_popup) {
			__destroy_ctx_popup(setting_info);
		}

		if (setting_info->key_press) {
			ecore_event_handler_del(setting_info->key_press);
		}

		if (setting_info->key_release) {
			ecore_event_handler_del(setting_info->key_release);
		}

		__destroy_navi(navi);
		__destroy_layout(setting);

		free(setting_info);
	}
}
