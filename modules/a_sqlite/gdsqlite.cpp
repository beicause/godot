/**************************************************************************/
/*  gdsqlite.cpp                                                          */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "gdsqlite.h"
#include "core/config/project_settings.h"
#include "vfs/gdsqlite_vfs.h"

using namespace godot;

void SQLite::_bind_methods() {
	// Methods.
	ClassDB::bind_method(D_METHOD("open_db"), &SQLite::open_db);
	ClassDB::bind_method(D_METHOD("close_db"), &SQLite::close_db);
	ClassDB::bind_method(D_METHOD("query", "query_string"), &SQLite::query);
	ClassDB::bind_method(D_METHOD("query_with_bindings", "query_string", "param_bindings"), &SQLite::query_with_bindings);

	ClassDB::bind_method(D_METHOD("create_table", "table_name", "table_data"), &SQLite::create_table);
	ClassDB::bind_method(D_METHOD("drop_table", "table_name"), &SQLite::drop_table);

	ClassDB::bind_method(D_METHOD("backup_to", "destination"), &SQLite::backup_to);
	ClassDB::bind_method(D_METHOD("restore_from", "source"), &SQLite::restore_from);

	ClassDB::bind_method(D_METHOD("insert_row", "table_name", "row_data"), &SQLite::insert_row);
	ClassDB::bind_method(D_METHOD("insert_rows", "table_name", "row_array", "rollback_on_err"), &SQLite::insert_rows, DEFVAL(true));

	ClassDB::bind_method(D_METHOD("select_rows", "table_name", "conditions", "columns"), &SQLite::select_rows);
	ClassDB::bind_method(D_METHOD("update_rows", "table_name", "conditions", "row_data", "rollback_on_err"), &SQLite::update_rows, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("delete_rows", "table_name", "conditions", "rollback_on_err"), &SQLite::delete_rows, DEFVAL(true));

	ClassDB::bind_method(D_METHOD("create_function", "function_name", "callable"), &SQLite::create_function);

	ClassDB::bind_method(D_METHOD("get_autocommit"), &SQLite::get_autocommit);

	// Properties.
	ClassDB::bind_method(D_METHOD("set_last_insert_rowid", "last_insert_rowid"), &SQLite::set_last_insert_rowid);
	ClassDB::bind_method(D_METHOD("get_last_insert_rowid"), &SQLite::get_last_insert_rowid);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "last_insert_rowid"), "set_last_insert_rowid", "get_last_insert_rowid");

	ClassDB::bind_method(D_METHOD("set_verbosity_level", "verbosity_level"), &SQLite::set_verbosity_level);
	ClassDB::bind_method(D_METHOD("get_verbosity_level"), &SQLite::get_verbosity_level);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "verbosity_level"), "set_verbosity_level", "get_verbosity_level");

	ClassDB::bind_method(D_METHOD("set_foreign_keys", "foreign_keys"), &SQLite::set_foreign_keys);
	ClassDB::bind_method(D_METHOD("get_foreign_keys"), &SQLite::get_foreign_keys);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "foreign_keys"), "set_foreign_keys", "get_foreign_keys");

	ClassDB::bind_method(D_METHOD("set_read_only", "read_only"), &SQLite::set_read_only);
	ClassDB::bind_method(D_METHOD("get_read_only"), &SQLite::get_read_only);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "read_only"), "set_read_only", "get_read_only");

	ClassDB::bind_method(D_METHOD("set_db_path", "path"), &SQLite::set_db_path);
	ClassDB::bind_method(D_METHOD("get_db_path"), &SQLite::get_db_path);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "db_path"), "set_db_path", "get_db_path");

	ClassDB::bind_method(D_METHOD("set_extension_name", "extension_name"), &SQLite::set_extension_name);
	ClassDB::bind_method(D_METHOD("get_extension_name"), &SQLite::get_extension_name);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "extension_name"), "set_extension_name", "get_extension_name");

	ClassDB::bind_method(D_METHOD("set_query_result", "query_result"), &SQLite::set_query_result);
	ClassDB::bind_method(D_METHOD("get_query_result"), &SQLite::get_query_result);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "query_result", PROPERTY_HINT_ARRAY_TYPE, "Dictionary"), "set_query_result", "get_query_result");

	ClassDB::bind_method(D_METHOD("get_query_result_by_reference"), &SQLite::get_query_result_by_reference);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "query_result_by_reference", PROPERTY_HINT_ARRAY_TYPE, "Dictionary"), "set_query_result", "get_query_result_by_reference");

	// Constants.
	BIND_ENUM_CONSTANT(QUIET);
	BIND_ENUM_CONSTANT(NORMAL);
	BIND_ENUM_CONSTANT(VERBOSE);
	BIND_ENUM_CONSTANT(VERY_VERBOSE);
}

