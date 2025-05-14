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
#include "servers/rendering/renderer_rd/rasterize_mesh.h"
#include "servers/rendering/renderer_rd/storage_rd/material_storage.h"
#include "servers/rendering/rendering_device.h"

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
	queue_update_pipeline();
}

void RasterizedMeshTexture::set_height(int p_height) {
	ERR_FAIL_COND(p_height <= 0 || p_height > 16384);
	size.height = p_height;
	queue_update_pipeline();
}

void RasterizedMeshTexture::set_mesh(const Ref<Mesh> &p_mesh) {
	if (mesh.is_valid()) {
		mesh->disconnect_changed(callable_mp(this, &RasterizedMeshTexture::queue_update_mesh));
	}
	mesh = p_mesh;
	if (mesh.is_valid()) {
		mesh->connect_changed(callable_mp(this, &RasterizedMeshTexture::queue_update_mesh));
	}
	queue_update_mesh();
}

Ref<Mesh> RasterizedMeshTexture::get_mesh() const {
	return mesh;
}

void RasterizedMeshTexture::set_bg_color(const Color &p_color) {
	bg_color = p_color;
	queue_update();
}

Color RasterizedMeshTexture::get_bg_color() const {
	return bg_color;
}

void RasterizedMeshTexture::set_material(const Ref<ShaderMaterial> &p_material) {
	if (material.is_valid()) {
		material->disconnect_changed(callable_mp(this, &RasterizedMeshTexture::queue_update_pipeline));
		if (material->get_shader().is_valid()) {
			material->get_shader()->disconnect_changed(callable_mp(this, &RasterizedMeshTexture::queue_update_pipeline));
		}
	}
	material = p_material;
	if (material.is_valid()) {
		material->connect_changed(callable_mp(this, &RasterizedMeshTexture::queue_update_pipeline));
		if (material->get_shader().is_valid()) {
			material->get_shader()->disconnect_changed(callable_mp(this, &RasterizedMeshTexture::queue_update_pipeline));
		}
	}
	queue_update_pipeline();
}

Ref<ShaderMaterial> RasterizedMeshTexture::get_material() const {
	return material;
}

void RasterizedMeshTexture::set_generate_mipmaps(bool p_generate_mipmaps) {
	generate_mipmaps = p_generate_mipmaps;
	queue_update_pipeline();
}

bool RasterizedMeshTexture::is_generating_mipmaps() const {
	return generate_mipmaps;
}

RasterizedMeshTexture::RasterizedMeshTexture() {
	texture = RS::get_singleton()->texture_2d_placeholder_create();

	ERR_FAIL_NULL(RD::get_singleton());

	tex_format.texture_type = RD::TEXTURE_TYPE_2D;
	tex_format.format = RD::DATA_FORMAT_R8G8B8A8_UNORM;
	tex_format.usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_COLOR_ATTACHMENT_BIT | RD::TEXTURE_USAGE_CAN_COPY_TO_BIT | RD::TEXTURE_USAGE_CAN_COPY_FROM_BIT;

	pipeline_rasterization_state.front_face = RenderingDeviceCommons::POLYGON_FRONT_FACE_COUNTER_CLOCKWISE;
	pipeline_rasterization_state.cull_mode = RenderingDeviceCommons::POLYGON_CULL_BACK;

	RD::VertexAttribute pos;
	pos.location = 0,
	pos.format = RD::DATA_FORMAT_R32G32B32_SFLOAT,
	pos.stride = sizeof(float) * 3;

	RD::VertexAttribute uv;
	uv.location = 1,
	uv.format = RD::DATA_FORMAT_R32G32_SFLOAT,
	uv.stride = sizeof(float) * 2;

	RD::VertexAttribute color;
	color.location = 2,
	color.format = RD::DATA_FORMAT_R8G8B8A8_UNORM,
	color.stride = sizeof(uint8_t) * 4;

	vertex_attrs = { pos, uv, color };

	pipeline_color_blend_state.attachments.append(RD::PipelineColorBlendState::Attachment{});

	vertex_format = RD::get_singleton()->vertex_format_create(vertex_attrs);
}

