/**************************************************************************/
/*  skeleton_modifier_2d_jiggle.cpp                                       */
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

#include "skeleton_modifier_2d_jiggle.h"

#include "scene/2d/skeleton_2d.h"
#include "scene/resources/world_2d.h"

void SkeletonModifier2DJiggle::_process_modification(real_t p_delta) {
	ERR_FAIL_COND_MSG(!is_active() || get_skeleton() == nullptr,
			"Modification is not setup and therefore cannot execute!");
	if (target_node_cache.is_null()) {
		update_target_cache();
		return;
	}
	Node2D *target = Object::cast_to<Node2D>(ObjectDB::get_instance(target_node_cache));
	if (!target || !target->is_inside_tree()) {
		ERR_PRINT_ONCE("Target node is not in the scene tree. Cannot execute modification!");
		return;
	}

	for (int i = 0; i < jiggle_data_chain.size(); i++) {
		_execute_jiggle_joint(i, target, p_delta);
	}
}

void SkeletonModifier2DJiggle::_execute_jiggle_joint(int p_joint_idx, Node2D *p_target, float p_delta) {
	// Adopted from: https://wiki.unity3d.com/index.php/JiggleBone
	// With modifications by TwistedTwigleg.

	if (jiggle_data_chain[p_joint_idx].bone_idx <= -1 || jiggle_data_chain[p_joint_idx].bone_idx > get_skeleton()->get_bone_count()) {
		ERR_PRINT_ONCE("Jiggle joint " + itos(p_joint_idx) + " bone index is invalid. Cannot execute modification on joint...");
		return;
	}

	if (jiggle_data_chain[p_joint_idx].bone2d_node_cache.is_null() && !jiggle_data_chain[p_joint_idx].bone2d_node.is_empty()) {
		WARN_PRINT_ONCE("Bone2D cache for joint " + itos(p_joint_idx) + " is out of date. Updating...");
		jiggle_joint_update_bone2d_cache(p_joint_idx);
	}

	Bone2D *operation_bone = get_skeleton()->get_bone(jiggle_data_chain[p_joint_idx].bone_idx);
	if (!operation_bone) {
		ERR_PRINT_ONCE("Jiggle joint " + itos(p_joint_idx) + " does not have a Bone2D node or it cannot be found!");
		return;
	}

	Transform2D operation_bone_trans = operation_bone->get_global_transform();
	Vector2 target_position = p_target->get_global_position();

	jiggle_data_chain.write[p_joint_idx].force = (target_position - jiggle_data_chain[p_joint_idx].dynamic_position) * get_joint_stiffness(p_joint_idx) * p_delta;

	if (get_joint_use_gravity(p_joint_idx)) {
		jiggle_data_chain.write[p_joint_idx].force += get_joint_gravity(p_joint_idx) * p_delta;
	}

	jiggle_data_chain.write[p_joint_idx].acceleration = jiggle_data_chain[p_joint_idx].force / get_joint_mass(p_joint_idx);
	jiggle_data_chain.write[p_joint_idx].velocity += jiggle_data_chain[p_joint_idx].acceleration * (1 - get_joint_damping(p_joint_idx));

	jiggle_data_chain.write[p_joint_idx].dynamic_position += jiggle_data_chain[p_joint_idx].velocity + jiggle_data_chain[p_joint_idx].force;
	jiggle_data_chain.write[p_joint_idx].dynamic_position += operation_bone_trans.get_origin() - jiggle_data_chain[p_joint_idx].last_position;
	jiggle_data_chain.write[p_joint_idx].last_position = operation_bone_trans.get_origin();

	// Collision detection/response
	if (use_colliders) {
		if (get_execution_mode() == SkeletonModifier2D::ExecutionMode::EXECUTION_MODE_PROCESS_PHYSICS) {
			Ref<World2D> world_2d = get_skeleton()->get_world_2d();
			ERR_FAIL_COND(world_2d.is_null());
			PhysicsDirectSpaceState2D *space_state = PhysicsServer2D::get_singleton()->space_get_direct_state(world_2d->get_space());
			PhysicsDirectSpaceState2D::RayResult ray_result;

			PhysicsDirectSpaceState2D::RayParameters ray_params;
			ray_params.from = operation_bone_trans.get_origin();
			ray_params.to = jiggle_data_chain[p_joint_idx].dynamic_position;
			ray_params.collision_mask = collision_mask;

			// Add exception support?
			bool ray_hit = space_state->intersect_ray(ray_params, ray_result);

			if (ray_hit) {
				jiggle_data_chain.write[p_joint_idx].dynamic_position = jiggle_data_chain[p_joint_idx].last_noncollision_position;
				jiggle_data_chain.write[p_joint_idx].acceleration = Vector2(0, 0);
				jiggle_data_chain.write[p_joint_idx].velocity = Vector2(0, 0);
			} else {
				jiggle_data_chain.write[p_joint_idx].last_noncollision_position = jiggle_data_chain[p_joint_idx].dynamic_position;
			}
		} else {
			WARN_PRINT_ONCE("Jiggle 2D modifier: You cannot detect colliders without the stack mode being set to _physics_process!");
		}
	}

	// Rotate the bone using the dynamic position!
	operation_bone_trans = operation_bone_trans.looking_at(jiggle_data_chain[p_joint_idx].dynamic_position);
	operation_bone_trans.set_rotation(operation_bone_trans.get_rotation() - operation_bone->get_bone_angle());

	// Reset scale
	operation_bone_trans.set_scale(operation_bone->get_global_scale());

	operation_bone->set_global_transform(operation_bone_trans);
	get_skeleton()->set_bone_local_pose_override(jiggle_data_chain[p_joint_idx].bone_idx, operation_bone->get_transform(), get_influence(), true);
}

