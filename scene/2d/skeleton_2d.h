/**************************************************************************/
/*  skeleton_2d.h                                                         */
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

#ifndef SKELETON_2D_H
#define SKELETON_2D_H

#include "scene/2d/node_2d.h"

class Skeleton2D;

class Bone2D : public Node2D {
	GDCLASS(Bone2D, Node2D);

	friend class Skeleton2D;

	Skeleton2D *skeleton = nullptr;
	Bone2D *parent_bone = nullptr;

	Transform2D rest;
	Transform2D accum_transform;

	bool autocalculate_length_and_angle = true;
	real_t bone_length = 16;

	int bone_idx = -1;
	int parent_idx = -1;

	void calculate_length_and_rotation();

#ifdef TOOLS_ENABLED
	RID editor_gizmo_rid;
	bool _editor_get_bone_shape(Vector<Vector2> *p_shape, Vector<Vector2> *p_outline_shape, Bone2D *p_other_bone);
	bool _editor_show_bone_gizmo = true;
#endif // TOOLS ENABLED

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	void set_rest(const Transform2D &p_rest);
	Transform2D get_rest() const;
	void apply_rest();

	Transform2D get_accum_transform() const;

	PackedStringArray get_configuration_warnings() const override;

	void set_autocalculate_length_and_angle(bool p_autocalculate);
	bool get_autocalculate_length_and_angle() const;

	void set_bone_length(real_t p_length);
	real_t get_bone_length() const;

	int get_bone_idx() const;
	int get_parent_idx() const;

#ifdef TOOLS_ENABLED
	void _editor_set_show_bone_gizmo(bool p_show_gizmo);
	bool _editor_get_show_bone_gizmo() const;
#endif // TOOLS_ENABLED

	Bone2D();
	~Bone2D();
};

struct Bone2DComparator {
	bool operator()(const Bone2D *a, const Bone2D *b) const {
		return !(a->is_greater_than(b));
	}
};

class SkeletonModifier3D;

class Skeleton2D : public Node2D {
	GDCLASS(Skeleton2D, Node2D);

	friend class Bone2D;

	Vector<Bone2D *> bones;

	bool bone_setup_dirty = true;
	void _make_bone_setup_dirty();
	void _update_bone_setup();

	bool transform_dirty = true;
	void _make_transform_dirty();
	void _update_transform();

	RID skeleton;

	void _update_skeleton_deferred();
	bool updating = false; // Is updating now?

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	enum ModifierCallbackModeProcess {
		MODIFIER_CALLBACK_MODE_PROCESS_PHYSICS,
		MODIFIER_CALLBACK_MODE_PROCESS_IDLE,
	};
	enum {
		NOTIFICATION_UPDATE_SKELETON = 50
	};

	int get_bone_count() const;
	Bone2D *get_bone(int p_idx);

	RID get_skeleton() const;

	// To process modifiers.
	ModifierCallbackModeProcess modifier_callback_mode_process = MODIFIER_CALLBACK_MODE_PROCESS_IDLE;
	LocalVector<ObjectID> modifiers;
	bool modifiers_dirty = false;
	void _find_modifiers();
	void _process_modifiers();
	void _process_changed();
	void _make_modifiers_dirty();

	void set_modifier_callback_mode_process(ModifierCallbackModeProcess p_mode);
	ModifierCallbackModeProcess get_modifier_callback_mode_process() const;

	// Posing API
	Transform2D get_bone_pose(int p_bone) const;
	Vector2 get_bone_pose_position(int p_bone) const;
	real_t get_bone_pose_rotation(int p_bone) const;
	Vector2 get_bone_pose_scale(int p_bone) const;
	void set_bone_pose(int p_bone, const Transform2D &p_pose);
	void set_bone_pose_position(int p_bone, const Vector2 &p_position);
	void set_bone_pose_rotation(int p_bone, const real_t &p_rotation);
	void set_bone_pose_scale(int p_bone, const Vector2 &p_scale);

	Transform2D get_bone_global_pose(int p_bone) const;
	void set_bone_global_pose(int p_bone, const Transform2D &p_pose);

	void reset_bone_pose(int p_bone);
	void reset_bone_poses();

	Skeleton2D();
	~Skeleton2D();
};

VARIANT_ENUM_CAST(Skeleton2D::ModifierCallbackModeProcess);

#endif // SKELETON_2D_H
