/**************************************************************************/
/*  skeleton_ik_2d_fabr.cpp                                               */
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

#include "skeleton_ik_2d_fabr.h"
#include "scene/2d/skeleton_2d.h"

void SkeletonIK2DFABR::_process_modification() {
	if (!is_active() || get_skeleton() == nullptr || !get_skeleton()->is_inside_tree() || !get_skeleton()->has_node(target_node)) {
		ERR_PRINT_ONCE("SkeletonIk2DFABR is not setup properly. Cannot execute modification!");
		return;
	}

	if (target_node_id.is_null()) {
		WARN_PRINT_ONCE("Target cache is out of date. Attempting to update...");
		update_target_node();
		return;
	}

	if (fabrik_data_chain.size() <= 1) {
		ERR_PRINT_ONCE("FABRIK requires at least two joints to operate! Cannot execute modification!");
		return;
	}

	Bone2D *target = Object::cast_to<Bone2D>(ObjectDB::get_instance(target_node_id));
	if (!target || !target->is_inside_tree()) {
		ERR_PRINT_ONCE("Target node is not in the scene tree. Cannot execute modification!");
		return;
	}
	target_global_pose = target->get_global_transform();

	if (fabrik_data_chain[0].bone_node_id.is_null() && !fabrik_data_chain[0].bone_node.is_empty()) {
		fabrik_joint_update_node(0);
		WARN_PRINT("Bone2D cache for origin joint is out of date. Updating...");
	}

	Bone2D *origin_bone2d_node = Object::cast_to<Bone2D>(ObjectDB::get_instance(fabrik_data_chain[0].bone_node_id));
	if (!origin_bone2d_node || !origin_bone2d_node->is_inside_tree()) {
		ERR_PRINT_ONCE("Origin joint's Bone2D node is not in the scene tree. Cannot execute modification!");
		return;
	}

	origin_global_pose = origin_bone2d_node->get_global_transform();

	if (fabrik_transform_chain.size() != fabrik_data_chain.size()) {
		fabrik_transform_chain.resize(fabrik_data_chain.size());
	}

	for (int i = 0; i < fabrik_data_chain.size(); i++) {
		// Update the transform chain
		if (fabrik_data_chain[i].bone_node_id.is_null() && !fabrik_data_chain[i].bone_node.is_empty()) {
			WARN_PRINT_ONCE("Bone2D cache for joint " + itos(i) + " is out of date.. Attempting to update...");
			fabrik_joint_update_node(i);
		}
		Bone2D *joint_bone2d_node = Object::cast_to<Bone2D>(ObjectDB::get_instance(fabrik_data_chain[i].bone_node_id));
		if (!joint_bone2d_node) {
			ERR_PRINT_ONCE("FABRIK Joint " + itos(i) + " does not have a Bone2D node set! Cannot execute modification!");
			return;
		}
		fabrik_transform_chain.write[i] = joint_bone2d_node->get_global_transform();
	}

	Bone2D *final_bone2d_node = Object::cast_to<Bone2D>(ObjectDB::get_instance(fabrik_data_chain[fabrik_data_chain.size() - 1].bone_node_id));
	float final_bone2d_angle = final_bone2d_node->get_global_rotation();
	if (tip_use_target_rotation) {
		final_bone2d_angle = target_global_pose.get_rotation();
	}
	Vector2 final_bone2d_direction = Vector2(Math::cos(final_bone2d_angle), Math::sin(final_bone2d_angle));
	float final_bone2d_length = final_bone2d_node->get_bone_length() * MIN(final_bone2d_node->get_global_scale().x, final_bone2d_node->get_global_scale().y);
	float target_distance = (final_bone2d_node->get_global_position() + (final_bone2d_direction * final_bone2d_length)).distance_to(target->get_global_position());
	chain_iterations = 0;

	while (target_distance > chain_tolarance) {
		chain_backwards();
		chain_forwards();

		final_bone2d_angle = final_bone2d_node->get_global_rotation();
		if (tip_use_target_rotation) {
			final_bone2d_angle = target_global_pose.get_rotation();
		}
		final_bone2d_direction = Vector2(Math::cos(final_bone2d_angle), Math::sin(final_bone2d_angle));
		target_distance = (final_bone2d_node->get_global_position() + (final_bone2d_direction * final_bone2d_length)).distance_to(target->get_global_position());

		chain_iterations += 1;
		if (chain_iterations >= chain_max_iterations) {
			break;
		}
	}

	// Apply all of the saved transforms to the Bone2D nodes
	for (int i = 0; i < fabrik_data_chain.size(); i++) {
		Bone2D *joint_bone2d_node = Object::cast_to<Bone2D>(ObjectDB::get_instance(fabrik_data_chain[i].bone_node_id));
		if (!joint_bone2d_node) {
			ERR_PRINT_ONCE("FABRIK Joint " + itos(i) + " does not have a Bone2D node set!");
			continue;
		}
		Transform2D chain_trans = fabrik_transform_chain[i];

		// Apply rotation
		if (i + 1 < fabrik_data_chain.size()) {
			chain_trans = chain_trans.looking_at(fabrik_transform_chain[i + 1].get_origin());
		} else {
			if (tip_use_target_rotation) {
				chain_trans.set_rotation(target_global_pose.get_rotation());
			} else {
				chain_trans = chain_trans.looking_at(target_global_pose.get_origin());
			}
		}
		// Adjust for the bone angle
		chain_trans.set_rotation(chain_trans.get_rotation() - joint_bone2d_node->get_rotation());

		// Reset scale
		chain_trans.set_scale(joint_bone2d_node->get_global_scale());

		// Apply to the bone, and to the override
		joint_bone2d_node->set_global_transform(chain_trans);
		get_skeleton()->set_bone_pose(fabrik_data_chain[i].bone_idx, joint_bone2d_node->get_transform());
	}
}

