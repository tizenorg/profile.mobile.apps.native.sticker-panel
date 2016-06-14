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

#include <Elementary.h>
#include <efl_extension.h>

#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

#include "sticker_panel.h"
#include "sample.h"
#include "log.h"

#define STICKER_ICON_INFO_KEY "__STICKER_icon_info_KEY__"

#define STICKER_IMG_NAME_TOKEN_SUB "sub"
#define STICKER_IMG_NAME_TOKEN_TH "th"
#define STICKER_IMG_NAME_TOKEN_GIF ".gif"

#define DEFAULT_INTERVAL 1000

typedef struct _icon_info_s {
	/* Innate features */
	char *value;
	char *dir_path;
	char *keyword;
	char *thumbnail_file;

	int type;
	int repeat;
	int interval;
	int play_type;
	int th_frame;
	int width;
	int height;

	Eina_List *icon_image_list;
	Eina_Bool animation;

	/* Acquired features */
	Elm_Object_Item *item;
	Evas_Object *icon;
	Ecore_Animator *animator;

	int current_repeat;
	int current_frame;
} icon_info_s;

typedef struct _icon_image_s {
	char *file;
	int frame_order;
	int diff_time;
} icon_image_s;



static int __icon_image_list_sort_cb(const void *d1, const void *d2)
{
	icon_image_s *tmp1 = (icon_image_s *) d1;
	icon_image_s *tmp2 = (icon_image_s *) d2;

	if (tmp1 == NULL || tmp1->file == NULL) {
		return 1;
	} else if (tmp2 == NULL || tmp2->file == NULL) {
		return -1;
	}

	return strcmp(tmp1->file, tmp2->file);
}



static icon_info_s *_icon_info_create_directory(const char *dir_path)
{
	icon_info_s *icon_info = NULL;
	DIR *dir_info = NULL;
	struct dirent ent_struct;
	struct dirent *dir_entry = NULL;
	int ret = 0;

	retv_if(!dir_path, NULL);

	icon_info = calloc(1, sizeof(icon_info_s));
	retv_if(!icon_info, NULL);

	icon_info->type = ICON_INFO_TYPE_DIRECTORY;
	icon_info->dir_path = strdup(dir_path);
	goto_if(!icon_info->dir_path, error);

	dir_info = opendir(dir_path);
	goto_if(!dir_info, error);

	while (!readdir_r(dir_info, &ent_struct, &dir_entry) && dir_entry) {
		icon_image_s *icon_image = NULL;
		char *d_name = NULL;
		char *filename = NULL;
		char *ext = NULL;
		char *tmp = NULL;
		char *save_ptr = NULL;
		char full_path[PATH_LEN] = {0, };
		char thumbnail_file[PATH_LEN] = {0, };
		struct stat stat_buf = {0, };

		snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, dir_entry->d_name);
		lstat(full_path, &stat_buf);
		if (S_ISDIR(stat_buf.st_mode))
			continue;

		if (strstr(dir_entry->d_name, ".gif")
			|| strstr(dir_entry->d_name, "_th.")) {
			continue;
		}

		d_name = strdup(dir_entry->d_name);
		continue_if(!d_name);

		icon_image = calloc(1, sizeof(icon_image_s));
		goto_if(!icon_image, not_sticker);

		icon_image->file = strdup(full_path);
		goto_if(!icon_image->file, not_sticker);

		filename = strtok_r(d_name, ".", &save_ptr);
		goto_if(!filename, not_sticker);

		ext = strtok_r(NULL, ".", &save_ptr);
		goto_if(!ext, not_sticker);

		tmp = strtok_r(filename, "_", &save_ptr);
		goto_if(!tmp, not_sticker);

		if (!icon_info->keyword) {
			icon_info->keyword = strdup(tmp);
			goto_if(!icon_info->keyword, not_sticker);
		}

		if ((tmp = strtok_r(NULL, "_", &save_ptr))) {
			if (!strcmp(tmp, STICKER_IMG_NAME_TOKEN_SUB)) {
				goto not_sticker;
			} else if (!strcmp(tmp, STICKER_IMG_NAME_TOKEN_TH)) {
				if (icon_info->thumbnail_file)
					goto not_sticker;

				snprintf(thumbnail_file
						, sizeof(thumbnail_file)
						, "%s/%s_%s.%s"
						, icon_info->dir_path
						, icon_info->keyword
						, STICKER_IMG_NAME_TOKEN_TH
						, ext);
				icon_info->thumbnail_file = strdup(thumbnail_file);
				goto_if(!icon_info->thumbnail_file, not_sticker);
			} else {
				icon_image->frame_order = atoi(tmp);
			}
		} else goto next;

		if ((tmp = strtok_r(NULL, "_", &save_ptr)))
			icon_image->diff_time = atoi(tmp);
		else
			goto next;

		if ((tmp = strtok_r(NULL, "_", &save_ptr)))
			icon_info->repeat = atoi(tmp);
		else
			goto next;

		if ((tmp = strtok_r(NULL, "_", &save_ptr)))
			icon_info->interval = atoi(tmp);
		else
			goto next;

		if ((tmp = strtok_r(NULL, "_", &save_ptr)))
			icon_info->play_type = atoi(tmp);
		else
			goto next;

		if ((tmp = strtok_r(NULL, "_", &save_ptr)))
			icon_info->th_frame = atoi(tmp);

next:
		icon_info->icon_image_list = eina_list_append(icon_info->icon_image_list, icon_image);
		goto_if(!icon_info->icon_image_list, not_sticker);
		free(d_name);
		continue;

not_sticker:
		if (icon_image) {
			free(icon_image->file);
			free(icon_image);
		}
		free(d_name);
	}

	icon_info->icon_image_list = eina_list_sort(
			icon_info->icon_image_list
			, eina_list_count(icon_info->icon_image_list)
			, __icon_image_list_sort_cb);

	ret = closedir(dir_info);
	if (ret != 0) {
		_E("Failed to close directory(%s)", dir_path);
	}
	return icon_info;

