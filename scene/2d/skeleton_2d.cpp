/**************************************************************************/
/*  skeleton_2d.cpp                                                       */
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

#include "skeleton_2d.h"
#include "core/math/transform_interpolator.h"
#include "scene/2d/skeleton_modifier_2d.h"

#ifdef TOOLS_ENABLED
#include "editor/editor_data.h"
#include "editor/editor_settings.h"
#include "editor/plugins/canvas_item_editor_plugin.h"
#endif //TOOLS_ENABLED

void Bone2D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			Node *parent = get_parent();
			parent_bone = Object::cast_to<Bone2D>(parent);
			skeleton = nullptr;
			while (parent) {
				skeleton = Object::cast_to<Skeleton2D>(parent);
				if (skeleton) {
					break;
				}
				if (!Object::cast_to<Bone2D>(parent)) {
					break; //skeletons must be chained to Bone2Ds.
				}

				parent = parent->get_parent();
			}

			if (skeleton) {
				skeleton->bones.push_back(this);
				skeleton->_make_bone_setup_dirty();
				get_parent()->connect(SNAME("child_order_changed"), callable_mp(skeleton, &Skeleton2D::_make_bone_setup_dirty), CONNECT_REFERENCE_COUNTED);
			}
#ifdef TOOLS_ENABLED
			// Only draw the gizmo in the editor!
			if (Engine::get_singleton()->is_editor_hint() == false) {
				return;
			}
			queue_redraw();
#endif // TOOLS_ENABLED
		} break;
		case NOTIFICATION_LOCAL_TRANSFORM_CHANGED: {
			if (skeleton) {
				skeleton->_make_transform_dirty();
			}
#ifdef TOOLS_ENABLED
			// Only draw the gizmo in the editor!
			if (Engine::get_singleton()->is_editor_hint() == false) {
				return;
			}
			queue_redraw();

			if (get_parent()) {
				Bone2D *p_bone = Object::cast_to<Bone2D>(get_parent());
				if (p_bone) {
					p_bone->queue_redraw();
				}
			}
#endif // TOOLS_ENABLED
		} break;

		case NOTIFICATION_EXIT_TREE: {
			if (skeleton) {
				for (int i = 0; i < skeleton->bones.size(); i++) {
					if (skeleton->bones[i] == this) {
						skeleton->bones.remove_at(i);
						break;
					}
				}
				skeleton->_make_bone_setup_dirty();
				get_parent()->disconnect(SNAME("child_order_changed"), callable_mp(skeleton, &Skeleton2D::_make_bone_setup_dirty));
			}
			parent_bone = nullptr;
		} break;

		case NOTIFICATION_READY: {
			if (autocalculate_length_and_angle) {
				calculate_length_and_rotation();
			}
		} break;