RasterizedMeshTexture::~RasterizedMeshTexture() {
	RS::get_singleton()->free(texture);

	ERR_FAIL_NULL(RD::get_singleton());

	if (framebuffer_texture_id.is_valid()) {
		RD::get_singleton()->free(framebuffer_texture_id);
	}
	if (index_buffer_id.is_valid()) {
		RD::get_singleton()->free(index_buffer_id);
	}
	if (vertex_buffer_pos_id.is_valid()) {
		RD::get_singleton()->free(vertex_buffer_pos_id);
	}
	if (vertex_buffer_uv_id.is_valid()) {
		RD::get_singleton()->free(vertex_buffer_uv_id);
	}
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
	ClassDB::bind_method(D_METHOD("set_generate_mipmaps", "generate_mipmaps"), &RasterizedMeshTexture::set_generate_mipmaps);
	ClassDB::bind_method(D_METHOD("is_generating_mipmaps"), &RasterizedMeshTexture::is_generating_mipmaps);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "width", PROPERTY_HINT_RANGE, "1,2048,or_greater,suffix:px"), "set_width", "get_width");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "height", PROPERTY_HINT_RANGE, "1,2048,or_greater,suffix:px"), "set_height", "get_height");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "mesh", PROPERTY_HINT_RESOURCE_TYPE, "Mesh"), "set_mesh", "get_mesh");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "bg_color"), "set_bg_color", "get_bg_color");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "material", PROPERTY_HINT_RESOURCE_TYPE, "ShaderMaterial"), "set_material", "get_material");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "generate_mipmaps"), "set_generate_mipmaps", "is_generating_mipmaps");
}

static uint32_t get_image_required_mipmaps(uint32_t p_width, uint32_t p_height /*, uint32_t p_depth*/) {
	uint32_t w = p_width;
	uint32_t h = p_height;
	// uint32_t d = p_depth;
	uint32_t mipmaps = 1;
	while (true) {
		if (w == 1 && h == 1 /*&& d == 1*/) {
			break;
		}
		w = MAX(1u, w >> 1);
		h = MAX(1u, h >> 1);
		// d = Math.Max(1u, d >> 1);
		mipmaps++;
	}
	return mipmaps;
}

static RD::RenderPrimitive primitive_type_to_render_primitive(RS::PrimitiveType p_primitive) {
	switch (p_primitive) {
		case RS::PRIMITIVE_POINTS:
			return RD::RENDER_PRIMITIVE_POINTS;
		case RS::PRIMITIVE_LINES:
			return RD::RENDER_PRIMITIVE_LINES;
		case RS::PRIMITIVE_LINE_STRIP:
			return RD::RENDER_PRIMITIVE_LINESTRIPS;
		case RS::PRIMITIVE_TRIANGLES:
			return RD::RENDER_PRIMITIVE_TRIANGLES;
		case RS::PRIMITIVE_TRIANGLE_STRIP:
			return RD::RENDER_PRIMITIVE_TRIANGLE_STRIPS;
		default:
			return RD::RENDER_PRIMITIVE_MAX;
	}
}

void RasterizedMeshTexture::update() {
	ERR_FAIL_NULL(RD::get_singleton());
	if (mesh_dirty) {
		reset_vertex();
		mesh_dirty = false;
	}
	if (pipeline_dirty || framebuffer_id.is_null() || pipeline_id.is_null()) {
		reset_pipeline();
		pipeline_dirty = false;
	}
	draw_list_draw();
	emit_changed();
	update_queued = false;
}

void RasterizedMeshTexture::queue_update() {
	if (update_queued) {
		return;
	}
	update_queued = true;
	callable_mp(this, &RasterizedMeshTexture::update).call_deferred();
}

void RasterizedMeshTexture::queue_update_pipeline() {
	pipeline_dirty = true;
	queue_update();
}

void RasterizedMeshTexture::queue_update_mesh() {
	mesh_dirty = true;
	queue_update();
}

