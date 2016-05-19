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

#ifndef __STICKER_PANEL_DB_H__
#define __STICKER_PANEL_DB_H__

#include <sqlite3.h>
#include "sticker_panel.h"

extern sqlite3 *_db_open(void);
extern void _db_close(sqlite3 *db);

extern sqlite3_stmt *_db_prepare(sqlite3 *db, const char *query);
extern int _db_next(sqlite3 *db, sqlite3_stmt *stmt);
extern sticker_panel_error_e _db_reset(sqlite3 *db, sqlite3_stmt *stmt);

extern sticker_panel_error_e _db_bind_bool(sqlite3 *db, sqlite3_stmt *stmt, int idx, bool value);
extern sticker_panel_error_e _db_bind_int(sqlite3 *db, sqlite3_stmt *stmt, int idx, int value);
extern sticker_panel_error_e _db_bind_double(sqlite3 *db, sqlite3_stmt *stmt, int idx, double value);
extern sticker_panel_error_e _db_bind_str(sqlite3 *db, sqlite3_stmt *stmt, int idx, const char *str);

extern bool _db_get_bool(sqlite3_stmt *stmt, int index);
extern int _db_get_int(sqlite3_stmt *stmt, int index);
extern int _db_get_double(sqlite3_stmt *stmt, int index);
extern const char *_db_get_str(sqlite3_stmt *stmt, int index);

extern sticker_panel_error_e _db_finalize(sqlite3 *db, sqlite3_stmt *stmt);
extern sticker_panel_error_e _db_exec(sqlite3 *db, const char *query);

extern sticker_panel_error_e _db_create_table(sqlite3 *db);
extern sticker_panel_error_e _db_drop_table(sqlite3 *db);

extern sticker_panel_error_e _db_insert_version(sqlite3 *db, int version);
extern sticker_panel_error_e _db_delete_version(sqlite3 *db, int version);
extern sticker_panel_error_e _db_update_version(sqlite3 *db, int version);
extern sticker_panel_error_e _db_count_version(sqlite3 *db);

extern sticker_panel_error_e _db_count_recent_icon(sqlite3 *db, const char *id, int type, int *count);
extern sticker_panel_error_e _db_touch_recent_icon(sqlite3 *db, const char *id, int type);
extern sticker_panel_error_e _db_delete_recent_icon(sqlite3 *db, const char *id, int type);
extern sticker_panel_error_e _db_list_recent_icon(sqlite3 *db, Eina_List **list, int limit);

extern sticker_panel_error_e _db_initialize_group_icon(sqlite3 *db);
extern sticker_panel_error_e _db_count_group_icon(sqlite3 *db, const char *id, int *count);
extern sticker_panel_error_e _db_insert_group_icon(sqlite3 *db, const char *id, const char *name, const char *repository, int ordering, int category);
extern sticker_panel_error_e _db_delete_group_icon(sqlite3 *db, const char *id);
extern sticker_panel_error_e _db_update_group_icon(sqlite3 *db, const char *id, int ordering);
extern sticker_panel_error_e _db_list_group_icon(sqlite3 *db, Eina_List **group_icon_info_list);
extern sticker_panel_error_e _db_increment_group_icon(sqlite3 *db, int from, int to, int category);
extern sticker_panel_error_e _db_decrement_group_icon(sqlite3 *db, int from, int to, int category);

#endif /* __STICKER_PANEL_DB_H__ */