#ifdef TOOLS_ENABLED
		// Bone2D Editor gizmo drawing.
		// TODO: Bone2D gizmo drawing needs to be moved to an editor plugin.
		case NOTIFICATION_DRAW: {
			// Only draw the gizmo in the editor!
			if (Engine::get_singleton()->is_editor_hint() == false) {
				return;
			}

			if (editor_gizmo_rid.is_null()) {
				editor_gizmo_rid = RenderingServer::get_singleton()->canvas_item_create();
				RenderingServer::get_singleton()->canvas_item_set_parent(editor_gizmo_rid, get_canvas_item());
				RenderingServer::get_singleton()->canvas_item_set_z_as_relative_to_parent(editor_gizmo_rid, true);
				RenderingServer::get_singleton()->canvas_item_set_z_index(editor_gizmo_rid, 10);
			}
			RenderingServer::get_singleton()->canvas_item_clear(editor_gizmo_rid);

			if (!_editor_show_bone_gizmo) {
				return;
			}

			// Undo scaling
			Transform2D editor_gizmo_trans;
			editor_gizmo_trans.set_scale(Vector2(1, 1) / get_global_scale());
			RenderingServer::get_singleton()->canvas_item_set_transform(editor_gizmo_rid, editor_gizmo_trans);

			Color bone_color1 = EDITOR_GET("editors/2d/bone_color1");
			Color bone_color2 = EDITOR_GET("editors/2d/bone_color2");
			Color bone_outline_color = EDITOR_GET("editors/2d/bone_outline_color");
			Color bone_selected_color = EDITOR_GET("editors/2d/bone_selected_color");

			bool Bone2D_found = false;
			for (int i = 0; i < get_child_count(); i++) {
				Bone2D *child_node = nullptr;
				child_node = Object::cast_to<Bone2D>(get_child(i));
				if (!child_node) {
					continue;
				}
				Bone2D_found = true;

				Vector<Vector2> bone_shape;
				Vector<Vector2> bone_shape_outline;

				_editor_get_bone_shape(&bone_shape, &bone_shape_outline, child_node);

				Vector<Color> colors;
				colors.push_back(bone_color1);
				colors.push_back(bone_color2);
				colors.push_back(bone_color1);
				colors.push_back(bone_color2);

				Vector<Color> outline_colors;
				if (CanvasItemEditor::get_singleton()->editor_selection->is_selected(this)) {
					outline_colors.push_back(bone_selected_color);
					outline_colors.push_back(bone_selected_color);
					outline_colors.push_back(bone_selected_color);
					outline_colors.push_back(bone_selected_color);
					outline_colors.push_back(bone_selected_color);
					outline_colors.push_back(bone_selected_color);
				} else {
					outline_colors.push_back(bone_outline_color);
					outline_colors.push_back(bone_outline_color);
					outline_colors.push_back(bone_outline_color);
					outline_colors.push_back(bone_outline_color);
					outline_colors.push_back(bone_outline_color);
					outline_colors.push_back(bone_outline_color);
				}

				RenderingServer::get_singleton()->canvas_item_add_polygon(editor_gizmo_rid, bone_shape_outline, outline_colors);
				RenderingServer::get_singleton()->canvas_item_add_polygon(editor_gizmo_rid, bone_shape, colors);
			}

			if (!Bone2D_found) {
				Vector<Vector2> bone_shape;
				Vector<Vector2> bone_shape_outline;

				_editor_get_bone_shape(&bone_shape, &bone_shape_outline, nullptr);

				Vector<Color> colors;

				colors.push_back(bone_color1);
				colors.push_back(bone_color2);
				colors.push_back(bone_color1);
				colors.push_back(bone_color2);

				Vector<Color> outline_colors;
				if (CanvasItemEditor::get_singleton()->editor_selection->is_selected(this)) {
					outline_colors.push_back(bone_selected_color);
					outline_colors.push_back(bone_selected_color);
					outline_colors.push_back(bone_selected_color);
					outline_colors.push_back(bone_selected_color);
					outline_colors.push_back(bone_selected_color);
					outline_colors.push_back(bone_selected_color);
				} else {
					outline_colors.push_back(bone_outline_color);
					outline_colors.push_back(bone_outline_color);
					outline_colors.push_back(bone_outline_color);
					outline_colors.push_back(bone_outline_color);
					outline_colors.push_back(bone_outline_color);
					outline_colors.push_back(bone_outline_color);
				}

				RenderingServer::get_singleton()->canvas_item_add_polygon(editor_gizmo_rid, bone_shape_outline, outline_colors);
				RenderingServer::get_singleton()->canvas_item_add_polygon(editor_gizmo_rid, bone_shape, colors);
			}
		} break;
#endif // TOOLS_ENABLED
	}
}