error:
	if (icon_info) {
		free(icon_info->dir_path);
		free(icon_info);
	}
	return NULL;
}



static icon_info_s *_icon_info_create_file(const char *file_path)
{
	icon_info_s *icon_info = NULL;

	retv_if(!file_path, NULL);

	icon_info = calloc(1, sizeof(icon_info_s));
	retv_if(!icon_info, NULL);

	icon_info->thumbnail_file = strdup(file_path);
	goto_if(!icon_info->thumbnail_file, error);

	icon_info->type = ICON_INFO_TYPE_FILE;

	return icon_info;

error:
	if (icon_info) {
		free(icon_info->thumbnail_file);
		free(icon_info);
	}
	return NULL;
}



static void _icon_info_destroy(icon_info_s *icon_info)
{
	icon_image_s *icon_image = NULL;

	ret_if(!icon_info);

	free(icon_info->dir_path);
	free(icon_info->keyword);
	free(icon_info->thumbnail_file);

	EINA_LIST_FREE(icon_info->icon_image_list, icon_image) {
		if (!icon_image) {
			continue;
		}

		free(icon_image->file);
		free(icon_image);
	}

	free(icon_info);
}



static Eina_Bool _animator_cb(void *data)
{
	icon_info_s *icon_info = data;
	icon_image_s *icon_image = NULL;

	goto_if(!icon_info, error);
	goto_if(!icon_info->icon, error);
	goto_if(!icon_info->icon_image_list, error);

	icon_image = eina_list_nth(icon_info->icon_image_list, icon_info->current_frame);
	if (!icon_image) {
		if (icon_info->current_repeat < icon_info->repeat) {
			/* restart animation */
			icon_info->current_repeat++;
			icon_info->current_frame = 0;
			return ECORE_CALLBACK_RENEW;
		}
		icon_info->current_repeat = 0;
		icon_info->current_frame = 0;
		goto error;
	}

	ecore_animator_frametime_set((double) icon_image->diff_time / 1000);

	if (!elm_image_file_set(icon_info->icon, icon_image->file, NULL)) {
		_E("Failed to set icon");
	}

	icon_info->current_frame++;

	return ECORE_CALLBACK_RENEW;

error:
	icon_info->animator = NULL;
	return ECORE_CALLBACK_CANCEL;
}



