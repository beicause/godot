/**************************************************************************/
/*  midi.h                                                                */
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

#ifndef MIDI_H
#define MIDI_H

#include "sf_utils.h"
#include "sound_font.h"

using namespace godot;

class Midi : public Resource {
	GDCLASS(Midi, Resource);

	tml_message *_tml = nullptr;

protected:
	static void _bind_methods();

public:
	// Channel message type
	enum MessageType {
		NOTE_OFF = 0x80,
		NOTE_ON = 0x90,
		KEY_PRESSURE = 0xA0,
		CONTROL_CHANGE = 0xB0,
		PROGRAM_CHANGE = 0xC0,
		CHANNEL_PRESSURE = 0xD0,
		PITCH_BEND = 0xE0,
		SET_TEMPO = 0x51
	};

	// Midi controller numbers
	enum Controller {
		BANK_SELECT_MSB,
		MODULATIONWHEEL_MSB,
		BREATH_MSB,
		FOOT_MSB = 4,
		PORTAMENTO_TIME_MSB,
		DATA_ENTRY_MSB,
		VOLUME_MSB,
		BALANCE_MSB,
		PAN_MSB = 10,
		EXPRESSION_MSB,
		EFFECTS1_MSB,
		EFFECTS2_MSB,
		GPC1_MSB = 16,
		GPC2_MSB,
		GPC3_MSB,
		GPC4_MSB,
		BANK_SELECT_LSB = 32,
		MODULATIONWHEEL_LSB,
		BREATH_LSB,
		FOOT_LSB = 36,
		PORTAMENTO_TIME_LSB,
		DATA_ENTRY_LSB,
		VOLUME_LSB,
		BALANCE_LSB,
		PAN_LSB = 42,
		EXPRESSION_LSB,
		EFFECTS1_LSB,
		EFFECTS2_LSB,
		GPC1_LSB = 48,
		GPC2_LSB,
		GPC3_LSB,
		GPC4_LSB,
		SUSTAIN_SWITCH = 64,
		PORTAMENTO_SWITCH,
		SOSTENUTO_SWITCH,
		SOFT_PEDAL_SWITCH,
		LEGATO_SWITCH,
		HOLD2_SWITCH,
		SOUND_CTRL1,
		SOUND_CTRL2,
		SOUND_CTRL3,
		SOUND_CTRL4,
		SOUND_CTRL5,
		SOUND_CTRL6,
		SOUND_CTRL7,
		SOUND_CTRL8,
		SOUND_CTRL9,
		SOUND_CTRL10,
		GPC5,
		GPC6,
		GPC7,
		GPC8,
		PORTAMENTO_CTRL,
		FX_REVERB = 91,
		FX_TREMOLO,
		FX_CHORUS,
		FX_CELESTE_DETUNE,
		FX_PHASER,
		DATA_ENTRY_INCR,
		DATA_ENTRY_DECR,
		NRPN_LSB,
		NRPN_MSB,
		RPN_LSB,
		RPN_MSB,
		ALL_SOUND_OFF = 120,
		ALL_CTRL_OFF,
		LOCAL_CONTROL,
		ALL_NOTES_OFF,
		OMNI_OFF,
		OMNI_ON,
		POLY_OFF,
		POLY_ON
	};

	enum Keys {
		K_CHANNEL,
		K_TIME,
		K_TYPE,
		K_CHANNEL_PRESSURE,
		K_KEY_PRESSURE,
		K_CONTROL,
		K_CONTROL_VALUE,
		K_KEY,
		K_PROGRAM,
		K_VELOCITY,
		K_PITCH_BEND,
	};

	enum InfoKeys {
		K_USED_CHANNELS,
		K_USED_PROGRAMS,
		K_TOTAL_NOTES,
		K_TIME_FIRST_NOTE,
		K_TIME_LENGTH,
		K_NOTE_COUNT,
	};

	bool tml_header = false;

	/*gd_ignore*/
	void _set_tml_raw(tml_message *l) { _tml = l; }
	/*gd_ignore*/
	tml_message *_get_tml_raw() { return _tml; }

	bool is_tml_header() { return tml_header; };

	static Ref<Midi> create_from_path(const String &p_path);
	static Ref<Midi> create_from_file(Ref<FileAccess> file);
	static Ref<Midi> create_from_memory(const PackedByteArray &buffer);
	static Ref<Midi> create_from_dicts(const Array &dicts);
	static Ref<Midi> create_simple_array(const PackedByteArray &arr, int duration_ms = 600, int channel = 0, int vel = 100);
	static Ref<Midi> create_simple_time_array(const PackedByteArray &notes, const PackedInt32Array &times, int duration_ms = 600, int channel = 0, int vel = 100);

	Array to_dicts(int len = -1);
	Array to_simple_array(int selected_channel = -1);
	Dictionary get_info();
	int get_tempo_value();
	Dictionary read_msg();
	Ref<Midi> next();

	PackedFloat32Array render_all(Ref<SoundFont> sf);
	Array render_current(Ref<SoundFont> sf);
	/*gd_ignore*/
	static tml_message *render_current_raw(tml_message *t, Ref<SoundFont> sf, PackedFloat32Array *buffer);

	~Midi();
};
VARIANT_ENUM_CAST(Midi::MessageType);
VARIANT_ENUM_CAST(Midi::Controller);
VARIANT_ENUM_CAST(Midi::Keys);
VARIANT_ENUM_CAST(Midi::InfoKeys);

class ResourceFormatLoaderMidi : public ResourceFormatLoader {
public:
	virtual Ref<Resource> load(const String &p_path, const String &p_original_path = "", Error *r_error = nullptr, bool p_use_sub_threads = false, float *r_progress = nullptr, CacheMode p_cache_mode = CACHE_MODE_REUSE) override;
	virtual void get_recognized_extensions(List<String> *p_extensions) const override;
	virtual bool handles_type(const String &p_type) const override;
	virtual String get_resource_type(const String &p_path) const override;
};

#endif // MIDI_H