SQLite::~SQLite() {
	/* Close the database connection if it is still open */
	if (db) {
		close_db();
	}
}

bool SQLite::open_db() {
	ERR_FAIL_COND_V_MSG(db, false, "GDSQLite Error: Can't open database if connection is already open!");

	char *zErrMsg = 0;
	int rc;
	if (db_path.find(":memory:") == -1) {
		/* Add the extension_name to the database path if no extension is present */
		/* Skip if the extension_name is an empty string to allow for paths without extension */
		if (db_path.get_extension().is_empty() && !extension_name.is_empty()) {
			String ending = String(".") + extension_name;
			db_path += ending;
		}

		if (!read_only) {
			/* Find the real path */
			db_path = ProjectSettings::get_singleton()->globalize_path(db_path.strip_edges());
		}
	}

	// TODO: Switch back to the `alloc_c_string()`-method once the API gets updated
	const CharString dummy_path = db_path.utf8();
	const char *char_path = dummy_path.get_data();
	//const char *char_path = path.alloc_c_string();
	/* Try to open the database */
	if (read_only) {
		if (db_path.find(":memory:") == -1) {
			sqlite3_vfs_register(gdsqlite_vfs(), 0);
			rc = sqlite3_open_v2(char_path, &db, SQLITE_OPEN_READONLY, "godot");
		} else {
			ERR_PRINT("GDSQLite Error: Opening in-memory databases in read-only mode is currently not supported!");
			return false;
		}
	} else {
		/* The `SQLITE_OPEN_URI`-flag is solely required for in-memory databases with shared cache, but it is safe to use in most general cases */
		/* As discussed here: https://www.sqlite.org/uri.html */
		rc = sqlite3_open_v2(char_path, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_URI, nullptr);
		/* Identical to: `rc = sqlite3_open(char_path, &db);`*/
	}

	if (rc != SQLITE_OK) {
		ERR_PRINT("GDSQLite Error: Can't open database: " + String::utf8(sqlite3_errmsg(db)));
		return false;
	} else if (verbosity_level > VerbosityLevel::QUIET) {
		print_verbose("Opened database successfully (" + db_path + ")");
	}

	/* Try to enable foreign keys. */
	if (foreign_keys) {
		rc = sqlite3_exec(db, "PRAGMA foreign_keys=on;", nullptr, nullptr, &zErrMsg);
		if (rc != SQLITE_OK) {
			ERR_PRINT("GDSQLite Error: Can't enable foreign keys: " + String::utf8(zErrMsg));
			sqlite3_free(zErrMsg);
			return false;
		}
	}

	return true;
}

bool SQLite::close_db() {
	if (db) {
		// Cannot close database!
		if (sqlite3_close_v2(db) != SQLITE_OK) {
			ERR_PRINT("GDSQLite Error: Can't close database!");
			return false;
		} else {
			db = nullptr;
			if (verbosity_level > VerbosityLevel::QUIET) {
				print_verbose("Closed database (" + db_path + ")");
			}
			return true;
		}
	}

	ERR_PRINT("GDSQLite Error: Can't close database if connection is not open!");
	return false;
}