void RasterizedMeshTexture::reset_vertex() {
	if (mesh.is_null() || mesh->get_surface_count() == 0) {
		return;
	}
	primitive_type = primitive_type_to_render_primitive((RS::PrimitiveType)mesh->surface_get_primitive_type(0));
	Array surface_array = mesh->surface_get_arrays(0);
	Variant vertex_array = surface_array[Mesh::ARRAY_VERTEX];
	Vector<int> index_array = surface_array[Mesh::ARRAY_INDEX];
	Vector<Vector2> uv_array = surface_array[Mesh::ARRAY_TEX_UV];
	Vector<Color> color_array = surface_array[Mesh::ARRAY_COLOR];

	Vector<Vector3> vertex_array_vec3;
	Vector<uint8_t> vertex_data;
	is_2d_mesh = false;
	float max_pos = 0;

	if (vertex_array.get_type() == Variant::PACKED_VECTOR2_ARRAY) {
		is_2d_mesh = true;
		Vector<Vector2> array_vec2 = vertex_array;
		vertex_array_vec3.resize(array_vec2.size());
		for (int i = 0; i < array_vec2.size(); i++) {
			Vector2 vec2 = array_vec2[i];
			vertex_array_vec3.write[i] = Vector3(vec2.x, vec2.y, 0);
		}
	} else {
		vertex_array_vec3 = vertex_array;
	}

	for (int i = 0; i < vertex_array_vec3.size(); i++) {
		Vector3 v = vertex_array_vec3[i].abs();
		max_pos = MAX(max_pos, v[v.max_axis_index()]);
	}

	for (int i = 0; i < vertex_array_vec3.size(); i++) {
		Vector3 v = vertex_array_vec3[i];
		vertex_array_vec3.write[i] = v / (max_pos == 0 ? 1 : max_pos);
	}

	uint32_t vertex_count = vertex_array_vec3.size();
	vertex_data.resize(sizeof(float) * 3 * vertex_count);
	memcpy(vertex_data.ptrw(), vertex_array_vec3.ptr(), vertex_data.size());

	bool is_index_16 = vertex_count <= 0xffff;
	Vector<int16_t> index_array_16;
	if (is_index_16) {
		index_array_16.resize(index_array.size());
		for (int i = 0; i < index_array.size(); i++) {
			index_array_16.write[i] = index_array[i];
		}
	}
	if (index_array.size() > 0) {
		Vector<uint8_t> index_data;
		index_data.resize((is_index_16 ? sizeof(int16_t) : sizeof(int32_t)) * index_array.size());
		if (is_index_16) {
			memcpy(index_data.ptrw(), index_array_16.ptr(), index_data.size());
		} else {
			memcpy(index_data.ptrw(), index_array.ptr(), index_data.size());
		}

		if (index_buffer_id.is_valid()) {
			RD::get_singleton()->free(index_buffer_id);
		}
		index_buffer_id = RD::get_singleton()->index_buffer_create(index_array.size(), is_index_16 ? RD::INDEX_BUFFER_FORMAT_UINT16 : RD::INDEX_BUFFER_FORMAT_UINT32, index_data);

		index_array_id = RD::get_singleton()->index_array_create(index_buffer_id, 0, index_array.size());
	}
	Vector<uint8_t> uv_data;
	uv_data.resize(sizeof(float) * 2 * uv_array.size());
	memcpy(uv_data.ptrw(), uv_array.ptr(), uv_data.size());

	Vector<uint8_t> color_data;
	color_data.resize(vertex_count * 4);
	memset(color_data.ptrw(), 1, sizeof(color_data));
	const Color *src = color_array.ptr();
	for (uint32_t i = 0; i < color_array.size(); i++) {
		uint8_t color8[4] = {
			uint8_t(CLAMP(src[i].r * 255.0, 0.0, 255.0)),
			uint8_t(CLAMP(src[i].g * 255.0, 0.0, 255.0)),
			uint8_t(CLAMP(src[i].b * 255.0, 0.0, 255.0)),
			uint8_t(CLAMP(src[i].a * 255.0, 0.0, 255.0))
		};
		memcpy(color_data.ptrw() + i * 4, color8, 4);
	}

	if (vertex_buffer_pos_id.is_valid()) {
		RD::get_singleton()->free(vertex_buffer_pos_id);
	}
	if (vertex_buffer_uv_id.is_valid()) {
		RD::get_singleton()->free(vertex_buffer_uv_id);
	}
	if (vertex_buffer_color_id.is_valid()) {
		RD::get_singleton()->free(vertex_buffer_color_id);
	}
	vertex_buffer_pos_id = RD::get_singleton()->vertex_buffer_create(vertex_data.size(), vertex_data);
	vertex_buffer_uv_id = RD::get_singleton()->vertex_buffer_create(uv_data.size(), uv_data);
	vertex_buffer_color_id = RD::get_singleton()->vertex_buffer_create(color_data.size(), color_data);

	Vector<RID> vertex_buffers = { vertex_buffer_pos_id, vertex_buffer_uv_id, vertex_buffer_color_id };

	vertex_array_id = RD::get_singleton()->vertex_array_create(vertex_count, vertex_format, vertex_buffers);
}

void RasterizedMeshTexture::reset_pipeline() {
	tex_format.width = size.x;
	tex_format.height = size.y;
	tex_format.mipmaps = generate_mipmaps ? get_image_required_mipmaps(tex_format.width, tex_format.height) : 1;

	RID value = RD::get_singleton()->texture_create(tex_format, tex_view);
	RS::get_singleton()->texture_replace(texture, RS::get_singleton()->texture_rd_create(value));
	if (framebuffer_texture_id.is_valid()) {
		RD::get_singleton()->free(framebuffer_texture_id);
	}
	framebuffer_texture_id = value;

	framebuffer_id = RD::get_singleton()->framebuffer_create({ framebuffer_texture_id });

	pipeline_rasterization_state.cull_mode = is_2d_mesh ? RD::POLYGON_CULL_FRONT : RD::POLYGON_CULL_BACK;

	RID shader = RendererRD::RasterizeMeshRD::get_singleton()->get_default_shader_rd();
	if (material.is_valid() && material->get_shader_mode() == Shader::MODE_RASTERIZE_MESH && material->get_shader_rid().is_valid()) {
		RendererRD::RasterizeMeshRD::RasterizeMeshShaderData *shader_data = static_cast<RendererRD::RasterizeMeshRD::RasterizeMeshShaderData *>(RendererRD::MaterialStorage::get_singleton()->material_get_shader_data(material->get_rid()));
		shader = shader_data->shader_rd;
	}

	pipeline_id = RD::get_singleton()->render_pipeline_create(
			shader,
			RD::get_singleton()->framebuffer_get_format(framebuffer_id),
			vertex_format,
			primitive_type,
			pipeline_rasterization_state,
			pipeline_multisample_state,
			pipeline_depth_stencil_state,
			pipeline_color_blend_state);
}