void SkeletonIK2DFABR::chain_backwards() {
	int final_joint_index = fabrik_data_chain.size() - 1;
	Bone2D *final_bone2d_node = Object::cast_to<Bone2D>(ObjectDB::get_instance(fabrik_data_chain[final_joint_index].bone_node_id));
	Transform2D final_bone2d_trans = fabrik_transform_chain[final_joint_index];

	// Apply magnet position
	if (final_joint_index != 0) {
		final_bone2d_trans.set_origin(final_bone2d_trans.get_origin() + fabrik_data_chain[final_joint_index].magnet_position);
	}

	// Set the rotation of the tip bone
	final_bone2d_trans = final_bone2d_trans.looking_at(target_global_pose.get_origin());

	// Set the position of the tip bone
	float final_bone2d_angle = final_bone2d_trans.get_rotation();
	if (tip_use_target_rotation) {
		final_bone2d_angle = target_global_pose.get_rotation();
	}
	Vector2 final_bone2d_direction = Vector2(Math::cos(final_bone2d_angle), Math::sin(final_bone2d_angle));
	float final_bone2d_length = final_bone2d_node->get_bone_length() * MIN(final_bone2d_node->get_global_scale().x, final_bone2d_node->get_global_scale().y);
	final_bone2d_trans.set_origin(target_global_pose.get_origin() - (final_bone2d_direction * final_bone2d_length));

	// Save the transform
	fabrik_transform_chain.write[final_joint_index] = final_bone2d_trans;

	int i = final_joint_index;
	while (i >= 1) {
		Transform2D previous_pose = fabrik_transform_chain[i];
		i -= 1;
		Bone2D *current_bone2d_node = Object::cast_to<Bone2D>(ObjectDB::get_instance(fabrik_data_chain[i].bone_node_id));
		Transform2D current_pose = fabrik_transform_chain[i];

		// Apply magnet position
		if (i != 0) {
			current_pose.set_origin(current_pose.get_origin() + fabrik_data_chain[i].magnet_position);
		}

		float current_bone2d_node_length = current_bone2d_node->get_bone_length() * MIN(current_bone2d_node->get_global_scale().x, current_bone2d_node->get_global_scale().y);
		float length = current_bone2d_node_length / (current_pose.get_origin().distance_to(previous_pose.get_origin()));
		Vector2 finish_position = previous_pose.get_origin().lerp(current_pose.get_origin(), length);
		current_pose.set_origin(finish_position);

		// Save the transform
		fabrik_transform_chain.write[i] = current_pose;
	}
}

void SkeletonIK2DFABR::chain_forwards() {
	Transform2D origin_bone2d_trans = fabrik_transform_chain[0];
	origin_bone2d_trans.set_origin(origin_global_pose.get_origin());
	// Save the position
	fabrik_transform_chain.write[0] = origin_bone2d_trans;

	for (int i = 0; i < fabrik_data_chain.size() - 1; i++) {
		Bone2D *current_bone2d_node = Object::cast_to<Bone2D>(ObjectDB::get_instance(fabrik_data_chain[i].bone_node_id));
		Transform2D current_pose = fabrik_transform_chain[i];
		Transform2D next_pose = fabrik_transform_chain[i + 1];

		float current_bone2d_node_length = current_bone2d_node->get_bone_length() * MIN(current_bone2d_node->get_global_scale().x, current_bone2d_node->get_global_scale().y);
		float length = current_bone2d_node_length / (next_pose.get_origin().distance_to(current_pose.get_origin()));
		Vector2 finish_position = current_pose.get_origin().lerp(next_pose.get_origin(), length);
		current_pose.set_origin(finish_position);

		// Apply to the bone
		fabrik_transform_chain.write[i + 1] = current_pose;
	}
}

