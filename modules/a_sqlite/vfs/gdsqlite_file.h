/**************************************************************************/
/*  gdsqlite_file.h                                                       */
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

#ifndef GDSQLITE_FILE_H
#define GDSQLITE_FILE_H

#include "core/io/file_access.h"
#include <sqlite3.h>

struct gdsqlite_file {
	sqlite3_file base; /* Base class. Must be first. */
	Ref<FileAccess> file; /* File descriptor */

	static int close(sqlite3_file *pFile);
	static int read(sqlite3_file *pFile, void *zBuf, int iAmt, sqlite_int64 iOfst);
	static int write(sqlite3_file *pFile, const void *zBuf, int iAmt, sqlite_int64 iOfst);
	static int truncate(sqlite3_file *pFile, sqlite_int64 size);
	static int sync(sqlite3_file *pFile, int flags);
	static int fileSize(sqlite3_file *pFile, sqlite_int64 *pSize);
	static int lock(sqlite3_file *pFile, int eLock);
	static int unlock(sqlite3_file *pFile, int eLock);
	static int checkReservedLock(sqlite3_file *pFile, int *pResOut);
	static int fileControl(sqlite3_file *pFile, int op, void *pArg);
	static int sectorSize(sqlite3_file *pFile);
	static int deviceCharacteristics(sqlite3_file *pFile);
};

#endif // GDSQLITE_FILE_H