#ifdef TOOLS_ENABLED
bool Bone2D::_editor_get_bone_shape(Vector<Vector2> *p_shape, Vector<Vector2> *p_outline_shape, Bone2D *p_other_bone) {
	float bone_width = EDITOR_GET("editors/2d/bone_width");
	float bone_outline_width = EDITOR_GET("editors/2d/bone_outline_size");

	if (!is_inside_tree()) {
		return false; //may have been removed
	}
	if (!p_other_bone && bone_length <= 0) {
		return false;
	}

	Vector2 rel;
	if (p_other_bone) {
		rel = (p_other_bone->get_global_position() - get_global_position());
		rel = rel.rotated(-get_global_rotation()); // Undo Bone2D node's rotation so its drawn correctly regardless of the node's rotation
	} else {
<<<<<<< HEAD
		rel = Vector2(Math::cos(bone_angle), Math::sin(bone_angle)) * length * get_global_scale();
=======
		real_t angle_to_use = get_rotation();
		rel = Vector2(cos(angle_to_use), sin(angle_to_use)) * (bone_length * MIN(get_global_scale().x, get_global_scale().y));
		rel = rel.rotated(-get_rotation()); // Undo Bone2D node's rotation so its drawn correctly regardless of the node's rotation
>>>>>>> b80339adad (SkeletonModifier2D wip)
	}

	Vector2 relt = rel.rotated(Math_PI * 0.5).normalized() * bone_width;
	Vector2 reln = rel.normalized();
	Vector2 reltn = relt.normalized();

	if (p_shape) {
		p_shape->clear();
		p_shape->push_back(Vector2(0, 0));
		p_shape->push_back(rel * 0.2 + relt);
		p_shape->push_back(rel);
		p_shape->push_back(rel * 0.2 - relt);
	}

	if (p_outline_shape) {
		p_outline_shape->clear();
		p_outline_shape->push_back((-reln - reltn) * bone_outline_width);
		p_outline_shape->push_back((-reln + reltn) * bone_outline_width);
		p_outline_shape->push_back(rel * 0.2 + relt + reltn * bone_outline_width);
		p_outline_shape->push_back(rel + (reln + reltn) * bone_outline_width);
		p_outline_shape->push_back(rel + (reln - reltn) * bone_outline_width);
		p_outline_shape->push_back(rel * 0.2 - relt - reltn * bone_outline_width);
	}
	return true;
}

void Bone2D::_editor_set_show_bone_gizmo(bool p_show_gizmo) {
	_editor_show_bone_gizmo = p_show_gizmo;
	queue_redraw();
}

bool Bone2D::_editor_get_show_bone_gizmo() const {
	return _editor_show_bone_gizmo;
}
#endif // TOOLS_ENABLED

void Bone2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_rest", "rest"), &Bone2D::set_rest);
	ClassDB::bind_method(D_METHOD("get_rest"), &Bone2D::get_rest);
	ClassDB::bind_method(D_METHOD("apply_rest"), &Bone2D::apply_rest);

	ClassDB::bind_method(D_METHOD("set_autocalculate_length_and_angle", "auto_calculate"), &Bone2D::set_autocalculate_length_and_angle);
	ClassDB::bind_method(D_METHOD("get_autocalculate_length_and_angle"), &Bone2D::get_autocalculate_length_and_angle);
	ClassDB::bind_method(D_METHOD("set_bone_length", "length"), &Bone2D::set_bone_length);
	ClassDB::bind_method(D_METHOD("get_bone_length"), &Bone2D::get_bone_length);

	ADD_PROPERTY(PropertyInfo(Variant::TRANSFORM2D, "rest", PROPERTY_HINT_NONE, "suffix:px"), "set_rest", "get_rest");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "autocalculate_length_and_angle"), "set_autocalculate_length_and_angle", "get_autocalculate_length_and_angle");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "bone_length"), "set_bone_length", "get_bone_length");
}

void Bone2D::set_rest(const Transform2D &p_rest) {
	rest = p_rest;
	if (skeleton) {
		skeleton->_make_bone_setup_dirty();
	}

	update_configuration_warnings();
}

Transform2D Bone2D::get_rest() const {
	return rest;
}

void Bone2D::apply_rest() {
	set_transform(rest);
}

Transform2D Bone2D::get_accum_transform() const {
	return accum_transform;
}

int Bone2D::get_bone_idx() const {
	return bone_idx;
}

int Bone2D::get_parent_idx() const {
	return parent_idx;
}