void SkeletonModifier2DJiggle::_setup_modification() {
	for (int i = 0; i < jiggle_data_chain.size(); i++) {
		int bone_idx = jiggle_data_chain[i].bone_idx;
		if (bone_idx > 0 && bone_idx < get_skeleton()->get_bone_count()) {
			Bone2D *bone2d_node = get_skeleton()->get_bone(bone_idx);
			jiggle_data_chain.write[i].dynamic_position = bone2d_node->get_global_position();
		}

		jiggle_joint_update_bone2d_cache(i);
	}

	update_target_cache();
}

void SkeletonModifier2DJiggle::update_target_cache() {
	target_node_cache = ObjectID();
	ERR_FAIL_COND_MSG(get_skeleton() == nullptr || !get_skeleton()->is_inside_tree(), "Skeleton is not in scene tree");
	ERR_FAIL_COND_MSG(!has_node(target_node), "Jiggle joint node " + target_node + " not found in skeleton");
	Node2D *node = Object::cast_to<Node2D>(get_node(target_node));
	ERR_FAIL_COND_MSG(node == nullptr, "Jiggle joint node is not a Bone2D");
	target_node_cache = node->get_instance_id();
}

void SkeletonModifier2DJiggle::jiggle_joint_update_bone2d_cache(int p_joint_idx) {
	ERR_FAIL_INDEX_MSG(p_joint_idx, jiggle_data_chain.size(), "Cannot update bone2d cache: joint index out of range!");

	jiggle_data_chain.write[p_joint_idx].bone2d_node_cache = ObjectID();
	ERR_FAIL_COND_MSG(get_skeleton() == nullptr || !get_skeleton()->is_inside_tree(), "Skeleton is not in scene tree");
	ERR_FAIL_COND_MSG(!has_node(jiggle_data_chain[p_joint_idx].bone2d_node), "Jiggle joint node " + jiggle_data_chain[p_joint_idx].bone2d_node + " not found in skeleton");
	Bone2D *node = Object::cast_to<Bone2D>(get_node(jiggle_data_chain[p_joint_idx].bone2d_node));
	ERR_FAIL_COND_MSG(node == nullptr, "Jiggle joint node is not a Bone2D");
	jiggle_data_chain.write[p_joint_idx].bone2d_node_cache = node->get_instance_id();
	jiggle_data_chain.write[p_joint_idx].bone_idx = node->get_index_in_skeleton();
}

void SkeletonModifier2DJiggle::set_target_node(const NodePath &p_target_node) {
	target_node = p_target_node;
	if (get_skeleton() && get_skeleton()->is_inside_tree()) {
		update_target_cache();
	}
}

NodePath SkeletonModifier2DJiggle::get_target_node() const {
	return target_node;
}

