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

#include <Evas.h>
#include <Elementary.h>

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

#include "sticker_panel.h"
#include "sticker_panel_internal.h"
#include "conf.h"
#include "ui_manager.h"
#include "log.h"
#include "icon_info.h"
#include "db.h"

#define STICKER_IMG_NAME_TOKEN_SUB "sub"
#define STICKER_IMG_NAME_TOKEN_TH "th"



static int __icon_info_image_list_sort_cb(const void *d1, const void *d2)
{
	icon_info_image_s *tmp1 = (icon_info_image_s *) d1;
	icon_info_image_s *tmp2 = (icon_info_image_s *) d2;

	if (tmp1 == NULL || tmp1->file == NULL) {
		return 1;
	} else if (tmp2 == NULL || tmp2->file == NULL) {
		return -1;
	}

	return strcmp(tmp1->file, tmp2->file);
}

static icon_info_s *__icon_info_parse_directory(const char *dir_path)
{
	icon_info_s *icon_info = NULL;
	DIR *dir_info = NULL;
	struct dirent *dir_entry = NULL;
	int ret = 0;

	retv_if(!dir_path, NULL);

	icon_info = calloc(1, sizeof(icon_info_s));
	retv_if(!icon_info, NULL);

	icon_info->dir_path = strdup(dir_path);
	goto_if(!icon_info->dir_path, error);
	icon_info->type = ICON_INFO_TYPE_DIRECTORY;

	dir_info = opendir(dir_path);
	goto_if(!dir_info, error);

	while ((dir_entry = readdir(dir_info))) {
		icon_info_image_s *icon_info_image = NULL;
		char *d_name = NULL;
		char *filename = NULL;
		char *ext = NULL;
		char *tmp = NULL;
		char full_path[PATH_LEN] = {0, };
		char thumbnail_file[PATH_LEN] = {0, };
		struct stat stat_buf = {0, };

		snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, dir_entry->d_name);
		lstat(full_path, &stat_buf);
		if (S_ISDIR(stat_buf.st_mode))
			continue;

		d_name = strdup(dir_entry->d_name);
		continue_if(!d_name);

		icon_info_image = calloc(1, sizeof(icon_info_image_s));
		goto_if(!icon_info_image, not_sticker);

		icon_info_image->file = strdup(full_path);
		goto_if(!icon_info_image->file, not_sticker);

		filename = strtok(d_name, ".");
		goto_if(!filename, not_sticker);

		ext = strtok(NULL, ".");
		goto_if(!ext, not_sticker);

		tmp = strtok(filename, "_");
		goto_if(!tmp, not_sticker);

		if (!icon_info->keyword) {
			icon_info->keyword = strdup(tmp);
			goto_if(!icon_info->keyword, not_sticker);
		}

		if ((tmp = strtok(NULL, "_"))) {
			if (!strcmp(tmp, STICKER_IMG_NAME_TOKEN_SUB)) {
				goto not_sticker;
			} else if (!strcmp(tmp, STICKER_IMG_NAME_TOKEN_TH)) {
				if (icon_info->thumbnail_file) {
					goto not_sticker;
				}

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
				icon_info_image->frame_order = atoi(tmp);
			}
		} else goto next;

		if ((tmp = strtok(NULL, "_")))
			icon_info_image->diff_time = atoi(tmp);
		else goto next;

		if ((tmp = strtok(NULL, "_")))
			icon_info->repeat = atoi(tmp);
		else goto next;

		if ((tmp = strtok(NULL, "_")))
			icon_info->interval = atoi(tmp);
		else goto next;

		if ((tmp = strtok(NULL, "_")))
			icon_info->play_type = atoi(tmp);
		else goto next;

		if ((tmp = strtok(NULL, "_")))
			icon_info->th_frame = atoi(tmp);

next:
		icon_info->icon_info_image_list = eina_list_append(icon_info->icon_info_image_list, icon_info_image);
		goto_if(!icon_info->icon_info_image_list, not_sticker);
		free(d_name);
		continue;

not_sticker:
		if (icon_info_image) {
			free(icon_info_image->file);
			free(icon_info_image);
		}
		free(d_name);
	}

	icon_info->icon_info_image_list = eina_list_sort(
					icon_info->icon_info_image_list
					, eina_list_count(icon_info->icon_info_image_list)
					, __icon_info_image_list_sort_cb);

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



