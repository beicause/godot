/**************************************************************************/
/*  gdsqlite.h                                                            */
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

#ifndef GDSQLITE_H
#define GDSQLITE_H

#include "core/io/resource_loader.h"
#include <sqlite3.h>

enum OBJECT_TYPE {
	TABLE,
	TRIGGER
};
struct object_struct {
	String name, sql;
	OBJECT_TYPE type;
	Array base64_columns, row_array;
};

class SQLite : public Resource {
	GDCLASS(SQLite, Resource)

private:
	bool validate_table_dict(const Dictionary &p_table_dict);
	int backup_database(sqlite3 *source_db, sqlite3 *destination_db);

	sqlite3 *db = nullptr;
	Vector<Callable> function_registry;

	int64_t verbosity_level = 1;
	bool foreign_keys = false;
	bool read_only = false;
	String db_path = ":memory:";
	String extension_name = "db";
	Dictionary query_result;

protected:
	static void _bind_methods();

public:
	// Constants.
	enum VerbosityLevel {
		QUIET,
		NORMAL,
		VERBOSE,
		VERY_VERBOSE,
	};

	~SQLite();
	static Ref<SQLite> open(const String &p_path = ":memory:");
	// Functions.
	bool open_db();
	bool close_db();
	bool query(const String &p_query);
	bool query_with_bindings(const String &p_query, Array param_bindings);

	bool create_table(const String &p_name, const Dictionary &p_table_dict);
	bool drop_table(const String &p_name);

	bool backup_to(String destination_path);
	bool restore_from(String source_path);

	bool insert_row(const String &p_name, const Dictionary &p_row_dict);
	bool insert_rows(const String &p_name, const Dictionary &p_row_dict, bool p_rollback_on_err = true);

	Dictionary select_rows(const String &p_name, const String &p_conditions, const PackedStringArray &p_columns_array);
	bool update_rows(const String &p_name, const String &p_conditions, const Dictionary &p_updated_row_dict, bool p_rollback_on_err = true);
	bool delete_rows(const String &p_name, const String &p_conditions, bool p_rollback_on_err = true);

	bool create_function(const String &p_name, const Callable &p_callable);

	int get_autocommit() const;

	// Properties.
	void set_last_insert_rowid(const int64_t &p_last_insert_rowid);
	int64_t get_last_insert_rowid() const;

	void set_verbosity_level(const int64_t &p_verbosity_level);
	int64_t get_verbosity_level() const;

	void set_foreign_keys(const bool &p_foreign_keys);
	bool get_foreign_keys() const;

	void set_read_only(const bool &p_read_only);
	bool get_read_only() const;

	void set_db_path(const String &p_path);
	String get_db_path() const;

	void set_error_message(const String &p_error_message);
	String get_error_message() const;

	void set_extension_name(const String &p_extension_name);
	String get_extension_name() const;

	void set_query_result(const Dictionary &p_query_result);
	Dictionary get_query_result() const;

	Dictionary get_query_result_by_reference() const;
};
VARIANT_ENUM_CAST(SQLite::VerbosityLevel);

class ResourceFormatLoaderSQLite : public ResourceFormatLoader {
public:
	virtual Ref<Resource> load(const String &p_path, const String &p_original_path = "", Error *r_error = nullptr, bool p_use_sub_threads = false, float *r_progress = nullptr, CacheMode p_cache_mode = CACHE_MODE_REUSE) override;
	virtual void get_recognized_extensions(List<String> *p_extensions) const override;
	virtual bool handles_type(const String &p_type) const override;
	virtual String get_resource_type(const String &p_path) const override;
};

#endif // GDSQLITE_H
