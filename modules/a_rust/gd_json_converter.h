/**************************************************************************/
/*  gd_json_converter.h                                                   */
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

#ifndef GD_JSON_CONVERTER_H
#define GD_JSON_CONVERTER_H

#include "core/object/ref_counted.h"
#include "cxxbridge/cxx.h"
#include "cxxbridge/gd_json_converter.rs.h"
#include "modules/a_jsonnet/gd_jsonnet.h"

class JSONConverter : public RefCounted {
	GDCLASS(JSONConverter, RefCounted);

protected:
	static void _bind_methods() {
		ClassDB::bind_static_method("JSONConverter", D_METHOD("json5_to_json", "data", "pretty"), &JSONConverter::json5_to_json, DEFVAL(false));
		ClassDB::bind_static_method("JSONConverter", D_METHOD("json_to_json5", "data"), &JSONConverter::json_to_json5);
		ClassDB::bind_static_method("JSONConverter", D_METHOD("toml_to_json", "data", "pretty"), &JSONConverter::toml_to_json, DEFVAL(false));
		ClassDB::bind_static_method("JSONConverter", D_METHOD("json_to_toml", "data", "pretty"), &JSONConverter::json_to_toml, DEFVAL(false));
		ClassDB::bind_static_method("JSONConverter", D_METHOD("yaml_to_json", "data", "pretty"), &JSONConverter::yaml_to_json, DEFVAL(false));
		ClassDB::bind_static_method("JSONConverter", D_METHOD("json_to_yaml", "data"), &JSONConverter::json_to_yaml);
	}

public:
	static String json5_to_json(String data, bool pretty = false) {
		String ret;
		ret.parse_utf8(json_converter::json5_to_json(rust::Str(data.utf8().get_data()), pretty).c_str());
		return ret;
	}
	static String json_to_json5(String data) {
		String ret;
		ret.parse_utf8(json_converter::json_to_json5(rust::Str(data.utf8().get_data())).c_str());
		return ret;
	}

	static String toml_to_json(String data, bool pretty = false) {
		String ret;
		ret.parse_utf8(json_converter::toml_to_json(rust::Str(data.utf8().get_data()), pretty).c_str());
		return ret;
	}
	static String json_to_toml(String data, bool pretty = false) {
		String ret;
		ret.parse_utf8(json_converter::json_to_toml(rust::Str(data.utf8().get_data()), pretty).c_str());
		return ret;
	}

	static String yaml_to_json(String data, bool pretty = false) {
		return ryml_yaml_to_json(data, pretty);
	}
	static String json_to_yaml(String data) {
		return ryml_json_to_yaml(data);
	}
};

#endif // GD_JSON_CONVERTER_H
