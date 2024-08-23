/**************************************************************************/
/*  gd_lz4.h                                                              */
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

#ifndef GD_LZ4_H
#define GD_LZ4_H

#include "core/object/ref_counted.h"

using namespace godot;

class Lz4 : public RefCounted {
	GDCLASS(Lz4, RefCounted);

protected:
	static void _bind_methods();

public:
	static PackedByteArray decompress_block(const PackedByteArray &data, int dst_capacity = 0);
	static PackedByteArray compress_block_prepend_size(const PackedByteArray &data, int compression_level = 0);

	static PackedByteArray decompress_frame(PackedByteArray data);
	static PackedByteArray compress_frame(PackedByteArray data, int compression_level = 0);

	static String parse_as_string(PackedByteArray p_bytes, bool p_hint_compressed = false) {
		String ret;
		if (p_bytes.size() == 0) {
			return ret;
		}
		const char *chars = (const char *)p_bytes.ptr();
		Error err;
		if (!p_hint_compressed && ret.validate_utf8(chars, p_bytes.size()) == OK) {
			err = ret.parse_utf8(chars, p_bytes.size());
		} else {
			PackedByteArray dec = Lz4::decompress_frame(p_bytes);
			const char *dec_chars = (const char *)dec.ptr();
			err = ret.parse_utf8(dec_chars, dec.size());
		}
		if (err != OK) {
			ERR_PRINT(vformat("lz4 parse utf8 string failed: %s", error_names[err]));
		}
		return ret;
	}
};

#endif // GD_LZ4_H
