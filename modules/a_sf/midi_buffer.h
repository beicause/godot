/**************************************************************************/
/*  midi_buffer.h                                                         */
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

#pragma once

#include "core/object/ref_counted.h"
#include "core/templates/ring_buffer.h"
#include "midi.h"
#include "sound_font.h"

class AudioStreamPlayer;

class MidiBuffer : public RefCounted {
	GDCLASS(MidiBuffer, RefCounted);

	Ref<Midi> midi = nullptr;
	Ref<SoundFont> sf = nullptr;
	uint32_t capacity = 44100;
	RingBuffer<float> ring_buffer = RingBuffer<float>(nearest_shift(capacity));
	Mutex mutex;
	WorkerThreadPool::TaskID task;
	float cursor = 0;
	tml_message *tml = nullptr;
	int block_size = 256;

protected:
	static void _bind_methods();

public:
	static Ref<MidiBuffer> create(const Ref<SoundFont> &p_sf, const Ref<Midi> &p_midi);

	void set_midi(Ref<Midi> p_midi) {
		ERR_FAIL_COND(p_midi.is_null() || !p_midi.is_valid());
		midi = p_midi;
		set_cursor(cursor);
	}
	Ref<Midi> get_midi() const { return midi; }
	void set_sf(Ref<SoundFont> p_sf) { sf = p_sf; }
	Ref<SoundFont> get_sf() const { return sf; }
	int render(int p_length = -1);
	PackedFloat32Array get_buffer(int p_length);
	void render_threading(int p_length = -1);
	bool is_render_threading() const;
	void wait_render_threading();
	int fill_audio_buffer(AudioStreamPlayer *p_player, int p_length = -1);
	void clear() { ring_buffer.clear(); }
	void reset();
	uint32_t get_capacity() const {
		return capacity;
	}
	void set_capacity(uint32_t p_value) {
		capacity = p_value;
		ring_buffer.resize(nearest_shift(capacity));
	}
	int get_block_size() const { return block_size; }
	void set_block_size(int p_value) { block_size = p_value; }
	int get_space_left() const { return ring_buffer.space_left(); }
	int get_data_left() const { return ring_buffer.data_left(); }

	float get_cursor() const { return cursor; }
	void set_cursor(float p_cursor);

	PackedFloat32Array render_all() const;
};
