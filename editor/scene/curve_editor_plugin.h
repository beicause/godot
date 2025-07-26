/**************************************************************************/
/*  curve_editor_plugin.h                                                 */
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

#include "editor/inspector/editor_inspector.h"
#include "editor/inspector/editor_resource_preview.h"
#include "editor/plugins/editor_plugin.h"
#include "scene/resources/curve.h"

class EditorSpinSlider;
class MenuButton;
class PopupMenu;

class CurveEdit : public Control {
	GDCLASS(CurveEdit, Control);

public:
	CurveEdit();

	void set_snap_enabled(bool p_enabled);
	void set_snap_count(int p_snap_count);
	void use_preset(int p_preset_id);

	void set_curve(Ref<Curve> p_curve);
	Ref<Curve> get_curve();

	Size2 get_minimum_size() const override;

	enum PresetID {
		PRESET_CONSTANT = 0,
		PRESET_LINEAR,
		PRESET_EASE_IN,
		PRESET_EASE_OUT,
		PRESET_SMOOTHSTEP,
		PRESET_COUNT
	};

	enum TangentIndex {
		TANGENT_NONE = -1,
		TANGENT_LEFT = 0,
		TANGENT_RIGHT = 1
	};

	enum ArrowIndex {
		ARROW_NONE = -1,
		ARROW_HORIZONTAL_LEFT,
		ARROW_HORIZONTAL_RIGHT,
		ARROW_VERTICAL_BOTTOM,
		ARROW_VERTICAL_TOP,
		ARROW_MAX,
	};

protected:
	void _notification(int p_what);
	static void _bind_methods();

private:
	virtual void gui_input(const Ref<InputEvent> &p_event) override;
	void _curve_changed();

	int get_point_at(const Vector2 &p_pos) const;
	TangentIndex get_tangent_at(const Vector2 &p_pos) const;
	ArrowIndex get_arrow_at(const Vector2 &p_pos) const;

	float get_offset_without_collision(int p_current_index, float p_offset, bool p_prioritize_right = true);

	void add_point(const Vector2 &p_pos);
	void remove_point(int p_index);
	void set_point_position(int p_index, const Vector2 &p_pos);

	void remap_domain(real_t p_new_min, real_t p_new_max);
	void remap_value(real_t p_new_min, real_t p_new_max);
	void set_curve_data();

	void set_point_tangents(int p_index, float p_left, float p_right);
	void set_point_left_tangent(int p_index, float p_tangent);
	void set_point_right_tangent(int p_index, float p_tangent);
	void toggle_linear(int p_index, TangentIndex p_tangent = TANGENT_NONE);

	void update_view_transform();

	void plot_curve_accurate(float p_step, const Color &p_line_color, const Color &p_edge_line_color);

	void set_selected_index(int p_index);

	Vector2 get_tangent_view_pos(int p_index, TangentIndex p_tangent) const;
	Vector2 get_view_pos(const Vector2 &p_world_pos) const;
	Vector2 get_world_pos(const Vector2 &p_view_pos) const;

	Vector<Vector2> _get_arrow_polygon(ArrowIndex p_index, int p_arrow_size);

	void _redraw();

private:
	const float ASPECT_RATIO = 6.f / 13.f;
	const float LINE_WIDTH = 0.5f;
	const int STEP_SIZE = 2; // Number of pixels between plot points.

	Transform2D _world_to_view;

	Ref<Curve> curve;
	PopupMenu *_presets_menu = nullptr;

	int selected_index = -1;
	int hovered_index = -1;
	TangentIndex selected_tangent_index = TANGENT_NONE;
	TangentIndex hovered_tangent_index = TANGENT_NONE;

	Vector2 arrow_centers[ARROW_MAX];
	ArrowIndex selected_arrow_index = ARROW_NONE;
	ArrowIndex hovered_arrow_index = ARROW_NONE;

	// Make sure to use the scaled values below.
	const int BASE_POINT_RADIUS = 4;
	const int BASE_HOVER_RADIUS = 10;
	const int BASE_TANGENT_RADIUS = 3;
	const int BASE_TANGENT_HOVER_RADIUS = 8;
	const int BASE_TANGENT_LENGTH = 36;
	const int BASE_ARROW_RADIUS = 5;
	const int BASE_ARROW_HOVER_RADIUS = 12;

	int point_radius = BASE_POINT_RADIUS;
	int hover_radius = BASE_HOVER_RADIUS;
	int tangent_radius = BASE_TANGENT_RADIUS;
	int tangent_hover_radius = BASE_TANGENT_HOVER_RADIUS;
	int tangent_length = BASE_TANGENT_LENGTH;
	int arrow_radius = BASE_ARROW_RADIUS;
	int arrow_hover_radius = BASE_ARROW_HOVER_RADIUS;

	enum GrabMode {
		GRAB_NONE,
		GRAB_ADD,
		GRAB_MOVE,
		GRAB_ARROW,
	};
	GrabMode grabbing = GRAB_NONE;
	Vector2 initial_grab_pos;
	int initial_grab_index = -1;
	float initial_grab_left_tangent = 0;
	float initial_grab_right_tangent = 0;

	Vector4 initial_grab_curve_range;
	Array initial_grab_data;

	bool snap_enabled = false;
	int snap_count = 10;
};

// CurveEdit + toolbar
class CurveEditor : public VBoxContainer {
	GDCLASS(CurveEditor, VBoxContainer);

	// Make sure to use the scaled values below.
	const int BASE_SPACING = 4;
	int spacing = BASE_SPACING;

	Button *snap_button = nullptr;
	EditorSpinSlider *snap_count_edit = nullptr;
	MenuButton *presets_button = nullptr;
	CurveEdit *curve_editor_rect = nullptr;

	void _set_snap_enabled(bool p_enabled);
	void _set_snap_count(int p_snap_count);
	void _on_preset_item_selected(int p_preset_id);

protected:
	void _notification(int p_what);

public:
	static const int DEFAULT_SNAP;
	void set_curve(const Ref<Curve> &p_curve);

	CurveEditor();
};

class EditorInspectorPluginCurve : public EditorInspectorPlugin {
	GDCLASS(EditorInspectorPluginCurve, EditorInspectorPlugin);

public:
	virtual bool can_handle(Object *p_object) override;
	virtual void parse_begin(Object *p_object) override;
};

class CurveEditorPlugin : public EditorPlugin {
	GDCLASS(CurveEditorPlugin, EditorPlugin);

public:
	CurveEditorPlugin();

	virtual String get_plugin_name() const override { return "Curve"; }
};

class CurvePreviewGenerator : public EditorResourcePreviewGenerator {
	GDCLASS(CurvePreviewGenerator, EditorResourcePreviewGenerator);

public:
	virtual bool handles(const String &p_type) const override;
	virtual Ref<Texture2D> generate(const Ref<Resource> &p_from, const Size2 &p_size, Dictionary &p_metadata) const override;
};
