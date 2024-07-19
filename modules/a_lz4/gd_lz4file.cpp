/**************************************************************************/
/*  gd_lz4file.cpp                                                        */
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

#include "gd_lz4file.h"
#include "lz4.h"
#include <assert.h>

static void *LZ4_calloc(size_t n, size_t s) {
	void *p = memalloc(n * s);
	memset(p, 0, n * s);
	return p;
}

static LZ4F_errorCode_t returnErrorCode(LZ4F_errorCodes code) {
	return (LZ4F_errorCode_t) - (ptrdiff_t)code;
}
#undef RETURN_ERROR
#define RETURN_ERROR(e) return returnErrorCode(LZ4F_ERROR_##e)

/* =====   read API   ===== */

struct LZ4_readFile_s {
	LZ4F_dctx *dctxPtr;
	Ref<FileAccess> fp;
	LZ4_byte *srcBuf;
	size_t srcBufNext;
	size_t srcBufSize;
	size_t srcBufMaxSize;
};

static void LZ4F_freeReadFile(LZ4_readFile_t *lz4fRead) {
	if (lz4fRead == nullptr) {
		return;
	}
	LZ4F_freeDecompressionContext(lz4fRead->dctxPtr);
	memfree(lz4fRead->srcBuf);
	memfree(lz4fRead);
}

static void LZ4F_freeAndNullReadFile(LZ4_readFile_t **statePtr) {
	assert(statePtr != nullptr);
	LZ4F_freeReadFile(*statePtr);
	*statePtr = nullptr;
}

LZ4F_errorCode_t LZ4F_readOpen(LZ4_readFile_t **lz4fRead, Ref<FileAccess> fp) {
	size_t consumedSize;
	LZ4F_errorCode_t ret;

	if (fp == nullptr || lz4fRead == nullptr) {
		RETURN_ERROR(parameter_null);
	}

	*lz4fRead = (LZ4_readFile_t *)LZ4_calloc(1, sizeof(LZ4_readFile_t));
	if (*lz4fRead == nullptr) {
		RETURN_ERROR(allocation_failed);
	}

	ret = LZ4F_createDecompressionContext(&(*lz4fRead)->dctxPtr, LZ4F_VERSION);
	if (LZ4F_isError(ret)) {
		LZ4F_freeAndNullReadFile(lz4fRead);
		return ret;
	}

	(*lz4fRead)->fp = fp;
	PackedByteArray buf = (*lz4fRead)->fp->get_buffer(LZ4F_HEADER_SIZE_MAX);
	consumedSize = buf.size();
	{
		LZ4F_frameInfo_t info;
		LZ4F_errorCode_t const r = LZ4F_getFrameInfo((*lz4fRead)->dctxPtr, &info, buf.ptr(), &consumedSize);
		if (LZ4F_isError(r)) {
			LZ4F_freeAndNullReadFile(lz4fRead);
			return r;
		}

		switch (info.blockSizeID) {
			case LZ4F_default:
			case LZ4F_max64KB:
				(*lz4fRead)->srcBufMaxSize = 64 * 1024;
				break;
			case LZ4F_max256KB:
				(*lz4fRead)->srcBufMaxSize = 256 * 1024;
				break;
			case LZ4F_max1MB:
				(*lz4fRead)->srcBufMaxSize = 1 * 1024 * 1024;
				break;
			case LZ4F_max4MB:
				(*lz4fRead)->srcBufMaxSize = 4 * 1024 * 1024;
				break;
			default:
				LZ4F_freeAndNullReadFile(lz4fRead);
				RETURN_ERROR(maxBlockSize_invalid);
		}
	}

	(*lz4fRead)->srcBuf = (LZ4_byte *)memalloc((*lz4fRead)->srcBufMaxSize);
	(*lz4fRead)->srcBufSize = buf.size() - consumedSize;
	memcpy((*lz4fRead)->srcBuf, buf.ptr() + consumedSize, (*lz4fRead)->srcBufSize);
	return ret;
}

size_t LZ4F_read(LZ4_readFile_t *lz4fRead, void *buf, size_t size) {
	LZ4_byte *p = (LZ4_byte *)buf;
	size_t next = 0;

	if (lz4fRead == nullptr) {
		RETURN_ERROR(parameter_null);
	}

	while (next < size) {
		size_t srcsize = lz4fRead->srcBufSize - lz4fRead->srcBufNext;
		size_t dstsize = size - next;
		size_t ret;

		if (srcsize == 0) {
			PackedByteArray block_buf = lz4fRead->fp->get_buffer(lz4fRead->srcBufMaxSize);
			ret = block_buf.size();
			memcpy(lz4fRead->srcBuf, block_buf.ptr(), block_buf.size());
			if (ret > 0) {
				lz4fRead->srcBufSize = ret;
				srcsize = lz4fRead->srcBufSize;
				lz4fRead->srcBufNext = 0;
			} else if (ret == 0) {
				break;
			} else {
				RETURN_ERROR(io_read);
			}
		}

		ret = LZ4F_decompress(lz4fRead->dctxPtr,
				p, &dstsize,
				lz4fRead->srcBuf + lz4fRead->srcBufNext,
				&srcsize,
				nullptr);
		if (LZ4F_isError(ret)) {
			return ret;
		}

		lz4fRead->srcBufNext += srcsize;
		next += dstsize;
		p += dstsize;
	}

	return next;
}

LZ4F_errorCode_t LZ4F_readClose(LZ4_readFile_t *lz4fRead) {
	if (lz4fRead == nullptr) {
		RETURN_ERROR(parameter_null);
	}
	LZ4F_freeReadFile(lz4fRead);
	return LZ4F_OK_NoError;
}

