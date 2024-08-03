/**************************************************************************/
/*  gd_glicol.h                                                           */
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

#ifndef GD_GLICOL_H
#define GD_GLICOL_H

#include "core/object/ref_counted.h"
#include "cxxbridge/cxx.h"
#include "cxxbridge/gd_glicol.rs.h"

class Glicol : public RefCounted {
	GDCLASS(Glicol, RefCounted);

	rust::Box<glicol::Glicol> inst = glicol::glicol_create();

protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("process", "buf_len"), &Glicol::process);
		ClassDB::bind_method(D_METHOD("add_sample", "name_ptr", "arr", "channels", "sample_rate"), &Glicol::add_sample);
		ClassDB::bind_method(D_METHOD("update_code", "str_ptr"), &Glicol::update_code);
		ClassDB::bind_method(D_METHOD("send_msg", "str_ptr"), &Glicol::send_msg);
		ClassDB::bind_method(D_METHOD("live_coding_mode", "boolean"), &Glicol::live_coding_mode);
		ClassDB::bind_method(D_METHOD("set_bpm", "bpm"), &Glicol::set_bpm);
		ClassDB::bind_method(D_METHOD("set_track_amp", "amp"), &Glicol::set_track_amp);
		ClassDB::bind_method(D_METHOD("set_sr", "sr"), &Glicol::set_sr);
		ClassDB::bind_method(D_METHOD("set_seed", "seed"), &Glicol::set_seed);
		ClassDB::bind_method(D_METHOD("reset"), &Glicol::reset);
	}

public:
	PackedFloat32Array process(size_t buf_len) {
		PackedFloat32Array ret;
		ret.resize(buf_len);
		inst->process(buf_len, ret.ptrw());
		return ret;
	};
	void add_sample(String name_ptr, PackedFloat32Array arr, size_t channels, size_t sample_rate) {
		rust::Vec<float> sample;
		std::copy_n(arr.ptr(), arr.size(), sample.begin());
		inst->add_sample(rust::Str(name_ptr.utf8().get_data()), sample, channels, sample_rate);
	};
	void update_code(String str_ptr) { inst->update_code(rust::Str(str_ptr.utf8().get_data())); };
	void send_msg(String str_ptr) { inst->send_msg(rust::Str(str_ptr.utf8().get_data())); };
	void live_coding_mode(bool boolean) { inst->live_coding_mode(boolean); };
	void set_bpm(float bpm) { inst->set_bpm(bpm); };
	void set_track_amp(float amp) { inst->set_track_amp(amp); };
	void set_sr(float sr) { inst->set_sr(sr); };
	void set_seed(float seed) { inst->set_seed(seed); };
	void reset() { inst->reset(); };
};

#endif // GD_GLICOL_H