void SkeletonModifier2DJiggle::set_stiffness(float p_stiffness) {
	ERR_FAIL_COND_MSG(p_stiffness < 0, "Stiffness cannot be set to a negative value!");
	stiffness = p_stiffness;
}

float SkeletonModifier2DJiggle::get_stiffness() const {
	return stiffness;
}

void SkeletonModifier2DJiggle::set_mass(float p_mass) {
	ERR_FAIL_COND_MSG(p_mass <= 0, "Mass cannot be set to a <= 0 value!");
	mass = p_mass;
}

float SkeletonModifier2DJiggle::get_mass() const {
	return mass;
}

void SkeletonModifier2DJiggle::set_damping(float p_damping) {
	ERR_FAIL_COND_MSG(p_damping < 0, "Damping cannot be set to a negative value!");
	ERR_FAIL_COND_MSG(p_damping > 1, "Damping cannot be more than one!");
	damping = p_damping;
}

float SkeletonModifier2DJiggle::get_damping() const {
	return damping;
}

void SkeletonModifier2DJiggle::set_use_gravity(bool p_use_gravity) {
	use_gravity = p_use_gravity;
}

bool SkeletonModifier2DJiggle::get_use_gravity() const {
	return use_gravity;
}

void SkeletonModifier2DJiggle::set_gravity(Vector2 p_gravity) {
	gravity = p_gravity;
}

Vector2 SkeletonModifier2DJiggle::get_gravity() const {
	return gravity;
}

void SkeletonModifier2DJiggle::set_use_colliders(bool p_use_colliders) {
	use_colliders = p_use_colliders;
}

bool SkeletonModifier2DJiggle::get_use_colliders() const {
	return use_colliders;
}

void SkeletonModifier2DJiggle::set_collision_mask(int p_mask) {
	collision_mask = p_mask;
}

int SkeletonModifier2DJiggle::get_collision_mask() const {
	return collision_mask;
}

// Jiggle joint data functions
int SkeletonModifier2DJiggle::get_bone_chain_size() {
	return jiggle_data_chain.size();
}

void SkeletonModifier2DJiggle::set_bone_chain_size(int p_size) {
	ERR_FAIL_COND(p_size < 0);
	jiggle_data_chain.resize(p_size);
}

void SkeletonModifier2DJiggle::set_joint_bone(int p_joint_idx, const NodePath &p_target_node) {
	ERR_FAIL_INDEX_MSG(p_joint_idx, jiggle_data_chain.size(), "Jiggle joint out of range!");
	jiggle_data_chain.write[p_joint_idx].bone2d_node = p_target_node;
	if (get_skeleton() && get_skeleton()->is_inside_tree()) {
		jiggle_joint_update_bone2d_cache(p_joint_idx);
	}
}

NodePath SkeletonModifier2DJiggle::get_joint_bone(int p_joint_idx) const {
	ERR_FAIL_INDEX_V_MSG(p_joint_idx, jiggle_data_chain.size(), NodePath(), "Jiggle joint out of range!");
	return jiggle_data_chain[p_joint_idx].bone2d_node;
}

void SkeletonModifier2DJiggle::set_joint_bone_idx(int p_joint_idx, int p_bone_idx) {
	ERR_FAIL_INDEX_MSG(p_joint_idx, jiggle_data_chain.size(), "Jiggle joint out of range!");
	ERR_FAIL_COND_MSG(p_bone_idx < 0, "Bone index is out of range: The index is too low!");

	if (get_skeleton()) {
		ERR_FAIL_INDEX_MSG(p_bone_idx, get_skeleton()->get_bone_count(), "Passed-in Bone index is out of range!");
		jiggle_data_chain.write[p_joint_idx].bone_idx = p_bone_idx;
		jiggle_data_chain.write[p_joint_idx].bone2d_node_cache = get_skeleton()->get_bone(p_bone_idx)->get_instance_id();
		jiggle_data_chain.write[p_joint_idx].bone2d_node = get_skeleton()->get_path_to(get_skeleton()->get_bone(p_bone_idx));
	} else {
		WARN_PRINT("Cannot verify the Jiggle joint " + itos(p_joint_idx) + " bone index for this modification...");
		jiggle_data_chain.write[p_joint_idx].bone_idx = p_bone_idx;
	}
}

