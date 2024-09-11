/**************************************************************************/
/*  midi.cpp                                                              */
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

#include "midi.h"
#include "core/os/memory.h"
#include "io_utils.h"

static inline tml_message *load_from_memory_tml(PackedByteArray buffer) {
	return tml_load_memory(buffer.ptr(), buffer.size());
}

static inline tml_message *load_from_file_tml(Ref<FileAccess> file) {
	ERR_FAIL_COND_V(file.is_null(), nullptr);
	tml_stream stream = { file.ptr(), read_file_access };
	return tml_load(&stream);
}

static inline tml_message *load_from_path_tml(const String &p_path) {
	Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::ModeFlags::READ);
	ERR_FAIL_COND_V_MSG(file.is_null(), nullptr, vformat("Unable to open file: %s", p_path));
	return load_from_file_tml(file);
}

Ref<Midi> Midi::new_from_tml_root(tml_message *tml) {
	ERR_FAIL_COND_V(tml == nullptr, Ref<Midi>());
	Ref<Midi> mid = memnew(Midi);
	mid->_tml = tml;
	mid->tml_head = tml;
	return mid;
}
Ref<Midi> Midi::create_from_path(const String &p_path) {
	return new_from_tml_root(load_from_path_tml(p_path));
}

Ref<Midi> Midi::create_from_memory(const PackedByteArray &p_buffer) {
	return new_from_tml_root(load_from_memory_tml(p_buffer));
}

Ref<Midi> Midi::create_from_dicts(const Array &p_dicts) {
	if (p_dicts.size() == 0) {
		return nullptr;
	}
	tml_message *tml = (tml_message *)memalloc(p_dicts.size() * sizeof(tml_message));
	for (int i = 0; i < p_dicts.size(); i++) {
		tml_message *curr = &tml[i];
		Dictionary dict = p_dicts[i];
		if (dict.has(Keys::K_CHANNEL)) {
			curr->channel = dict[Keys::K_CHANNEL];
		}
		if (dict.has(Keys::K_TIME)) {
			curr->time = dict[Keys::K_TIME];
		}
		if (dict.has(Keys::K_TYPE)) {
			curr->type = dict[Keys::K_TYPE];
		}
		if (dict.has(Keys::K_CHANNEL_PRESSURE)) {
			curr->channel_pressure = (int)dict[Keys::K_CHANNEL_PRESSURE];
		}
		if (dict.has(Keys::K_KEY_PRESSURE)) {
			curr->key_pressure = (int)dict[Keys::K_KEY_PRESSURE];
		}
		if (dict.has(Keys::K_CONTROL)) {
			curr->control = (int)dict[Keys::K_CONTROL];
		}
		if (dict.has(Keys::K_CONTROL_VALUE)) {
			curr->control_value = (int)dict[Keys::K_CONTROL_VALUE];
		}
		if (dict.has(Keys::K_KEY)) {
			curr->key = (int)dict[Keys::K_KEY];
		}
		if (dict.has(Keys::K_PROGRAM)) {
			curr->program = (int)dict[Keys::K_PROGRAM];
		}
		if (dict.has(Keys::K_VELOCITY)) {
			curr->velocity = (int)dict[Keys::K_VELOCITY];
		} else if (curr->type == TML_NOTE_ON) {
			curr->velocity = 100;
		}
		if (dict.has(Keys::K_PITCH_BEND)) {
			curr->pitch_bend = dict[Keys::K_PITCH_BEND];
		}
		curr->next = i + 1 < p_dicts.size() ? &tml[i + 1] : nullptr;
	}
	return new_from_tml_root(tml);
}

