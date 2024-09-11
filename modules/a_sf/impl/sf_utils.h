/**************************************************************************/
/*  sf_utils.h                                                            */
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

#ifndef SF_UTILS_H
#define SF_UTILS_H

// for .sf3
#include "stb/stb_vorbis.h"
#include "tiny/tml.h"
#include "tiny/tsf.h"

#include <core/io/file_access.h>
#include <core/os/memory.h>

using namespace godot;

int read_file_access(void *file, void *ptr, unsigned int size);
int skip_file_access(void *file, unsigned int count);

inline tsf *load_from_memory_tsf(PackedByteArray buffer) {
	return tsf_load_memory(buffer.ptr(), buffer.size());
}

inline tsf *load_from_file_tsf(Ref<FileAccess> file) {
	ERR_FAIL_COND_V(file.is_null(), nullptr);
	tsf_stream stream = { file.ptr(), read_file_access, skip_file_access };
	return tsf_load(&stream);
}

inline tsf *load_from_path_tsf(const String &p_path) {
	Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::ModeFlags::READ);
	ERR_FAIL_COND_V_MSG(file.is_null(), nullptr, vformat("Unable to open file: %s", p_path));
	return load_from_file_tsf(file);
}

inline tml_message *load_from_memory_tml(PackedByteArray buffer) {
	return tml_load_memory(buffer.ptr(), buffer.size());
}

inline tml_message *load_from_file_tml(Ref<FileAccess> file) {
	ERR_FAIL_COND_V(file.is_null(), nullptr);
	tml_stream stream = { file.ptr(), read_file_access };
	return tml_load(&stream);
}

inline tml_message *load_from_path_tml(const String &p_path) {
	Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::ModeFlags::READ);
	ERR_FAIL_COND_V_MSG(file.is_null(), nullptr, vformat("Unable to open file: %s", p_path));
	return load_from_file_tml(file);
}

#endif // SF_UTILS_H