bool SQLite::query(const String &p_query) {
	return query_with_bindings(p_query, Array());
}

bool SQLite::query_with_bindings(const String &p_query, Array param_bindings) {
	const char *zErrMsg, *sql, *pzTail;
	int rc;

	if (verbosity_level > VerbosityLevel::NORMAL) {
		print_verbose(p_query);
	}
	// TODO: Switch back to the `alloc_c_string()`-method once the API gets updated
	const CharString dummy_query = p_query.utf8();
	sql = dummy_query.get_data();
	//sql = p_query.alloc_c_string();

	/* Clear the previous query results */
	query_result.clear();

	sqlite3_stmt *stmt;
	/* Prepare an SQL statement */
	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, &pzTail);
	zErrMsg = sqlite3_errmsg(db);
	if (rc != SQLITE_OK) {
		ERR_PRINT(vformat(" --> SQL error: %s", zErrMsg));
		sqlite3_finalize(stmt);
		return false;
	}

	/* Check if the param_bindings size exceeds the required parameter count */
	int parameter_count = sqlite3_bind_parameter_count(stmt);
	if (param_bindings.size() < parameter_count) {
		ERR_PRINT("GDSQLite Error: Insufficient number of parameters to satisfy required number of bindings in statement!");
		sqlite3_finalize(stmt);
		return false;
	}

	/* Bind any given parameters to the prepared statement */
	for (int i = 0; i < parameter_count; i++) {
		Variant binding_value = param_bindings.pop_front();
		switch (binding_value.get_type()) {
			case Variant::NIL:
				sqlite3_bind_null(stmt, i + 1);
				break;

			case Variant::BOOL:
			case Variant::INT:
				sqlite3_bind_int64(stmt, i + 1, int64_t(binding_value));
				break;

			case Variant::FLOAT:
				sqlite3_bind_double(stmt, i + 1, binding_value);
				break;

			case Variant::STRING:
				// TODO: Switch back to the `alloc_c_string()`-method once the API gets updated
				{
					const CharString dummy_binding = (binding_value.operator String()).utf8();
					const char *binding = dummy_binding.get_data();
					sqlite3_bind_text(stmt, i + 1, binding, -1, SQLITE_TRANSIENT);
				}
				//sqlite3_bind_text(stmt, i + 1, (binding_value.operator String()).alloc_c_string(), -1, SQLITE_TRANSIENT);
				break;

			case Variant::PACKED_BYTE_ARRAY: {
				PackedByteArray binding = ((PackedByteArray)binding_value);
				/* Calling .ptr() on an empty PackedByteArray returns an error */
				if (binding.size() == 0) {
					sqlite3_bind_null(stmt, i + 1);
					/* Identical to: `sqlite3_bind_blob64(stmt, i + 1, nullptr, 0, SQLITE_TRANSIENT);`*/
				} else {
					sqlite3_bind_blob64(stmt, i + 1, binding.ptr(), binding.size(), SQLITE_TRANSIENT);
				}
				break;
			}

			default:
				ERR_PRINT(vformat("GDSQLite Error: Binding a parameter of type %s (TYPE_*) is not supported!", Variant::get_type_name(binding_value.get_type())));
				sqlite3_finalize(stmt);
				return false;
		}
	}

	if (verbosity_level > VerbosityLevel::NORMAL) {
		char *expanded_sql = sqlite3_expanded_sql(stmt);
		print_verbose(String::utf8(expanded_sql));
		sqlite3_free(expanded_sql);
	}

	// Execute the statement and iterate over all the resulting rows.
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		int argc = sqlite3_column_count(stmt);

		/* Loop over all columns and add them to the Dictionary */
		for (int i = 0; i < argc; i++) {
			String col_name = sqlite3_column_name(stmt, i);
			/* Check the column type and do correct casting */
			switch (sqlite3_column_type(stmt, i)) {
				case SQLITE_INTEGER: {
					PackedInt64Array column = query_result.get_or_add(col_name, PackedInt64Array());
					column.push_back((int64_t)sqlite3_column_int64(stmt, i));
				} break;

				case SQLITE_FLOAT: {
					PackedFloat64Array column = query_result.get_or_add(col_name, PackedFloat64Array());
					column.push_back(sqlite3_column_double(stmt, i));
				} break;

				case SQLITE_TEXT: {
					PackedStringArray column = query_result.get_or_add(col_name, PackedStringArray());
					column.push_back((String)(const char *)sqlite3_column_text(stmt, i));
				} break;

				case SQLITE_BLOB: {
					Array column = query_result.get_or_add(col_name, Array());
					int bytes = sqlite3_column_bytes(stmt, i);
					PackedByteArray arr;
					arr.resize(bytes);
					memcpy(arr.ptrw(), sqlite3_column_blob(stmt, i), bytes);
					column.push_back(arr);
				} break;
				case SQLITE_NULL:
					break;
				default:
					break;
			}
		}
	}

	/* Clean up and delete the resources used by the prepared statement */
	sqlite3_finalize(stmt);

	rc = sqlite3_errcode(db);
	zErrMsg = sqlite3_errmsg(db);
	if (rc != SQLITE_OK) {
		ERR_PRINT(vformat(" --> SQL error: %s", zErrMsg));
		return false;
	} else if (verbosity_level > VerbosityLevel::NORMAL) {
		print_verbose(" --> Query succeeded");
	}

	/* Figure out if there's a subsequent statement which needs execution */
	String sTail = String(pzTail).strip_edges();
	if (!sTail.is_empty()) {
		return query_with_bindings(sTail, param_bindings);
	}

	if (!param_bindings.is_empty()) {
		WARN_PRINT(vformat("GDSQLite Warning: Provided number of bindings exceeded the required number in statement! (%s unused parameter(s))", param_bindings.size()));
	}

	return true;
}

