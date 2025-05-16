/**************************************************************************/
/*  rasterized_mesh_texture.cpp                                           */
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

#include "rasterized_mesh_texture.h"
#include "scene/resources/mesh.h"

int RasterizedMeshTexture::get_width() const {
	return size.width;
}

int RasterizedMeshTexture::get_height() const {
	return size.height;
}

bool RasterizedMeshTexture::has_alpha() const {
	return true;
}

RID RasterizedMeshTexture::get_rid() const {
	return texture;
}

Ref<Image> RasterizedMeshTexture::get_image() const {
	if (texture.is_null()) {
		return Ref<Image>();
	}
	return RenderingServer::get_singleton()->texture_2d_get(texture);
}

void RasterizedMeshTexture::set_width(int p_width) {
	ERR_FAIL_COND(p_width <= 0 || p_width > 16384);
	size.width = p_width;
	rasterizer_dirty = true;
	queue_update_texture();
}

void RasterizedMeshTexture::set_height(int p_height) {
	ERR_FAIL_COND(p_height <= 0 || p_height > 16384);
	size.height = p_height;
	rasterizer_dirty = true;
	queue_update_texture();
}

void RasterizedMeshTexture::set_mesh(const Ref<Mesh> &p_mesh) {
	mesh = p_mesh;
	mesh_drity = true;
	queue_update_texture();
}

Ref<Mesh> RasterizedMeshTexture::get_mesh() const {
	return mesh;
}

void RasterizedMeshTexture::set_bg_color(const Color &p_color) {
	bg_color = p_color;
	queue_update_texture();
}

Color RasterizedMeshTexture::get_bg_color() const {
	return bg_color;
}

void RasterizedMeshTexture::set_material(const Ref<ShaderMaterial> &p_material) {
	material = p_material;
	material_drity = true;
	queue_update_texture();
}

Ref<ShaderMaterial> RasterizedMeshTexture::get_material() const {
	return material;
}

void RasterizedMeshTexture::set_surface_index(int p_surface_index) {
	surface_index = p_surface_index;
	mesh_drity = true;
	RS::get_singleton()->mesh_rasterizer_draw(mesh_rasterizer);
}

int RasterizedMeshTexture::get_surface_index() const {
	return surface_index;
}

void RasterizedMeshTexture::set_generate_mipmaps(bool p_generate_mipmaps) {
	generate_mipmaps = p_generate_mipmaps;
	rasterizer_dirty = true;
	queue_update_texture();
}

bool RasterizedMeshTexture::is_generating_mipmaps() const {
	return generate_mipmaps;
}

RasterizedMeshTexture::RasterizedMeshTexture() {
	mesh_rasterizer = RS::get_singleton()->mesh_rasterizer_create(size.width, size.height, generate_mipmaps);
	texture = RS::get_singleton()->texture_rd_create(RS::get_singleton()->mesh_rasterizer_get_rd_texture(mesh_rasterizer));
}

RasterizedMeshTexture::~RasterizedMeshTexture() {
	RS::get_singleton()->free(texture);
	RS::get_singleton()->free(mesh_rasterizer);
}

void RasterizedMeshTexture::update_texture() {
	if (rasterizer_dirty) {
		RID new_mesh_rasterizer = RS::get_singleton()->mesh_rasterizer_create(size.width, size.height, generate_mipmaps);
		RID new_texture = RS::get_singleton()->texture_rd_create(RS::get_singleton()->mesh_rasterizer_get_rd_texture(new_mesh_rasterizer));
		RS::get_singleton()->texture_replace(texture, new_texture);
		RS::get_singleton()->free(mesh_rasterizer);
		mesh_rasterizer = new_mesh_rasterizer;
	}
	if (rasterizer_dirty || mesh_drity) {
		RS::get_singleton()->mesh_rasterizer_set_mesh(mesh_rasterizer, mesh.is_valid() ? mesh->get_rid() : RID(), surface_index);
		mesh_drity = false;
	}
	if (rasterizer_dirty || material_drity) {
		RS::get_singleton()->mesh_rasterizer_set_material(mesh_rasterizer, material.is_valid() ? material->get_rid() : RID());
		material_drity = false;
	}
	RS::get_singleton()->mesh_rasterizer_set_bg_color(mesh_rasterizer, bg_color);
	RS::get_singleton()->mesh_rasterizer_draw(mesh_rasterizer);
	rasterizer_dirty = false;
	update_queued = false;
	emit_changed();
}

void RasterizedMeshTexture::queue_update_texture() {
	if (update_queued) {
		return;
	}
	callable_mp(this, &RasterizedMeshTexture::update_texture).call_deferred();
	update_queued = true;
}

void RasterizedMeshTexture::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_width", "width"), &RasterizedMeshTexture::set_width);
	ClassDB::bind_method(D_METHOD("set_height", "height"), &RasterizedMeshTexture::set_height);
	ClassDB::bind_method(D_METHOD("set_mesh", "mesh"), &RasterizedMeshTexture::set_mesh);
	ClassDB::bind_method(D_METHOD("get_mesh"), &RasterizedMeshTexture::get_mesh);
	ClassDB::bind_method(D_METHOD("set_bg_color", "color"), &RasterizedMeshTexture::set_bg_color);
	ClassDB::bind_method(D_METHOD("get_bg_color"), &RasterizedMeshTexture::get_bg_color);
	ClassDB::bind_method(D_METHOD("set_material", "material"), &RasterizedMeshTexture::set_material);
	ClassDB::bind_method(D_METHOD("get_material"), &RasterizedMeshTexture::get_material);
	ClassDB::bind_method(D_METHOD("set_surface_index", "surface_index"), &RasterizedMeshTexture::set_surface_index);
	ClassDB::bind_method(D_METHOD("get_surface_index"), &RasterizedMeshTexture::get_surface_index);
	ClassDB::bind_method(D_METHOD("set_generate_mipmaps", "generate_mipmaps"), &RasterizedMeshTexture::set_generate_mipmaps);
	ClassDB::bind_method(D_METHOD("is_generating_mipmaps"), &RasterizedMeshTexture::is_generating_mipmaps);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "width", PROPERTY_HINT_RANGE, "1,2048,or_greater,suffix:px"), "set_width", "get_width");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "height", PROPERTY_HINT_RANGE, "1,2048,or_greater,suffix:px"), "set_height", "get_height");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "bg_color"), "set_bg_color", "get_bg_color");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "mesh", PROPERTY_HINT_RESOURCE_TYPE, "Mesh"), "set_mesh", "get_mesh");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "surface_index", PROPERTY_HINT_RANGE, "0,10,1,or_greater"), "set_surface_index", "get_surface_index");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "material", PROPERTY_HINT_RESOURCE_TYPE, "ShaderMaterial"), "set_material", "get_material");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "generate_mipmaps"), "set_generate_mipmaps", "is_generating_mipmaps");
}
