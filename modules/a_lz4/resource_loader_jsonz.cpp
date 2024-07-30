/**************************************************************************/
/*  resource_loader_jsonz.cpp                                             */
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

#include "resource_loader_jsonz.h"
#include "core/io/json.h"
#include "resource_loader_txtz.h"

Ref<JSON> ResourceFormatLoaderJSONZ::load(const PackedByteArray &p_bytes, Error *r_error) {
	if (r_error) {
		*r_error = ERR_FILE_CANT_OPEN;
	}

	Ref<JSON> json;
	json.instantiate();

	PackedByteArray bytes = p_bytes;
	Ref<TXTZ> json_str = ResourceFormatLoaderTXTZ::load(bytes, r_error);

	Error err = json->parse(json_str->get_as_string());
	if (err != OK) {
		String err_text = "Error parsing JSON file, on line " + itos(json->get_error_line()) + ": " + json->get_error_message();
		if (r_error) {
			*r_error = err;
		}
		ERR_PRINT(err_text);
		return Ref<Resource>();
	}

	if (r_error) {
		*r_error = OK;
	}

	return json;
}

Ref<Resource> ResourceFormatLoaderJSONZ::load(const String &p_path, const String &p_original_path, Error *r_error, bool p_use_sub_threads, float *r_progress, CacheMode p_cache_mode) {
	if (r_error) {
		*r_error = ERR_FILE_CANT_OPEN;
	}

	if (!FileAccess::exists(p_path)) {
		*r_error = ERR_FILE_NOT_FOUND;
		return Ref<Resource>();
	}

	return load(FileAccess::get_file_as_bytes(p_path), r_error);
}

void ResourceFormatLoaderJSONZ::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("jsonz");
}

bool ResourceFormatLoaderJSONZ::handles_type(const String &p_type) const {
	return (p_type == "JSON");
}

String ResourceFormatLoaderJSONZ::get_resource_type(const String &p_path) const {
	String el = p_path.get_extension().to_lower();
	if (el == "jsonz") {
		return "JSON";
	}
	return "";
}