bool SQLite::create_table(const String &p_name, const Dictionary &p_table_dict) {
	if (!validate_table_dict(p_table_dict)) {
		return false;
	}

	String query_string, type_string, key_string;
	String integer_datatype = "int";
	/* Create SQL statement */
	query_string = "CREATE TABLE IF NOT EXISTS " + p_name + " (";
	key_string = "";

	Dictionary column_dict;
	Array columns = p_table_dict.keys();
	int64_t number_of_columns = columns.size();
	for (int64_t i = 0; i <= number_of_columns - 1; i++) {
		column_dict = p_table_dict[columns[i]];
		query_string += (String)columns[i] + " ";
		type_string = column_dict["data_type"];
		if (type_string.to_lower().begins_with(integer_datatype)) {
			query_string += "INTEGER";
		} else {
			query_string += type_string;
		}

		/* Primary key check */
		if (column_dict.get("primary_key", false)) {
			query_string += " PRIMARY KEY";
			/* Autoincrement check */
			if (column_dict.get("auto_increment", false)) {
				query_string += " AUTOINCREMENT";
			}
		}
		/* Not nullptr check */
		if (column_dict.get("not_nullptr", false)) {
			query_string += " NOT nullptr";
		}
		/* Unique check */
		if (column_dict.get("unique", false)) {
			query_string += " UNIQUE";
		}
		/* Default check */
		if (column_dict.has("default")) {
			query_string += " DEFAULT " + (String)column_dict["default"];
		}
		/* Apply foreign key constraint. */
		if (foreign_keys) {
			if (column_dict.get("foreign_key", false)) {
				const String foreign_key_definition = (column_dict["foreign_key"]);
				const PackedStringArray foreign_key_elements = foreign_key_definition.split(".");
				if (foreign_key_elements.size() == 2) {
					const String column_name = (columns[i]);
					const String foreign_key_table_name = (foreign_key_elements[0]);
					const String foreign_key_column_name = (foreign_key_elements[1]);
					key_string += ", FOREIGN KEY (" + column_name + ") REFERENCES " + foreign_key_table_name + "(" + foreign_key_column_name + ")";
				}
			}
		}

		if (i != number_of_columns - 1) {
			query_string += ",";
		}
	}

	query_string += key_string + ");";

	return query(query_string);
}

