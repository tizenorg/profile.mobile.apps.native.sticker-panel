/*
 * Samsung API
 * Copyright (c) 2009-2015 Samsung Electronics Co., Ltd.
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

#include <Evas.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "db.h"
#include "log.h"
#include "sticker_panel.h"
#include "sticker_panel_internal.h"
#include "icon_info.h"
#include "group_icon_info.h"
#include "conf.h"

#define STICKER_PANEL_DB_FILE "/opt/dbspace/.sticker_panel.db"




sqlite3 *_db_open(void)
{
	sqlite3 *db = NULL;
	int ret = SQLITE_OK;

	ret = sqlite3_open(STICKER_PANEL_DB_FILE, &db);
	if (SQLITE_OK != ret) {
		_E("%s", sqlite3_errmsg(db));
		return NULL;
	}

	return db;
}



void _db_close(sqlite3 *db)
{
	ret_if(!db);
	sqlite3_close(db);
}



sqlite3_stmt *_db_prepare(sqlite3 *db, const char *query)
{
	sqlite3_stmt *stmt = NULL;
	int ret = SQLITE_OK;

	retv_if(!query, NULL);

	ret = sqlite3_prepare_v2(db, query, strlen(query), &stmt, NULL);
	if (SQLITE_OK != ret) {
		_E("%s, %s", query, sqlite3_errmsg(db));
		return NULL;
	}

	return stmt;
}



int _db_next(sqlite3 *db, sqlite3_stmt *stmt)
{
	int ret = SQLITE_OK;

	retv_if(!stmt, -1);

	ret = sqlite3_step(stmt);
	switch (ret) {
	case SQLITE_ROW: /* SQLITE_ROW : 100 */
		return ret;
	case SQLITE_DONE: /* SQLITE_ROW : 101 */
		return ret;
	default:
		_E("%s", sqlite3_errmsg(db));
	}

	return -1;
}



sticker_panel_error_e _db_reset(sqlite3 *db, sqlite3_stmt *stmt)
{
	int ret = SQLITE_OK;

	retv_if(!stmt, STICKER_PANEL_ERROR_INVALID_PARAMETER);

	ret = sqlite3_reset(stmt);
	if (SQLITE_OK != ret) {
		_E("%s", sqlite3_errmsg(db));
		return STICKER_PANEL_ERROR_DB_FAILED;
	}

	sqlite3_clear_bindings(stmt);

	return STICKER_PANEL_ERROR_NONE;
}



sticker_panel_error_e _db_bind_bool(sqlite3 *db, sqlite3_stmt *stmt, int idx, bool value)
{
	int ret = SQLITE_OK;

	retv_if(!stmt, STICKER_PANEL_ERROR_INVALID_PARAMETER);

	ret = sqlite3_bind_int(stmt, idx, (int) value);
	if (SQLITE_OK != ret) {
		_E("%s", sqlite3_errmsg(db));
		return STICKER_PANEL_ERROR_DB_FAILED;
	}

	return STICKER_PANEL_ERROR_NONE;
}



sticker_panel_error_e _db_bind_int(sqlite3 *db, sqlite3_stmt *stmt, int idx, int value)
{
	int ret = SQLITE_OK;

	retv_if(!stmt, STICKER_PANEL_ERROR_INVALID_PARAMETER);

	ret = sqlite3_bind_int(stmt, idx, value);
	if (SQLITE_OK != ret) {
		_E("%s", sqlite3_errmsg(db));
		return STICKER_PANEL_ERROR_DB_FAILED;
	}

	return STICKER_PANEL_ERROR_NONE;
}



sticker_panel_error_e _db_bind_double(sqlite3 *db, sqlite3_stmt *stmt, int idx, double value)
{
	int ret = SQLITE_OK;

	retv_if(!stmt, STICKER_PANEL_ERROR_INVALID_PARAMETER);

	ret = sqlite3_bind_double(stmt, idx, value);
	if (SQLITE_OK != ret) {
		_E("%s", sqlite3_errmsg(db));
		return STICKER_PANEL_ERROR_DB_FAILED;
	}

	return STICKER_PANEL_ERROR_NONE;
}