PackedStringArray Bone2D::get_configuration_warnings() const {
	PackedStringArray warnings = Node::get_configuration_warnings();
	Node *parent = get_parent();
	if (!parent->is_class("Skeleton2D") && !parent->is_class("Bone2D")) {
		warnings.push_back(RTR("A Bone2D only works with a Skeleton2D or another Bone2D as parent node."));
	}

	if (rest == Transform2D(0, 0, 0, 0, 0, 0)) {
		warnings.push_back(RTR("This bone lacks a proper REST pose. Go to the Skeleton2D node and set one."));
	}

	return warnings;
}

void Bone2D::calculate_length_and_rotation() {
	// If there is at least a single child Bone2D node, we can calculate
	// the length and direction. We will always just use the first Bone2D for this.
	int child_count = get_child_count();
	Transform2D global_inv = get_global_transform().affine_inverse();

	for (int i = 0; i < child_count; i++) {
		Bone2D *child = Object::cast_to<Bone2D>(get_child(i));
		if (child) {
			Vector2 child_local_pos = global_inv.xform(child->get_global_position());
			bone_length = child_local_pos.length();
			set_rotation(child_local_pos.angle());
			return; // Finished!
		}
	}

	WARN_PRINT("No Bone2D children of node " + get_name() + ". Cannot calculate bone length or angle reliably.\nUsing transform rotation for bone angle.");
}

void Bone2D::set_autocalculate_length_and_angle(bool p_autocalculate) {
	autocalculate_length_and_angle = p_autocalculate;
	if (autocalculate_length_and_angle) {
		calculate_length_and_rotation();
	}
	notify_property_list_changed();
}

bool Bone2D::get_autocalculate_length_and_angle() const {
	return autocalculate_length_and_angle;
}

void Bone2D::set_bone_length(real_t p_length) {
	bone_length = p_length;

#ifdef TOOLS_ENABLED
	queue_redraw();
#endif // TOOLS_ENABLED
}

real_t Bone2D::get_bone_length() const {
	return bone_length;
}

Bone2D::Bone2D() {
	set_notify_local_transform(true);
	set_hide_clip_children(true);
	//this is a clever hack so the bone knows no rest has been set yet, allowing to show an error.
	rest = Transform2D(0, 0, 0, 0, 0, 0);
}

Bone2D::~Bone2D() {
#ifdef TOOLS_ENABLED
	if (!editor_gizmo_rid.is_null()) {
		ERR_FAIL_NULL(RenderingServer::get_singleton());
		RenderingServer::get_singleton()->free(editor_gizmo_rid);
	}
#endif // TOOLS_ENABLED
}

//////////////////////////////////////

<<<<<<< HEAD
bool Skeleton2D::_set(const StringName &p_path, const Variant &p_value) {
	if (p_path == SNAME("modification_stack")) {
		set_modification_stack(p_value);
		return true;
	}
	return false;
}

bool Skeleton2D::_get(const StringName &p_path, Variant &r_ret) const {
	if (p_path == SNAME("modification_stack")) {
		r_ret = get_modification_stack();
		return true;
	}
	return false;
}

void Skeleton2D::_get_property_list(List<PropertyInfo> *p_list) const {
	p_list->push_back(
			PropertyInfo(Variant::OBJECT, PNAME("modification_stack"),
					PROPERTY_HINT_RESOURCE_TYPE,
					"SkeletonModificationStack2D",
					PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_ALWAYS_DUPLICATE));
}

=======
>>>>>>> b80339adad (SkeletonModifier2D wip)
void Skeleton2D::_make_bone_setup_dirty() {
	if (bone_setup_dirty) {
		return;
	}
	bone_setup_dirty = true;
	if (is_inside_tree()) {
		callable_mp(this, &Skeleton2D::_update_bone_setup).call_deferred();
	}
}

void Skeleton2D::_update_bone_setup() {
	if (!bone_setup_dirty) {
		return;
	}

	bone_setup_dirty = false;
	RS::get_singleton()->skeleton_allocate_data(skeleton, bones.size(), true);

	bones.sort_custom<Bone2DComparator>(); //sorting so that they are always in the same order/index

	for (int i = 0; i < bones.size(); i++) {
		bones[i]->bone_idx = i;
		Bone2D *parent_bone = Object::cast_to<Bone2D>(bones[i]->get_parent());
		if (parent_bone) {
			bones[i]->parent_idx = parent_bone->bone_idx;
		} else {
			bones[i]->parent_idx = -1;
		}

		bones[i]->apply_rest();
	}

	transform_dirty = true;
	_update_transform();
	emit_signal(SNAME("bone_setup_changed"));
}