void SkeletonIK2DFABR::_update_bone_id() {
	for (int i = 0; i < fabrik_data_chain.size(); i++) {
		fabrik_joint_update_node(i);
	}
	update_target_node();
}

void SkeletonIK2DFABR::update_target_node() {
	target_node_id = ObjectID();
	Bone2D *node = Object::cast_to<Bone2D>(get_skeleton()->get_node(target_node));
	target_node_id = node->get_instance_id();
}

void SkeletonIK2DFABR::fabrik_joint_update_node(int p_joint_idx) {
	fabrik_data_chain.write[p_joint_idx].bone_node_id = ObjectID();
	Bone2D *node = Object::cast_to<Bone2D>(get_skeleton()->get_node(fabrik_data_chain[p_joint_idx].bone_node));
	fabrik_data_chain.write[p_joint_idx].bone_node_id = node->get_instance_id();
	fabrik_data_chain.write[p_joint_idx].bone_idx = node->get_bone_idx();
}

void SkeletonIK2DFABR::_skeleton_changed(Skeleton2D *p_old, Skeleton2D *p_new) {
	_update_bone_id();
}
void SkeletonIK2DFABR::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			_update_bone_id();
		} break;
	}
}

void SkeletonIK2DFABR::set_target_node(const NodePath &p_target_node) {
	target_node = p_target_node;
	update_target_node();
}

NodePath SkeletonIK2DFABR::get_target_node() const {
	return target_node;
}

void SkeletonIK2DFABR::set_bone_chain_size(int p_length) {
	fabrik_data_chain.resize(p_length);
}

int SkeletonIK2DFABR::get_bone_chain_size() {
	return fabrik_data_chain.size();
}

void SkeletonIK2DFABR::set_joint_bone(int p_joint_idx, const NodePath &p_bone) {
	ERR_FAIL_INDEX_MSG(p_joint_idx, fabrik_data_chain.size(), "FABRIK joint out of range!");
	fabrik_data_chain.write[p_joint_idx].bone_node = p_bone;
	fabrik_joint_update_node(p_joint_idx);
}

NodePath SkeletonIK2DFABR::get_joint_bone(int p_joint_idx) const {
	ERR_FAIL_INDEX_V_MSG(p_joint_idx, fabrik_data_chain.size(), NodePath(), "FABRIK joint out of range!");
	return fabrik_data_chain[p_joint_idx].bone_node;
}

void SkeletonIK2DFABR::set_joint_bone_idx(int p_joint_idx, int p_bone_idx) {
	ERR_FAIL_INDEX_MSG(p_joint_idx, fabrik_data_chain.size(), "FABRIK joint out of range!");

	if (get_skeleton()) {
		ERR_FAIL_INDEX_MSG(p_bone_idx, get_skeleton()->get_bone_count(), "Passed-in Bone index is out of range!");
		fabrik_data_chain.write[p_joint_idx].bone_idx = p_bone_idx;
		fabrik_data_chain.write[p_joint_idx].bone_node_id = get_skeleton()->get_bone(p_bone_idx)->get_instance_id();
		fabrik_data_chain.write[p_joint_idx].bone_node = get_skeleton()->get_path_to(get_skeleton()->get_bone(p_bone_idx));
	} else {
		WARN_PRINT("Cannot verify the FABRIK joint " + itos(p_joint_idx) + " bone index for this modification...");
		fabrik_data_chain.write[p_joint_idx].bone_idx = p_bone_idx;
	}
}

int SkeletonIK2DFABR::get_joint_bone_idx(int p_joint_idx) const {
	ERR_FAIL_INDEX_V_MSG(p_joint_idx, fabrik_data_chain.size(), -1, "FABRIK joint out of range!");
	return fabrik_data_chain[p_joint_idx].bone_idx;
}

void SkeletonIK2DFABR::set_joint_magnet(int p_joint_idx, Vector2 p_magnet) {
	ERR_FAIL_INDEX_MSG(p_joint_idx, fabrik_data_chain.size(), "FABRIK joint out of range!");
	fabrik_data_chain.write[p_joint_idx].magnet_position = p_magnet;
}