bool SQLite::validate_table_dict(const Dictionary &p_table_dict) {
	Dictionary column_dict;
	Array columns = p_table_dict.keys();
	int64_t number_of_columns = columns.size();
	for (int64_t i = 0; i <= number_of_columns - 1; i++) {
		if (p_table_dict[columns[i]].get_type() != Variant::DICTIONARY) {
			ERR_PRINT("GDSQLite Error: All values of the table dictionary should be of type Dictionary");
			return false;
		}

		column_dict = p_table_dict[columns[i]];
		if (!column_dict.has("data_type")) {
			ERR_PRINT("GDSQLite Error: The field \"data_type\" is a required part of the table dictionary");
			return false;
		}

		if (column_dict["data_type"].get_type() != Variant::STRING) {
			ERR_PRINT("GDSQLite Error: The field \"data_type\" should be of type String");
			return false;
		}

		if (column_dict.has("default")) {
			Variant::Type default_type = column_dict["default"].get_type();

			CharString dummy_data_type = ((String)column_dict["data_type"]).utf8();
			const char *char_data_type = dummy_data_type.get_data();

			/* Get the type of the "datatype"-field and compare with the type of the "default"-value */
			/* Some types are not checked and might be added in a future version */
			Variant::Type data_type_type = default_type;
			if (strcmp(char_data_type, "int") == 0) {
				data_type_type = Variant::Type::INT;
			} else if (strcmp(char_data_type, "text") == 0) {
				data_type_type = Variant::Type::STRING;
			} else if (strcmp(char_data_type, "real") == 0) {
				data_type_type = Variant::Type::FLOAT;
			}

			if (data_type_type != default_type) {
				ERR_PRINT(vformat("GDSQLite Error: The type of the field \"default\" ( %s ) should be the same type as the \"datatype\"-field ( %s )", Variant::get_type_name(default_type), Variant::get_type_name(data_type_type)));
				return false;
			}
		}
	}

	return true;
}

bool SQLite::drop_table(const String &p_name) {
	String query_string;
	/* Create SQL statement */
	query_string = "DROP TABLE " + p_name + ";";

	return query(query_string);
}

bool SQLite::backup_to(String destination_path) {
	ERR_FAIL_COND_V_MSG(db == nullptr, false, "SQLite Error: database has not opened");
	destination_path = ProjectSettings::get_singleton()->globalize_path(destination_path.strip_edges());
	CharString dummy_path = destination_path.utf8();
	const char *char_path = dummy_path.get_data();

	sqlite3 *destination_db;
	int result = sqlite3_open_v2(char_path, &destination_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_URI, nullptr);
	if (result == SQLITE_OK) {
		result = backup_database(db, destination_db);
	}
	(void)sqlite3_close_v2(destination_db);
	return result == SQLITE_OK;
}

bool SQLite::restore_from(String source_path) {
	ERR_FAIL_COND_V_MSG(db == nullptr, false, "SQLite Error: database has not opened");
	source_path = ProjectSettings::get_singleton()->globalize_path(source_path.strip_edges());
	CharString dummy_path = source_path.utf8();
	const char *char_path = dummy_path.get_data();

	sqlite3 *source_db;
	int result = sqlite3_open_v2(char_path, &source_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_URI, nullptr);
	if (result == SQLITE_OK) {
		result = backup_database(source_db, db);
	}
	(void)sqlite3_close_v2(source_db);
	return result == SQLITE_OK;
}

int SQLite::backup_database(sqlite3 *source_db, sqlite3 *destination_db) {
	int rc;
	sqlite3_backup *backup = sqlite3_backup_init(destination_db, "main", source_db, "main");
	if (backup) {
		(void)sqlite3_backup_step(backup, -1);
		(void)sqlite3_backup_finish(backup);
	}

	rc = sqlite3_errcode(destination_db);
	return rc;
}

