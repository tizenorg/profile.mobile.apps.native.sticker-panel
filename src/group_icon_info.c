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

#include <stdlib.h>
#include <sqlite3.h>

#include "conf.h"
#include "log.h"
#include "sticker_panel_internal.h"
#include "group_icon_info.h"
#include "icon_info.h"
#include "db.h"
#include "conf.h"



static const char *const RECENT_NAME = "Recent";
static const char *const USER_DEFINED_ID = "__sticker_panel_user_defined_tabbar__";
static const char *const USER_DEFINED_NAME = "User";
static const char *const USER_DIR = "/usr";



group_icon_info_s *_group_icon_info_create(const char *id, const char *name, const char *repository, int ordering, int category)
{
	group_icon_info_s *group_icon_info = NULL;

	retv_if(!id, NULL);
	retv_if(!name, NULL);

	group_icon_info = calloc(1, sizeof(group_icon_info_s));
	retv_if(!group_icon_info, NULL);

	group_icon_info->id = strdup(id);
	goto_if(!group_icon_info->id, error);

	group_icon_info->name = strdup(name);
	goto_if(!group_icon_info->name, error);

	if (repository) {
		group_icon_info->repository = strdup(repository);
		goto_if(!group_icon_info->repository, error);
	}

	group_icon_info->ordering = ordering;
	group_icon_info->category = category;
	group_icon_info->removable = !!strncmp(USER_DIR, group_icon_info->id, strlen(USER_DIR));

	return group_icon_info;

error:
	if (group_icon_info) {
		free(group_icon_info->id);
		free(group_icon_info->name);
		free(group_icon_info->repository);
		free(group_icon_info);
	}

	return NULL;
}



void _group_icon_info_destroy(group_icon_info_s *group_icon_info)
{
	ret_if(!group_icon_info);

	free(group_icon_info->id);
	free(group_icon_info->name);
	free(group_icon_info->repository);
	free(group_icon_info->toolbar_icon_path);
	if (group_icon_info->list) {
		_icon_info_list_destroy(group_icon_info->list);
	}

	free(group_icon_info);
}



static Eina_List *__create_recent_group_icon_info_list(sqlite3 *db)
{
	Eina_List *group_icon_info_list = NULL;
	group_icon_info_s *group_icon_info = NULL;

	retv_if(!db, NULL);

	group_icon_info = _group_icon_info_create(RECENT_ID, RECENT_NAME, NULL, 0, 0);
	retv_if(!group_icon_info, NULL);

	group_icon_info->list = _icon_info_list_create_recent_package(db);
	if (!group_icon_info->list) {
		_E("cannot create the recent list");
	}
	group_icon_info->permutable = 0;
	group_icon_info->initialized = 1;
	group_icon_info->recent = 1;
	group_icon_info->toolbar_icon_path = strdup(DEFAULT_ICON);
	if (!group_icon_info->toolbar_icon_path)
		_E("cannot get toolbar's icon path");

	group_icon_info_list = eina_list_append(group_icon_info_list, group_icon_info);

	return group_icon_info_list;
}



static char *__strdup_tab_thumbnail_path(const char *dir_path)
{
	char file_path[PATH_LEN] = {0, };
	char *tab_thumbnail = NULL;
	DIR *dir_info = NULL;
	struct dirent ent_struct;
	struct dirent *dir_entry = NULL;
	struct stat stat_buf = {0, };
	int ret = 0;

	retv_if(!dir_path, NULL);

	dir_info = opendir(dir_path);
	retv_if(!dir_info, NULL);

	while (!readdir_r(dir_info, &ent_struct, &dir_entry) && dir_entry) {
		if (!strcmp(".", dir_entry->d_name) || !strcmp("..", dir_entry->d_name))
			continue;

		snprintf(file_path, sizeof(file_path), "%s/%s", dir_path, dir_entry->d_name);
		lstat(file_path, &stat_buf);
		if (S_ISDIR(stat_buf.st_mode))
			continue;

		break;
	}

	ret = closedir(dir_info);
	if (ret != 0) {
		_E("Failed to close directory(%s)", dir_path);
	}

	tab_thumbnail = strdup(file_path);
	if (!tab_thumbnail) {
		_E("Failed to duplicate string");
		return NULL;
	}

	return tab_thumbnail;
}



