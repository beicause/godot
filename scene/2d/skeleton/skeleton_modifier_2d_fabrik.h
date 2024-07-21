/**************************************************************************/
/*  skeleton_modifier_2d_fabrik.h                                         */
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

#ifndef SKELETON_MODIFIER_2D_FABRIK_H
#define SKELETON_MODIFIER_2D_FABRIK_H
#include "scene/2d/skeleton/skeleton_modifier_2d.h"

class SkeletonModifier2DFABRIK : public SkeletonModifier2D {
	GDCLASS(SkeletonModifier2DFABRIK, SkeletonModifier2D);

private:
	struct FABRJoint {
		int bone_idx = -1;
		NodePath bone_node_path;
		ObjectID bone_node_id;
		Vector2 magnet = Vector2(0, 0);
	};

	Vector<FABRJoint> fabrik_data_chain;

	// Unlike in 3D, we need a vector of Transform2D objects to perform FABRIK.
	// This is because FABRIK (unlike CCDIK) needs to operate on transforms that are NOT
	// affected by each other, making the transforms stored in Bone2D unusable, as well as those in Skeleton2D.
	// For this reason, this modification stores a vector of Transform2Ds used for the calculations, which are then applied at the end.
	Vector<Transform2D> fabrik_transform_chain;

	NodePath target_node_path;
	ObjectID target_node_id;

	bool tip_use_target_rotation = false;

	void update_target_node();

	float chain_tolarance = 0.01;
	int chain_max_iterations = 10;
	int chain_iterations = 0;
	Transform2D target_global_pose;
	Transform2D origin_global_pose;

	void fabrik_joint_update_node(int p_joint_idx);
	void chain_backwards();
	void chain_forwards();
	void _update_bones_id();

protected:
	static void _bind_methods();
	virtual void _setup_modification() override;
	virtual void _process_modification(real_t p_delta) override;

public:
	void set_target_node(const NodePath &p_target_node);
	NodePath get_target_node() const;

	int get_bone_chain_size();
	void set_bone_chain_size(int p_size);

	void set_joint_bone(int p_joint_idx, const NodePath &p_bone);
	NodePath get_joint_bone(int p_joint_idx) const;

	void set_joint_bone_idx(int p_joint_idx, int p_bone_idx);
	int get_joint_bone_idx(int p_joint_idx) const;

	void set_joint_magnet(int p_joint_idx, Vector2 p_magnet_position);
	Vector2 get_joint_magnet(int p_joint_idx) const;

	void set_tip_use_target_rotation(bool p_tip_use_target_rotation);
	bool is_tip_use_target_rotation() const;

	void set_joint_bones(Array p_node_paths);
	Array get_joint_bones() const;

	void set_joint_magnets(PackedVector2Array p_magnets);
	PackedVector2Array get_joint_magnets() const;
};

#endif // SKELETON_MODIFIER_2D_FABRIK_H
