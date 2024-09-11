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

#include "midi.h"
#include "sound_font.h"
#include <core/object/ref_counted.h>
#include <core/object/worker_thread_pool.h>
#include <core/templates/ring_buffer.h>
#include <servers/audio/effects/audio_stream_generator.h>

#if defined(__GNUC__) || defined(__clang__)
#define CLZ32(x) ((x) == 0 ? 32 : __builtin_clz(x))
#elif defined(_MSC_VER)
#include <intrin.h>
#define CLZ32(x) ((x) == 0 ? 32 : __lzcnt(x))
#elif defined(__ARMCC_VERSION) || defined(__ARM_ARCH)
#define CLZ32(x) ((x) == 0 ? 32 : __clz(x))
#else
static inline int clz32_generic(unsigned int x) {
	if (x == 0) {
		return 32;
	}
	int n = 0;
	if (x <= 0x0000FFFF) {
		n += 16;
		x <<= 16;
	}
	if (x <= 0x00FFFFFF) {
		n += 8;
		x <<= 8;
	}
	if (x <= 0x0FFFFFFF) {
		n += 4;
		x <<= 4;
	}
	if (x <= 0x3FFFFFFF) {
		n += 2;
		x <<= 2;
	}
	if (x <= 0x7FFFFFFF) {
		n += 1;
	}
	return n;
}
#define CLZ32(x) clz32_generic(x)
#endif

inline uint32_t msb(uint32_t x) {
	x |= 1;
	return sizeof(uint32_t) * CHAR_BIT - CLZ32(x);
}

class AudioStreamPlayer;

class MidiBuffer : public RefCounted {
	GDCLASS(MidiBuffer, RefCounted);

	Ref<Midi> midi = nullptr;
	Ref<SoundFont> sf = nullptr;
	int capacity = 44100;
	RingBuffer<float> ring_buffer = RingBuffer<float>(msb(capacity));
	SpinLock spin_lock;
	float cursor = 0;
	tml_message *tml = nullptr;
	int block_size = 256;

protected:
	static void _bind_methods();

public:
	void set_midi(Ref<Midi> p_midi) {
		ERR_FAIL_COND(p_midi.is_null() || !p_midi.is_valid());
		midi = p_midi;
		set_cursor(cursor);
	}
	Ref<Midi> get_midi() { return midi; }
	void set_sf(Ref<SoundFont> p_sf) { sf = p_sf; }
	Ref<SoundFont> get_sf() { return sf; }
	int render(int p_length = -1);
	PackedFloat32Array get_buffer(int p_length);
	WorkerThreadPool::TaskID render_async(int p_length = -1);
	int fill_audio_buffer(AudioStreamPlayer *p_player, int p_length = -1);
	void clear() { ring_buffer.clear(); }
	void reset();
	int get_capacity() {
		return capacity;
	}
	void set_capacity(int p_value) {
		capacity = p_value;
		ring_buffer.resize(msb(capacity));
	}
	int get_block_size() { return block_size; }
	void set_block_size(int p_value) { block_size = p_value; }
	int get_space_left() { return ring_buffer.space_left(); }
	int get_data_left() { return ring_buffer.data_left(); }

	float get_cursor() { return cursor; }
	void set_cursor(float p_cursor) {
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

	PackedFloat32Array render_all();
};