Vector2 SkeletonIK2DFABR::get_joint_magnet(int p_joint_idx) const {
	ERR_FAIL_INDEX_V_MSG(p_joint_idx, fabrik_data_chain.size(), Vector2(), "FABRIK joint out of range!");
	return fabrik_data_chain[p_joint_idx].magnet_position;
}

void SkeletonIK2DFABR::set_tip_use_target_rotation(bool p_use_target_rotation) {
	tip_use_target_rotation = p_use_target_rotation;
}

bool SkeletonIK2DFABR::is_tip_use_target_rotation() const {
	return tip_use_target_rotation;
}

void SkeletonIK2DFABR::set_joint_bones(Array p_node_paths) {
	set_bone_chain_size(p_node_paths.size());
	for (int i = 0; i < p_node_paths.size(); i++) {
		set_joint_bone(i, p_node_paths[i]);
	}
	_update_bone_id();
}
Array SkeletonIK2DFABR::get_joint_bones() const {
	Array ret;
	for (int i = 0; i < fabrik_data_chain.size(); i++) {
		ret.push_back(get_joint_bone(i));
	}
	return ret;
}
void SkeletonIK2DFABR::set_joint_magnets(PackedVector2Array p_magnets) {
	int size = MIN(get_bone_chain_size(), p_magnets.size());
	for (int i = 0; i < size; i++) {
		set_joint_magnet(i, p_magnets[i]);
	}
}
PackedVector2Array SkeletonIK2DFABR::get_joint_magnets() const {
	PackedVector2Array ret;
	for (int i = 0; i < fabrik_data_chain.size(); i++) {
		ret.push_back(get_joint_magnet(i));
	}
	return ret;
}
void SkeletonIK2DFABR::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_target_node", "p_target_node"), &SkeletonIK2DFABR::set_target_node);
	ClassDB::bind_method(D_METHOD("get_target_node"), &SkeletonIK2DFABR::get_target_node);
	ClassDB::bind_method(D_METHOD("get_bone_chain_size"), &SkeletonIK2DFABR::get_bone_chain_size);
	ClassDB::bind_method(D_METHOD("set_bone_chain_size", "p_size"), &SkeletonIK2DFABR::set_bone_chain_size);
	ClassDB::bind_method(D_METHOD("set_joint_bone", "p_joint_idx", "p_bone"), &SkeletonIK2DFABR::set_joint_bone);
	ClassDB::bind_method(D_METHOD("get_joint_bone", "p_joint_idx"), &SkeletonIK2DFABR::get_joint_bone);
	ClassDB::bind_method(D_METHOD("set_joint_bone_idx", "p_joint_idx", "p_bone_idx"), &SkeletonIK2DFABR::set_joint_bone_idx);
	ClassDB::bind_method(D_METHOD("get_joint_bone_idx", "p_joint_idx"), &SkeletonIK2DFABR::get_joint_bone_idx);
	ClassDB::bind_method(D_METHOD("set_joint_magnet", "p_joint_idx", "p_magnet_position"), &SkeletonIK2DFABR::set_joint_magnet);
	ClassDB::bind_method(D_METHOD("get_joint_magnet", "p_joint_idx"), &SkeletonIK2DFABR::get_joint_magnet);
	ClassDB::bind_method(D_METHOD("set_tip_use_target_rotation", "p_tip_use_target_rotation"), &SkeletonIK2DFABR::set_tip_use_target_rotation);
	ClassDB::bind_method(D_METHOD("is_tip_use_target_rotation"), &SkeletonIK2DFABR::is_tip_use_target_rotation);
	ClassDB::bind_method(D_METHOD("set_joint_bones", "p_node_paths"), &SkeletonIK2DFABR::set_joint_bones);
	ClassDB::bind_method(D_METHOD("get_joint_bones"), &SkeletonIK2DFABR::get_joint_bones);
	ClassDB::bind_method(D_METHOD("set_joint_magnets", "p_magnets"), &SkeletonIK2DFABR::set_joint_magnets);
	ClassDB::bind_method(D_METHOD("get_joint_magnets"), &SkeletonIK2DFABR::get_joint_magnets);

	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "target_node", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Bone2D"), "set_target_node", "get_target_node");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "bone_chain_size", PROPERTY_HINT_RANGE, "0, 100, 1"), "set_bone_chain_size", "get_bone_chain_size");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "tip_use_target_rotation"), "set_tip_use_target_rotation", "is_tip_use_target_rotation");

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "joint_bones", PROPERTY_HINT_ARRAY_TYPE, "NodePath"), "set_joint_bones", "get_joint_bones");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR2_ARRAY, "joint_magnets"), "set_joint_magnets", "get_joint_magnets");
}