static void _del_animated_icon_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	icon_info_s *icon_info = data;

	ret_if(!icon_info);

	if (!icon_info->animator)
		return;

	ecore_animator_del(icon_info->animator);
	icon_info->animator= NULL;
}



static Evas_Object *_create_animated_icon(Evas_Object *parent, icon_info_s *icon_info)
{
	icon_image_s *icon_image = NULL;
	Evas_Object *icon = NULL;
	int list_count = 0;

	retv_if(!parent, NULL);
	retv_if(!icon_info, NULL);

	icon = elm_image_add(parent);
	retv_if(!icon, NULL);

	icon_image = eina_list_nth(icon_info->icon_image_list, 0);
	retv_if(!icon_image, NULL);

	if (!elm_image_file_set(icon, icon_image->file, NULL)) {
		_E("Failed to set icon");
		evas_object_del(icon);
		return NULL;
	}
	evas_object_event_callback_add(icon, EVAS_CALLBACK_DEL, _del_animated_icon_cb, icon_info);

	list_count = eina_list_count(icon_info->icon_image_list);
	if (list_count > 1) {
		icon_info->animator = ecore_animator_add(_animator_cb, icon_info);
		if (!icon_info->animator) {
			_E("Failed to add animator");
			return NULL;
		}
	}

	return icon;
}



static Evas_Object *_create_static_icon(Evas_Object *parent, const char *icon_path)
{
	Evas_Object *icon = NULL;

	retv_if(!parent, NULL);
	retv_if(!icon_path, NULL);

	icon = elm_image_add(parent);
	retv_if(!icon, NULL);

	if (!elm_image_file_set(icon, icon_path, NULL)) {
		_E("Failed to set icon");
		evas_object_del(icon);
		return NULL;
	}

	return icon;
}



void _destroy_icon(Evas_Object *icon)
{
	ret_if(!icon);
	evas_object_del(icon);
}



static void _icon_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	icon_info_s *icon_info = (icon_info_s *) data;
	int count = 0;

	ret_if(!icon_info);

	if (icon_info->animator) {
		ecore_animator_del(icon_info->animator);
		icon_info->animator = NULL;
	}

	count = eina_list_count(icon_info->icon_image_list);
	if (count == 0)
		return;

	icon_info->animator = ecore_animator_add(_animator_cb, icon_info);
	if (!icon_info->animator)
		_E("Failed to add animator");
}



Evas_Object *sticker_icon_create(Evas_Object *parent, int type, const char *value)
{
	icon_info_s *icon_info = NULL;

	retv_if(!parent, NULL);
	retv_if(!type, NULL);
	retv_if(!value, NULL);

	_D("type : %d, value : %s", type, value);

	if (type == ICON_INFO_TYPE_DIRECTORY) {
		icon_info = _icon_info_create_directory(value);
		retv_if(!icon_info, NULL);

		icon_info->icon = _create_animated_icon(parent, icon_info);
		goto_if(!icon_info->icon, error);
	} else if (type == ICON_INFO_TYPE_FILE) {
		icon_info = _icon_info_create_file(value);
		retv_if(!icon_info, NULL);

		icon_info->icon = _create_static_icon(parent, value);
		goto_if(!icon_info->icon, error);
	} else {
		_E("type error(%d)", type);
		goto error;
	}

	evas_object_data_set(icon_info->icon, STICKER_ICON_INFO_KEY, icon_info);
	evas_object_smart_callback_add(icon_info->icon, "clicked", _icon_clicked_cb, icon_info);

	return icon_info->icon;

error:
	if (icon_info) {
		if (icon_info->icon) {
			evas_object_data_del(icon_info->icon, STICKER_ICON_INFO_KEY);
			evas_object_del(icon_info->icon);
		}

		_icon_info_destroy(icon_info);
	}

	return NULL;
}



void sticker_icon_destroy(Evas_Object *icon)
{
	icon_info_s *icon_info = NULL;

	ret_if(!icon);

	icon_info = evas_object_data_del(icon, STICKER_ICON_INFO_KEY);
	ret_if(!icon_info);

	_destroy_icon(icon_info->icon);
	_icon_info_destroy(icon_info);
}