sticker_panel_error_e _db_bind_str(sqlite3 *db, sqlite3_stmt *stmt, int idx, const char *str)
{
	int ret = SQLITE_OK;

	retv_if(!stmt, STICKER_PANEL_ERROR_INVALID_PARAMETER);

	if (str) {
		ret = sqlite3_bind_text(stmt, idx, str, -1, SQLITE_TRANSIENT);
	} else {
		ret = sqlite3_bind_null(stmt, idx);
	}

	if (SQLITE_OK != ret) {
		_E("%s", sqlite3_errmsg(db));
		return STICKER_PANEL_ERROR_DB_FAILED;
	}

	return STICKER_PANEL_ERROR_NONE;
}



bool _db_get_bool(sqlite3_stmt *stmt, int index)
{
	retv_if(!stmt, false);
	return (bool) (!!sqlite3_column_int(stmt, index));
}



int _db_get_int(sqlite3_stmt *stmt, int index)
{
	retv_if(!stmt, 0);
	return sqlite3_column_int(stmt, index);
}



int _db_get_double(sqlite3_stmt *stmt, int index)
{
	retv_if(!stmt, 0);
	return sqlite3_column_double(stmt, index);
}



const char *_db_get_str(sqlite3_stmt *stmt, int index)
{
	retv_if(!stmt, NULL);
	return (const char *) sqlite3_column_text(stmt, index);
}



sticker_panel_error_e _db_finalize(sqlite3 *db, sqlite3_stmt *stmt)
{
	int ret = SQLITE_OK;

	retv_if(!stmt, STICKER_PANEL_ERROR_INVALID_PARAMETER);

	ret = sqlite3_finalize(stmt);
	if (SQLITE_OK != ret) {
		_E("%s", sqlite3_errmsg(db));
		return STICKER_PANEL_ERROR_DB_FAILED;
	}

	return STICKER_PANEL_ERROR_NONE;
}



sticker_panel_error_e _db_exec(sqlite3 *db, const char *query)
{
	sqlite3_stmt *stmt = NULL;

	retv_if(!query, STICKER_PANEL_ERROR_INVALID_PARAMETER);

	stmt = _db_prepare(db, query);
	retv_if(!stmt, STICKER_PANEL_ERROR_DB_FAILED);

	goto_if(_db_next(db, stmt) == -1, ERROR);
	goto_if(STICKER_PANEL_ERROR_NONE != _db_finalize(db, stmt), ERROR);

	return STICKER_PANEL_ERROR_NONE;

ERROR:
	if (stmt) _db_finalize(db, stmt);
	return STICKER_PANEL_ERROR_DB_FAILED;
}



sticker_panel_error_e _db_create_table(sqlite3 *db)
{
	int count = 0;
	int i = 0;
	const char *const TABLES[] = {
		"CREATE TABLE IF NOT EXISTS db_checksum (version INT);",
		"CREATE TABLE IF NOT EXISTS recent_icon ("
			"id TEXT"
			", type INTEGER"
			", time INTEGER"
			", PRIMARY KEY(id, type)"
			");",
		"CREATE TABLE IF NOT EXISTS group_icon ("
			"id TEXT PRIMARY KEY"
			", name TEXT"
			", repository TEXT"
			", ordering INTEGER"
			", category INTEGER"
			");",
	};

	count = sizeof(TABLES) / sizeof(char *);
	for (; i < count; i++) {
		break_if(_db_exec(db, TABLES[i]) != STICKER_PANEL_ERROR_NONE);
	}

	return STICKER_PANEL_ERROR_NONE;
}



sticker_panel_error_e _db_drop_table(sqlite3 *db)
{
	int count = 0;
	int i = 0;
	const char *const TABLES[] = {
		"DROP TABLE IF EXISTS db_checksum;",
		"DROP TABLE IF EXISTS recent_icon;",
		"DROP TABLE IF EXISTS group_icon;",
	};

	count = sizeof(TABLES) / sizeof(char *);
	for (; i < count; i++) {
		_D("Drop a table[%s]", TABLES[i]);
		break_if(_db_exec(db, TABLES[i]) != STICKER_PANEL_ERROR_NONE);
	}

	return STICKER_PANEL_ERROR_NONE;
}