Ref<Midi> Midi::create_simple_array(const PackedByteArray &p_arr, int p_duration_ms, int p_channel, int p_vel) {
	if (p_arr.size() == 0) {
		return nullptr;
	}
	tml_message *tml = (tml_message *)memalloc(p_arr.size() * 2 * sizeof(tml_message));
	for (int i = 0; i < p_arr.size(); i++) {
		tml_message *curr = &tml[i * 2];
		tml_message *next = &tml[i * 2 + 1];
		uint8_t key = p_arr[i];
		curr->key = key;
		curr->type = NOTE_ON;
		curr->velocity = p_vel;
		curr->channel = p_channel;
		curr->time = p_duration_ms * i;
		curr->next = next;
		next->key = key;
		next->type = NOTE_OFF;
		next->channel = p_channel;
		next->time = curr->time + p_duration_ms;
		next->next = i + 1 < p_arr.size() ? &tml[i * 2 + 2] : nullptr;
	}
	return new_from_tml_root(tml);
}

Ref<Midi> Midi::create_simple_time_array(const PackedByteArray &p_notes, const PackedInt32Array &p_times, int p_duration_ms, int p_channel, int p_vel) {
	if (p_notes.size() == 0) {
		return nullptr;
	}
	tml_message *tml = (tml_message *)memalloc(p_notes.size() * 2 * sizeof(tml_message));
	for (int i = 0; i < p_notes.size(); i++) {
		tml_message *curr = &tml[i * 2];
		tml_message *next = &tml[i * 2 + 1];
		uint8_t key = p_notes[i];
		curr->key = key;
		curr->type = NOTE_ON;
		curr->velocity = p_vel;
		curr->channel = p_channel;
		curr->time = p_times[i];
		curr->next = next;
		next->key = key;
		next->type = NOTE_OFF;
		next->channel = p_channel;
		next->time = curr->time + p_duration_ms;
		next->next = i + 1 < p_notes.size() ? &tml[i * 2 + 2] : nullptr;
	}
	return new_from_tml_root(tml);
}

Array Midi::to_dicts(int p_length) const {
	Array res;
	for (tml_message *curr = _tml; curr != nullptr; curr = curr->next) {
		if (p_length >= 0 && res.size() >= p_length) {
			break;
		}
		Dictionary dict;
		dict[Keys::K_CHANNEL] = curr->channel;
		dict[Keys::K_TIME] = curr->time;
		dict[Keys::K_TYPE] = curr->type;
		dict[Keys::K_CHANNEL_PRESSURE] = curr->channel_pressure;
		dict[Keys::K_KEY_PRESSURE] = curr->key_pressure;
		dict[Keys::K_CONTROL] = curr->control;
		dict[Keys::K_CONTROL_VALUE] = curr->control_value;
		dict[Keys::K_KEY] = curr->key;
		dict[Keys::K_PROGRAM] = curr->program;
		dict[Keys::K_VELOCITY] = curr->velocity;
		dict[Keys::K_PITCH_BEND] = curr->pitch_bend;
		res.append(dict);
	}
	return res;
}

Array Midi::to_simple_array(int p_channel) const {
	Array res;
	PackedByteArray notes;
	PackedInt32Array times;
	for (tml_message *curr = _tml; curr != nullptr; curr = curr->next) {
		if (curr->type == NOTE_ON && (p_channel < 0 || curr->channel == p_channel)) {
			notes.append(curr->key);
			times.append(curr->time);
		}
	}
	res.append(notes);
	res.append(times);
	return res;
}
// Get infos about this loaded MIDI file, returns the note count
// NULL can be passed for any output value pointer if not needed.
//   used_channels:   Will be set to how many channels play notes
//                    (i.e. 1 if channel 15 is used but no other)
//   used_programs:   Will be set to how many different programs are used
//   total_notes:     Will be set to the total number of note on messages
//   time_first_note: Will be set to the time of the first note on message
//   time_length:     Will be set to the total time in milliseconds
Dictionary Midi::get_info() const {
	ERR_FAIL_COND_V(_tml == nullptr, Dictionary());
	int used_channels = -1;
	int used_programs = -1;
	int total_notes = -1;
	unsigned int time_first_note = -1;
	unsigned int time_length = -1;
	int note_count = tml_get_info(_tml, &used_channels, &used_programs, &total_notes, &time_first_note, &time_length);
	Dictionary res = Dictionary();
	res[InfoKeys::K_USED_CHANNELS] = used_channels;
	res[InfoKeys::K_USED_PROGRAMS] = used_programs;
	res[InfoKeys::K_TOTAL_NOTES] = total_notes;
	res[InfoKeys::K_TIME_FIRST_NOTE] = time_first_note;
	res[InfoKeys::K_TIME_LENGTH] = time_length;
	res[InfoKeys::K_NOTE_COUNT] = note_count;
	return res;
}