int SkeletonModifier2DJiggle::get_joint_bone_idx(int p_joint_idx) const {
	ERR_FAIL_INDEX_V_MSG(p_joint_idx, jiggle_data_chain.size(), -1, "Jiggle joint out of range!");
	return jiggle_data_chain[p_joint_idx].bone_idx;
}

void SkeletonModifier2DJiggle::set_joint_override(int p_joint_idx, bool p_override) {
	ERR_FAIL_INDEX(p_joint_idx, jiggle_data_chain.size());
	jiggle_data_chain.write[p_joint_idx].override_defaults = p_override;
}

bool SkeletonModifier2DJiggle::get_joint_override(int p_joint_idx) const {
	ERR_FAIL_INDEX_V(p_joint_idx, jiggle_data_chain.size(), false);
	return jiggle_data_chain[p_joint_idx].override_defaults;
}

void SkeletonModifier2DJiggle::set_joint_stiffness(int p_joint_idx, float p_stiffness) {
	if (!jiggle_data_chain[p_joint_idx].override_defaults) {
		return;
	}
	ERR_FAIL_COND_MSG(p_stiffness < 0, "Stiffness cannot be set to a negative value!");
	ERR_FAIL_INDEX(p_joint_idx, jiggle_data_chain.size());
	jiggle_data_chain.write[p_joint_idx].stiffness = p_stiffness;
}

float SkeletonModifier2DJiggle::get_joint_stiffness(int p_joint_idx) const {
	ERR_FAIL_INDEX_V(p_joint_idx, jiggle_data_chain.size(), -1);
	return jiggle_data_chain[p_joint_idx].override_defaults ? jiggle_data_chain[p_joint_idx].stiffness : get_stiffness();
}

void SkeletonModifier2DJiggle::set_joint_mass(int p_joint_idx, float p_mass) {
	if (!jiggle_data_chain[p_joint_idx].override_defaults) {
		return;
	}
	ERR_FAIL_COND_MSG(p_mass <= 0, "Mass cannot be set to a <= 0 value!");
	ERR_FAIL_INDEX(p_joint_idx, jiggle_data_chain.size());
	jiggle_data_chain.write[p_joint_idx].mass = p_mass;
}

float SkeletonModifier2DJiggle::get_joint_mass(int p_joint_idx) const {
	ERR_FAIL_INDEX_V(p_joint_idx, jiggle_data_chain.size(), -1);
	return jiggle_data_chain[p_joint_idx].override_defaults ? jiggle_data_chain[p_joint_idx].mass : get_mass();
}

void SkeletonModifier2DJiggle::set_joint_damping(int p_joint_idx, float p_damping) {
	if (!jiggle_data_chain[p_joint_idx].override_defaults) {
		return;
	}
	ERR_FAIL_COND_MSG(p_damping < 0, "Damping cannot be set to a negative value!");
	ERR_FAIL_INDEX(p_joint_idx, jiggle_data_chain.size());
	jiggle_data_chain.write[p_joint_idx].damping = p_damping;
}

float SkeletonModifier2DJiggle::get_joint_damping(int p_joint_idx) const {
	ERR_FAIL_INDEX_V(p_joint_idx, jiggle_data_chain.size(), -1);
	return jiggle_data_chain[p_joint_idx].override_defaults ? jiggle_data_chain[p_joint_idx].damping : get_damping();
}

void SkeletonModifier2DJiggle::set_joint_use_gravity(int p_joint_idx, bool p_use_gravity) {
	if (!jiggle_data_chain[p_joint_idx].override_defaults) {
		return;
	}
	ERR_FAIL_INDEX(p_joint_idx, jiggle_data_chain.size());
	jiggle_data_chain.write[p_joint_idx].use_gravity = p_use_gravity;
}

bool SkeletonModifier2DJiggle::get_joint_use_gravity(int p_joint_idx) const {
	ERR_FAIL_INDEX_V(p_joint_idx, jiggle_data_chain.size(), false);
	return jiggle_data_chain[p_joint_idx].override_defaults ? jiggle_data_chain[p_joint_idx].use_gravity : get_use_gravity();
}

void SkeletonModifier2DJiggle::set_joint_gravity(int p_joint_idx, Vector2 p_gravity) {
	if (!jiggle_data_chain[p_joint_idx].override_defaults) {
		return;
	}
	ERR_FAIL_INDEX(p_joint_idx, jiggle_data_chain.size());
	jiggle_data_chain.write[p_joint_idx].gravity = p_gravity;
}

