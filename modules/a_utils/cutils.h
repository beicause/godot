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
#include "core/object/worker_thread_pool.h"

class CUtils : public RefCounted {
	GDCLASS(CUtils, RefCounted);
	struct _ImageOkHslData {
		uint8_t *buf = nullptr;
		Image::Format format = Image::Format::FORMAT_RGBA8;
		Vector3 hsl;
		Vector3 apply;
	};
	struct _ImageBlendData {
		uint8_t *image_data = nullptr;
		Image::Format format = Image::Format::FORMAT_RGBA8;
		Color color;
	};

	static inline Color _get_color_at_ofs(const uint8_t *ptr, uint32_t ofs, Image::Format format) {
		switch (format) {
			case Image::Format::FORMAT_L8: {
				float l = ptr[ofs] / 255.0;
				return Color(l, l, l, 1);
			}
			case Image::Format::FORMAT_LA8: {
				float l = ptr[ofs * 2 + 0] / 255.0;
				float a = ptr[ofs * 2 + 1] / 255.0;
				return Color(l, l, l, a);
			}
			case Image::Format::FORMAT_R8: {
				float r = ptr[ofs] / 255.0;
				return Color(r, 0, 0, 1);
			}
			case Image::Format::FORMAT_RG8: {
				float r = ptr[ofs * 2 + 0] / 255.0;
				float g = ptr[ofs * 2 + 1] / 255.0;
				return Color(r, g, 0, 1);
			}
			case Image::Format::FORMAT_RGB8: {
				float r = ptr[ofs * 3 + 0] / 255.0;
				float g = ptr[ofs * 3 + 1] / 255.0;
				float b = ptr[ofs * 3 + 2] / 255.0;
				return Color(r, g, b, 1);
			}
			case Image::Format::FORMAT_RGBA8: {
				float r = ptr[ofs * 4 + 0] / 255.0;
				float g = ptr[ofs * 4 + 1] / 255.0;
				float b = ptr[ofs * 4 + 2] / 255.0;
				float a = ptr[ofs * 4 + 3] / 255.0;
				return Color(r, g, b, a);
			}
			case Image::Format::FORMAT_RGBA4444: {
				uint16_t u = ((uint16_t *)ptr)[ofs];
				float r = ((u >> 12) & 0xF) / 15.0;
				float g = ((u >> 8) & 0xF) / 15.0;
				float b = ((u >> 4) & 0xF) / 15.0;
				float a = (u & 0xF) / 15.0;
				return Color(r, g, b, a);
			}
			case Image::Format::FORMAT_RGB565: {
				uint16_t u = ((uint16_t *)ptr)[ofs];
				float r = (u & 0x1F) / 31.0;
				float g = ((u >> 5) & 0x3F) / 63.0;
				float b = ((u >> 11) & 0x1F) / 31.0;
				return Color(r, g, b, 1.0);
			}
			case Image::Format::FORMAT_RF: {
				float r = ((float *)ptr)[ofs];
				return Color(r, 0, 0, 1);
			}
			case Image::Format::FORMAT_RGF: {
				float r = ((float *)ptr)[ofs * 2 + 0];
				float g = ((float *)ptr)[ofs * 2 + 1];
				return Color(r, g, 0, 1);
			}
			case Image::Format::FORMAT_RGBF: {
				float r = ((float *)ptr)[ofs * 3 + 0];
				float g = ((float *)ptr)[ofs * 3 + 1];
				float b = ((float *)ptr)[ofs * 3 + 2];
				return Color(r, g, b, 1);
			}
			case Image::Format::FORMAT_RGBAF: {
				float r = ((float *)ptr)[ofs * 4 + 0];
				float g = ((float *)ptr)[ofs * 4 + 1];
				float b = ((float *)ptr)[ofs * 4 + 2];
				float a = ((float *)ptr)[ofs * 4 + 3];
				return Color(r, g, b, a);
			}
			case Image::Format::FORMAT_RH: {
				uint16_t r = ((uint16_t *)ptr)[ofs];
				return Color(Math::half_to_float(r), 0, 0, 1);
			}
			case Image::Format::FORMAT_RGH: {
				uint16_t r = ((uint16_t *)ptr)[ofs * 2 + 0];
				uint16_t g = ((uint16_t *)ptr)[ofs * 2 + 1];
				return Color(Math::half_to_float(r), Math::half_to_float(g), 0, 1);
			}
			case Image::Format::FORMAT_RGBH: {
				uint16_t r = ((uint16_t *)ptr)[ofs * 3 + 0];
				uint16_t g = ((uint16_t *)ptr)[ofs * 3 + 1];
				uint16_t b = ((uint16_t *)ptr)[ofs * 3 + 2];
				return Color(Math::half_to_float(r), Math::half_to_float(g), Math::half_to_float(b), 1);
			}
			case Image::Format::FORMAT_RGBAH: {
				uint16_t r = ((uint16_t *)ptr)[ofs * 4 + 0];
				uint16_t g = ((uint16_t *)ptr)[ofs * 4 + 1];
				uint16_t b = ((uint16_t *)ptr)[ofs * 4 + 2];
				uint16_t a = ((uint16_t *)ptr)[ofs * 4 + 3];
				return Color(Math::half_to_float(r), Math::half_to_float(g), Math::half_to_float(b), Math::half_to_float(a));
			}
			case Image::Format::FORMAT_RGBE9995: {
				return Color::from_rgbe9995(((uint32_t *)ptr)[ofs]);
			}
			default: {
				ERR_FAIL_V_MSG(Color(), "Can't get_pixel() on compressed image, sorry.");
			}
		}
	}