// Read the tempo (microseconds per quarter note) value from a message with the type TML_SET_TEMPO
int Midi::get_tempo_value() const {
	ERR_FAIL_COND_V(_tml == nullptr, 0);
	return tml_get_tempo_value(_tml);
}

Dictionary Midi::read_msg() const {
	ERR_FAIL_COND_V(_tml == nullptr, Dictionary());
	int time = _tml->time;
	int type = _tml->type;
	int channel = _tml->channel;
	char channel_pressure = _tml->channel_pressure;
	char key_pressure = _tml->key_pressure;
	char control = _tml->control;
	char control_value = _tml->control_value;
	char key = _tml->key;
	char program = _tml->program;
	char velocity = _tml->velocity;
	unsigned short pitch_bend = _tml->pitch_bend;
	Dictionary res = Dictionary();
	res[time] = time;
	res[type] = type;
	res[Keys::K_CHANNEL] = channel;
	res[channel_pressure] = channel_pressure;
	res[Keys::K_KEY_PRESSURE] = key_pressure;
	res[Keys::K_CONTROL] = control;
	res[Keys::K_CONTROL_VALUE] = control_value;
	res[Keys::K_KEY] = key;
	res[Keys::K_PROGRAM] = program;
	res[Keys::K_VELOCITY] = velocity;
	res[Keys::K_PITCH_BEND] = pitch_bend;
	return res;
}

Ref<Midi> Midi::next() const {
	ERR_FAIL_COND_V(_tml == nullptr, nullptr);
	Ref<Midi> n = memnew(Midi);
	n->_tml = _tml->next;
	return n;
}

float Midi::get_length_sec() const {
	float time = 0;
	for (tml_message *tml = _tml; tml != nullptr; tml = tml->next) {
		if (tml->time >= time) {
			time = tml->time;
		}
	}
	return time;
}

Midi::~Midi() {
	if (is_tml_head()) {
		tml_free(tml_head);
	}
}