sticker_panel_error_e _db_insert_version(sqlite3 *db, int version)
{
	const char *const QUERY_SYNTAX = "INSERT INTO db_checksum (version) VALUES (?);";
	sqlite3_stmt *st = NULL;

	st = _db_prepare(db, QUERY_SYNTAX);
	retv_if(!st, STICKER_PANEL_ERROR_DB_FAILED);

	goto_if(_db_bind_int(db, st, 1, version) != STICKER_PANEL_ERROR_NONE, error);
	goto_if(_db_next(db, st) == -1, error);

	_db_reset(db, st);
	_db_finalize(db, st);

	/* keep the sticker panel DB opened */

	return STICKER_PANEL_ERROR_NONE;

error:
	_db_finalize(db, st);
	return STICKER_PANEL_ERROR_DB_FAILED;
}



sticker_panel_error_e _db_delete_version(sqlite3 *db, int version)
{
	const char *const QUERY_SYNTAX = "DELETE FROM db_checksum WHERE version = ?;";
	sqlite3_stmt *st = NULL;

	st = _db_prepare(db, QUERY_SYNTAX);
	retv_if(!st, STICKER_PANEL_ERROR_DB_FAILED);

	goto_if(_db_bind_int(db, st, 1, version) != STICKER_PANEL_ERROR_NONE, error);
	goto_if(_db_next(db, st) == -1, error);

	_db_reset(db, st);
	_db_finalize(db, st);

	/* keep the sticker panel DB opened */

	return STICKER_PANEL_ERROR_NONE;

error:
	_db_finalize(db, st);
	return STICKER_PANEL_ERROR_DB_FAILED;
}



sticker_panel_error_e _db_update_version(sqlite3 *db, int version)
{
	const char *const QUERY_SYNTAX = "UPDATE db_checksum SET version = ?;";
	sqlite3_stmt *st = NULL;

	st = _db_prepare(db, QUERY_SYNTAX);
	retv_if(!st, STICKER_PANEL_ERROR_DB_FAILED);

	goto_if(_db_bind_int(db, st, 1, version) != STICKER_PANEL_ERROR_NONE, error);
	goto_if(_db_next(db, st) == -1, error);

	_db_reset(db, st);
	_db_finalize(db, st);

	/* keep the sticker panel DB opened */

	return STICKER_PANEL_ERROR_NONE;

error:
	_db_finalize(db, st);
	return STICKER_PANEL_ERROR_DB_FAILED;
}



sticker_panel_error_e _db_count_version(sqlite3 *db)
{
	const char *const QUERY_SYNTAX = "SELECT COUNT(*) FROM db_checksum;";
	sqlite3_stmt *st = NULL;
	int count = 0;

	st = _db_prepare(db, QUERY_SYNTAX);
	retv_if(!st, STICKER_PANEL_ERROR_DB_FAILED);

	if (_db_next(db, st) == -1) {
		_E("_db_next error");
		_db_finalize(db, st);
		return -1;
	}

	count = _db_get_int(st, 0);
	_db_reset(db, st);
	_db_finalize(db, st);

	/* keep the sticker panel DB opened */

	return count;
}



sticker_panel_error_e _db_count_recent_icon(sqlite3 *db, const char *id, int type, int *count)
{
	const char *const QUERY_SYNTAX = "SELECT COUNT(*) FROM recent_icon WHERE id = ? and type = ?;";
	sqlite3_stmt *st = NULL;

	st = _db_prepare(db, QUERY_SYNTAX);
	retv_if(!st, STICKER_PANEL_ERROR_DB_FAILED);

	goto_if(_db_bind_str(db, st, 1, id) != STICKER_PANEL_ERROR_NONE, error);
	goto_if(_db_bind_int(db, st, 2, type) != STICKER_PANEL_ERROR_NONE, error);
	goto_if(_db_next(db, st) == -1, error);

	*count = _db_get_int(st, 0);

	_db_reset(db, st);
	_db_finalize(db, st);

	/* keep the sticker panel DB opened */

	return STICKER_PANEL_ERROR_NONE;

error:
	_db_finalize(db, st);
	return STICKER_PANEL_ERROR_DB_FAILED;
}



