/**************************************************************************/
/*  skeleton_modifier_2d.cpp                                              */
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

#include "skeleton_modifier_2d.h"

PackedStringArray SkeletonModifier2D::get_configuration_warnings() const {
	PackedStringArray warnings = Node2D::get_configuration_warnings();
	if (skeleton_id.is_null()) {
		warnings.push_back(RTR("Skeleton2D node not set! SkeletonModifier2D must be child of Skeleton2D or set a path to an external skeleton."));
	}
	return warnings;
}

void SkeletonModifier2D::set_execution_mode(SkeletonModifier2D::ExecutionMode p_execution_mode) {
	execution_mode = p_execution_mode;
}
SkeletonModifier2D::ExecutionMode SkeletonModifier2D::get_execution_mode() const {
	return execution_mode;
}

Skeleton2D *SkeletonModifier2D::get_skeleton() const {
	return Object::cast_to<Skeleton2D>(ObjectDB::get_instance(skeleton_id));
}

void SkeletonModifier2D::_update_skeleton_path() {
	skeleton_id = ObjectID();

	// Make sure parent is a Skeleton2D.
	Skeleton2D *sk = Object::cast_to<Skeleton2D>(get_parent());
	if (sk) {
		skeleton_id = sk->get_instance_id();
	}
}

void SkeletonModifier2D::_update_skeleton() {
	if (!is_inside_tree()) {
		return;
	}
	Skeleton2D *old_sk = get_skeleton();
	if (old_sk && old_sk->is_connected("ready", callable_mp(this, &SkeletonModifier2D::setup_modification))) {
		old_sk->disconnect("ready", callable_mp(this, &SkeletonModifier2D::setup_modification));
	}
	_update_skeleton_path();
	Skeleton2D *new_sk = get_skeleton();
	if (old_sk != new_sk) {
		_skeleton_changed(old_sk, new_sk);
	}
	new_sk->connect("ready", callable_mp(this, &SkeletonModifier2D::setup_modification));
	update_configuration_warnings();
}

void SkeletonModifier2D::_skeleton_changed(Skeleton2D *p_old, Skeleton2D *p_new) {
	//
}

/* Process */

void SkeletonModifier2D::set_active(bool p_active) {
	active = p_active;
	set_process_internal(active);
	set_physics_process_internal(active);
	_set_active(active);
}

bool SkeletonModifier2D::is_active() const {
	return active;
}

void SkeletonModifier2D::_set_active(bool p_active) {
	//
}

void SkeletonModifier2D::set_influence(real_t p_influence) {
	influence = p_influence;
}

real_t SkeletonModifier2D::get_influence() const {
	return influence;
}

void SkeletonModifier2D::process_modification(real_t p_delta) {
	if (!active) {
		return;
	}

	_process_modification(p_delta);

	emit_signal(SNAME("modification_processed"));
}

void SkeletonModifier2D::setup_modification() {
	set_active(is_active());
	_setup_modification();
	emit_signal(SNAME("modification_setup"));
}

void SkeletonModifier2D::_setup_modification() {
	GDVIRTUAL_CALL(_setup_modification);
}

void SkeletonModifier2D::_process_modification(real_t p_delta) {
	GDVIRTUAL_CALL(_process_modification, p_delta);
}

void SkeletonModifier2D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE:
		case NOTIFICATION_PARENTED: {
			_update_skeleton();
		} break;
		case NOTIFICATION_INTERNAL_PROCESS: {
			if (get_execution_mode() == ExecutionMode::EXECUTION_MODE_PROCESS_IDLE) {
				process_modification(get_process_delta_time());
			}
		} break;
		case NOTIFICATION_INTERNAL_PHYSICS_PROCESS: {
			if (get_execution_mode() == ExecutionMode::EXECUTION_MODE_PROCESS_PHYSICS) {
				process_modification(get_physics_process_delta_time());
			}
		} break;
	}
}

void SkeletonModifier2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_skeleton"), &SkeletonModifier2D::get_skeleton);

	ClassDB::bind_method(D_METHOD("set_active", "active"), &SkeletonModifier2D::set_active);
	ClassDB::bind_method(D_METHOD("is_active"), &SkeletonModifier2D::is_active);

	ClassDB::bind_method(D_METHOD("set_influence", "influence"), &SkeletonModifier2D::set_influence);
	ClassDB::bind_method(D_METHOD("get_influence"), &SkeletonModifier2D::get_influence);

	ClassDB::bind_method(D_METHOD("set_execution_mode", "execution_mode"), &SkeletonModifier2D::set_execution_mode);
	ClassDB::bind_method(D_METHOD("get_execution_mode"), &SkeletonModifier2D::get_execution_mode);

	BIND_ENUM_CONSTANT(ExecutionMode::EXECUTION_MODE_PROCESS_IDLE);
	BIND_ENUM_CONSTANT(ExecutionMode::EXECUTION_MODE_PROCESS_PHYSICS);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "active"), "set_active", "is_active");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "influence", PROPERTY_HINT_RANGE, "0,1,0.001"), "set_influence", "get_influence");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "execution_mode", PROPERTY_HINT_ENUM, "IDLE,Physics"), "set_execution_mode", "get_execution_mode");

	ADD_SIGNAL(MethodInfo("modification_processed"));
	ADD_SIGNAL(MethodInfo("modification_setup"));

	GDVIRTUAL_BIND(_process_modification);
	GDVIRTUAL_BIND(_setup_modification);
}