bool SQLite::insert_row(const String &p_name, const Dictionary &p_row_dict) {
	String query_string, key_string, value_string = "";
	Array keys = p_row_dict.keys();
	Array param_bindings = p_row_dict.values();
	query_string = "INSERT INTO " + p_name;

	int64_t number_of_keys = p_row_dict.size();
	for (int64_t i = 0; i <= number_of_keys - 1; i++) {
		key_string += (String)keys[i];
		value_string += "?";
		if (i != number_of_keys - 1) {
			key_string += ",";
			value_string += ",";
		}
	}
	query_string += " (" + key_string + ") VALUES (" + value_string + ");";

	return query_with_bindings(query_string, param_bindings);
}

bool SQLite::insert_rows(const String &p_name, const Dictionary &p_row_dict, bool p_rollback_on_err) {
	ERR_FAIL_COND_V_MSG(p_row_dict.is_empty(), false, "dictionary is empty");
	int row = 0;
	bool check_kv = true;

	query("BEGIN;");
	while (true) {
		Dictionary insert;
		bool stop = false;
		for (int k = 0; k < p_row_dict.size(); k++) {
			if (check_kv) {
				if (!p_row_dict.keys()[k].is_string()) {
					ERR_PRINT("dictionary key is not String");
					query(p_rollback_on_err ? "ROLLBACK;" : "COMMIT;");
					return false;
				}
				if (!(p_row_dict.values()[k].is_array())) {
					ERR_PRINT("dictionary value is not Array");
					query(p_rollback_on_err ? "ROLLBACK;" : "COMMIT;");
					return false;
				}
			}
			String key = p_row_dict.keys()[k];
			Array col = p_row_dict[key];
			if (row >= col.size()) {
				stop = true;
				break;
			}
			insert[key] = col[row];
		}
		if (stop) {
			break;
		}
		if (!insert_row(p_name, insert)) {
			ERR_PRINT(vformat("Insert failed, db: %s, sql: %s", p_name, insert));
			query(p_rollback_on_err ? "ROLLBACK;" : "COMMIT;");
			return false;
		}
		check_kv = false;
		row++;
	}
	query("COMMIT;");
	return true;
}

Dictionary SQLite::select_rows(const String &p_name, const String &p_conditions, const PackedStringArray &p_columns_array) {
	String query_string;
	/* Create SQL statement */
	query_string = "SELECT ";

	int64_t number_of_columns = p_columns_array.size();
	for (int64_t i = 0; i <= number_of_columns - 1; i++) {
		query_string += p_columns_array[i];

		if (i != number_of_columns - 1) {
			query_string += ", ";
		}
	}
	query_string += " FROM " + p_name;
	if (!p_conditions.is_empty()) {
		query_string += " WHERE " + p_conditions;
	}
	query_string += ";";

	query(query_string);
	/* Return the duplicated result */
	return get_query_result();
}

bool SQLite::update_rows(const String &p_name, const String &p_conditions, const Dictionary &p_updated_row_dict, bool p_rollback_on_err) {
	String query_string;
	Array param_bindings;

	int64_t number_of_keys = p_updated_row_dict.size();
	Array keys = p_updated_row_dict.keys();
	Array values = p_updated_row_dict.values();

	query("BEGIN;");
	/* Create SQL statement */
	query_string += "UPDATE " + p_name + " SET ";

	for (int64_t i = 0; i <= number_of_keys - 1; i++) {
		query_string += keys[i] + String("=?");
		param_bindings.append(values[i]);
		if (i != number_of_keys - 1) {
			query_string += ", ";
		}
	}
	query_string += " WHERE " + p_conditions + ";";

	if (!query_with_bindings(query_string, param_bindings)) {
		ERR_PRINT(vformat("Insert failed, db: %s, sql: %s", p_name, query_string));
		query(p_rollback_on_err ? "ROLLBACK;" : "COMMIT;");
		return false;
	}
	query("COMMIT;");
	return true;
}