static Eina_List *__append_preset_group_icon_info_list(Eina_List *group_icon_info_list, const char *preset_dir_path, sqlite3 *db)
{
	DIR *dir_info = NULL;
	struct dirent ent_struct;
	struct dirent *dir_entry = NULL;
	struct stat stat_buf;
	int max = 0;
	int ret = 0;

	retv_if(!preset_dir_path, group_icon_info_list);

	_D("preset dir path : %s", preset_dir_path);

	dir_info = opendir(preset_dir_path);
	retv_if(!dir_info, group_icon_info_list);

	max = eina_list_count(group_icon_info_list);
	while (!readdir_r(dir_info, &ent_struct, &dir_entry) && dir_entry) {
		group_icon_info_s *group_icon_info = NULL;
		char dir_path[PATH_LEN] = {0, };
		char icon_path[PATH_LEN] = {0, };

		snprintf(dir_path, sizeof(dir_path), "%s/%s", preset_dir_path, dir_entry->d_name);

		memset(&stat_buf, 0, sizeof(struct stat));
		lstat(dir_path, &stat_buf);

		if (!S_ISDIR(stat_buf.st_mode)) {
			continue;
		}

		if (!strcmp(".", dir_entry->d_name) || !strcmp("..", dir_entry->d_name)) {
			continue;
		}

		group_icon_info = _group_icon_info_list_get(group_icon_info_list, dir_path);
		if (!group_icon_info) {
			group_icon_info = _group_icon_info_create(dir_path, dir_entry->d_name, NULL, max, 0);
			break_if(!group_icon_info);
			group_icon_info_list = eina_list_append(group_icon_info_list, group_icon_info);

			ret = _db_insert_group_icon(db, dir_path, dir_entry->d_name, NULL, max, 0);
			if (STICKER_PANEL_ERROR_NONE != ret)
				_E("cannot insert a group icon");

			max++;
		}

		/* OPTIMIZATION : We've postponed these routines to the animator */
		//group_icon_info->list = _icon_info_list_create_preset_package(dir_path);

		snprintf(icon_path, sizeof(icon_path), "%s/%s", dir_path, TAB_THUMBNAIL_DIR_PATH);
		group_icon_info->toolbar_icon_path = __strdup_tab_thumbnail_path(icon_path);
		if (!group_icon_info->toolbar_icon_path)
			_E("cannot get toolbar's icon path");

		group_icon_info->permutable = 1;
		group_icon_info->initialized = 1;
	}

	ret = closedir(dir_info);
	if (ret != 0) {
		_E("Failed to close directory");
	}

	return group_icon_info_list;
}



static Eina_List *__append_user_group_icon_info_list(Eina_List *group_icon_info_list, sqlite3 *db)
{
	group_icon_info_s *group_icon_info = NULL;

	retv_if(!db, group_icon_info_list);

	group_icon_info = _group_icon_info_create(USER_DEFINED_ID, USER_DEFINED_NAME, NULL, 0, 0);
	retv_if(!group_icon_info, group_icon_info_list);

	group_icon_info->list = _icon_info_list_create_user_defined(USER_DEFINED_DIR_PATH);
	if (!group_icon_info->list) {
		_E("No user defined stickers");
		_group_icon_info_destroy(group_icon_info);
		return group_icon_info_list;
	}
	group_icon_info->permutable = 0;
	group_icon_info->initialized = 1;
	group_icon_info->toolbar_icon_path = strdup(DEFAULT_ICON);
	if (!group_icon_info->toolbar_icon_path)
		_E("cannot get toolbar's icon path");

	group_icon_info_list = eina_list_append(group_icon_info_list, group_icon_info);

	return group_icon_info_list;
}