void Skeleton2D::_make_transform_dirty() {
	if (transform_dirty) {
		return;
	}
	transform_dirty = true;
	if (is_inside_tree()) {
		callable_mp(this, &Skeleton2D::_update_transform).call_deferred();
	}
}

void Skeleton2D::_update_transform() {
	if (bone_setup_dirty) {
		_update_bone_setup();
		return; //above will update transform anyway
	}
	if (!transform_dirty) {
		return;
	}

	transform_dirty = false;

	for (int i = 0; i < bones.size(); i++) {
		ERR_CONTINUE(bones[i]->parent_idx >= i);
		if (bones[i]->parent_idx >= 0) {
			bones[i]->accum_transform = bones[bones[i]->parent_idx]->accum_transform * bones[i]->get_transform();
		} else {
			bones[i]->accum_transform = bones[i]->get_transform();
		}
	}

	for (int i = 0; i < bones.size(); i++) {
		Transform2D rest_inverse = bones[i]->rest.affine_inverse();
		Transform2D final_xform = bones[i]->accum_transform * rest_inverse;
		RS::get_singleton()->skeleton_bone_set_transform_2d(skeleton, i, final_xform);
	}
}

int Skeleton2D::get_bone_count() const {
	ERR_FAIL_COND_V(!is_inside_tree(), 0);

	if (bone_setup_dirty) {
		const_cast<Skeleton2D *>(this)->_update_bone_setup();
	}

	return bones.size();
}

Bone2D *Skeleton2D::get_bone(int p_idx) {
	ERR_FAIL_COND_V(!is_inside_tree(), nullptr);
	ERR_FAIL_INDEX_V(p_idx, bones.size(), nullptr);

	return bones[p_idx];
}

void Skeleton2D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			if (bone_setup_dirty) {
				_update_bone_setup();
			}
			if (transform_dirty) {
				_update_transform();
			}
			request_ready();
		} break;

		case NOTIFICATION_ENTER_TREE: {
			_process_changed();
		} break;

		case NOTIFICATION_TRANSFORM_CHANGED: {
			RS::get_singleton()->skeleton_set_base_transform_2d(skeleton, get_global_transform());
		} break;

		case NOTIFICATION_INTERNAL_PROCESS:
		case NOTIFICATION_INTERNAL_PHYSICS_PROCESS: {
			_find_modifiers();
			if (!modifiers.is_empty()) {
				_update_skeleton_deferred();
			}
		} break;

		case NOTIFICATION_UPDATE_SKELETON: {
			// Process modifiers.
			_find_modifiers();
			if (!modifiers.is_empty()) {
				_process_modifiers();
			}

			emit_signal(SceneStringName(skeleton_updated));

		} break;
	}
}
void Skeleton2D::_update_skeleton_deferred() {
	if (is_inside_tree()) {
		if (!updating) {
			notify_deferred_thread_group(NOTIFICATION_UPDATE_SKELETON); // It must never be called more than once in a single frame.
		}
	}
}
void Skeleton2D::_process_changed() {
	if (modifier_callback_mode_process == MODIFIER_CALLBACK_MODE_PROCESS_IDLE) {
		set_process_internal(true);
		set_physics_process_internal(false);
	} else if (modifier_callback_mode_process == MODIFIER_CALLBACK_MODE_PROCESS_PHYSICS) {
		set_process_internal(false);
		set_physics_process_internal(true);
	}
}

