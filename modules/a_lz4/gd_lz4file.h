/**************************************************************************/
/*  gd_lz4file.h                                                          */
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

#ifndef GD_LZ4FILE_H
#define GD_LZ4FILE_H

#include "core/io/file_access.h"
#include "thirdparty/lz4/lz4frame_static.h"

typedef struct LZ4_readFile_s LZ4_readFile_t;
typedef struct LZ4_writeFile_s LZ4_writeFile_t;

/*! LZ4F_readOpen() :
 * Set read lz4file handle.
 * `lz4f` will set a lz4file handle.
 * `fp` must be the return value of the lz4 file opened by fopen.
 */
LZ4F_errorCode_t LZ4F_readOpen(LZ4_readFile_t **lz4fRead, Ref<FileAccess> fp);

/*! LZ4F_read() :
 * Read lz4file content to buffer.
 * `lz4f` must use LZ4_readOpen to set first.
 * `buf` read data buffer.
 * `size` read data buffer size.
 */
size_t LZ4F_read(LZ4_readFile_t *lz4fRead, void *buf, size_t size);

/*! LZ4F_readClose() :
 * Close lz4file handle.
 * `lz4f` must use LZ4_readOpen to set first.
 */
LZ4F_errorCode_t LZ4F_readClose(LZ4_readFile_t *lz4fRead);

/*! LZ4F_writeOpen() :
 * Set write lz4file handle.
 * `lz4f` will set a lz4file handle.
 * `fp` must be the return value of the lz4 file opened by fopen.
 */
LZ4F_errorCode_t LZ4F_writeOpen(LZ4_writeFile_t **lz4fWrite, Ref<FileAccess> fp, const LZ4F_preferences_t *prefsPtr);

/*! LZ4F_write() :
 * Write buffer to lz4file.
 * `lz4f` must use LZ4F_writeOpen to set first.
 * `buf` write data buffer.
 * `size` write data buffer size.
 */
size_t LZ4F_write(LZ4_writeFile_t *lz4fWrite, PackedByteArray buf);

/*! LZ4F_writeClose() :
 * Close lz4file handle.
 * `lz4f` must use LZ4F_writeOpen to set first.
 */
LZ4F_errorCode_t LZ4F_writeClose(LZ4_writeFile_t *lz4fWrite);

#endif // GD_LZ4FILE_H