sticker_panel_error_e _db_touch_recent_icon(sqlite3 *db, const char *id, int type)
{
	const char *const QUERY_INSERT = "INSERT INTO recent_icon (id, type, time) VALUES (?, ?, DATETIME('now'));";
	const char *const QUERY_UPDATE = "UPDATE recent_icon SET time = DATETIME('now') WHERE id = ? and type = ?;";
	sqlite3_stmt *st = NULL;
	int count = 0;
	int ret = STICKER_PANEL_ERROR_NONE;

	ret = _db_count_recent_icon(db, id, type,  &count);
	retv_if(ret != STICKER_PANEL_ERROR_NONE, STICKER_PANEL_ERROR_DB_FAILED);

	if (count) {
		st = _db_prepare(db, QUERY_UPDATE);
		retv_if(!st, STICKER_PANEL_ERROR_DB_FAILED);
	} else {
		st = _db_prepare(db, QUERY_INSERT);
		retv_if(!st, STICKER_PANEL_ERROR_DB_FAILED);
	}

	goto_if(_db_bind_str(db, st, 1, id) != STICKER_PANEL_ERROR_NONE, error);
	goto_if(_db_bind_int(db, st, 2, type) != STICKER_PANEL_ERROR_NONE, error);
	goto_if(_db_next(db, st) == -1, error);

	_db_reset(db, st);
	_db_finalize(db, st);

	/* keep the sticker panel DB opened */

	return STICKER_PANEL_ERROR_NONE;

error:
	_db_finalize(db, st);
	return STICKER_PANEL_ERROR_DB_FAILED;
}



sticker_panel_error_e _db_delete_recent_icon(sqlite3 *db, const char *id, int type)
{
	const char *const QUERY_SYNTAX = "DELETE FROM recent_icon WHERE id = ? and type = ?;";
	sqlite3_stmt *st = NULL;

	st = _db_prepare(db, QUERY_SYNTAX);
	retv_if(!st, STICKER_PANEL_ERROR_DB_FAILED);

	goto_if(_db_bind_str(db, st, 1, id) != STICKER_PANEL_ERROR_NONE, error);
	goto_if(_db_bind_int(db, st, 2, type) != STICKER_PANEL_ERROR_NONE, error);
	goto_if(_db_next(db, st) == -1, error);

	_db_reset(db, st);
	_db_finalize(db, st);

	/* keep the sticker panel DB opened */

	return STICKER_PANEL_ERROR_NONE;

error:
	_db_finalize(db, st);
	return STICKER_PANEL_ERROR_DB_FAILED;
}



sticker_panel_error_e _db_list_recent_icon(sqlite3 *db, Eina_List **icon_list, int limit)
{
	const char *const QUERY_LIST = "SELECT id, type FROM recent_icon ORDER BY time DESC LIMIT ?";
	const char *id = NULL;
	icon_info_s *icon_info = NULL;
	sqlite3_stmt *st = NULL;
	int type = 0;
	int ret = -1;

	st = _db_prepare(db, QUERY_LIST);
	retv_if(!st, STICKER_PANEL_ERROR_DB_FAILED);

	goto_if(_db_bind_int(db, st, 1, limit) != STICKER_PANEL_ERROR_NONE, error);

	do {
		ret = _db_next(db, st);
		if (SQLITE_DONE == ret) {
			break;
		} else if (-1 == ret) {
			_E("_db_next() error");
			goto error;
		}

		id = _db_get_str(st, 0);
		goto_if(!id, error);

		type = _db_get_int(st, 1);

		icon_info = _icon_info_create(id, type);
		continue_if(!icon_info);

		*icon_list = eina_list_append(*icon_list, icon_info);
	} while (SQLITE_ROW == ret);

	_db_reset(db, st);
	_db_finalize(db, st);

	/* keep the sticker panel DB opened */

	return STICKER_PANEL_ERROR_NONE;

error:
	EINA_LIST_FREE(*icon_list, icon_info) {
		_icon_info_destroy(icon_info);
	}

	_db_finalize(db, st);
	return STICKER_PANEL_ERROR_DB_FAILED;
}



