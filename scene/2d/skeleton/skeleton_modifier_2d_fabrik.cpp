/**************************************************************************/
/*  skeleton_modifier_2d_fabrik.cpp                                       */
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

#include "skeleton_modifier_2d_fabrik.h"
#include "scene/2d/skeleton_2d.h"

void SkeletonModifier2DFABRIK::_setup_modification() {
	_update_bones_id();
}

void SkeletonModifier2DFABRIK::_process_modification(real_t p_delta) {
	if (!is_active() || get_skeleton() == nullptr || !get_skeleton()->is_inside_tree()) {
		ERR_PRINT_ONCE("SkeletonIk2DFABR is not setup. Cannot execute modification!");
		return;
	}
	if (!has_node(target_node)) {
		ERR_PRINT_ONCE("Target node " + target_node + " not found");
		return;
	}

	if (target_node_cache.is_null()) {
		update_target_node();
		return;
	}

	if (fabrik_data_chain.size() <= 1) {
		ERR_PRINT_ONCE("FABRIK requires at least two joints to operate! Cannot execute modification!");
		return;
	}

	Node2D *target = Object::cast_to<Node2D>(ObjectDB::get_instance(target_node_cache));
	if (!target || !target->is_inside_tree()) {
		ERR_PRINT_ONCE("Target node is not in the scene tree. Cannot execute modification!");
		return;
	}
	target_global_pose = target->get_global_transform();

	if (fabrik_data_chain[0].bone_node_cache.is_null() && !fabrik_data_chain[0].bone_node.is_empty()) {
		fabrik_joint_update_node(0);
		return;
	}

	Bone2D *origin_bone2d_node = Object::cast_to<Bone2D>(ObjectDB::get_instance(fabrik_data_chain[0].bone_node_cache));
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
		if (fabrik_data_chain[i].bone_node_cache.is_null() && !fabrik_data_chain[i].bone_node.is_empty()) {
			WARN_PRINT_ONCE("Bone2D cache for joint " + itos(i) + " is out of date.. Attempting to update...");
			fabrik_joint_update_node(i);
		}
		Bone2D *joint_bone2d_node = Object::cast_to<Bone2D>(ObjectDB::get_instance(fabrik_data_chain[i].bone_node_cache));
		if (!joint_bone2d_node) {
			ERR_PRINT_ONCE("FABRIK Joint " + itos(i) + " does not have a Bone2D node set! Cannot execute modification!");
			return;
		}
		fabrik_transform_chain.write[i] = joint_bone2d_node->get_global_transform();
	}

	Bone2D *final_bone2d_node = Object::cast_to<Bone2D>(ObjectDB::get_instance(fabrik_data_chain[fabrik_data_chain.size() - 1].bone_node_cache));
	float final_bone2d_angle = tip_use_target_rotation ? target_global_pose.get_rotation() : final_bone2d_node->get_global_rotation();
	Vector2 final_bone2d_direction = Vector2::from_angle(final_bone2d_angle);
	float final_bone2d_length = final_bone2d_node->get_length() * MIN(final_bone2d_node->get_global_scale().x, final_bone2d_node->get_global_scale().y);
	;
	float target_distance = (final_bone2d_node->get_global_position() + (final_bone2d_direction * final_bone2d_length)).distance_to(target->get_global_position());
	chain_iterations = 0;

	while (target_distance > chain_tolarance) {
		chain_backwards();
		chain_forwards();

		final_bone2d_angle = tip_use_target_rotation ? target_global_pose.get_rotation() : final_bone2d_node->get_global_rotation();

		final_bone2d_direction = Vector2::from_angle(final_bone2d_angle);
		target_distance = (final_bone2d_node->get_global_position() + (final_bone2d_direction * final_bone2d_length)).distance_to(target->get_global_position());

		chain_iterations += 1;
		if (chain_iterations >= chain_max_iterations) {
			break;
		}
	}

	// Apply all of the saved transforms to the Bone2D nodes
	for (int i = 0; i < fabrik_data_chain.size(); i++) {
		Bone2D *joint_bone2d_node = Object::cast_to<Bone2D>(ObjectDB::get_instance(fabrik_data_chain[i].bone_node_cache));
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
		chain_trans.set_rotation(chain_trans.get_rotation() - joint_bone2d_node->get_bone_angle());

		// Reset scale
		chain_trans.set_scale(joint_bone2d_node->get_global_scale());

		// Apply to the bone, and to the override
		joint_bone2d_node->set_global_transform(chain_trans);
		get_skeleton()->set_bone_local_pose_override(fabrik_data_chain[i].bone_idx, joint_bone2d_node->get_transform(), get_influence(), true);
	}
}

