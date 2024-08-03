#include "resource_importer_json_converter.h"
#include "core/io/json.h"
#include "modules/a_lz4/gd_lz4.h"
#include "modules/a_rust/gd_json_converter.h"

String ResourceImporterJSONConverter::get_importer_name() const {
	return "json_converter";
}
String ResourceImporterJSONConverter::get_visible_name() const {
	return "JSON Converter";
}
String ResourceImporterJSONConverter::get_save_extension() const {
	return "jsonz";
}

String ResourceImporterJSONConverter::get_resource_type() const {
	return "JSON";
}

int ResourceImporterJSONConverter::get_preset_count() const {
	return 0;
}
String ResourceImporterJSONConverter::get_preset_name(int p_idx) const {
	return "";
}
void ResourceImporterJSONConverter::get_import_options(const String &p_path, List<ImportOption> *r_options, int p_preset) const {
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "pretty"), false));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "compress"), true));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "compression_level", PROPERTY_HINT_RANGE, "0,12"), 9));
}
bool ResourceImporterJSONConverter::get_option_visibility(const String &p_path, const String &p_option, const HashMap<StringName, Variant> &p_options) const {
	return true;
}
void ResourceImporterJSONConverter::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("json");
	p_extensions->push_back("json5");
	p_extensions->push_back("toml");
	p_extensions->push_back("yaml");
	p_extensions->push_back("yml");
}

Error ResourceImporterJSONConverter::import(const String &p_source_file, const String &p_save_path, const HashMap<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata) {
	const bool pretty = p_options["pretty"];
	const bool compress = p_options["compress"];
	const int compression_level = p_options["compression_level"];

	Ref<JSON> json;
	json.instantiate();
	String file_content = FileAccess::get_file_as_string(p_source_file);
	String file_ext = p_source_file.get_extension();
	String json_text;
	if (file_ext == "json") {
		json_text = file_content;
		if (pretty) {
			Ref<JSON> j;
			j.instantiate();
			j->parse(json_text);
			json_text = j->stringify(j->get_data(), "\t", true, true);
		}
	} else if (file_ext == "json5") {
		json_text = JSONConverter::json5_to_json(file_content, pretty);
	} else if (file_ext == "toml") {
		json_text = JSONConverter::toml_to_json(file_content, pretty);
	} else if (file_ext == "yaml") {
		json_text = JSONConverter::yaml_to_json(file_content, pretty);
	} else {
		return ERR_FILE_UNRECOGNIZED;
	}
	Error err = json->parse(json_text, true);
	if (err != OK) {
		String err_text = "Error parsing JSON file at '" + p_source_file + "', on line " + itos(json->get_error_line()) + ": " + json->get_error_message();
		ERR_PRINT(err_text);
		return ERR_INVALID_DATA;
	}
	Ref<FileAccess> file = FileAccess::open(p_save_path + ".jsonz", FileAccess::WRITE);
	if (compress) {
		PackedByteArray b = json->get_parsed_text().to_utf8_buffer();
		file->store_buffer(Lz4::compress_frame(b, compression_level));
	} else {
		file->store_string(json->get_parsed_text());
	}
	file->close();
	return err;
}