static icon_info_s *__icon_info_parse_file(const char *file_path)
{
	icon_info_s *icon_info = NULL;

	retv_if(!file_path, NULL);

	icon_info = calloc(1, sizeof(icon_info_s));
	retv_if(!icon_info, NULL);

	icon_info->dir_path = strdup(file_path);
	goto_if(!icon_info->dir_path, error);
	icon_info->type = ICON_INFO_TYPE_FILE;

	return icon_info;

error:
	free(icon_info);
	return NULL;
}



static icon_info_s *__icon_info_parse_id(const char *id)
{
	icon_info_s *icon_info = NULL;

	/* FIXME */

	return icon_info;
}



icon_info_s *_icon_info_create(const char *id, int type)
{
	icon_info_s *icon_info = NULL;

	retv_if(!id, NULL);

	switch (type) {
	case ICON_INFO_TYPE_DIRECTORY:
		icon_info = __icon_info_parse_directory(id);
		break;
	case ICON_INFO_TYPE_FILE:
		icon_info = __icon_info_parse_file(id);
		break;
	case ICON_INFO_TYPE_ID:
		icon_info = __icon_info_parse_id(id);
		break;
	default:
		_E("Cannot reach here, %s, %d", id, type);
		break;
	}
	retv_if(!icon_info, NULL);

	return icon_info;
}



icon_info_s *_icon_info_create_thumbnail(const char *thumbnail_file)
{
	icon_info_s *icon_info = NULL;
	char *tmp = NULL;

	retv_if(!thumbnail_file, NULL);

	icon_info = calloc(1, sizeof(icon_info_s));
	retv_if(!icon_info, NULL);

	tmp = strdup(thumbnail_file);
	if (!tmp) {
		free(icon_info);
		return NULL;
	}

	icon_info->thumbnail_file = tmp;

	return icon_info;
}



void _icon_info_destroy(icon_info_s *icon_info)
{
	icon_info_image_s *icon_info_image = NULL;

	ret_if(!icon_info);

	free(icon_info->dir_path);
	free(icon_info->keyword);
	free(icon_info->thumbnail_file);

	EINA_LIST_FREE(icon_info->icon_info_image_list, icon_info_image) {
		if (!icon_info_image) {
			continue;
		}

		free(icon_info_image->file);
		free(icon_info_image);
	}

	if (icon_info->icon) {
		evas_object_del(icon_info->icon);
	}

	free(icon_info);
}



Eina_List *_icon_info_list_create_recent_package(sqlite3 *db)
{
	Eina_List *icon_info_list = NULL;
	int ret = STICKER_PANEL_ERROR_NONE;

	ret = _db_list_recent_icon(db, &icon_info_list, 40);
	if (ret != STICKER_PANEL_ERROR_NONE) {
		_D("There is no recent icons");
	}

	return icon_info_list;
}



Eina_List *_icon_info_list_create_preset_package(const char *dir_path)
{
	Eina_List *icon_info_list = NULL;
	DIR *dir_info = NULL;
	struct dirent *dir_entry = NULL;
	struct stat stat_buf;
	int ret = 0;

	retv_if(!dir_path, NULL);

	dir_info = opendir(dir_path);
	retv_if(!dir_info, NULL);

	while ((dir_entry = readdir(dir_info))) {
		char icon_path[PATH_LEN] = {0, };
		snprintf(icon_path, sizeof(icon_path), "%s/%s", dir_path, dir_entry->d_name);

		memset(&stat_buf, 0, sizeof(struct stat));
		lstat(icon_path, &stat_buf);

		/* FIXME : We can get files */
		if (!S_ISDIR(stat_buf.st_mode)) {
			continue;
		}

		if (!strcmp(".", dir_entry->d_name) || !strcmp("..", dir_entry->d_name)) {
			continue;
		}

		if (strcmp(TAB_THUMBNAIL_DIR_PATH, dir_entry->d_name)) {
			icon_info_s *icon_info = NULL;
			icon_info = _icon_info_create(icon_path, ICON_INFO_TYPE_DIRECTORY);
			if (icon_info) {
				icon_info_list = eina_list_append(icon_info_list, icon_info);
			}
		}
	}

	ret = closedir(dir_info);
	if (ret != 0) {
		_E("Failed to close directory(%s)", dir_path);
	}

	return icon_info_list;
}



Eina_List *_icon_info_list_create_user_defined(const char *dir_path)
{
	Eina_List *icon_info_list = NULL;

	/* FIXME */

	return icon_info_list;
}



void _icon_info_list_destroy(Eina_List *icon_info_list)
{
	icon_info_s *icon_info = NULL;

	ret_if(!icon_info_list);

	EINA_LIST_FREE(icon_info_list, icon_info) {
		_icon_info_destroy(icon_info);
	}
}
