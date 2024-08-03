#pragma once

#include "core/object/ref_counted.h"
#include "cxx.h"
#include "gd_json_converter.rs.h"
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