void SkeletonModifier2DFABRIK::chain_backwards() {
	int final_joint_index = fabrik_data_chain.size() - 1;
	Bone2D *final_bone2d_node = Object::cast_to<Bone2D>(ObjectDB::get_instance(fabrik_data_chain[final_joint_index].bone_node_cache));
	Transform2D final_bone2d_trans = fabrik_transform_chain[final_joint_index];

	// Apply magnet position
	if (final_joint_index != 0) {
		final_bone2d_trans.set_origin(final_bone2d_trans.get_origin() + fabrik_data_chain[final_joint_index].magnet);
	}

	// Set the rotation of the tip bone
	final_bone2d_trans = final_bone2d_trans.looking_at(target_global_pose.get_origin());

	// Set the position of the tip bone
	float final_bone2d_angle = tip_use_target_rotation ? target_global_pose.get_rotation() : final_bone2d_trans.get_rotation();

	Vector2 final_bone2d_direction = Vector2::from_angle(final_bone2d_angle);
	float final_bone2d_length = final_bone2d_node->get_length() * MIN(final_bone2d_node->get_global_scale().x, final_bone2d_node->get_global_scale().y);
	;
	final_bone2d_trans.set_origin(target_global_pose.get_origin() - (final_bone2d_direction * final_bone2d_length));

	// Save the transform
	fabrik_transform_chain.write[final_joint_index] = final_bone2d_trans;

	int i = final_joint_index;
	while (i >= 1) {
		Transform2D previous_pose = fabrik_transform_chain[i];
		i -= 1;
		Bone2D *current_bone2d_node = Object::cast_to<Bone2D>(ObjectDB::get_instance(fabrik_data_chain[i].bone_node_cache));
		Transform2D current_pose = fabrik_transform_chain[i];

		// Apply magnet position
		if (i != 0) {
			current_pose.set_origin(current_pose.get_origin() + fabrik_data_chain[i].magnet);
		}

		float current_bone2d_node_length = current_bone2d_node->get_length() * MIN(current_bone2d_node->get_global_scale().x, current_bone2d_node->get_global_scale().y);
		;
		float length = current_bone2d_node_length / (current_pose.get_origin().distance_to(previous_pose.get_origin()));
		Vector2 finish_position = previous_pose.get_origin().lerp(current_pose.get_origin(), length);
		current_pose.set_origin(finish_position);

		// Save the transform
		fabrik_transform_chain.write[i] = current_pose;
	}
}

void SkeletonModifier2DFABRIK::chain_forwards() {
	Transform2D origin_bone2d_trans = fabrik_transform_chain[0];
	origin_bone2d_trans.set_origin(origin_global_pose.get_origin());
	// Save the position
	fabrik_transform_chain.write[0] = origin_bone2d_trans;

	for (int i = 0; i < fabrik_data_chain.size() - 1; i++) {
		Bone2D *current_bone2d_node = Object::cast_to<Bone2D>(ObjectDB::get_instance(fabrik_data_chain[i].bone_node_cache));
		Transform2D current_pose = fabrik_transform_chain[i];
		Transform2D next_pose = fabrik_transform_chain[i + 1];

		float current_bone2d_node_length = current_bone2d_node->get_length() * MIN(current_bone2d_node->get_global_scale().x, current_bone2d_node->get_global_scale().y);
		;
		float length = current_bone2d_node_length / (next_pose.get_origin().distance_to(current_pose.get_origin()));
		Vector2 finish_position = current_pose.get_origin().lerp(next_pose.get_origin(), length);
		current_pose.set_origin(finish_position);

		// Apply to the bone
		fabrik_transform_chain.write[i + 1] = current_pose;
	}
}

void SkeletonModifier2DFABRIK::_update_bones_id() {
	for (int i = 0; i < fabrik_data_chain.size(); i++) {
		fabrik_joint_update_node(i);
	}
	update_target_node();
}

void SkeletonModifier2DFABRIK::update_target_node() {
	target_node_cache = ObjectID();
	ERR_FAIL_COND_MSG(!has_node(target_node), "Target node " + target_node + " not found");
	Node2D *node = Object::cast_to<Node2D>(get_node(target_node));
	ERR_FAIL_COND_MSG(node == nullptr, "Target node is not a Node2D");
	target_node_cache = node->get_instance_id();
}

