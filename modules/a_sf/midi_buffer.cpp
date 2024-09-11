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

Ref<MidiBuffer> MidiBuffer::new_with_args(Ref<SoundFont> sf, Ref<Midi> midi) {
	Ref<MidiBuffer> buf = memnew(MidiBuffer);
	if (midi.is_valid()) {
		buf->set_midi(midi);
	}
	if (sf.is_valid()) {
		buf->set_sf(sf);
	}
	return buf;
}

void MidiBuffer::set_midi(Ref<Midi> _midi) {
	this->midi = _midi;
	_tml = midi->_get_tml_raw();
	ring_buffer.clear();
}
void MidiBuffer::set_sf(Ref<SoundFont> _sf) {
	this->sf = _sf;
}
int MidiBuffer::push_buffer(int length) {
	ERR_FAIL_COND_V(midi.is_null() || sf.is_null(), 0);
	PackedFloat32Array buffer = PackedFloat32Array();
	while (buffer.size() < length) {
		_tml = midi->render_current_raw(_tml, sf, &buffer);
		if (_tml == nullptr) {
			break;
		}
		_tml = _tml->next;
	}
	if (ring_buffer.space_left() < buffer.size()) {
		spin_lock.lock();
		ring_buffer.resize(msb(ring_buffer.data_left() + buffer.size()));
		spin_lock.unlock();
	}
	ring_buffer.write(buffer.ptr(), buffer.size());
	return buffer.size();
}

PackedFloat32Array MidiBuffer::get_buffer(int length) {
	PackedFloat32Array res;
	res.resize(length);
	spin_lock.lock();
	size_t num = ring_buffer.read(res.ptrw(), length);
	spin_lock.unlock();
	res.resize(num);
	return res;
}

int MidiBuffer::fill_audio_buffer(Ref<AudioStreamGeneratorPlayback> playback, int length) {
	ERR_FAIL_COND_V(_tml == nullptr || playback.is_null(), -1);
	if (length == -1) {
		length = playback->get_frames_available();
	}
	if ((running_id == -1 || WorkerThreadPool::get_singleton()->is_task_completed(running_id)) && ring_buffer.data_left() < rb_target_size) {
		running_id = WorkerThreadPool::get_singleton()->add_template_task(this, &MidiBuffer::push_buffer, rb_target_size);
	}
	PackedFloat32Array buffer = get_buffer(length);
	PackedVector2Array b;
	b.resize(buffer.size());
	for (int i = 0; i < buffer.size(); i++) {
		b.set(i, Vector2(buffer[i], buffer[i]));
	}
	playback->push_buffer(b);
	return buffer.size();
}

void MidiBuffer::reset_tml() {
	ERR_FAIL_COND(midi.is_null());
	_tml = midi->_get_tml_raw();
}

void MidiBuffer::stop() {
	_tml = nullptr;
	ring_buffer.clear();
}

void MidiBuffer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_buffer", "length"), &MidiBuffer::get_buffer);
	ClassDB::bind_method(D_METHOD("push_buffer", "length"), &MidiBuffer::push_buffer);
	ClassDB::bind_method(D_METHOD("fill_audio_buffer", "playback", "length"), &MidiBuffer::fill_audio_buffer, DEFVAL(-1));
	ClassDB::bind_method(D_METHOD("reset_tml"), &MidiBuffer::reset_tml);
	ClassDB::bind_method(D_METHOD("set_midi", "midi"), &MidiBuffer::set_midi);
	ClassDB::bind_method(D_METHOD("set_sf", "sf"), &MidiBuffer::set_sf);
	ClassDB::bind_method(D_METHOD("stop"), &MidiBuffer::stop);
	ClassDB::bind_method(D_METHOD("get_rb_init_capacity"), &MidiBuffer::get_rb_init_capacity);
	ClassDB::bind_method(D_METHOD("set_rb_init_capacity", "v"), &MidiBuffer::set_rb_init_capacity);
	ClassDB::bind_method(D_METHOD("get_rb_target_size"), &MidiBuffer::get_rb_target_size);
	ClassDB::bind_method(D_METHOD("set_rb_target_size", "v"), &MidiBuffer::set_rb_target_size);

	ClassDB::bind_static_method("MidiBuffer", D_METHOD("new_with_args", "sf", "midi"), &MidiBuffer::new_with_args, DEFVAL(Ref<Midi>()), DEFVAL(Ref<SoundFont>()));
	ADD_PROPERTY(PropertyInfo(Variant::INT, "rb_init_capacity"), "set_rb_init_capacity", "get_rb_init_capacity");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "rb_target_size"), "set_rb_target_size", "get_rb_target_size");
}
