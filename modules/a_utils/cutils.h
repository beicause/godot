/**************************************************************************/
/*  cutils.h                                                              */
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

#ifndef CUTILS_H
#define CUTILS_H

#include "core/io/image.h"
#include "core/object/ref_counted.h"
#include <functional>

class CUtils : public RefCounted {
	GDCLASS(CUtils, RefCounted);

protected:
	static void _bind_methods();

public:
	enum BlendMode {
		NORMAL,
		MULTIPLY,
		SCREEN,
		OVERLAY,
		HARD_LIGHT,
		SOFT_LIGHT,
		DODGE,
		LIGHTEN,
		DARKEN,
		ADDITIVE,
		ADDSUB
	};
	static Color blend(const Color &p_c1, const Color &p_c2, float p_opaque, BlendMode p_blend_mode = NORMAL) {
		Color res;
		Color c1 = p_c1;
		Color c2 = p_c2;
		switch (p_blend_mode) {
			case NORMAL: {
				res = p_opaque * c1 + (1 - p_opaque) * c2;
			} break;
			case MULTIPLY: {
				res = p_opaque * c1 * c2 + (1 - p_opaque) * c2;
			} break;
			case SCREEN: {
				res = p_opaque * (Color(1, 1, 1) - (Color(1, 1, 1) - c1) * (Color(1, 1, 1) - c2)) + (1 - p_opaque) * c2;
			} break;
			case OVERLAY: {
				auto overlau_f = [](float a, float b) { return (a < 0.5) ? (2 * a * b) : (1 - 2 * (1 - a) * (1 - b)); };
				res = p_opaque * Color(overlau_f(c1.r, c2.r), overlau_f(c1.g, c2.g), overlau_f(c1.b, c2.b)) + (1 - p_opaque) * c2;
			} break;
			case HARD_LIGHT: {
				res = p_opaque * 0.5 * (c1 * c2 + blend(c1, c2, 1, OVERLAY)) + (1 - p_opaque) * c2;
			} break;
			case SOFT_LIGHT: {
				auto soft_light_f = [](float a, float b) {
					return (b < 0.5) ? (2 * a * b + a * a * (1 - 2 * b)) : (2 * a * (1 - b) + Math::sqrt(a) * (2 * b - 1));
				};
				res = p_opaque * Color(soft_light_f(c1.r, c2.r), soft_light_f(c1.g, c2.g), soft_light_f(c1.b, c2.b)) + (1 - p_opaque) * c2;
			} break;
			case DODGE: {
				auto dodge_f = [](float a, float b) { return (a == 1.0) ? a : MIN(b / (1 - a), 1); };
				res = p_opaque * Color(dodge_f(c1.r, c2.r), dodge_f(c1.g, c2.g), dodge_f(c1.b, c2.b)) + (1 - p_opaque) * c2;
			} break;
			case LIGHTEN: {
				res = p_opaque * Color(MAX(c1.r, c2.r), MAX(c1.g, c2.g), MAX(c1.b, c2.b)) + (1 - p_opaque) * c2;
			} break;
			case DARKEN: {
				res = p_opaque * Color(MIN(c1.r, c2.r), MIN(c1.g, c2.g), MIN(c1.b, c2.b)) + (1 - p_opaque) * c2;
			} break;
			case ADDITIVE: {
				res = p_opaque * c1 + c2;
			} break;
			case ADDSUB: {
				res = c2 * (c1 - Color(0.5, 0.5, 0.5)) * 2 * p_opaque;
			} break;
		}
		res.a = p_c1.a;
		return res.clamp();
	}
	static Vector2 mix_audio_frame(Vector2 a, Vector2 b) {
		Vector2 ret;
		Vector2 s = a * b;
		ret.x = a.x + b.x + s.x <= 0 ? 0 : -SIGN(b.x) * s.x;
		ret.y = a.y + b.y + s.y <= 0 ? 0 : -SIGN(b.y) * s.y;
		return ret;
	}