void RasterizedMeshTexture::draw_list_draw() {
	if (!(pipeline_id.is_valid() &&
				framebuffer_id.is_valid() &&
				vertex_array_id.is_valid())) {
		return;
	}

	RD::DrawListID draw_list = RD::get_singleton()->draw_list_begin(framebuffer_id, RD::DRAW_CLEAR_COLOR_ALL, { bg_color });
	RD::get_singleton()->draw_list_bind_render_pipeline(draw_list, pipeline_id);
	RD::get_singleton()->draw_list_bind_vertex_array(draw_list, vertex_array_id);

	RID material_rid = RendererRD::RasterizeMeshRD::get_singleton()->get_default_material();
	if (material.is_valid() && material->get_shader_mode() == Shader::MODE_RASTERIZE_MESH) {
		material_rid = material->get_rid();
	}
	RendererRD::MaterialStorage *material_storage = RendererRD::MaterialStorage::get_singleton();
	RendererRD::RasterizeMeshRD::RasterizeMeshMaterialData *material_data = static_cast<RendererRD::RasterizeMeshRD::RasterizeMeshMaterialData *>(material_storage->material_get_data(material_rid, RendererRD::MaterialStorage::SHADER_TYPE_RASTERIZE_MESH));
	RendererRD::RasterizeMeshRD::RasterizeMeshShaderData *shader_data = static_cast<RendererRD::RasterizeMeshRD::RasterizeMeshShaderData *>(material_storage->material_get_shader_data(material_rid));

	RD::get_singleton()->draw_list_bind_uniform_set(draw_list, shader_data->base_uniforms, RendererRD::RasterizeMeshRD::BASE_UNIFORM_SET);
	if (material_data->material_uniforms.is_valid()) {
		RD::get_singleton()->draw_list_bind_uniform_set(draw_list, material_data->material_uniforms, RendererRD::RasterizeMeshRD::MATERIAL_UNIFORM_SET);
	}

	if (index_array_id.is_valid()) {
		RD::get_singleton()->draw_list_bind_index_array(draw_list, index_array_id);
	}

	RD::get_singleton()->draw_list_draw(draw_list, index_array_id.is_valid(), 1);
	RD::get_singleton()->draw_list_end();
	if (generate_mipmaps) {
		create_mipmaps();
	}
}

void RasterizedMeshTexture::create_mipmaps() {
	if (framebuffer_texture_id.is_null()) {
		return;
	}
	Ref<Image> img = Image::create_from_data(size.x, size.y, true, Image::FORMAT_RGBA8, RD::get_singleton()->texture_get_data(framebuffer_texture_id, 0));
	img->generate_mipmaps();
	Vector<uint8_t> data = img->get_data();
	RD::TextureFormat fmt = RD::get_singleton()->texture_get_format(framebuffer_texture_id);
	int mipmap_count = fmt.mipmaps;
	fmt.mipmaps = 1;
	for (int i = 1; i < mipmap_count; i++) {
		Size2i mipmap_size = Size2i(MAX(1, size.x / (1 << i)), MAX(1, size.y / (1 << i)));
		fmt.width = mipmap_size.x;
		fmt.height = mipmap_size.y;
		int start = img->get_mipmap_offset(i);
		Vector<uint8_t> d;
		d.resize(mipmap_size.x * mipmap_size.y * 4);
		memcpy(d.ptrw(), data.ptr() + start, d.size());

		RID tex = RD::get_singleton()->texture_create(fmt, tex_view, { d });
		Error err = RD::get_singleton()->texture_copy(tex, framebuffer_texture_id, Vector3(), Vector3(), Vector3(mipmap_size.x, mipmap_size.y, 0), 0, i, 0, 0);

		ERR_FAIL_COND_MSG(err != OK, vformat("Failed to generate mipmaps: %s", error_names[err]));
	}
}
