/**************************************************************************/
/*  sound_font.h                                                          */
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

#ifndef SOUND_FONT_H
#define SOUND_FONT_H

#include "core/io/resource.h"
#include "core/io/resource_loader.h"
#include "sf_utils.h"

using namespace godot;

class SoundFont : public Resource {
	GDCLASS(SoundFont, Resource);

	tsf *_tsf = nullptr;

protected:
	static void _bind_methods();

public:
	enum OutputMode {
		OUTPUT_STEREO_INTERLEAVED,
		OUTPUT_STEREO_UNWEAVED,
		OUTPUT_MONO
	};
	void set_tsf(tsf *f) {
		_tsf = f;
	}

	tsf *get_tsf() {
		return _tsf;
	}

	static Ref<SoundFont> create_from_path(const String &p_path);
	static Ref<SoundFont> create_from_file(Ref<FileAccess> file);
	static Ref<SoundFont> create_from_memory(const PackedByteArray &buffer);

	int get_preset_num();
	int get_voice_num();
	int get_max_voice_num();
	OutputMode get_output_mode();
	float get_out_sample_rate();
	float get_global_gain_db();
	void set_preset_num(int preset_num);
	void set_voice_num(int voice_num);
	void set_max_voice_num(int maxVoiceNum);
	void set_output_mode(OutputMode output_mode);
	void set_out_sample_rate(float out_sample_rate);
	void set_global_gain_db(float global_gain_db);

	Ref<SoundFont> copy();
	void reset();
	int get_preset_index(int bank, int preset_number);
	int get_preset_count();
	String get_preset_name(int preset_index);
	String bank_get_preset_name(int bank, int preset_number);
	void set_output(OutputMode output_mode, int sample_rate, float global_gain_db = 0);
	void set_volume(float global_gain);
	int set_max_voices(int max_voices);
	int note_on(int preset_index, int key, float vel);
	int bank_note_on(int bank, int preset_number, int key, float vel);
	void note_off(int preset_index, int key);
	int bank_note_off(int bank, int preset_number, int key);
	void note_off_all();
	int active_voice_count();
	PackedByteArray render_short(int samples);
	PackedFloat32Array render_float(int samples);
	int channel_set_preset_index(int channel, int preset_index);
	int channel_set_preset_number(int channel, int preset_number, int flag_midi_drums = 0);
	int channel_set_bank(int channel, int bank);
	int channel_set_bank_preset(int channel, int bank, int preset_number);
	int channel_set_pan(int channel, float pan);
	int channel_set_volume(int channel, float volume);
	int channel_set_pitch_wheel(int channel, int pitch_wheel);
	int channel_set_pitch_range(int channel, float pitch_range);
	int channel_set_tuning(int channel, float tuning);
	int channel_note_on(int channel, int key, float vel);
	void channel_note_off(int channel, int key);
	void channel_note_off_all(int channel);
	void channel_sounds_off_all(int channel);
	int channel_midi_control(int channel, int controller, int control_value);
	int channel_get_preset_index(int channel);
	int channel_get_preset_bank(int channel);
	int channel_get_preset_number(int channel);
	float channel_get_pan(int channel);
	float channel_get_volume(int channel);
	int channel_get_pitch_wheel(int channel);
	float channel_get_pitch_range(int channel);
	float channel_get_tuning(int channel);
	~SoundFont();

	void (*render_float_raw)(tsf *f, float *buffer, int samples, int flag_mixing) = tsf_render_float;
};
VARIANT_ENUM_CAST(SoundFont::OutputMode);

class ResourceFormatLoaderSoundFont : public ResourceFormatLoader {
public:
	virtual Ref<Resource> load(const String &p_path, const String &p_original_path = "", Error *r_error = nullptr, bool p_use_sub_threads = false, float *r_progress = nullptr, CacheMode p_cache_mode = CACHE_MODE_REUSE) override;
	virtual void get_recognized_extensions(List<String> *p_extensions) const override;
	virtual bool handles_type(const String &p_type) const override;
	virtual String get_resource_type(const String &p_path) const override;
};

#endif // SOUND_FONT_H
