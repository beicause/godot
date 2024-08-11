/**************************************************************************/
/*  cutils.cpp                                                            */
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

#include "cutils.h"

void ArrayIter::_bind_methods() {
	ClassDB::bind_static_method("ArrayIter", D_METHOD("create", "p_array"), &ArrayIter::create);
	ClassDB::bind_method(D_METHOD("set_array", "p_array"), &ArrayIter::set_array);
	ClassDB::bind_method(D_METHOD("get_array"), &ArrayIter::get_array);
	ClassDB::bind_method(D_METHOD("for_each", "p_callable"), &ArrayIter::for_each);
	ClassDB::bind_method(D_METHOD("collect"), &ArrayIter::collect);
	ClassDB::bind_method(D_METHOD("clear_tasks"), &ArrayIter::clear_tasks);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "array"), "set_array", "get_array");
}

void ImageIter::_bind_methods() {
	ClassDB::bind_static_method("ImageIter", D_METHOD("create", "p_img"), &ImageIter::create);
	ClassDB::bind_method(D_METHOD("set_image", "p_img"), &ImageIter::set_image);
	ClassDB::bind_method(D_METHOD("get_image"), &ImageIter::get_image);
	ClassDB::bind_method(D_METHOD("blend", "p_color"), &ImageIter::blend);
	ClassDB::bind_method(D_METHOD("set_ok_hsl", "p_hsl", "p_strength"), &ImageIter::set_ok_hsl);
	ClassDB::bind_method(D_METHOD("inverted"), &ImageIter::inverted);
	ClassDB::bind_method(D_METHOD("for_each", "p_callable"), &ImageIter::for_each);
	ClassDB::bind_method(D_METHOD("collect"), &ImageIter::collect);
	ClassDB::bind_method(D_METHOD("clear_tasks"), &ImageIter::clear_tasks);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "image"), "set_image", "get_image");
}
void CUtils::_bind_methods() {
	ClassDB::bind_static_method("CUtils", D_METHOD("mix_audio_frame", "a", "b"), &CUtils::mix_audio_frame);
	ClassDB::bind_static_method("CUtils", D_METHOD("mix_audio_buffer", "a", "b"), &CUtils::mix_audio_buffer);
}