	static PackedVector2Array mix_audio_buffer(PackedVector2Array a, PackedVector2Array b) {
		PackedVector2Array ret;
		ERR_FAIL_COND_V_MSG(a.size() != b.size(), ret, "Audio buffer sizes don't match.");
		ret.resize(a.size());
		for (int i = 0; i < a.size(); i++) {
			ret.set(i, mix_audio_frame(a[i], b[i]));
		}
		return ret;
	}
};
VARIANT_ENUM_CAST(CUtils::BlendMode);

class ArrayIter : public RefCounted {
	GDCLASS(ArrayIter, RefCounted);
	Vector<Callable> tasks;
	Array array;

protected:
	static void _bind_methods();

public:
	static Ref<ArrayIter> create(Array p_array) {
		Ref<ArrayIter> iter;
		iter.instantiate();
		iter->array = p_array;
		return iter;
	}
	void set_array(Array p_array) { array = p_array; }
	Array get_array() { return array; }

	Ref<ArrayIter> for_each(Callable p_callable) {
		tasks.push_back(p_callable);
		return this;
	}
	Array collect() {
		for (int i = 0; i < array.size(); i++) {
			for (const Callable &callable : tasks) {
				callable.call(array, i);
			}
		}
		return array;
	}
	void clear_tasks() {
		tasks.clear();
	}
};

class ImageIter : public RefCounted {
	GDCLASS(ImageIter, RefCounted);
	typedef void(native_func)(int x, int y);
	Vector<Callable> tasks;
	Vector<std::function<native_func>> native_tasks;
	Ref<Image> img;

protected:
	static void _bind_methods();

public:
	static Ref<ImageIter> create(Ref<Image> p_img) {
		Ref<ImageIter> iter;
		iter.instantiate();
		iter->img = p_img;
		return iter;
	}
	void set_image(Ref<Image> p_img) { img = p_img; }
	Ref<Image> get_image() { return img; }

	Ref<ImageIter> blend_alpha(Color p_color) {
		std::function<native_func> func = [&](int x, int y) {
			img->set_pixel(x, y, img->get_pixel(x, y).blend(p_color));
		};
		native_tasks.push_back(func);
		return this;
	}
	Ref<ImageIter> blend(Color p_color, float p_opaque, CUtils::BlendMode p_mode) {
		std::function<native_func> func = [&](int x, int y) {
			img->set_pixel(x, y, CUtils::blend(img->get_pixel(x, y), p_color, p_opaque, p_mode));
		};
		native_tasks.push_back(func);
		return this;
	}
	Ref<ImageIter> set_ok_hsl(Vector3 p_hsl, Vector3 p_strength) {
		std::function<native_func> func = [&](int x, int y) {
			Vector3 apply = Vector3(1, 1, 1) - p_strength;
			Vector3 hsl_a = p_hsl * p_strength;

			Color c = img->get_pixel(x, y);
			int h = c.get_ok_hsl_h();
			int s = c.get_ok_hsl_s();
			int l = c.get_ok_hsl_l();
			c.set_ok_hsl(h * apply.x + hsl_a.x, s * apply.x + hsl_a.y, l * apply.z + hsl_a.z);
			img->set_pixel(x, y, c);
		};
		native_tasks.push_back(func);
		return this;
	}
	Ref<ImageIter> inverted() {
		std::function<native_func> func = [&](int x, int y) {
			img->set_pixel(x, y, img->get_pixel(x, y).inverted());
		};
		native_tasks.push_back(func);
		return this;
	}
	Ref<ImageIter> for_each(Callable p_callable) {
		tasks.push_back(p_callable);
		return this;
	}
	Ref<Image> collect() {
		ERR_FAIL_COND_V_MSG(img.is_null(), nullptr, "image is null");
		for (int x = 0; x < img->get_width(); x++) {
			for (int y = 0; y < img->get_height(); y++) {
				for (const auto &func : native_tasks) {
					func(x, y);
				}
				for (const Callable &callable : tasks) {
					callable.call(img, x, y);
				}
			}
		}
		return img;
	}
	void clear_tasks() {
		tasks.clear();
	}
};

#endif // CUTILS_H
