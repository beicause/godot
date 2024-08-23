/**************************************************************************/
/*  gd_lz4.cpp                                                            */
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

#include "gd_lz4.h"
#include <lz4.h>
#include <lz4frame_static.h>
#include <lz4hc.h>

extern "C" {
void *LZ4_malloc(size_t s) {
	return memalloc(s);
}

void *LZ4_calloc(size_t n, size_t s) {
	void *p = memalloc(n * s);
	memset(p, 0, n * s);
	return p;
}

void LZ4_free(void *p) {
	memfree(p);
}
}

PackedByteArray Lz4::decompress_block(const PackedByteArray &data, int dst_capacity) {
	PackedByteArray dst = PackedByteArray();
	if (data.size() == 0) {
		return dst;
	}
	if (dst_capacity <= 0) {
		memcpy(&dst_capacity, data.ptr(), sizeof(int));
	}
	dst.resize(dst_capacity);
	int res = LZ4_decompress_safe((const char *)(data.ptr() + sizeof(int)), (char *)(dst.ptrw()),
			data.size() - sizeof(int), dst.size());
	if (res < 0) {
		ERR_PRINT(vformat("LZ4 decompress block error: %s", LZ4F_getErrorName(res)));
		res = 0;
	}
	dst.resize(res);
	return dst;
}
PackedByteArray Lz4::compress_block_prepend_size(const PackedByteArray &data, int compression_level) {
	PackedByteArray dst = PackedByteArray();
	if (data.size() == 0) {
		return dst;
	}
	dst.resize(LZ4_compressBound(data.size()) + sizeof(int));
	int compressed_size;
	if (compression_level > 0) {
		compressed_size = LZ4_compress_HC((const char *)(data.ptr()), (char *)(dst.ptrw() + sizeof(int)), data.size(), dst.size() - sizeof(int), compression_level);
	} else {
		compressed_size = LZ4_compress_fast((const char *)(data.ptr()), (char *)(dst.ptrw() + sizeof(int)), data.size(), dst.size() - sizeof(int), -compression_level);
	}
	dst.resize(compressed_size + sizeof(int));
	memcpy(dst.ptrw(), &compressed_size, sizeof(int));
	return dst;
}

struct LZ4_read_buffer {
	LZ4F_dctx *dctxPtr = nullptr;
	PackedByteArray fp;
	size_t fp_ptr = 0UL;
	LZ4_byte *srcBuf = nullptr;
	size_t srcBufNext = 0UL;
	size_t srcBufSize = 0UL;
	size_t srcBufMaxSize = 0UL;
};