sticker_panel_error_e _db_initialize_group_icon(sqlite3 *db)
{
	const char *const QUERY_SYNTAX = "INSERT INTO group_icon (id, name, repository, ordering, category) VALUES (?, ?, ?, ?, ?);";
	sqlite3_stmt *st = NULL;
	/* FIXME : This part can be moved out to the text file */
	/* ordering is from '1'. '0' is reserved for the recent tabbar. */
	struct group_icon_info_s {
		char *id;
		char *name;
		char *repository;
		int ordering;
		int category;
	} group_icons[] = {
		{
			.id = PRELOADED_PRESET_DIR_PATH"/Animal_Body",
			.name = "Animal body",
			.repository = "",
			.ordering = 1,
			.category = 0,
		},
		{
			.id = PRELOADED_PRESET_DIR_PATH"/Animal_Face",
			.name = "Animal face",
			.repository = "",
			.ordering = 2,
			.category = 0,
		},
		{
			.id = PRELOADED_PRESET_DIR_PATH"/Couple",
			.name = "Couple",
			.repository = "",
			.ordering = 3,
			.category = 0,
		},
		{
			.id = PRELOADED_PRESET_DIR_PATH"/Kids",
			.name = "Kids",
			.repository = "",
			.ordering = 4,
			.category = 0,
		},
		{
			.id = PRELOADED_PRESET_DIR_PATH"/Office",
			.name = "Office",
			.repository = "",
			.ordering = 5,
			.category = 0,
		},
		{
			.id = PRELOADED_PRESET_DIR_PATH"/Woman",
			.name = "Woman",
			.repository = "",
			.ordering = 6,
			.category = 0,
		},
	};
	register int i = 0;
	int count = 0;

	st = _db_prepare(db, QUERY_SYNTAX);
	retv_if(!st, STICKER_PANEL_ERROR_DB_FAILED);

	count = sizeof(group_icons) / sizeof(struct group_icon_info_s);
	for (; i < count; i++) {
		goto_if(_db_bind_str(db, st, 1, group_icons[i].id) != STICKER_PANEL_ERROR_NONE, error);
		goto_if(_db_bind_str(db, st, 2, group_icons[i].name) != STICKER_PANEL_ERROR_NONE, error);
		goto_if(_db_bind_str(db, st, 3, group_icons[i].repository) != STICKER_PANEL_ERROR_NONE, error);
		goto_if(_db_bind_int(db, st, 4, group_icons[i].ordering) != STICKER_PANEL_ERROR_NONE, error);
		goto_if(_db_bind_int(db, st, 5, group_icons[i].category) != STICKER_PANEL_ERROR_NONE, error);
		goto_if(_db_next(db, st) == -1, error);
		_db_reset(db, st);
	}

	_db_reset(db, st);
	_db_finalize(db, st);

	/* keep the sticker panel DB opened */

	return STICKER_PANEL_ERROR_NONE;

error:
	_db_finalize(db, st);
	return STICKER_PANEL_ERROR_DB_FAILED;
}



