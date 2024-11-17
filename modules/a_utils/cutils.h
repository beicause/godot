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

#include "core/object/ref_counted.h"
#include "scene/theme/default_font.gen.h"
#include "scene/theme/default_theme.h"

class CUtils : public RefCounted {
	GDCLASS(CUtils, RefCounted);

protected:
	static void _bind_methods();

public:
	static Ref<Theme> make_default_theme(float p_scale = 1.0, Ref<Font> p_font = Ref<Font>()) {
		Ref<Theme> t;
		t.instantiate();

		TextServer::SubpixelPositioning p_font_subpixel = TextServer::SUBPIXEL_POSITIONING_AUTO;
		TextServer::Hinting p_font_hinting = TextServer::HINTING_LIGHT;
		TextServer::FontAntialiasing p_font_antialiased = TextServer::FONT_ANTIALIASING_GRAY;
		bool p_font_msdf = false;
		bool p_font_generate_mipmaps = false;

		Ref<StyleBox> default_style;
		Ref<Texture2D> default_icon;
		Ref<Font> default_font;
		Ref<FontVariation> bold_font;
		Ref<FontVariation> bold_italics_font;
		Ref<FontVariation> italics_font;
		float default_scale = CLAMP(p_scale, 0.5, 8.0);

		if (p_font.is_valid()) {
			// Use the custom font defined in the Project Settings.
			default_font = p_font;
		} else {
			// Use the default DynamicFont (separate from the editor font).
			// The default DynamicFont is chosen to have a small file size since it's
			// embedded in both editor and export template binaries.
			Ref<FontFile> dynamic_font;
			dynamic_font.instantiate();
			dynamic_font->set_data_ptr(_font_OpenSans_SemiBold, _font_OpenSans_SemiBold_size);
			dynamic_font->set_subpixel_positioning(p_font_subpixel);
			dynamic_font->set_hinting(p_font_hinting);
			dynamic_font->set_antialiasing(p_font_antialiased);
			dynamic_font->set_multichannel_signed_distance_field(p_font_msdf);
			dynamic_font->set_generate_mipmaps(p_font_generate_mipmaps);

			default_font = dynamic_font;
		}

		if (default_font.is_valid()) {
			bold_font.instantiate();
			bold_font->set_base_font(default_font);
			bold_font->set_variation_embolden(1.2);

			bold_italics_font.instantiate();
			bold_italics_font->set_base_font(default_font);
			bold_italics_font->set_variation_embolden(1.2);
			bold_italics_font->set_variation_transform(Transform2D(1.0, 0.2, 0.0, 1.0, 0.0, 0.0));

			italics_font.instantiate();
			italics_font->set_base_font(default_font);
			italics_font->set_variation_transform(Transform2D(1.0, 0.2, 0.0, 1.0, 0.0, 0.0));
		}

		fill_default_theme(t, default_font, bold_font, bold_italics_font, italics_font, default_icon, default_style, default_scale);
		return t;
	}
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
};
VARIANT_ENUM_CAST(CUtils::BlendMode);

#endif // CUTILS_H