PackedByteArray Lz4::decompress_frame(PackedByteArray data) {
#define clean_buf_return(v)                          \
	memfree(lz4fRead.srcBuf);                        \
	LZ4F_freeDecompressionContext(lz4fRead.dctxPtr); \
	return v
#define clean_return(v)                              \
	LZ4F_freeDecompressionContext(lz4fRead.dctxPtr); \
	return v

	if (data.size() == 0) {
		return PackedByteArray();
	}
	LZ4_read_buffer lz4fRead;
	size_t consumedSize;
	LZ4F_errorCode_t err_code;

	err_code = LZ4F_createDecompressionContext(&lz4fRead.dctxPtr, LZ4F_VERSION);
	if (LZ4F_isError(err_code)) {
		clean_return(PackedByteArray());
	}

	lz4fRead.fp = data;
	PackedByteArray chunk_buf = lz4fRead.fp.slice(0, LZ4F_HEADER_SIZE_MAX);
	lz4fRead.fp_ptr += LZ4F_HEADER_SIZE_MAX;
	consumedSize = chunk_buf.size();

	LZ4F_frameInfo_t info;
	LZ4F_errorCode_t const r = LZ4F_getFrameInfo(lz4fRead.dctxPtr, &info, chunk_buf.ptr(), &consumedSize);
	if (LZ4F_isError(r)) {
		clean_return(PackedByteArray());
	}

	switch (info.blockSizeID) {
		case LZ4F_default:
		case LZ4F_max64KB:
			lz4fRead.srcBufMaxSize = 64 * 1024;
			break;
		case LZ4F_max256KB:
			lz4fRead.srcBufMaxSize = 256 * 1024;
			break;
		case LZ4F_max1MB:
			lz4fRead.srcBufMaxSize = 1 * 1024 * 1024;
			break;
		case LZ4F_max4MB:
			lz4fRead.srcBufMaxSize = 4 * 1024 * 1024;
			break;
		default:
			clean_return(PackedByteArray());
	}

	lz4fRead.srcBuf = (LZ4_byte *)memalloc(lz4fRead.srcBufMaxSize);
	lz4fRead.srcBufSize = chunk_buf.size() - consumedSize;
	memcpy(lz4fRead.srcBuf, chunk_buf.ptr() + consumedSize, lz4fRead.srcBufSize);

	auto read_func = [&](int dst_capacity) {
		PackedByteArray dst;
		if (dst_capacity < 0) {
			return PackedByteArray();
		}
		dst.resize(dst_capacity);
		LZ4_byte *p = (LZ4_byte *)dst.ptrw();
		size_t next = 0;
		size_t size = dst.size();

		while (next < size) {
			size_t srcsize = lz4fRead.srcBufSize - lz4fRead.srcBufNext;
			size_t dstsize = size - next;
			size_t ret;

			if (srcsize == 0) {
				PackedByteArray buf = lz4fRead.fp.slice(lz4fRead.fp_ptr, lz4fRead.srcBufMaxSize);
				lz4fRead.fp_ptr += buf.size();
				ret = buf.size();
				memcpy(lz4fRead.srcBuf, buf.ptr(), buf.size());
				if (ret > 0) {
					lz4fRead.srcBufSize = ret;
					srcsize = lz4fRead.srcBufSize;
					lz4fRead.srcBufNext = 0;
				} else if (ret == 0) {
					break;
				} else {
					return PackedByteArray();
				}
			}

			ret = LZ4F_decompress(lz4fRead.dctxPtr,
					p, &dstsize,
					lz4fRead.srcBuf + lz4fRead.srcBufNext,
					&srcsize,
					nullptr);
			if (LZ4F_isError(ret)) {
				return PackedByteArray();
			}

			lz4fRead.srcBufNext += srcsize;
			next += dstsize;
			p += dstsize;
		}
		dst.resize(next);
		return dst;
	};

	constexpr int chunk_size = 16 * 1024;
	PackedByteArray ret_array;
	while (true) {
		PackedByteArray buf = read_func(chunk_size);
		if (buf.size() == 0) {
			break;
		}
		ret_array.append_array(buf);
	}
	clean_buf_return(ret_array);

#undef clean_buf_return
#undef clean_return
}

PackedByteArray Lz4::compress_frame(PackedByteArray data, int compression_level) {
	PackedByteArray ret;
	if (data.size() == 0) {
		return ret;
	}
	LZ4F_preferences_t prefs = LZ4F_INIT_PREFERENCES;
	prefs.compressionLevel = compression_level;
	size_t size_bound = LZ4F_compressFrameBound(data.size(), &prefs);
	ret.resize(size_bound);
	size_t size = LZ4F_compressFrame(ret.ptrw(), ret.size(), data.ptr(), data.size(), &prefs);
	ret.resize(size);
	return ret;
}

void Lz4::_bind_methods() {
	ClassDB::bind_static_method("Lz4", D_METHOD("decompress_block", "data", "dst_capacity"), &Lz4::decompress_block, DEFVAL(0));
	ClassDB::bind_static_method("Lz4", D_METHOD("compress_block_prepend_size", "data", "compression_level"), &Lz4::compress_block_prepend_size, DEFVAL(0));
	ClassDB::bind_static_method("Lz4", D_METHOD("decompress_frame", "data"), &Lz4::decompress_frame);
	ClassDB::bind_static_method("Lz4", D_METHOD("compress_frame", "data", "compression_level"), &Lz4::compress_frame, DEFVAL(0));
	ClassDB::bind_static_method("Lz4", D_METHOD("parse_as_string", "p_bytes", "p_hint_compressed"), &Lz4::parse_as_string, DEFVAL(false));
}