Vector2 SkeletonModifier2DJiggle::get_joint_gravity(int p_joint_idx) const {
	ERR_FAIL_INDEX_V(p_joint_idx, jiggle_data_chain.size(), Vector2(0, 0));
	return jiggle_data_chain[p_joint_idx].override_defaults ? jiggle_data_chain[p_joint_idx].gravity : get_gravity();
}

void SkeletonModifier2DJiggle::set_all_bones(TypedArray<NodePath> p_node_paths) {
	set_bone_chain_size(p_node_paths.size());
	for (int i = 0; i < p_node_paths.size(); i++) {
		set_joint_bone(i, p_node_paths[i]);
	}
}
TypedArray<NodePath> SkeletonModifier2DJiggle::get_all_bones() const {
	TypedArray<NodePath> ret;
	for (int i = 0; i < jiggle_data_chain.size(); i++) {
		ret.push_back(get_joint_bone(i));
	}
	return ret;
}

void SkeletonModifier2DJiggle::set_all_overrides(TypedArray<bool> p_overrides) {
	int size = MIN(get_bone_chain_size(), p_overrides.size());
	for (int i = 0; i < size; i++) {
		set_joint_override(i, p_overrides[i]);
	}
}

TypedArray<bool> SkeletonModifier2DJiggle::get_all_overrides() const {
	TypedArray<bool> ret;
	for (int i = 0; i < jiggle_data_chain.size(); i++) {
		ret.push_back(get_joint_override(i));
	}
	return ret;
}

void SkeletonModifier2DJiggle::set_all_stiffness(PackedFloat32Array p_stiffness) {
	int size = MIN(get_bone_chain_size(), p_stiffness.size());
	for (int i = 0; i < size; i++) {
		set_joint_stiffness(i, p_stiffness[i]);
	}
}
PackedFloat32Array SkeletonModifier2DJiggle::get_all_stiffness() const {
	PackedFloat32Array ret;
	for (int i = 0; i < jiggle_data_chain.size(); i++) {
		ret.push_back(get_joint_stiffness(i));
	}
	return ret;
}

void SkeletonModifier2DJiggle::set_all_mass(PackedFloat32Array p_mass) {
	int size = MIN(get_bone_chain_size(), p_mass.size());
	for (int i = 0; i < size; i++) {
		set_joint_mass(i, p_mass[i]);
	}
}
PackedFloat32Array SkeletonModifier2DJiggle::get_all_mass() const {
	PackedFloat32Array ret;
	for (int i = 0; i < jiggle_data_chain.size(); i++) {
		ret.push_back(get_joint_mass(i));
	}
	return ret;
}

void SkeletonModifier2DJiggle::set_all_dampings(PackedFloat32Array p_dampings) {
	int size = MIN(get_bone_chain_size(), p_dampings.size());
	for (int i = 0; i < size; i++) {
		set_joint_damping(i, p_dampings[i]);
	}
}
PackedFloat32Array SkeletonModifier2DJiggle::get_all_dampings() const {
	PackedFloat32Array ret;
	for (int i = 0; i < jiggle_data_chain.size(); i++) {
		ret.push_back(get_joint_damping(i));
	}
	return ret;
}

void SkeletonModifier2DJiggle::set_all_use_gravity(TypedArray<bool> p_use_gravity) {
	int size = MIN(get_bone_chain_size(), p_use_gravity.size());
	for (int i = 0; i < size; i++) {
		set_joint_use_gravity(i, p_use_gravity[i]);
	}
}
TypedArray<bool> SkeletonModifier2DJiggle::get_all_use_gravity() const {
	TypedArray<bool> ret;
	for (int i = 0; i < jiggle_data_chain.size(); i++) {
		ret.push_back(get_joint_use_gravity(i));
	}
	return ret;
}

void SkeletonModifier2DJiggle::set_all_gravities(PackedVector2Array p_gravities) {
	int size = MIN(get_bone_chain_size(), p_gravities.size());
	for (int i = 0; i < size; i++) {
		set_joint_gravity(i, p_gravities[i]);
	}
}
PackedVector2Array SkeletonModifier2DJiggle::get_all_gravities() const {
	PackedVector2Array ret;
	for (int i = 0; i < jiggle_data_chain.size(); i++) {
		ret.push_back(get_joint_gravity(i));
	}
	return ret;
}