/* =====   write API   ===== */

struct LZ4_writeFile_s {
	LZ4F_cctx *cctxPtr;
	Ref<FileAccess> fp;
	LZ4_byte *dstBuf;
	size_t maxWriteSize;
	size_t dstBufMaxSize;
	LZ4F_errorCode_t errCode;
};

static void LZ4F_freeWriteFile(LZ4_writeFile_t *state) {
	if (state == nullptr) {
		return;
	}
	LZ4F_freeCompressionContext(state->cctxPtr);
	memfree(state->dstBuf);
	memfree(state);
}

static void LZ4F_freeAndNullWriteFile(LZ4_writeFile_t **statePtr) {
	assert(statePtr != nullptr);
	LZ4F_freeWriteFile(*statePtr);
	*statePtr = nullptr;
}

LZ4F_errorCode_t LZ4F_writeOpen(LZ4_writeFile_t **lz4fWrite, Ref<FileAccess> fp, const LZ4F_preferences_t *prefsPtr) {
	PackedByteArray buf;
	buf.resize(LZ4F_HEADER_SIZE_MAX);
	size_t ret;

	if (fp == nullptr || lz4fWrite == nullptr) {
		RETURN_ERROR(parameter_null);
	}

	*lz4fWrite = (LZ4_writeFile_t *)LZ4_calloc(1, sizeof(LZ4_writeFile_t));
	if (*lz4fWrite == nullptr) {
		RETURN_ERROR(allocation_failed);
	}
	if (prefsPtr != nullptr) {
		switch (prefsPtr->frameInfo.blockSizeID) {
			case LZ4F_default:
			case LZ4F_max64KB:
				(*lz4fWrite)->maxWriteSize = 64 * 1024;
				break;
			case LZ4F_max256KB:
				(*lz4fWrite)->maxWriteSize = 256 * 1024;
				break;
			case LZ4F_max1MB:
				(*lz4fWrite)->maxWriteSize = 1 * 1024 * 1024;
				break;
			case LZ4F_max4MB:
				(*lz4fWrite)->maxWriteSize = 4 * 1024 * 1024;
				break;
			default:
				LZ4F_freeAndNullWriteFile(lz4fWrite);
				RETURN_ERROR(maxBlockSize_invalid);
		}
	} else {
		(*lz4fWrite)->maxWriteSize = 64 * 1024;
	}

	(*lz4fWrite)->dstBufMaxSize = LZ4F_compressBound((*lz4fWrite)->maxWriteSize, prefsPtr);
	(*lz4fWrite)->dstBuf = (LZ4_byte *)memalloc((*lz4fWrite)->dstBufMaxSize);
	if ((*lz4fWrite)->dstBuf == nullptr) {
		LZ4F_freeAndNullWriteFile(lz4fWrite);
		RETURN_ERROR(allocation_failed);
	}

	ret = LZ4F_createCompressionContext(&(*lz4fWrite)->cctxPtr, LZ4F_VERSION);
	if (LZ4F_isError(ret)) {
		LZ4F_freeAndNullWriteFile(lz4fWrite);
		return ret;
	}

	ret = LZ4F_compressBegin((*lz4fWrite)->cctxPtr, buf.ptrw(), LZ4F_HEADER_SIZE_MAX, prefsPtr);
	if (LZ4F_isError(ret)) {
		LZ4F_freeAndNullWriteFile(lz4fWrite);
		return ret;
	}

	buf.resize(ret);
	fp->store_buffer(buf);

	(*lz4fWrite)->fp = fp;
	(*lz4fWrite)->errCode = LZ4F_OK_NoError;
	return LZ4F_OK_NoError;
}

size_t LZ4F_write(LZ4_writeFile_t *lz4fWrite, PackedByteArray buf) {
	const LZ4_byte *p = (const LZ4_byte *)buf.ptr();
	size_t remain = buf.size();
	size_t chunk;
	size_t ret;

	if (lz4fWrite == nullptr) {
		RETURN_ERROR(parameter_null);
	}
	while (remain) {
		if (remain > lz4fWrite->maxWriteSize) {
			chunk = lz4fWrite->maxWriteSize;
		} else {
			chunk = remain;
		}

		ret = LZ4F_compressUpdate(lz4fWrite->cctxPtr,
				lz4fWrite->dstBuf, lz4fWrite->dstBufMaxSize,
				p, chunk,
				nullptr);
		if (LZ4F_isError(ret)) {
			lz4fWrite->errCode = ret;
			return ret;
		}

		PackedByteArray dstBuf;
		dstBuf.resize(ret);
		memcpy(dstBuf.ptrw(), lz4fWrite->dstBuf, ret);
		lz4fWrite->fp->store_buffer(dstBuf);

		p += chunk;
		remain -= chunk;
	}

	return buf.size();
}

LZ4F_errorCode_t LZ4F_writeClose(LZ4_writeFile_t *lz4fWrite) {
	LZ4F_errorCode_t ret = LZ4F_OK_NoError;

	if (lz4fWrite == nullptr) {
		RETURN_ERROR(parameter_null);
	}

	if (lz4fWrite->errCode == LZ4F_OK_NoError) {
		ret = LZ4F_compressEnd(lz4fWrite->cctxPtr,
				lz4fWrite->dstBuf, lz4fWrite->dstBufMaxSize,
				nullptr);
		if (LZ4F_isError(ret)) {
			goto out;
		}
		PackedByteArray dstBuf;
		dstBuf.resize(ret);
		memcpy(dstBuf.ptrw(), lz4fWrite->dstBuf, ret);
		lz4fWrite->fp->store_buffer(dstBuf);
	}

out:
	LZ4F_freeWriteFile(lz4fWrite);
	return ret;
}