void SkeletonModifier2DFABRIK::fabrik_joint_update_node(int p_joint_idx) {
	fabrik_data_chain.write[p_joint_idx].bone_node_cache = ObjectID();
	NodePath node_path = fabrik_data_chain[p_joint_idx].bone_node;
	ERR_FAIL_COND_MSG(get_skeleton() == nullptr || !get_skeleton()->is_inside_tree(), "Skeleton is not in scene tree");
	ERR_FAIL_COND_MSG(!has_node(node_path), "FABRIK joint node " + node_path + " not found in skeleton");

	Bone2D *node = Object::cast_to<Bone2D>(get_node(node_path));
	ERR_FAIL_COND_MSG(node == nullptr, "FABRIK joint node is not a Bone2D");
	fabrik_data_chain.write[p_joint_idx].bone_node_cache = node->get_instance_id();
	fabrik_data_chain.write[p_joint_idx].bone_idx = node->get_index_in_skeleton();
}

void SkeletonModifier2DFABRIK::set_target_node(const NodePath &p_target_node) {
	target_node = p_target_node;
	if (get_skeleton() && get_skeleton()->is_inside_tree()) {
		update_target_node();
	}
}

NodePath SkeletonModifier2DFABRIK::get_target_node() const {
	return target_node;
}

void SkeletonModifier2DFABRIK::set_bone_chain_size(int p_size) {
	fabrik_data_chain.resize(p_size);
}

int SkeletonModifier2DFABRIK::get_bone_chain_size() {
	return fabrik_data_chain.size();
}

void SkeletonModifier2DFABRIK::set_joint_bone(int p_joint_idx, const NodePath &p_bone) {
	ERR_FAIL_INDEX_MSG(p_joint_idx, fabrik_data_chain.size(), "FABRIK joint out of range!");
	fabrik_data_chain.write[p_joint_idx].bone_node = p_bone;
	if (get_skeleton() && get_skeleton()->is_inside_tree()) {
		fabrik_joint_update_node(p_joint_idx);
	}
}

NodePath SkeletonModifier2DFABRIK::get_joint_bone(int p_joint_idx) const {
	ERR_FAIL_INDEX_V_MSG(p_joint_idx, fabrik_data_chain.size(), NodePath(), "FABRIK joint out of range!");
	return fabrik_data_chain[p_joint_idx].bone_node;
}

void SkeletonModifier2DFABRIK::set_joint_bone_idx(int p_joint_idx, int p_bone_idx) {
	ERR_FAIL_INDEX_MSG(p_joint_idx, fabrik_data_chain.size(), "FABRIK joint out of range!");

	if (get_skeleton()) {
		ERR_FAIL_INDEX_MSG(p_bone_idx, get_skeleton()->get_bone_count(), "Passed-in Bone index is out of range!");
		fabrik_data_chain.write[p_joint_idx].bone_idx = p_bone_idx;
		fabrik_data_chain.write[p_joint_idx].bone_node_cache = get_skeleton()->get_bone(p_bone_idx)->get_instance_id();
		fabrik_data_chain.write[p_joint_idx].bone_node = get_skeleton()->get_path_to(get_skeleton()->get_bone(p_bone_idx));
	} else {
		WARN_PRINT("Cannot verify the FABRIK joint " + itos(p_joint_idx) + " bone index for this modification...");
		fabrik_data_chain.write[p_joint_idx].bone_idx = p_bone_idx;
	}
}

int SkeletonModifier2DFABRIK::get_joint_bone_idx(int p_joint_idx) const {
	ERR_FAIL_INDEX_V_MSG(p_joint_idx, fabrik_data_chain.size(), -1, "FABRIK joint out of range!");
	return fabrik_data_chain[p_joint_idx].bone_idx;
}

void SkeletonModifier2DFABRIK::set_joint_magnet(int p_joint_idx, Vector2 p_magnet) {
	ERR_FAIL_INDEX_MSG(p_joint_idx, fabrik_data_chain.size(), "FABRIK joint out of range!");
	fabrik_data_chain.write[p_joint_idx].magnet = p_magnet;
}

Vector2 SkeletonModifier2DFABRIK::get_joint_magnet(int p_joint_idx) const {
	ERR_FAIL_INDEX_V_MSG(p_joint_idx, fabrik_data_chain.size(), Vector2(), "FABRIK joint out of range!");
	return fabrik_data_chain[p_joint_idx].magnet;
}