sticker_panel_error_e _db_count_group_icon(sqlite3 *db, const char *id, int *count)
{
	const char *const QUERY_COUNT_ID = "SELECT COUNT(*) FROM group_icon WHERE id = ?;";
	const char *const QUERY_COUNT_ALL = "SELECT COUNT(*) FROM group_icon;";
	const char *query = NULL;
	sqlite3_stmt *st = NULL;

	retv_if(!db, STICKER_PANEL_ERROR_INVALID_PARAMETER);
	retv_if(!count, STICKER_PANEL_ERROR_INVALID_PARAMETER);

	if (id)
		query = QUERY_COUNT_ID;
	else
		query = QUERY_COUNT_ALL;

	st = _db_prepare(db, query);
	retv_if(!st, STICKER_PANEL_ERROR_DB_FAILED);

	if (id)
		goto_if(_db_bind_str(db, st, 1, id) != STICKER_PANEL_ERROR_NONE, error);

	goto_if(_db_next(db, st) == -1, error);

	*count = _db_get_int(st, 0);

	_db_reset(db, st);
	_db_finalize(db, st);

	/* keep the sticker panel DB opened */

	return STICKER_PANEL_ERROR_NONE;

error:
	_db_finalize(db, st);
	return STICKER_PANEL_ERROR_DB_FAILED;
}



sticker_panel_error_e _db_insert_group_icon(sqlite3 *db, const char *id, const char *name, const char *repository, int ordering, int category)
{
	const char *const QUERY_SYNTAX = "INSERT INTO group_icon (id, name, repository, ordering, category) VALUES (?, ?, ?, ?, ?);";
	sqlite3_stmt *st = NULL;

	st = _db_prepare(db, QUERY_SYNTAX);
	retv_if(!st, STICKER_PANEL_ERROR_DB_FAILED);

	goto_if(_db_bind_str(db, st, 1, id) != STICKER_PANEL_ERROR_NONE, error);
	goto_if(_db_bind_str(db, st, 2, name) != STICKER_PANEL_ERROR_NONE, error);
	goto_if(_db_bind_str(db, st, 3, repository) != STICKER_PANEL_ERROR_NONE, error);
	goto_if(_db_bind_int(db, st, 4, ordering) != STICKER_PANEL_ERROR_NONE, error);
	goto_if(_db_bind_int(db, st, 5, category) != STICKER_PANEL_ERROR_NONE, error);
	goto_if(_db_next(db, st) == -1, error);

	_db_reset(db, st);
	_db_finalize(db, st);

	/* keep the sticker panel DB opened */

	return STICKER_PANEL_ERROR_NONE;

error:
	_db_finalize(db, st);
	return STICKER_PANEL_ERROR_DB_FAILED;
}



sticker_panel_error_e _db_delete_group_icon(sqlite3 *db, const char *id)
{
	const char *const QUERY_SYNTAX = "DELETE FROM group_icon WHERE id = ?;";
	sqlite3_stmt *st = NULL;

	st = _db_prepare(db, QUERY_SYNTAX);
	retv_if(!st, STICKER_PANEL_ERROR_DB_FAILED);

	goto_if(_db_bind_str(db, st, 1, id) != STICKER_PANEL_ERROR_NONE, error);
	goto_if(_db_next(db, st) == -1, error);

	_db_reset(db, st);
	_db_finalize(db, st);

	/* keep the sticker panel DB opened */

	return STICKER_PANEL_ERROR_NONE;

error:
	_db_finalize(db, st);
	return STICKER_PANEL_ERROR_DB_FAILED;
}



sticker_panel_error_e _db_update_group_icon(sqlite3 *db, const char *id, int ordering)
{
	const char *const QUERY_SYNTAX = "UPDATE group_icon SET ordering = ? WHERE id = ?;";
	sqlite3_stmt *st = NULL;

	st = _db_prepare(db, QUERY_SYNTAX);
	retv_if(!st, STICKER_PANEL_ERROR_DB_FAILED);

	goto_if(_db_bind_int(db, st, 1, ordering) != STICKER_PANEL_ERROR_NONE, error);
	goto_if(_db_bind_str(db, st, 2, id) != STICKER_PANEL_ERROR_NONE, error);
	goto_if(_db_next(db, st) == -1, error);

	_db_reset(db, st);
	_db_finalize(db, st);

	/* keep the sticker panel DB opened */

	return STICKER_PANEL_ERROR_NONE;

error:
	_db_finalize(db, st);
	return STICKER_PANEL_ERROR_DB_FAILED;
}



