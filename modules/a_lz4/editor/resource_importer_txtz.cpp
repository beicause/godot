/**************************************************************************/
/*  resource_importer_txtz.cpp                                            */
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

#include "resource_importer_txtz.h"
#include "editor/editor_settings.h"
#include "modules/a_lz4/gd_lz4.h"
#include "modules/a_lz4/resource_loader_txtz.h"

String ResourceImporterTXTZ::get_importer_name() const {
	return "txtz";
}
String ResourceImporterTXTZ::get_visible_name() const {
	return "TXTZ";
}
String ResourceImporterTXTZ::get_save_extension() const {
	return "txtz";
}

String ResourceImporterTXTZ::get_resource_type() const {
	return "TXTZ";
}

int ResourceImporterTXTZ::get_preset_count() const {
	return 0;
}
String ResourceImporterTXTZ::get_preset_name(int p_idx) const {
	return "";
}
void ResourceImporterTXTZ::get_import_options(const String &p_path, List<ImportOption> *r_options, int p_preset) const {
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "compress"), true));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "compression_level", PROPERTY_HINT_RANGE, "0,12"), 9));
}
bool ResourceImporterTXTZ::get_option_visibility(const String &p_path, const String &p_option, const HashMap<StringName, Variant> &p_options) const {
	return true;
}
void ResourceImporterTXTZ::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("json5");
	p_extensions->push_back("jsonnet");
	p_extensions->push_back("libsonnet");
	const Vector<String> textfile_ext = ((String)(EDITOR_GET("docks/filesystem/textfile_extensions"))).split(",", false);
	for (const String &E : textfile_ext) {
		p_extensions->push_back(E);
	}
}

Error ResourceImporterTXTZ::import(const String &p_source_file, const String &p_save_path, const HashMap<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata) {
	const bool compress = p_options["compress"];
	const int compression_level = p_options["compression_level"];

	PackedByteArray file_content = FileAccess::get_file_as_bytes(p_source_file);
	Ref<FileAccess> file = FileAccess::open(p_save_path + ".txtz", FileAccess::WRITE);
	file->store_8(compress ? TXTZFile::Compression::COMPRESSED : TXTZFile::Compression::UNCOMPRESSED);
	file->store_buffer(compress ? Lz4::compress_frame(file_content, compression_level) : file_content);
	return OK;
}