void SkeletonModifier2DJiggle::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_target_node", "p_target_node"), &SkeletonModifier2DJiggle::set_target_node);
	ClassDB::bind_method(D_METHOD("get_target_node"), &SkeletonModifier2DJiggle::get_target_node);
	ClassDB::bind_method(D_METHOD("set_stiffness", "p_stiffness"), &SkeletonModifier2DJiggle::set_stiffness);
	ClassDB::bind_method(D_METHOD("get_stiffness"), &SkeletonModifier2DJiggle::get_stiffness);
	ClassDB::bind_method(D_METHOD("set_mass", "p_mass"), &SkeletonModifier2DJiggle::set_mass);
	ClassDB::bind_method(D_METHOD("get_mass"), &SkeletonModifier2DJiggle::get_mass);
	ClassDB::bind_method(D_METHOD("set_damping", "p_damping"), &SkeletonModifier2DJiggle::set_damping);
	ClassDB::bind_method(D_METHOD("get_damping"), &SkeletonModifier2DJiggle::get_damping);
	ClassDB::bind_method(D_METHOD("set_use_gravity", "p_use_gravity"), &SkeletonModifier2DJiggle::set_use_gravity);
	ClassDB::bind_method(D_METHOD("get_use_gravity"), &SkeletonModifier2DJiggle::get_use_gravity);
	ClassDB::bind_method(D_METHOD("set_gravity", "p_gravity"), &SkeletonModifier2DJiggle::set_gravity);
	ClassDB::bind_method(D_METHOD("get_gravity"), &SkeletonModifier2DJiggle::get_gravity);
	ClassDB::bind_method(D_METHOD("set_use_colliders", "p_use_colliders"), &SkeletonModifier2DJiggle::set_use_colliders);
	ClassDB::bind_method(D_METHOD("get_use_colliders"), &SkeletonModifier2DJiggle::get_use_colliders);
	ClassDB::bind_method(D_METHOD("set_collision_mask", "p_mask"), &SkeletonModifier2DJiggle::set_collision_mask);
	ClassDB::bind_method(D_METHOD("get_collision_mask"), &SkeletonModifier2DJiggle::get_collision_mask);
	ClassDB::bind_method(D_METHOD("get_bone_chain_size"), &SkeletonModifier2DJiggle::get_bone_chain_size);
	ClassDB::bind_method(D_METHOD("set_bone_chain_size", "p_size"), &SkeletonModifier2DJiggle::set_bone_chain_size);

	// Jiggle joint data functions
	ClassDB::bind_method(D_METHOD("set_joint_bone", "p_joint_idx", "p_target_node"), &SkeletonModifier2DJiggle::set_joint_bone);
	ClassDB::bind_method(D_METHOD("get_joint_bone", "p_joint_idx"), &SkeletonModifier2DJiggle::get_joint_bone);
	ClassDB::bind_method(D_METHOD("set_joint_bone_idx", "p_joint_idx", "p_bone_idx"), &SkeletonModifier2DJiggle::set_joint_bone_idx);
	ClassDB::bind_method(D_METHOD("get_joint_bone_idx", "p_joint_idx"), &SkeletonModifier2DJiggle::get_joint_bone_idx);
	ClassDB::bind_method(D_METHOD("set_joint_override", "p_joint_idx", "p_override"), &SkeletonModifier2DJiggle::set_joint_override);
	ClassDB::bind_method(D_METHOD("get_joint_override", "p_joint_idx"), &SkeletonModifier2DJiggle::get_joint_override);
	ClassDB::bind_method(D_METHOD("set_joint_stiffness", "p_joint_idx", "p_stiffness"), &SkeletonModifier2DJiggle::set_joint_stiffness);
	ClassDB::bind_method(D_METHOD("get_joint_stiffness", "p_joint_idx"), &SkeletonModifier2DJiggle::get_joint_stiffness);
	ClassDB::bind_method(D_METHOD("set_joint_mass", "p_joint_idx", "p_mass"), &SkeletonModifier2DJiggle::set_joint_mass);
	ClassDB::bind_method(D_METHOD("get_joint_mass", "p_joint_idx"), &SkeletonModifier2DJiggle::get_joint_mass);
	ClassDB::bind_method(D_METHOD("set_joint_damping", "p_joint_idx", "p_damping"), &SkeletonModifier2DJiggle::set_joint_damping);
	ClassDB::bind_method(D_METHOD("get_joint_damping", "p_joint_idx"), &SkeletonModifier2DJiggle::get_joint_damping);
	ClassDB::bind_method(D_METHOD("set_joint_use_gravity", "p_joint_idx", "p_use_gravity"), &SkeletonModifier2DJiggle::set_joint_use_gravity);
	ClassDB::bind_method(D_METHOD("get_joint_use_gravity", "p_joint_idx"), &SkeletonModifier2DJiggle::get_joint_use_gravity);
	ClassDB::bind_method(D_METHOD("set_joint_gravity", "p_joint_idx", "p_gravity"), &SkeletonModifier2DJiggle::set_joint_gravity);
	ClassDB::bind_method(D_METHOD("get_joint_gravity", "p_joint_idx"), &SkeletonModifier2DJiggle::get_joint_gravity);

	ClassDB::bind_method(D_METHOD("set_all_bones", "p_node_paths"), &SkeletonModifier2DJiggle::set_all_bones);
	ClassDB::bind_method(D_METHOD("get_all_bones"), &SkeletonModifier2DJiggle::get_all_bones);
	ClassDB::bind_method(D_METHOD("set_all_overrides", "p_overrides"), &SkeletonModifier2DJiggle::set_all_overrides);
	ClassDB::bind_method(D_METHOD("get_all_overrides"), &SkeletonModifier2DJiggle::get_all_overrides);
	ClassDB::bind_method(D_METHOD("set_all_stiffness", "p_stiffness"), &SkeletonModifier2DJiggle::set_all_stiffness);
	ClassDB::bind_method(D_METHOD("get_all_stiffness"), &SkeletonModifier2DJiggle::get_all_stiffness);
	ClassDB::bind_method(D_METHOD("set_all_mass", "p_mass"), &SkeletonModifier2DJiggle::set_all_mass);
	ClassDB::bind_method(D_METHOD("get_all_mass"), &SkeletonModifier2DJiggle::get_all_mass);
	ClassDB::bind_method(D_METHOD("set_all_dampings", "p_dampings"), &SkeletonModifier2DJiggle::set_all_dampings);
	ClassDB::bind_method(D_METHOD("get_all_dampings"), &SkeletonModifier2DJiggle::get_all_dampings);
	ClassDB::bind_method(D_METHOD("set_all_use_gravity", "p_use_gravity"), &SkeletonModifier2DJiggle::set_all_use_gravity);
	ClassDB::bind_method(D_METHOD("get_all_use_gravity"), &SkeletonModifier2DJiggle::get_all_use_gravity);
	ClassDB::bind_method(D_METHOD("set_all_gravities", "p_gravities"), &SkeletonModifier2DJiggle::set_all_gravities);
	ClassDB::bind_method(D_METHOD("get_all_gravities"), &SkeletonModifier2DJiggle::get_all_gravities);

	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "target_node", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Node2D"), "set_target_node", "get_target_node");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "bone_chain_size", PROPERTY_HINT_RANGE, "0,100,1"), "set_bone_chain_size", "get_bone_chain_size");
	ADD_GROUP("Default Joint Settings", "");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "stiffness"), "set_stiffness", "get_stiffness");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "mass"), "set_mass", "get_mass");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "damping", PROPERTY_HINT_RANGE, "0, 1, 0.01"), "set_damping", "get_damping");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_gravity"), "set_use_gravity", "get_use_gravity");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "gravity"), "set_gravity", "get_gravity");
	ADD_GROUP("Each Joint Settings", "");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "all_bones", PROPERTY_HINT_ARRAY_TYPE, "NodePath"), "set_all_bones", "get_all_bones");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "all_overrides"), "set_all_overrides", "get_all_overrides");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_FLOAT32_ARRAY, "all_stiffness"), "set_all_stiffness", "get_all_stiffness");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_FLOAT32_ARRAY, "all_mass"), "set_all_mass", "get_all_mass");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_FLOAT32_ARRAY, "all_dampings"), "set_all_dampings", "get_all_dampings");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "all_use_gravity"), "set_all_use_gravity", "get_all_use_gravity");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR2_ARRAY, "all_gravities"), "set_all_gravities", "get_all_gravities");
}
