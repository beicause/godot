/**************************************************************************/
/*  mcu.cpp                                                               */
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

#include "mcu.h"

Dictionary Mcu::get_scheme(Color c, bool is_dark) {
	Dictionary res = Dictionary();
	mcu::CorePalette palette = mcu::CorePalette::Of(color2argb(c));
	mcu::Scheme scheme = is_dark ? mcu::MaterialDarkColorSchemeFromPalette(palette) : mcu::MaterialLightColorSchemeFromPalette(palette);
	res[primary] = argb2color(scheme.primary);
	res[onPrimary] = argb2color(scheme.on_primary);
	res[primaryContainer] = argb2color(scheme.primary_container);
	res[onPrimaryContainer] = argb2color(scheme.on_primary_container);
	res[secondary] = argb2color(scheme.secondary);
	res[onSecondary] = argb2color(scheme.on_secondary);
	res[secondaryContainer] = argb2color(scheme.secondary_container);
	res[onSecondaryContainer] = argb2color(scheme.on_secondary_container);
	res[tertiary] = argb2color(scheme.tertiary);
	res[onTertiary] = argb2color(scheme.on_tertiary);
	res[tertiaryContainer] = argb2color(scheme.tertiary_container);
	res[onTertiaryContainer] = argb2color(scheme.on_tertiary_container);
	res[error] = argb2color(scheme.error);
	res[onError] = argb2color(scheme.on_error);
	res[errorContainer] = argb2color(scheme.error_container);
	res[onErrorContainer] = argb2color(scheme.on_error_container);
	res[background] = argb2color(scheme.background);
	res[onBackground] = argb2color(scheme.on_background);
	res[surface] = argb2color(scheme.surface);
	res[onSurface] = argb2color(scheme.on_surface);
	res[surfaceVariant] = argb2color(scheme.surface_variant);
	res[onSurfaceVariant] = argb2color(scheme.on_surface_variant);
	res[outline] = argb2color(scheme.outline);
	res[outlineVariant] = argb2color(scheme.outline_variant);
	res[shadow] = argb2color(scheme.shadow);
	res[scrim] = argb2color(scheme.scrim);
	res[inverseSurface] = argb2color(scheme.inverse_surface);
	res[inverseOnSurface] = argb2color(scheme.inverse_on_surface);
	res[inversePrimary] = argb2color(scheme.inverse_primary);

	res[surfaceContainer] = argb2color(palette.neutral().get(is_dark ? 12 : 94));
	res[surfaceContainerHigh] = argb2color(palette.neutral().get(is_dark ? 17 : 92));
	return res;
}

void Mcu::_bind_methods() {
	BIND_ENUM_CONSTANT(primary);
	BIND_ENUM_CONSTANT(onPrimary);
	BIND_ENUM_CONSTANT(primaryContainer);
	BIND_ENUM_CONSTANT(onPrimaryContainer);
	BIND_ENUM_CONSTANT(secondary);
	BIND_ENUM_CONSTANT(onSecondary);
	BIND_ENUM_CONSTANT(secondaryContainer);
	BIND_ENUM_CONSTANT(onSecondaryContainer);
	BIND_ENUM_CONSTANT(tertiary);
	BIND_ENUM_CONSTANT(onTertiary);
	BIND_ENUM_CONSTANT(tertiaryContainer);
	BIND_ENUM_CONSTANT(onTertiaryContainer);
	BIND_ENUM_CONSTANT(error);
	BIND_ENUM_CONSTANT(onError);
	BIND_ENUM_CONSTANT(errorContainer);
	BIND_ENUM_CONSTANT(onErrorContainer);
	BIND_ENUM_CONSTANT(background);
	BIND_ENUM_CONSTANT(onBackground);
	BIND_ENUM_CONSTANT(surface);
	BIND_ENUM_CONSTANT(onSurface);
	BIND_ENUM_CONSTANT(surfaceVariant);
	BIND_ENUM_CONSTANT(onSurfaceVariant);
	BIND_ENUM_CONSTANT(outline);
	BIND_ENUM_CONSTANT(outlineVariant);
	BIND_ENUM_CONSTANT(shadow);
	BIND_ENUM_CONSTANT(scrim);
	BIND_ENUM_CONSTANT(inverseSurface);
	BIND_ENUM_CONSTANT(inverseOnSurface);
	BIND_ENUM_CONSTANT(inversePrimary);
	BIND_ENUM_CONSTANT(surfaceContainer);
	BIND_ENUM_CONSTANT(surfaceContainerHigh);
	ClassDB::bind_static_method("Mcu", D_METHOD("get_scheme", "c", "is_dark"), &Mcu::get_scheme, DEFVAL(true));
}

void Hct::_bind_methods() {
	ClassDB::bind_static_method("Hct", D_METHOD("from_argb", "argb"), &Hct::from_argb);
	ClassDB::bind_static_method("Hct", D_METHOD("from_color", "c"), &Hct::from_color);
	ClassDB::bind_static_method("Hct", D_METHOD("from_hct", "hue", "chroma", "tone"), &Hct::from_hct);

	ClassDB::bind_method(D_METHOD("get_hue"), &Hct::get_hue);
	ClassDB::bind_method(D_METHOD("get_chroma"), &Hct::get_chroma);
	ClassDB::bind_method(D_METHOD("get_tone"), &Hct::get_tone);
	ClassDB::bind_method(D_METHOD("set_hue", "value"), &Hct::set_hue);
	ClassDB::bind_method(D_METHOD("set_chroma", "value"), &Hct::set_chroma);
	ClassDB::bind_method(D_METHOD("set_tone", "value"), &Hct::set_tone);

	ClassDB::bind_method(D_METHOD("to_color"), &Hct::to_color);
	ClassDB::bind_method(D_METHOD("to_argb"), &Hct::to_argb);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "hue"), "set_hue", "get_hue");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "chroma"), "set_chroma", "get_chroma");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "tone"), "set_tone", "get_tone");
}
