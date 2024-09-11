/**************************************************************************/
/*  midi_buffer.cpp                                                       */
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

#include "midi_buffer.h"
#include "scene/audio/audio_stream_player.h"
#include "servers/audio/effects/audio_stream_generator.h"

int MidiBuffer::render(int p_length) {
	if (p_length == 0 || tml == nullptr) {
		return 0;
	}
	ERR_FAIL_COND_V(midi.is_null() || sf.is_null() || !midi->is_tml_valid(), 0);
	mutex.lock();
	if (p_length < 0 || p_length > ring_buffer.space_left()) {
		p_length = ring_buffer.space_left();
	}
	if (p_length <= 0) {
		mutex.unlock();
		return 0;
	}
	float *buffer = memnew_arr(float, p_length);
	int pos = 0;
	float p_length_ms = (float)p_length * 1000 / sf->get_sample_rate();
	float start_time = cursor;
	float block_size_time = (float)block_size * 1000 / sf->get_sample_rate();

	for (; cursor < start_time + p_length_ms; cursor += block_size_time) {
		for (; tml != nullptr; tml = tml->next) {
			if (tml->time > cursor) {
				break;
			}
			switch (tml->type) {
				case Midi::MessageType::PROGRAM_CHANGE: //channel program (preset) change (special handling for 10th MIDI channel with drums)
					sf->channel_set_preset_number(tml->channel, tml->program, (tml->channel == 9));
					break;
				case Midi::MessageType::NOTE_ON: //play a note
					sf->channel_note_on(tml->channel, tml->key, tml->velocity / 127.0f);
					break;
				case Midi::MessageType::NOTE_OFF: //stop a note
					sf->channel_note_off(tml->channel, tml->key);
					break;
				case Midi::MessageType::PITCH_BEND: //pitch wheel modification
					sf->channel_set_pitch_wheel(tml->channel, tml->pitch_bend);
					break;
				case Midi::MessageType::CONTROL_CHANGE: //MIDI controller messages
					sf->channel_midi_control(tml->channel, tml->control, tml->control_value);
					break;
			}
		}
		int count = block_size;
		if (pos + count >= p_length) {
			count = p_length - pos;
		}
		if (count <= 0) {
			break;
		}
		tsf_render_float(sf->_tsf, &buffer[pos], count, 0);
		pos += count;
	}
	int res = ring_buffer.write(buffer, p_length);
	mutex.unlock();
	memdelete_arr(buffer);
	return res;
}

PackedFloat32Array MidiBuffer::get_buffer(int p_length) {
	PackedFloat32Array res;
	mutex.lock();
	p_length = MIN(p_length, ring_buffer.data_left());
	res.resize(p_length);
	size_t num = ring_buffer.read(res.ptrw(), p_length);
	mutex.unlock();
	res.resize(num);
	return res;
}

void MidiBuffer::render_threading(int p_length) {
	task = WorkerThreadPool::get_singleton()->add_task(callable_mp(this, &MidiBuffer::render).bind(p_length));
}

bool MidiBuffer::is_render_threading() const {
	return WorkerThreadPool::get_singleton()->is_task_completed(task);
}

void MidiBuffer::wait_render_threading() {
	WorkerThreadPool::get_singleton()->wait_for_task_completion(task);
}

int MidiBuffer::fill_audio_buffer(AudioStreamPlayer *p_player, int p_length) {
	ERR_FAIL_COND_V(midi.is_null() || sf.is_null() || !midi->is_tml_valid() || p_player == nullptr, -1);
	Ref<AudioStreamGeneratorPlayback> playback = p_player->get_stream_playback();
	ERR_FAIL_COND_V(playback.is_null(), -1);
	if (p_length == -1) {
		p_length = playback->get_frames_available();
	}
	PackedFloat32Array buffer = get_buffer(p_length);
	PackedVector2Array b;
	b.resize(buffer.size());
	for (int i = 0; i < buffer.size(); i++) {
		b.set(i, Vector2(buffer[i], buffer[i]));
	}
	playback->push_buffer(b);
	return buffer.size();
}

void MidiBuffer::reset() {
	ERR_FAIL_COND(midi.is_null() || sf.is_null() || !midi->is_tml_valid());
	if (midi->is_tml_head()) {
		tml = midi->tml_head;
	}
	set_cursor(0);
	sf->reset();
	mutex.lock();
	ring_buffer.clear();
	mutex.unlock();
}

