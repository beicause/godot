/**************************************************************************/
/*  gd_duckdb.h                                                           */
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

#ifndef GD_DUCKDB_H
#define GD_DUCKDB_H

#include "core/config/project_settings.h"
#include "core/object/ref_counted.h"
#include "core/os/os.h"
#include "thirdparty/duckdb.h"

class DuckDB : public RefCounted {
	GDCLASS(DuckDB, RefCounted);

	duckdb_database db = nullptr;
	duckdb_connection con = nullptr;

	String get_res_path(String path) {
		if (OS::get_singleton()->has_feature("editor")) {
			return ProjectSettings::get_singleton()->globalize_path(path);
		} else {
			return OS::get_singleton()->get_executable_path().get_base_dir().path_join(path);
		}
	}

protected:
	static void _bind_methods();

public:
	void open_file(String path) {
		String abs_path = path;
		String res_path = get_res_path(path);
		if (path.begins_with("res://")) {
			abs_path = res_path;
		} else if (path.begins_with("user://")) {
			abs_path = ProjectSettings::get_singleton()->globalize_path(path);
		} else {
			abs_path = res_path.path_join(path);
		}
		duckdb_open(path.utf8().get_data(), &db);
		duckdb_connect(db, &con);
	}
	void query(String sql) {
	}
};
#endif // GD_DUCKDB_H