bool SQLite::delete_rows(const String &p_name, const String &p_conditions, bool p_rollback_on_err) {
	String query_string;
	query("BEGIN;");
	/* Create SQL statement */
	query_string = "DELETE FROM " + p_name;
	/* If it's empty or * everything is to be deleted */
	if (!p_conditions.is_empty() && (p_conditions != "*")) {
		query_string += " WHERE " + p_conditions;
	}
	query_string += ";";

	if (!query(query_string)) {
		ERR_PRINT(vformat("Insert failed, db: %s, sql: %s", p_name, query_string));
		query(p_rollback_on_err ? "ROLLBACK;" : "COMMIT;");
		return false;
	}
	query("COMMIT;");
	return true;
}

static void function_callback(sqlite3_context *context, int argc, sqlite3_value **argv) {
	void *temp = sqlite3_user_data(context);
	Callable callable = *(Callable *)temp;

	/* Check if the callable is valid */
	if (!callable.is_valid()) {
		ERR_PRINT("GDSQLite Error: Supplied function reference is invalid! Aborting callback...");
		return;
	}

	Array argument_array = Array();
	Variant argument_value;
	for (int i = 0; i <= argc - 1; i++) {
		sqlite3_value *value = *argv;
		/* Check the value type and do correct casting */
		switch (sqlite3_value_type(value)) {
			case SQLITE_INTEGER:
				argument_value = (int64_t)sqlite3_value_int64(value);
				break;

			case SQLITE_FLOAT:
				argument_value = sqlite3_value_double(value);
				break;

			case SQLITE_TEXT:
				argument_value = (const char *)sqlite3_value_text(value);
				break;

			case SQLITE_BLOB: {
				int bytes = sqlite3_value_bytes(value);
				PackedByteArray arr;
				arr.resize(bytes);
				memcpy(arr.ptrw(), sqlite3_value_blob(value), bytes);
				argument_value = arr;
				break;
			}

			case SQLITE_NULL:
				break;

			default:
				break;
		}

		argument_array.append(argument_value);
		argv += 1;
	}

	Variant output = callable.callv(argument_array);

	switch (output.get_type()) {
		case Variant::NIL:
			sqlite3_result_null(context);
			break;

		case Variant::BOOL:
		case Variant::INT:
			sqlite3_result_int64(context, int64_t(output));
			break;

		case Variant::FLOAT:
			sqlite3_result_double(context, output);
			break;

		case Variant::STRING:
			// TODO: Switch back to the `alloc_c_string()`-method once the API gets updated
			{
				const CharString dummy_binding = ((String)output).utf8();
				const char *binding = dummy_binding.get_data();
				sqlite3_result_text(context, binding, -1, SQLITE_STATIC);
			}
			//sqlite3_result_text(context, (output.operator String()).alloc_c_string(), -1, SQLITE_STATIC);
			break;

		case Variant::PACKED_BYTE_ARRAY: {
			PackedByteArray arr = ((const PackedByteArray &)output);
			sqlite3_result_blob(context, arr.ptr(), arr.size(), SQLITE_TRANSIENT);
			break;
		}

		default:
			break;
	}
}