PackedFloat32Array MidiBuffer::render_all() const {
	LocalVector<float> res;
	ERR_FAIL_COND_V(tml == nullptr || sf.is_null(), res);
	for (tml_message *t = tml; t != nullptr; t = t->next) {
		switch (t->type) {
			case Midi::MessageType::PROGRAM_CHANGE: //channel program (preset) change (special handling for 10th MIDI channel with drums)
				sf->channel_set_preset_number(t->channel, t->program, (t->channel == 9));
				break;
			case Midi::MessageType::NOTE_ON: //play a note
				sf->channel_note_on(t->channel, t->key, t->velocity / 127.0f);
				break;
			case Midi::MessageType::NOTE_OFF: //stop a note
				sf->channel_note_off(t->channel, t->key);
				break;
			case Midi::MessageType::PITCH_BEND: //pitch wheel modification
				sf->channel_set_pitch_wheel(t->channel, t->pitch_bend);
				break;
			case Midi::MessageType::CONTROL_CHANGE: //MIDI controller messages
				sf->channel_midi_control(t->channel, t->control, t->control_value);
				break;
		}
		if (t->next == nullptr) {
			break;
		}
		int _block_size = (t->next->time - t->time) * sf->get_sample_rate() / 1000;
		if (_block_size <= 0) {
			continue;
		}
		int prev_size = res.size();
		res.resize(res.size() + _block_size);
		tsf_render_float(sf->_tsf, res.ptr() + prev_size, _block_size, 0);
	}
	return res;
}

void MidiBuffer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_midi", "p_midi"), &MidiBuffer::set_midi);
	ClassDB::bind_method(D_METHOD("get_midi"), &MidiBuffer::get_midi);
	ClassDB::bind_method(D_METHOD("set_sf", "p_sf"), &MidiBuffer::set_sf);
	ClassDB::bind_method(D_METHOD("get_sf"), &MidiBuffer::get_sf);
	ClassDB::bind_method(D_METHOD("render", "p_length"), &MidiBuffer::render, DEFVAL(-1));
	ClassDB::bind_method(D_METHOD("get_buffer", "p_length"), &MidiBuffer::get_buffer);
	ClassDB::bind_method(D_METHOD("render_threading", "p_length"), &MidiBuffer::render_threading, DEFVAL(-1));
	ClassDB::bind_method(D_METHOD("is_render_threading"), &MidiBuffer::is_render_threading);
	ClassDB::bind_method(D_METHOD("wait_render_threading"), &MidiBuffer::wait_render_threading);
	ClassDB::bind_method(D_METHOD("fill_audio_buffer", "p_player", "p_length"), &MidiBuffer::fill_audio_buffer, DEFVAL(-1));
	ClassDB::bind_method(D_METHOD("clear"), &MidiBuffer::clear);
	ClassDB::bind_method(D_METHOD("reset"), &MidiBuffer::reset);
	ClassDB::bind_method(D_METHOD("get_capacity"), &MidiBuffer::get_capacity);
	ClassDB::bind_method(D_METHOD("set_capacity", "p_value"), &MidiBuffer::set_capacity);
	ClassDB::bind_method(D_METHOD("get_block_size"), &MidiBuffer::get_block_size);
	ClassDB::bind_method(D_METHOD("set_block_size", "p_value"), &MidiBuffer::set_block_size);
	ClassDB::bind_method(D_METHOD("get_space_left"), &MidiBuffer::get_space_left);
	ClassDB::bind_method(D_METHOD("get_data_left"), &MidiBuffer::get_data_left);
	ClassDB::bind_method(D_METHOD("get_cursor"), &MidiBuffer::get_cursor);
	ClassDB::bind_method(D_METHOD("set_cursor", "p_cursor"), &MidiBuffer::set_cursor);
	ClassDB::bind_method(D_METHOD("render_all"), &MidiBuffer::render_all);

	ClassDB::bind_static_method("MidiBuffer", D_METHOD("create", "sf", "midi"), &MidiBuffer::create);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "capacity"), "set_capacity", "get_capacity");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "cursor"), "set_cursor", "get_cursor");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "block_size"), "set_block_size", "get_block_size");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "midi", PROPERTY_HINT_RESOURCE_TYPE, "Midi"), "set_midi", "get_midi");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "sf", PROPERTY_HINT_RESOURCE_TYPE, "SoundFont"), "set_sf", "get_sf");
}

Ref<MidiBuffer> MidiBuffer::create(const Ref<SoundFont> &p_sf, const Ref<Midi> &p_midi) {
	Ref<MidiBuffer> ret;
	ret.instantiate();
	ret->sf = p_sf;
	ret->midi = p_midi;
	return ret;
}

void MidiBuffer::set_cursor(float p_cursor) {
	if (midi.is_null()) {
		return;
	}
	for (tml = midi->_tml; tml != nullptr; tml = tml->next) {
		if (tml->time >= p_cursor) {
			cursor = tml->time;
			break;
		}
	}
}