sticker_panel_error_e _db_list_group_icon(sqlite3 *db, Eina_List **group_icon_info_list)
{
	const char *const QUERY_LIST = "SELECT id, name, repository, ordering, category FROM group_icon ORDER BY category, ordering";
	group_icon_info_s *group_icon_info = NULL;
	sqlite3_stmt *st = NULL;
	int ret = -1;

	st = _db_prepare(db, QUERY_LIST);
	retv_if(!st, STICKER_PANEL_ERROR_DB_FAILED);

	do {
		const char *id = NULL;
		const char *name = NULL;
		const char *repository = NULL;
		int ordering = 0;
		int category = 0;

		ret = _db_next(db, st);
		if (SQLITE_DONE == ret) {
			break;
		} else if (-1 == ret) {
			_E("_db_next() error");
			goto error;
		}

		id = _db_get_str(st, 0);
		goto_if(!id, error);

		name = _db_get_str(st, 1);
		goto_if(!name, error);

		/* repository can be NULL */
		repository = _db_get_str(st, 2);
		ordering = _db_get_int(st, 3);
		category = _db_get_int(st, 4);

		group_icon_info = _group_icon_info_create(id, name, repository, ordering, category);
		continue_if(!group_icon_info);

		*group_icon_info_list = eina_list_append(*group_icon_info_list, group_icon_info);
	} while (SQLITE_ROW == ret);

	_db_reset(db, st);
	_db_finalize(db, st);

	/* keep the sticker panel DB opened */

	return STICKER_PANEL_ERROR_NONE;

error:
	EINA_LIST_FREE(*group_icon_info_list, group_icon_info) {
		_group_icon_info_destroy(group_icon_info);
	}

	_db_finalize(db, st);
	return STICKER_PANEL_ERROR_DB_FAILED;
}



sticker_panel_error_e _db_increment_group_icon(sqlite3 *db, int from, int to, int category)
{
	const char *const QUERY_SYNTAX = "UPDATE group_icon SET ordering = ordering + 1 "
	   							"WHERE ordering >= ? AND ordering <= ? AND category = ?;";
	sqlite3_stmt *st = NULL;

	st = _db_prepare(db, QUERY_SYNTAX);
	retv_if(!st, STICKER_PANEL_ERROR_DB_FAILED);

	goto_if(_db_bind_int(db, st, 1, from) != STICKER_PANEL_ERROR_NONE, error);
	goto_if(_db_bind_int(db, st, 2, to) != STICKER_PANEL_ERROR_NONE, error);
	goto_if(_db_bind_int(db, st, 3, category) != STICKER_PANEL_ERROR_NONE, error);
	goto_if(_db_next(db, st) == -1, error);

	_db_reset(db, st);
	_db_finalize(db, st);

	/* keep the sticker panel DB opened */

	return STICKER_PANEL_ERROR_NONE;

error:
	_db_finalize(db, st);
	return STICKER_PANEL_ERROR_DB_FAILED;
}



sticker_panel_error_e _db_decrement_group_icon(sqlite3 *db, int from, int to, int category)
{
	const char *const QUERY_SYNTAX = "UPDATE group_icon SET ordering = ordering - 1 "
	   							"WHERE ordering >= ? AND ordering <= ? AND category = ?;";
	sqlite3_stmt *st = NULL;

	st = _db_prepare(db, QUERY_SYNTAX);
	retv_if(!st, STICKER_PANEL_ERROR_DB_FAILED);

	goto_if(_db_bind_int(db, st, 1, from) != STICKER_PANEL_ERROR_NONE, error);
	goto_if(_db_bind_int(db, st, 2, to) != STICKER_PANEL_ERROR_NONE, error);
	goto_if(_db_bind_int(db, st, 3, category) != STICKER_PANEL_ERROR_NONE, error);
	goto_if(_db_next(db, st) == -1, error);

	_db_reset(db, st);
	_db_finalize(db, st);

	/* keep the sticker panel DB opened */

	return STICKER_PANEL_ERROR_NONE;

error:
	_db_finalize(db, st);
	return STICKER_PANEL_ERROR_DB_FAILED;
}