void SkeletonModifier2DFABRIK::set_tip_use_target_rotation(bool p_use_target_rotation) {
	tip_use_target_rotation = p_use_target_rotation;
}

bool SkeletonModifier2DFABRIK::is_tip_use_target_rotation() const {
	return tip_use_target_rotation;
}

void SkeletonModifier2DFABRIK::set_all_bones(TypedArray<NodePath> p_node_paths) {
	set_bone_chain_size(p_node_paths.size());
	for (int i = 0; i < p_node_paths.size(); i++) {
		set_joint_bone(i, p_node_paths[i]);
	}
}
TypedArray<NodePath> SkeletonModifier2DFABRIK::get_all_bones() const {
	TypedArray<NodePath> ret;
	for (int i = 0; i < fabrik_data_chain.size(); i++) {
		ret.push_back(get_joint_bone(i));
	}
	return ret;
}

void SkeletonModifier2DFABRIK::set_all_magnets(PackedVector2Array p_magnets) {
	int size = MIN(get_bone_chain_size(), p_magnets.size());
	for (int i = 0; i < size; i++) {
		set_joint_magnet(i, p_magnets[i]);
	}
}
PackedVector2Array SkeletonModifier2DFABRIK::get_all_magnets() const {
	PackedVector2Array ret;
	for (int i = 0; i < fabrik_data_chain.size(); i++) {
		ret.push_back(get_joint_magnet(i));
	}
	return ret;
}
void SkeletonModifier2DFABRIK::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_target_node", "p_target_node"), &SkeletonModifier2DFABRIK::set_target_node);
	ClassDB::bind_method(D_METHOD("get_target_node"), &SkeletonModifier2DFABRIK::get_target_node);
	ClassDB::bind_method(D_METHOD("get_bone_chain_size"), &SkeletonModifier2DFABRIK::get_bone_chain_size);
	ClassDB::bind_method(D_METHOD("set_bone_chain_size", "p_size"), &SkeletonModifier2DFABRIK::set_bone_chain_size);
	ClassDB::bind_method(D_METHOD("set_joint_bone", "p_joint_idx", "p_bone"), &SkeletonModifier2DFABRIK::set_joint_bone);
	ClassDB::bind_method(D_METHOD("get_joint_bone", "p_joint_idx"), &SkeletonModifier2DFABRIK::get_joint_bone);
	ClassDB::bind_method(D_METHOD("set_joint_bone_idx", "p_joint_idx", "p_bone_idx"), &SkeletonModifier2DFABRIK::set_joint_bone_idx);
	ClassDB::bind_method(D_METHOD("get_joint_bone_idx", "p_joint_idx"), &SkeletonModifier2DFABRIK::get_joint_bone_idx);
	ClassDB::bind_method(D_METHOD("set_joint_magnet", "p_joint_idx", "p_magnet_position"), &SkeletonModifier2DFABRIK::set_joint_magnet);
	ClassDB::bind_method(D_METHOD("get_joint_magnet", "p_joint_idx"), &SkeletonModifier2DFABRIK::get_joint_magnet);
	ClassDB::bind_method(D_METHOD("set_tip_use_target_rotation", "p_tip_use_target_rotation"), &SkeletonModifier2DFABRIK::set_tip_use_target_rotation);
	ClassDB::bind_method(D_METHOD("is_tip_use_target_rotation"), &SkeletonModifier2DFABRIK::is_tip_use_target_rotation);
	ClassDB::bind_method(D_METHOD("set_all_bones", "p_node_paths"), &SkeletonModifier2DFABRIK::set_all_bones);
	ClassDB::bind_method(D_METHOD("get_all_bones"), &SkeletonModifier2DFABRIK::get_all_bones);
	ClassDB::bind_method(D_METHOD("set_all_magnets", "p_magnets"), &SkeletonModifier2DFABRIK::set_all_magnets);
	ClassDB::bind_method(D_METHOD("get_all_magnets"), &SkeletonModifier2DFABRIK::get_all_magnets);

	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "target_node", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Node2D"), "set_target_node", "get_target_node");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "bone_chain_size", PROPERTY_HINT_RANGE, "0, 100, 1"), "set_bone_chain_size", "get_bone_chain_size");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "tip_use_target_rotation"), "set_tip_use_target_rotation", "is_tip_use_target_rotation");

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "all_bones", PROPERTY_HINT_ARRAY_TYPE, "NodePath"), "set_all_bones", "get_all_bones");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR2_ARRAY, "all_magnets"), "set_all_magnets", "get_all_magnets");
}
