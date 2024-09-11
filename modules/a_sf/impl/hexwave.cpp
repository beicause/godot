/**************************************************************************/
/*  hexwave.cpp                                                           */
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

#define STB_HEXWAVE_IMPLEMENTATION
#include "hexwave.h"

void MHexWave::init(int width, int oversample) {
	hexwave_init(width, oversample, nullptr);
}
void MHexWave::shutdown() {
	hexwave_shutdown(nullptr);
}
Ref<MHexWave> MHexWave::create(int reflect, float peak_time, float half_height, float zero_wait) {
	Ref<MHexWave> ret;
	ret.instantiate();
	hexwave_create(&ret->hex, reflect, peak_time, half_height, zero_wait);
	return ret;
}

void MHexWave::change(int reflect, float peak_time, float half_height, float zero_wait) {
	hexwave_change(&hex, reflect, peak_time, half_height, zero_wait);
}

PackedFloat32Array MHexWave::generate_samples(int num_samples, float freq) {
	PackedFloat32Array ret;
	ret.resize(num_samples);
	hexwave_generate_samples(ret.ptrw(), num_samples, &hex, freq);
	return ret;
}

void MHexWave::_bind_methods() {
	ClassDB::bind_static_method("MHexWave", D_METHOD("init", "width", "oversample"), &MHexWave::init);
	ClassDB::bind_static_method("MHexWave", D_METHOD("shutdown"), &MHexWave::shutdown);
	ClassDB::bind_static_method("MHexWave", D_METHOD("create", "reflect", "peak_time", "half_height", "zero_wait"), &MHexWave::create);
	ClassDB::bind_method(D_METHOD("change", "reflect", "peak_time", "half_height", "zero_wait"), &MHexWave::change);
	ClassDB::bind_method(D_METHOD("generate_samples", "num_samples", "freq"), &MHexWave::generate_samples);
}