bool SQLite::create_function(const String &p_name, const Callable &p_callable) {
	ERR_FAIL_COND_V_MSG(p_name.is_empty(), false, "SQLite user function name can't be empty");
	/* The exact memory position of the Vector's elements changes during memory reallocation (= when adding additional elements) */
	/* Luckily, the pointer to the managed object (of the std::unique_ptr) won't change during execution! (= consistent) */
	/* The std::unique_ptr is stored in a Vector and is thus allocated on the heap */
	function_registry.push_back(p_callable);

	int rc;
	CharString dummy_name = p_name.utf8();
	const char *zFunctionName = dummy_name.get_data();
	//const char *zFunctionName = p_name.alloc_c_string();
	bool arg_valid = false;
	int nArg = p_callable.get_argument_count(&arg_valid);
	ERR_FAIL_COND_V_MSG(!arg_valid, false, "SQLite user function argument is invalid");
	int eTextRep = SQLITE_UTF8;

	/* Get a void pointer to the managed object of the smart pointer that is stored at the back of the vector */
	void *pApp = (void *)&function_registry[function_registry.size() - 1];
	void (*xFunc)(sqlite3_context *, int, sqlite3_value **) = function_callback;
	void (*xStep)(sqlite3_context *, int, sqlite3_value **) = nullptr;
	void (*xFinal)(sqlite3_context *) = nullptr;

	/* Create the actual function */
	rc = sqlite3_create_function(db, zFunctionName, nArg, eTextRep, pApp, xFunc, xStep, xFinal);
	if (rc) {
		ERR_PRINT("GDSQLite Error: " + String(sqlite3_errmsg(db)));
		return false;
	} else if (verbosity_level > VerbosityLevel::NORMAL) {
		print_verbose("Successfully added function \"" + p_name + "\" to function registry");
	}
	return true;
}

// Properties.
void SQLite::set_last_insert_rowid(const int64_t &p_last_insert_rowid) {
	if (db) {
		sqlite3_set_last_insert_rowid(db, p_last_insert_rowid);
	}
}

int64_t SQLite::get_last_insert_rowid() const {
	if (db) {
		return sqlite3_last_insert_rowid(db);
	}
	/* Return the default value */
	return 0;
}

void SQLite::set_verbosity_level(const int64_t &p_verbosity_level) {
	verbosity_level = p_verbosity_level;
}

int64_t SQLite::get_verbosity_level() const {
	return verbosity_level;
}

void SQLite::set_foreign_keys(const bool &p_foreign_keys) {
	foreign_keys = p_foreign_keys;
}

bool SQLite::get_foreign_keys() const {
	return foreign_keys;
}

void SQLite::set_read_only(const bool &p_read_only) {
	read_only = p_read_only;
}

bool SQLite::get_read_only() const {
	return read_only;
}

void SQLite::set_db_path(const String &p_path) {
	db_path = p_path;
}

String SQLite::get_db_path() const {
	return db_path;
}

void SQLite::set_extension_name(const String &p_extension_name) {
	extension_name = p_extension_name;
}

String SQLite::get_extension_name() const {
	return extension_name;
}

void SQLite::set_query_result(const Dictionary &p_query_result) {
	query_result = p_query_result;
}

Dictionary SQLite::get_query_result() const {
	return query_result.duplicate(true);
}

Dictionary SQLite::get_query_result_by_reference() const {
	return query_result;
}

int SQLite::get_autocommit() const {
	if (db) {
		return sqlite3_get_autocommit(db);
	}
	/* Return the default value */
	return 1; // A non-zero value indicates the autocommit is on!
}

Ref<Resource> ResourceFormatLoaderSQLite::load(const String &p_path, const String &p_original_path, Error *r_error, bool p_use_sub_threads, float *r_progress, CacheMode p_cache_mode) {
	if (r_error) {
		*r_error = ERR_FILE_CANT_OPEN;
	}

	if (!FileAccess::exists(p_path)) {
		*r_error = ERR_FILE_NOT_FOUND;
		return Ref<Resource>();
	}

	Ref<SQLite> ret;
	ret.instantiate();
	ret->set_db_path(p_path);
	return ret;
}

void ResourceFormatLoaderSQLite::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("db");
}

bool ResourceFormatLoaderSQLite::handles_type(const String &p_type) const {
	return p_type == "SQLite";
}

String ResourceFormatLoaderSQLite::get_resource_type(const String &p_path) const {
	String el = p_path.get_extension().to_lower();
	if (el == "db") {
		return "SQLite";
	}
	return "";
}