void Skeleton2D::_find_modifiers() {
	if (!modifiers_dirty) {
		return;
	}
	modifiers.clear();
	for (int i = 0; i < get_child_count(); i++) {
		SkeletonModifier2D *c = Object::cast_to<SkeletonModifier2D>(get_child(i));
		if (c) {
			modifiers.push_back(c->get_instance_id());
		}
	}
	modifiers_dirty = false;
}
void Skeleton2D::_process_modifiers() {
	for (const ObjectID &oid : modifiers) {
		Object *t_obj = ObjectDB::get_instance(oid);
		if (!t_obj) {
			continue;
		}
		SkeletonModifier2D *mod = cast_to<SkeletonModifier2D>(t_obj);
		if (!mod) {
			continue;
		}
		real_t influence = mod->get_influence();
		if (influence < 1.0) {
			LocalVector<Transform2D> old_poses;
			for (int i = 0; i < get_bone_count(); i++) {
				old_poses.push_back(get_bone_pose(i));
			}
			mod->process_modification();
			LocalVector<Transform2D> new_poses;
			for (int i = 0; i < get_bone_count(); i++) {
				new_poses.push_back(get_bone_pose(i));
			}
			for (int i = 0; i < get_bone_count(); i++) {
				if (old_poses[i] == new_poses[i]) {
					continue; // Avoid unneeded calculation.
				}
				set_bone_pose(i, old_poses[i].interpolate_with(new_poses[i], influence));
			}
		} else {
			mod->process_modification();
		}
		_make_transform_dirty();
	}
}

void Skeleton2D::_make_modifiers_dirty() {
	modifiers_dirty = true;
	_update_skeleton_deferred();
}

RID Skeleton2D::get_skeleton() const {
	return skeleton;
}

void Skeleton2D::set_modifier_callback_mode_process(Skeleton2D::ModifierCallbackModeProcess p_mode) {
	if (modifier_callback_mode_process == p_mode) {
		return;
	}
	modifier_callback_mode_process = p_mode;
	_process_changed();
}

Skeleton2D::ModifierCallbackModeProcess Skeleton2D::get_modifier_callback_mode_process() const {
	return modifier_callback_mode_process;
}

Transform2D Skeleton2D::get_bone_pose(int p_bone) const {
	ERR_FAIL_INDEX_V_MSG(p_bone, bones.size(), Transform2D(), "Bone index is out of range!");
	return bones[p_bone]->get_transform();
}
Vector2 Skeleton2D::get_bone_pose_position(int p_bone) const {
	ERR_FAIL_INDEX_V_MSG(p_bone, bones.size(), Vector2(), "Bone index is out of range!");
	return bones[p_bone]->get_position();
}
real_t Skeleton2D::get_bone_pose_rotation(int p_bone) const {
	ERR_FAIL_INDEX_V_MSG(p_bone, bones.size(), 0.0, "Bone index is out of range!");
	return bones[p_bone]->get_rotation();
}
Vector2 Skeleton2D::get_bone_pose_scale(int p_bone) const {
	ERR_FAIL_INDEX_V_MSG(p_bone, bones.size(), Vector2(), "Bone index is out of range!");
	return bones[p_bone]->get_scale();
}
void Skeleton2D::set_bone_pose(int p_bone, const Transform2D &p_pose) {
	ERR_FAIL_INDEX_MSG(p_bone, bones.size(), "Bone index is out of range!");
	bones[p_bone]->set_transform(p_pose);
}
void Skeleton2D::set_bone_pose_position(int p_bone, const Vector2 &p_position) {
	ERR_FAIL_INDEX_MSG(p_bone, bones.size(), "Bone index is out of range!");
	bones[p_bone]->set_position(p_position);
}
void Skeleton2D::set_bone_pose_rotation(int p_bone, const real_t &p_rotation) {
	ERR_FAIL_INDEX_MSG(p_bone, bones.size(), "Bone index is out of range!");
	bones[p_bone]->set_rotation(p_rotation);
}
void Skeleton2D::set_bone_pose_scale(int p_bone, const Vector2 &p_scale) {
	ERR_FAIL_INDEX_MSG(p_bone, bones.size(), "Bone index is out of range!");
	bones[p_bone]->set_scale(p_scale);
}
Transform2D Skeleton2D::get_bone_global_pose(int p_bone) const {
	ERR_FAIL_INDEX_V_MSG(p_bone, bones.size(), Transform2D(), "Bone index is out of range!");
	return bones[p_bone]->get_global_transform();
}
void Skeleton2D::set_bone_global_pose(int p_bone, const Transform2D &p_pose) {
	ERR_FAIL_INDEX_MSG(p_bone, bones.size(), "Bone index is out of range!");
	bones[p_bone]->set_global_transform(p_pose);
}
void Skeleton2D::reset_bone_pose(int p_bone) {
	ERR_FAIL_INDEX_MSG(p_bone, bones.size(), "Bone index is out of range!");
	bones[p_bone]->apply_rest();
}
void Skeleton2D::reset_bone_poses() {
	for (int i = 0; i < bones.size(); i++) {
		bones[i]->apply_rest();
	}
}

