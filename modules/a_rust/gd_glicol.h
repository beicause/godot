#pragma once

#include "core/object/ref_counted.h"
#include "cxx.h"
#include "gd_glicol.rs.h"

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
