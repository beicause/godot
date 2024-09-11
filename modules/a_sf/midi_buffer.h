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

#ifndef MIDI_BUFFER_H
#define MIDI_BUFFER_H

#include "midi.h"

#include <core/object/ref_counted.h>
#include <core/object/worker_thread_pool.h>
#include <core/templates/ring_buffer.h>
#include <servers/audio/effects/audio_stream_generator.h>

using namespace godot;

#ifndef CLZ32
inline uint32_t __popcnt(uint32_t x) {
	x -= ((x >> 1) & 0x55555555);
	x = (((x >> 2) & 0x33333333) + (x & 0x33333333));
	x = (((x >> 4) + x) & 0x0f0f0f0f);
	x += (x >> 8);
	x += (x >> 16);
	return x & 0x0000003f;
}
inline uint32_t __clz(uint32_t x) {
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);
	return 32 - __popcnt(x);
}
#define CLZ32(x) __clz(x)
#endif

inline uint32_t msb(uint32_t x) {
	x |= 1;
	return sizeof(uint32_t) * CHAR_BIT - CLZ32(x);
}

class MidiBuffer : public RefCounted {
	GDCLASS(MidiBuffer, RefCounted);

	Ref<Midi> midi = nullptr;
	Ref<SoundFont> sf = nullptr;
	tml_message *_tml = nullptr;
	int rb_init_capacity = 44100 / 2;
	int rb_target_size = rb_init_capacity * 2;
	WorkerThreadPool::TaskID running_id = -1;
	RingBuffer<float> ring_buffer = RingBuffer<float>(msb(rb_init_capacity));
	SpinLock spin_lock;

protected:
	static void _bind_methods();

public:
	static Ref<MidiBuffer> new_with_args(Ref<SoundFont> sf = Ref<SoundFont>(), Ref<Midi> midi = Ref<Midi>());
	void set_midi(Ref<Midi> midi);
	void set_sf(Ref<SoundFont> sf);
	int push_buffer(int length);
	PackedFloat32Array get_buffer(int length);
	int fill_audio_buffer(Ref<AudioStreamGeneratorPlayback> playback, int length = -1);
	void reset_tml();
	void stop();
	int get_rb_init_capacity() { return rb_init_capacity; }
	void set_rb_init_capacity(int value) { rb_init_capacity = value; }
	int get_rb_target_size() { return rb_target_size; }
	void set_rb_target_size(int value) { rb_target_size = value; }
};

#endif // MIDI_BUFFER_H