Eina_List *_group_icon_info_list_create(sqlite3 *db)
{
	Eina_List *group_icon_info_list = NULL;
	Eina_List *list = NULL;
	int ret = STICKER_PANEL_ERROR_NONE;

	ret = _db_list_group_icon(db, &group_icon_info_list);
	retv_if(STICKER_PANEL_ERROR_NONE != ret, NULL);

	list = __create_recent_group_icon_info_list(db);
	goto_if(!list, error);
	group_icon_info_list = eina_list_merge(list, group_icon_info_list);

	list = __append_preset_group_icon_info_list(group_icon_info_list, PRELOADED_PRESET_DIR_PATH, db);
	goto_if(!list, error);

	list = __append_preset_group_icon_info_list(group_icon_info_list, DOWNLOADED_PRESET_DIR_PATH, db);
	if (!list) {
		_D("No downloaded preset");
	}

	list = __append_user_group_icon_info_list(group_icon_info_list, db);
	if (!list) {
		_D("No user defined preset");
	}

	group_icon_info_list = _group_icon_info_list_sort(group_icon_info_list);
	group_icon_info_list = _group_icon_info_list_trim(group_icon_info_list, db);

	return group_icon_info_list;

error:
	_group_icon_info_list_destroy(group_icon_info_list);
	return NULL;
}



void _group_icon_info_list_destroy(Eina_List *group_icon_info_list)
{
	group_icon_info_s *group_icon_info = NULL;

	ret_if(!group_icon_info_list);

	EINA_LIST_FREE(group_icon_info_list, group_icon_info) {
		_group_icon_info_destroy(group_icon_info);
	}
}



static int __group_icon_info_list_sort_cb(const void *d1, const void *d2)
{
	const group_icon_info_s *tmp1 = d1;
	const group_icon_info_s *tmp2 = d2;

	if (!tmp1)
		return 1;
	else if (!tmp2)
		return -1;

	return tmp1->ordering - tmp2->ordering;
}



Eina_List *_group_icon_info_list_sort(Eina_List *group_icon_info_list)
{
	group_icon_info_list = eina_list_sort(group_icon_info_list
					, eina_list_count(group_icon_info_list)
					, __group_icon_info_list_sort_cb);

	return group_icon_info_list;
}



Eina_List *_group_icon_info_list_trim(Eina_List *group_icon_info_list, sqlite3 *db)
{
	const Eina_List *l = NULL;
	const Eina_List *ln = NULL;
	group_icon_info_s *group_icon_info = NULL;
	int ordering = 0;

	EINA_LIST_FOREACH_SAFE(group_icon_info_list, l, ln, group_icon_info) {
		int ret = STICKER_PANEL_ERROR_NONE;

		if (!group_icon_info->initialized) {
			_D("Group icon[%s] is removed", group_icon_info->id);
			ret = _db_delete_group_icon(db, group_icon_info->id);
			if (STICKER_PANEL_ERROR_NONE != ret) {
				_E("cannot delete a group icon in the DB");
			}
			group_icon_info_list = eina_list_remove(group_icon_info_list, group_icon_info);
			_group_icon_info_destroy(group_icon_info);
		} else if (group_icon_info->ordering != ordering) {
			group_icon_info->ordering = ordering;
			ret = _db_update_group_icon(db, group_icon_info->id, ordering);
			if (STICKER_PANEL_ERROR_NONE != ret) {
				_E("cannot update a group icon to the DB");
			}
			_D("Group icon[%s]'s ordering is %d", group_icon_info->id, group_icon_info->ordering);
			ordering++;
		} else {
			_D("Group icon[%s]'s ordering is %d", group_icon_info->id, group_icon_info->ordering);
			ordering++;
		}
	}

	return group_icon_info_list;
}



group_icon_info_s *_group_icon_info_list_get(Eina_List *group_icon_info_list, const char *id)
{
	const Eina_List *l = NULL;
	group_icon_info_s *group_icon_info = NULL;

	retv_if(!group_icon_info_list, NULL);
	retv_if(!id, NULL);

	EINA_LIST_FOREACH(group_icon_info_list, l, group_icon_info) {
		continue_if(!group_icon_info->id);
		if (!strcmp(group_icon_info->id, id)) {
			return group_icon_info;
		}
	}

	return NULL;
}
