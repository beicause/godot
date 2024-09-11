/**************************************************************************/
/*  hexwave.h                                                             */
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

#ifndef HEXWAVE_H
#define HEXWAVE_H
#include "core/object/ref_counted.h"
#define malloc(s) memalloc(s) //0
#define free(s) memfree(s) //((void) 0)
#define realloc(s, sz) memrealloc(s, sz) //0
#include "stb/stb_hexwave.h"

using namespace godot;

class MHexWave : public RefCounted {
	GDCLASS(MHexWave, RefCounted);
	HexWave hex;

protected:
	static void _bind_methods();

public:
	static void init(int width, int oversample);
	//         width: size of BLEP, from 4..64, larger is slower & more memory but less aliasing
	//    oversample: 2+, number of subsample positions, larger uses more memory but less noise
	//   user_buffer: optional, if provided the library will perform no allocations.
	//                16*width*(oversample+1) bytes, must stay allocated as long as library is used
	//                technically it only needs:   8*( width * (oversample  + 1))
	//                                           + 8*((width *  oversample) + 1)  bytes
	//
	// width can be larger than 64 if you define STB_HEXWAVE_MAX_BLEP_LENGTH to a larger value

	static void shutdown();
	//       user_buffer: pass in same parameter as passed to hexwave_init

	static Ref<MHexWave> create(int reflect, float peak_time, float half_height, float zero_wait);
	// see docs above for description
	//
	//   reflect is tested as 0 or non-zero
	//   peak_time is clamped to 0..1
	//   half_height is not clamped
	//   zero_wait is clamped to 0..1

	void change(int reflect, float peak_time, float half_height, float zero_wait);
	// see docs

	PackedFloat32Array generate_samples(int num_samples, float freq);
	//            output: buffer where the library will store generated floating point audio samples
	// number_of_samples: the number of audio samples to generate
	//               osc: pointer to a Hexwave initialized with 'hexwave_create'
	//   oscillator_freq: frequency of the oscillator divided by the sample rate
};

#undef malloc
#undef free
#undef realloc

#endif // HEXWAVE_H
