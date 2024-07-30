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

#include "core/io/file_access.h"
#include "core/object/ref_counted.h"
#include "gd_lz4file.h"

using namespace godot;

class Lz4 : public RefCounted {
	GDCLASS(Lz4, RefCounted);

protected:
	static void _bind_methods() {
		ClassDB::bind_static_method("Lz4", D_METHOD("decompress_block", "data", "dst_capacity"), &Lz4::decompress_block, DEFVAL(0));
		ClassDB::bind_static_method("Lz4", D_METHOD("compress_block_prepend_size", "data", "compression_level"), &Lz4::compress_block_prepend_size, DEFVAL(0));
		ClassDB::bind_static_method("Lz4", D_METHOD("decompress_frame", "data"), &Lz4::decompress_frame);
		ClassDB::bind_static_method("Lz4", D_METHOD("compress_frame", "data", "compression_level"), &Lz4::compress_frame, DEFVAL(0));
	}

public:
	static PackedByteArray decompress_block(const PackedByteArray &data, int dst_capacity = 0);
	static PackedByteArray compress_block_prepend_size(const PackedByteArray &data, int compression_level = 0);

	static PackedByteArray decompress_frame(PackedByteArray data);
	static PackedByteArray compress_frame(PackedByteArray data, int compression_level = 0);

	static String parse_as_string(PackedByteArray p_bytes) {
		PackedByteArray bytes_null_term = p_bytes;
		bytes_null_term.resize(bytes_null_term.size() + 1);
		const char *chars_null_term = reinterpret_cast<const char *>(bytes_null_term.ptr());
		String ret;
		if (ret.validate_utf8(chars_null_term, bytes_null_term.size())) {
			ret.parse_utf8(chars_null_term);
		} else {
			bytes_null_term = Lz4::decompress_frame(p_bytes);
			bytes_null_term.resize(bytes_null_term.size() + 1);
			const char *decompressed_chars_null_term = reinterpret_cast<const char *>(bytes_null_term.ptr());

			if (ret.validate_utf8(decompressed_chars_null_term)) {
				ret.parse_utf8(decompressed_chars_null_term);
			}
		}
		return ret;
	}
};

class Lz4File : public RefCounted {
	GDCLASS(Lz4File, RefCounted);

	LZ4_readFile_t *lz4fRead = nullptr;
	LZ4_writeFile_t *lz4fWrite = nullptr;

	static LZ4F_errorCode_t returnErrorCode(LZ4F_errorCodes code) {
		return (LZ4F_errorCode_t) - (ptrdiff_t)code;
	}

protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("open_read", "path"), &Lz4File::open_read);
		ClassDB::bind_method(D_METHOD("open_write", "path"), &Lz4File::open_write);
		ClassDB::bind_method(D_METHOD("open_read_file", "f"), &Lz4File::open_read_file);
		ClassDB::bind_method(D_METHOD("open_write_file", "f"), &Lz4File::open_write_file);
		ClassDB::bind_method(D_METHOD("read", "size"), &Lz4File::read, DEFVAL(0));
		ClassDB::bind_method(D_METHOD("write", "data"), &Lz4File::write);
		ClassDB::bind_method(D_METHOD("close_write"), &Lz4File::close_write);
		ClassDB::bind_method(D_METHOD("close_read"), &Lz4File::close_read);
	};

public:
	Error open_read(String path) {
		Ref<FileAccess> f = FileAccess::open(path, FileAccess::READ);
		if (f.is_null()) {
			return ERR_CANT_OPEN;
		}
		return open_read_file(f);
	}
	Error open_write(String path) {
		Ref<FileAccess> f = FileAccess::open(path, FileAccess::WRITE);
		if (f.is_null()) {
			return ERR_CANT_OPEN;
		}
		return open_write_file(f);
	}
	Error open_read_file(Ref<FileAccess> f) {
		LZ4F_errorCode_t err = LZ4F_readOpen(&lz4fRead, f);
		return LZ4F_isError(err) ? ERR_FILE_CANT_READ : OK;
	}
	Error open_write_file(Ref<FileAccess> f) {
		LZ4F_errorCode_t err = LZ4F_writeOpen(&lz4fWrite, f, nullptr);
		return LZ4F_isError(err) ? ERR_FILE_CANT_WRITE : OK;
	}

	PackedByteArray read(int size = 0) {
		if (size > 0) {
			if (lz4fRead == nullptr) {
				ERR_PRINT("LZ4 Reader is null, please open read first");
				return PackedByteArray();
			}
			PackedByteArray ret;
			ret.resize(size);
			size_t res = LZ4F_read(lz4fRead, ret.ptrw(), size);
			if (LZ4F_isError(res)) {
				ERR_PRINT(vformat("LZ4 read error: %s", LZ4F_getErrorName(res)));
				ret.resize(0);
			}
			ret.resize(res);
			return ret;
		} else {
			PackedByteArray ret;
			constexpr int chunk_size = 16 * 1024;
			while (true) {
				PackedByteArray chunk = read(chunk_size);
				if (chunk.size() == 0) {
					break;
				}
				ret.append_array(chunk);
			}
			return ret;
		}
	}

	int write(PackedByteArray data) {
		if (data.size() == 0) {
			return 0;
		}
		if (lz4fWrite == nullptr) {
			ERR_PRINT("LZ4 writer is null, please open write first");
			returnErrorCode(LZ4F_ERROR_parameter_null);
		}
		return LZ4F_write(lz4fWrite, data);
	}

	Error close_write() {
		if (lz4fWrite != nullptr) {
			LZ4F_errorCode_t ret = LZ4F_writeClose(lz4fWrite);
			lz4fWrite = nullptr;
			if (LZ4F_isError(ret)) {
				ERR_PRINT(vformat("LZ4 write error: %s", LZ4F_getErrorName(ret)));
				return ERR_FILE_CANT_WRITE;
			}
		}
		return OK;
	}

	Error close_read() {
		if (lz4fRead != nullptr) {
			LZ4F_errorCode_t ret = LZ4F_readClose(lz4fRead);
			lz4fRead = nullptr;
			if (LZ4F_isError(ret)) {
				ERR_PRINT(vformat("LZ4 read error: %s", LZ4F_getErrorName(ret)));
				return ERR_FILE_CANT_READ;
			}
		}
		return OK;
	}

	~Lz4File() {
		close_read();
		close_write();
	}
};

#endif // GD_LZ4_H