void Skeleton2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_skeleton"), &Skeleton2D::get_skeleton);
	ClassDB::bind_method(D_METHOD("set_modifier_callback_mode_process", "p_mode"), &Skeleton2D::set_modifier_callback_mode_process);
	ClassDB::bind_method(D_METHOD("get_modifier_callback_mode_process"), &Skeleton2D::get_modifier_callback_mode_process);
	ClassDB::bind_method(D_METHOD("get_bone", "idx"), &Skeleton2D::get_bone);
	ClassDB::bind_method(D_METHOD("get_bone_count"), &Skeleton2D::get_bone_count);
	ClassDB::bind_method(D_METHOD("get_bone_pose", "p_bone"), &Skeleton2D::get_bone_pose);
	ClassDB::bind_method(D_METHOD("get_bone_pose_position", "p_bone"), &Skeleton2D::get_bone_pose_position);
	ClassDB::bind_method(D_METHOD("get_bone_pose_rotation", "p_bone"), &Skeleton2D::get_bone_pose_rotation);
	ClassDB::bind_method(D_METHOD("get_bone_pose_scale", "p_bone"), &Skeleton2D::get_bone_pose_scale);
	ClassDB::bind_method(D_METHOD("set_bone_pose", "p_bone", "p_pose"), &Skeleton2D::set_bone_pose);
	ClassDB::bind_method(D_METHOD("set_bone_pose_position", "p_bone", "p_position"), &Skeleton2D::set_bone_pose_position);
	ClassDB::bind_method(D_METHOD("set_bone_pose_rotation", "p_bone", "p_rotation"), &Skeleton2D::set_bone_pose_rotation);
	ClassDB::bind_method(D_METHOD("set_bone_pose_scale", "p_bone", "p_scale"), &Skeleton2D::set_bone_pose_scale);
	ClassDB::bind_method(D_METHOD("get_bone_global_pose", "p_bone"), &Skeleton2D::get_bone_global_pose);
	ClassDB::bind_method(D_METHOD("set_bone_global_pose", "p_bone", "p_pose"), &Skeleton2D::set_bone_global_pose);
	ClassDB::bind_method(D_METHOD("reset_bone_pose", "p_bone"), &Skeleton2D::reset_bone_pose);
	ClassDB::bind_method(D_METHOD("reset_bone_poses"), &Skeleton2D::reset_bone_poses);

	ADD_GROUP("Modifier", "modifier_");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "modifier_callback_mode_process", PROPERTY_HINT_ENUM, "Physics,Idle"), "set_modifier_callback_mode_process", "get_modifier_callback_mode_process");

	ADD_SIGNAL(MethodInfo("bone_setup_changed"));
	ADD_SIGNAL(MethodInfo("skeleton_updated"));

	BIND_CONSTANT(NOTIFICATION_UPDATE_SKELETON);
	BIND_ENUM_CONSTANT(MODIFIER_CALLBACK_MODE_PROCESS_PHYSICS);
	BIND_ENUM_CONSTANT(MODIFIER_CALLBACK_MODE_PROCESS_IDLE);
}

Skeleton2D::Skeleton2D() {
	skeleton = RS::get_singleton()->skeleton_create();
	set_notify_transform(true);
	set_hide_clip_children(true);
}

Skeleton2D::~Skeleton2D() {
	ERR_FAIL_NULL(RenderingServer::get_singleton());
	RS::get_singleton()->free(skeleton);
}
