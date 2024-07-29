/**************************************************************************/
/*  resource_loader_txtz.cpp                                              */
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

#include "resource_loader_txtz.h"
#include "modules/a_lz4/gd_lz4.h"

void TXTZFile::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_compression", "p_compression"), &TXTZFile::set_compression);
	ClassDB::bind_method(D_METHOD("get_compression"), &TXTZFile::get_compression);
	ClassDB::bind_method(D_METHOD("set_data", "p_data"), &TXTZFile::set_data);
	ClassDB::bind_method(D_METHOD("get_data"), &TXTZFile::get_data);
	ClassDB::bind_method(D_METHOD("get_as_string"), &TXTZFile::get_as_string);

	BIND_ENUM_CONSTANT(UNKNOWN);
	BIND_ENUM_CONSTANT(COMPRESSED);
	BIND_ENUM_CONSTANT(UNCOMPRESSED);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "compression", PROPERTY_HINT_ENUM, "	UNKNOWN,COMPRESSED,UNCOMPRESSED"), "set_compression", "get_compression");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_BYTE_ARRAY, "data"), "set_data", "get_data");
}

String TXTZFile::get_as_string() {
	return Lz4::parse_as_string(get_data(), compression == COMPRESSED);
}
Ref<TXTZFile> ResourceFormatLoaderTXTZ::load(const PackedByteArray &p_bytes, Error *r_error) {
	if (r_error) {
		*r_error = ERR_FILE_CANT_OPEN;
	}
	Ref<TXTZFile> ret;
	ret.instantiate();
	ret->set_compression((TXTZFile::Compression)p_bytes[0]);
	ret->set_data(p_bytes.slice(1));
	if (r_error) {
		*r_error = OK;
	}

	return ret;
}

Ref<Resource> ResourceFormatLoaderTXTZ::load(const String &p_path, const String &p_original_path, Error *r_error, bool p_use_sub_threads, float *r_progress, CacheMode p_cache_mode) {
	if (r_error) {
		*r_error = ERR_FILE_CANT_OPEN;
	}

	if (!FileAccess::exists(p_path)) {
		*r_error = ERR_FILE_NOT_FOUND;
		return Ref<Resource>();
	}

	return load(FileAccess::get_file_as_bytes(p_path), r_error);
}

void ResourceFormatLoaderTXTZ::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("txtz");
}

bool ResourceFormatLoaderTXTZ::handles_type(const String &p_type) const {
	return (p_type == "TXTZ");
}

String ResourceFormatLoaderTXTZ::get_resource_type(const String &p_path) const {
	String el = p_path.get_extension().to_lower();
	if (el == "txtz") {
		return "TXTZ";
	}
	return "";
}

Error ResourceFormatSaverTXTZ::save(const Ref<Resource> &p_resource, const String &p_path, uint32_t p_flags) {
	Ref<TXTZFile> txtz = p_resource;
	ERR_FAIL_COND_V(txtz.is_null(), ERR_INVALID_PARAMETER);

	Error err;
	Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::WRITE, &err);
	ERR_FAIL_COND_V_MSG(err, err, "Cannot open file '" + p_path + "'.");

	file->store_8(txtz->get_compression());
	file->store_buffer(txtz->get_data());

	if (file->get_error() != OK && file->get_error() != ERR_FILE_EOF) {
		return ERR_CANT_CREATE;
	}

	return OK;
}

void ResourceFormatSaverTXTZ::get_recognized_extensions(const Ref<Resource> &p_resource, List<String> *p_extensions) const {
	Ref<TXTZFile> lottie = p_resource;
	if (lottie.is_valid()) {
		p_extensions->push_back("txtz");
	}
}

bool ResourceFormatSaverTXTZ::recognize(const Ref<Resource> &p_resource) const {
	return p_resource->is_class("TXTFile");
}
