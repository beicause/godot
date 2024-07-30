/**************************************************************************/
/*  resource_loader_txtz.h                                                */
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

#ifndef RESOURCE_LOADER_TXTZ_H
#define RESOURCE_LOADER_TXTZ_H
#include "core/io/resource_loader.h"
#include "modules/a_lz4/gd_lz4.h"

class TXTZ : public Resource {
	GDCLASS(TXTZ, Resource);
	PackedByteArray bytes;
	bool compress = true;

protected:
	static void _bind_methods();

public:
	String get_as_string() {
		String ret;
		const char *raw_bytes = reinterpret_cast<const char *>(bytes.ptr());
		if ((!compress) && ret.validate_utf8(raw_bytes, bytes.size())) {
			ret.parse_utf8(raw_bytes);
		} else {
			const char *compressed_bytes = reinterpret_cast<const char *>(Lz4::decompress_frame(bytes).ptr());
			if (ret.validate_utf8(compressed_bytes, bytes.size())) {
				ret.parse_utf8(compressed_bytes);
			}
		}
		return ret;
	}

	PackedByteArray get_buffer(bool decompress = false) {
		if (decompress) {
			return Lz4::decompress_frame(bytes);
		}
		return bytes;
	}
	void set_buffer(PackedByteArray p_buffer, bool compress = false) {
		if (compress) {
			bytes = Lz4::compress_frame(p_buffer);
		} else {
			bytes = p_buffer;
		}
	}
	void write_string(String p_string) {
		if (compress) {
			bytes = Lz4::compress_frame(p_string.to_utf8_buffer());
		} else {
			bytes = p_string.to_utf8_buffer();
		}
	}
	void set_compress(bool p_compress) { compress = p_compress; }
	bool is_compress() { return compress; }
};

class ResourceFormatLoaderTXTZ : public ResourceFormatLoader {
public:
	static Ref<TXTZ> load(const PackedByteArray &p_bytes, Error *r_error = nullptr);

	virtual Ref<Resource> load(const String &p_path, const String &p_original_path = "", Error *r_error = nullptr, bool p_use_sub_threads = false, float *r_progress = nullptr, CacheMode p_cache_mode = CACHE_MODE_REUSE) override;
	virtual void get_recognized_extensions(List<String> *p_extensions) const override;
	virtual bool handles_type(const String &p_type) const override;
	virtual String get_resource_type(const String &p_path) const override;
};

#endif // RESOURCE_LOADER_TXTZ_H