void Midi::_bind_methods() {
	BIND_ENUM_CONSTANT(NOTE_OFF);
	BIND_ENUM_CONSTANT(NOTE_ON);
	BIND_ENUM_CONSTANT(KEY_PRESSURE);
	BIND_ENUM_CONSTANT(CONTROL_CHANGE);
	BIND_ENUM_CONSTANT(PROGRAM_CHANGE);
	BIND_ENUM_CONSTANT(CHANNEL_PRESSURE);
	BIND_ENUM_CONSTANT(PITCH_BEND);
	BIND_ENUM_CONSTANT(SET_TEMPO);

	BIND_ENUM_CONSTANT(BANK_SELECT_MSB);
	BIND_ENUM_CONSTANT(MODULATIONWHEEL_MSB);
	BIND_ENUM_CONSTANT(BREATH_MSB);
	BIND_ENUM_CONSTANT(FOOT_MSB);
	BIND_ENUM_CONSTANT(PORTAMENTO_TIME_MSB);
	BIND_ENUM_CONSTANT(DATA_ENTRY_MSB);
	BIND_ENUM_CONSTANT(VOLUME_MSB);
	BIND_ENUM_CONSTANT(BALANCE_MSB);
	BIND_ENUM_CONSTANT(PAN_MSB);
	BIND_ENUM_CONSTANT(EXPRESSION_MSB);
	BIND_ENUM_CONSTANT(EFFECTS1_MSB);
	BIND_ENUM_CONSTANT(EFFECTS2_MSB);
	BIND_ENUM_CONSTANT(GPC1_MSB);
	BIND_ENUM_CONSTANT(GPC2_MSB);
	BIND_ENUM_CONSTANT(GPC3_MSB);
	BIND_ENUM_CONSTANT(GPC4_MSB);
	BIND_ENUM_CONSTANT(BANK_SELECT_LSB);
	BIND_ENUM_CONSTANT(MODULATIONWHEEL_LSB);
	BIND_ENUM_CONSTANT(BREATH_LSB);
	BIND_ENUM_CONSTANT(FOOT_LSB);
	BIND_ENUM_CONSTANT(PORTAMENTO_TIME_LSB);
	BIND_ENUM_CONSTANT(DATA_ENTRY_LSB);
	BIND_ENUM_CONSTANT(VOLUME_LSB);
	BIND_ENUM_CONSTANT(BALANCE_LSB);
	BIND_ENUM_CONSTANT(PAN_LSB);
	BIND_ENUM_CONSTANT(EXPRESSION_LSB);
	BIND_ENUM_CONSTANT(EFFECTS1_LSB);
	BIND_ENUM_CONSTANT(EFFECTS2_LSB);
	BIND_ENUM_CONSTANT(GPC1_LSB);
	BIND_ENUM_CONSTANT(GPC2_LSB);
	BIND_ENUM_CONSTANT(GPC3_LSB);
	BIND_ENUM_CONSTANT(GPC4_LSB);
	BIND_ENUM_CONSTANT(SUSTAIN_SWITCH);
	BIND_ENUM_CONSTANT(PORTAMENTO_SWITCH);
	BIND_ENUM_CONSTANT(SOSTENUTO_SWITCH);
	BIND_ENUM_CONSTANT(SOFT_PEDAL_SWITCH);
	BIND_ENUM_CONSTANT(LEGATO_SWITCH);
	BIND_ENUM_CONSTANT(HOLD2_SWITCH);
	BIND_ENUM_CONSTANT(SOUND_CTRL1);
	BIND_ENUM_CONSTANT(SOUND_CTRL2);
	BIND_ENUM_CONSTANT(SOUND_CTRL3);
	BIND_ENUM_CONSTANT(SOUND_CTRL4);
	BIND_ENUM_CONSTANT(SOUND_CTRL5);
	BIND_ENUM_CONSTANT(SOUND_CTRL6);
	BIND_ENUM_CONSTANT(SOUND_CTRL7);
	BIND_ENUM_CONSTANT(SOUND_CTRL8);
	BIND_ENUM_CONSTANT(SOUND_CTRL9);
	BIND_ENUM_CONSTANT(SOUND_CTRL10);
	BIND_ENUM_CONSTANT(GPC5);
	BIND_ENUM_CONSTANT(GPC6);
	BIND_ENUM_CONSTANT(GPC7);
	BIND_ENUM_CONSTANT(GPC8);
	BIND_ENUM_CONSTANT(PORTAMENTO_CTRL);
	BIND_ENUM_CONSTANT(FX_REVERB);
	BIND_ENUM_CONSTANT(FX_TREMOLO);
	BIND_ENUM_CONSTANT(FX_CHORUS);
	BIND_ENUM_CONSTANT(FX_CELESTE_DETUNE);
	BIND_ENUM_CONSTANT(FX_PHASER);
	BIND_ENUM_CONSTANT(DATA_ENTRY_INCR);
	BIND_ENUM_CONSTANT(DATA_ENTRY_DECR);
	BIND_ENUM_CONSTANT(NRPN_LSB);
	BIND_ENUM_CONSTANT(NRPN_MSB);
	BIND_ENUM_CONSTANT(RPN_LSB);
	BIND_ENUM_CONSTANT(RPN_MSB);
	BIND_ENUM_CONSTANT(ALL_SOUND_OFF);
	BIND_ENUM_CONSTANT(ALL_CTRL_OFF);
	BIND_ENUM_CONSTANT(LOCAL_CONTROL);
	BIND_ENUM_CONSTANT(ALL_NOTES_OFF);
	BIND_ENUM_CONSTANT(OMNI_OFF);
	BIND_ENUM_CONSTANT(OMNI_ON);
	BIND_ENUM_CONSTANT(POLY_OFF);
	BIND_ENUM_CONSTANT(POLY_ON);

	BIND_ENUM_CONSTANT(K_CHANNEL);
	BIND_ENUM_CONSTANT(K_TIME);
	BIND_ENUM_CONSTANT(K_TYPE);
	BIND_ENUM_CONSTANT(K_CHANNEL_PRESSURE);
	BIND_ENUM_CONSTANT(K_KEY_PRESSURE);
	BIND_ENUM_CONSTANT(K_CONTROL);
	BIND_ENUM_CONSTANT(K_CONTROL_VALUE);
	BIND_ENUM_CONSTANT(K_KEY);
	BIND_ENUM_CONSTANT(K_PROGRAM);
	BIND_ENUM_CONSTANT(K_VELOCITY);
	BIND_ENUM_CONSTANT(K_PITCH_BEND);

	BIND_ENUM_CONSTANT(K_USED_CHANNELS);
	BIND_ENUM_CONSTANT(K_USED_PROGRAMS);
	BIND_ENUM_CONSTANT(K_TOTAL_NOTES);
	BIND_ENUM_CONSTANT(K_TIME_FIRST_NOTE);
	BIND_ENUM_CONSTANT(K_TIME_LENGTH);
	BIND_ENUM_CONSTANT(K_NOTE_COUNT);

	ClassDB::bind_static_method("Midi", D_METHOD("create_from_path", "path"), &Midi::create_from_path);
	ClassDB::bind_static_method("Midi", D_METHOD("create_from_memory", "buffer"), &Midi::create_from_memory);
	ClassDB::bind_static_method("Midi", D_METHOD("create_from_dicts", "dicts"), &Midi::create_from_dicts);
	ClassDB::bind_static_method("Midi", D_METHOD("create_simple_array", "arr", "duration_ms", "channel", "vel"), &Midi::create_simple_array, DEFVAL(600), DEFVAL(0), DEFVAL(100));
	ClassDB::bind_static_method("Midi", D_METHOD("create_simple_time_array", "notes", "times", "duration_ms", "channel", "vel"), &Midi::create_simple_time_array, DEFVAL(600), DEFVAL(0), DEFVAL(100));
	ClassDB::bind_method(D_METHOD("is_tml_header"), &Midi::is_tml_head);
	ClassDB::bind_method(D_METHOD("is_tml_valid"), &Midi::is_tml_valid);
	ClassDB::bind_method(D_METHOD("to_dicts", "p_length"), &Midi::to_dicts, DEFVAL(-1));
	ClassDB::bind_method(D_METHOD("to_simple_array", "p_selected_channel"), &Midi::to_simple_array, DEFVAL(-1));
	ClassDB::bind_method(D_METHOD("get_info"), &Midi::get_info);
	ClassDB::bind_method(D_METHOD("get_tempo_value"), &Midi::get_tempo_value);
	ClassDB::bind_method(D_METHOD("read_msg"), &Midi::read_msg);
	ClassDB::bind_method(D_METHOD("next"), &Midi::next);
	ClassDB::bind_method(D_METHOD("get_length_sec"), &Midi::get_length_sec);
}

Ref<Resource> ResourceFormatLoaderMidi::load(const String &p_path, const String &p_original_path, Error *r_error, bool p_use_sub_threads, float *r_progress, CacheMode p_cache_mode) {
	if (r_error) {
		*r_error = ERR_FILE_CANT_OPEN;
	}

	if (!FileAccess::exists(p_path)) {
		*r_error = ERR_FILE_NOT_FOUND;
		return Ref<Resource>();
	}

	Ref<Midi> mid = Midi::create_from_path(p_path);
	if (r_error) {
		*r_error = OK;
	}
	return mid;
}

void ResourceFormatLoaderMidi::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("mid");
}

bool ResourceFormatLoaderMidi::handles_type(const String &p_type) const {
	return (p_type == "Midi");
}

String ResourceFormatLoaderMidi::get_resource_type(const String &p_path) const {
	String el = p_path.get_extension().to_lower();
	if (el == "mid") {
		return "Midi";
	}
	return "";
}