	static inline void _set_color_at_ofs(uint8_t *ptr, uint32_t ofs, const Color &p_color, Image::Format format) {
		switch (format) {
			case Image::Format::FORMAT_L8: {
				ptr[ofs] = uint8_t(CLAMP(p_color.get_v() * 255.0, 0, 255));
			} break;
			case Image::Format::FORMAT_LA8: {
				ptr[ofs * 2 + 0] = uint8_t(CLAMP(p_color.get_v() * 255.0, 0, 255));
				ptr[ofs * 2 + 1] = uint8_t(CLAMP(p_color.a * 255.0, 0, 255));
			} break;
			case Image::Format::FORMAT_R8: {
				ptr[ofs] = uint8_t(CLAMP(p_color.r * 255.0, 0, 255));
			} break;
			case Image::Format::FORMAT_RG8: {
				ptr[ofs * 2 + 0] = uint8_t(CLAMP(p_color.r * 255.0, 0, 255));
				ptr[ofs * 2 + 1] = uint8_t(CLAMP(p_color.g * 255.0, 0, 255));
			} break;
			case Image::Format::FORMAT_RGB8: {
				ptr[ofs * 3 + 0] = uint8_t(CLAMP(p_color.r * 255.0, 0, 255));
				ptr[ofs * 3 + 1] = uint8_t(CLAMP(p_color.g * 255.0, 0, 255));
				ptr[ofs * 3 + 2] = uint8_t(CLAMP(p_color.b * 255.0, 0, 255));
			} break;
			case Image::Format::FORMAT_RGBA8: {
				ptr[ofs * 4 + 0] = uint8_t(CLAMP(p_color.r * 255.0, 0, 255));
				ptr[ofs * 4 + 1] = uint8_t(CLAMP(p_color.g * 255.0, 0, 255));
				ptr[ofs * 4 + 2] = uint8_t(CLAMP(p_color.b * 255.0, 0, 255));
				ptr[ofs * 4 + 3] = uint8_t(CLAMP(p_color.a * 255.0, 0, 255));

			} break;
			case Image::Format::FORMAT_RGBA4444: {
				uint16_t rgba = 0;

				rgba = uint16_t(CLAMP(p_color.r * 15.0, 0, 15)) << 12;
				rgba |= uint16_t(CLAMP(p_color.g * 15.0, 0, 15)) << 8;
				rgba |= uint16_t(CLAMP(p_color.b * 15.0, 0, 15)) << 4;
				rgba |= uint16_t(CLAMP(p_color.a * 15.0, 0, 15));

				((uint16_t *)ptr)[ofs] = rgba;

			} break;
			case Image::Format::FORMAT_RGB565: {
				uint16_t rgba = 0;

				rgba = uint16_t(CLAMP(p_color.r * 31.0, 0, 31));
				rgba |= uint16_t(CLAMP(p_color.g * 63.0, 0, 33)) << 5;
				rgba |= uint16_t(CLAMP(p_color.b * 31.0, 0, 31)) << 11;

				((uint16_t *)ptr)[ofs] = rgba;

			} break;
			case Image::Format::FORMAT_RF: {
				((float *)ptr)[ofs] = p_color.r;
			} break;
			case Image::Format::FORMAT_RGF: {
				((float *)ptr)[ofs * 2 + 0] = p_color.r;
				((float *)ptr)[ofs * 2 + 1] = p_color.g;
			} break;
			case Image::Format::FORMAT_RGBF: {
				((float *)ptr)[ofs * 3 + 0] = p_color.r;
				((float *)ptr)[ofs * 3 + 1] = p_color.g;
				((float *)ptr)[ofs * 3 + 2] = p_color.b;
			} break;
			case Image::Format::FORMAT_RGBAF: {
				((float *)ptr)[ofs * 4 + 0] = p_color.r;
				((float *)ptr)[ofs * 4 + 1] = p_color.g;
				((float *)ptr)[ofs * 4 + 2] = p_color.b;
				((float *)ptr)[ofs * 4 + 3] = p_color.a;
			} break;
			case Image::Format::FORMAT_RH: {
				((uint16_t *)ptr)[ofs] = Math::make_half_float(p_color.r);
			} break;
			case Image::Format::FORMAT_RGH: {
				((uint16_t *)ptr)[ofs * 2 + 0] = Math::make_half_float(p_color.r);
				((uint16_t *)ptr)[ofs * 2 + 1] = Math::make_half_float(p_color.g);
			} break;
			case Image::Format::FORMAT_RGBH: {
				((uint16_t *)ptr)[ofs * 3 + 0] = Math::make_half_float(p_color.r);
				((uint16_t *)ptr)[ofs * 3 + 1] = Math::make_half_float(p_color.g);
				((uint16_t *)ptr)[ofs * 3 + 2] = Math::make_half_float(p_color.b);
			} break;
			case Image::Format::FORMAT_RGBAH: {
				((uint16_t *)ptr)[ofs * 4 + 0] = Math::make_half_float(p_color.r);
				((uint16_t *)ptr)[ofs * 4 + 1] = Math::make_half_float(p_color.g);
				((uint16_t *)ptr)[ofs * 4 + 2] = Math::make_half_float(p_color.b);
				((uint16_t *)ptr)[ofs * 4 + 3] = Math::make_half_float(p_color.a);
			} break;
			case Image::Format::FORMAT_RGBE9995: {
				((uint32_t *)ptr)[ofs] = p_color.to_rgbe9995();

			} break;
			default: {
				ERR_FAIL_MSG("Can't set_pixel() on compressed image, sorry.");
			}
		}
	}
	static void _image_set_ok_hsl(void *p_data, uint32_t idx) {
		_ImageOkHslData *data = (_ImageOkHslData *)p_data;
		Color c = _get_color_at_ofs(data->buf, idx, data->format);
		c.set_ok_hsl(data->hsl.x + data->apply.x * c.get_ok_hsl_h(), data->hsl.y + data->apply.y * c.get_ok_hsl_s(), data->hsl.z + data->apply.z * c.get_ok_hsl_l());
		_set_color_at_ofs(data->buf, idx, c, data->format);
	}
	static void _image_blend(void *p_data, uint32_t idx) {
		_ImageBlendData *data = (_ImageBlendData *)p_data;
		Color c = _get_color_at_ofs(data->image_data, idx, data->format).blend(data->color);
		_set_color_at_ofs(data->image_data, idx, c, data->format);
	}

protected:
	static void _bind_methods();

public:
	static Ref<Image> image_set_ok_hsl(Ref<Image> p_image, Vector3 p_hsl, Vector3 p_strength) {
		Ref<Image> img = p_image->duplicate();
		Vector3 hsl = p_hsl * p_strength;
		Vector3 apply = Vector3(1, 1, 1) - p_strength;
		_ImageOkHslData data{ img->ptrw(), img->get_format(), hsl, apply };

		WorkerThreadPool::GroupID task_id = WorkerThreadPool::get_singleton()->add_native_group_task(&CUtils::_image_set_ok_hsl, &data, img->get_width() * img->get_height());
		WorkerThreadPool::get_singleton()->wait_for_group_task_completion(task_id);

		return img;
	}

	static Ref<Image> image_blend(Ref<Image> p_image, Color p_color) {
		Ref<Image> img = p_image->duplicate();
		_ImageBlendData data{ img->ptrw(), img->get_format(), p_color };

		WorkerThreadPool::GroupID task_id = WorkerThreadPool::get_singleton()->add_native_group_task(&CUtils::_image_blend, &data, img->get_width() * img->get_height());
		WorkerThreadPool::get_singleton()->wait_for_group_task_completion(task_id);

		return img;
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
		Vector2 *data[] = { ret.ptrw(), a.ptrw(), b.ptrw() };
		WorkerThreadPool::GroupID task_id = WorkerThreadPool::get_singleton()->add_native_group_task([](void *p_data, uint32_t idx) {
			Vector2 **data = (Vector2 **)p_data;
			data[0][idx] = CUtils::mix_audio_frame(data[1][idx], data[2][idx]);
		},
				&data, ret.size());
		WorkerThreadPool::get_singleton()->wait_for_group_task_completion(task_id);
		return ret;
	}
};

#endif // CUTILS_H
