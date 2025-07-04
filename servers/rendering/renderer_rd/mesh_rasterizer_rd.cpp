/**************************************************************************/
/*  mesh_rasterizer_rd.cpp                                                */
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

#include "mesh_rasterizer_rd.h"
#include "framebuffer_cache_rd.h"
#include "servers/rendering/renderer_rd/storage_rd/mesh_storage.h"
#include "servers/rendering/renderer_rd/storage_rd/utilities.h"

namespace RendererRD {

MeshRasterizerRD *MeshRasterizerRD::singleton = nullptr;

MeshRasterizerRD *MeshRasterizerRD::get_singleton() {
	return singleton;
}

void MeshRasterizerRD::RasterizeMeshShaderData::set_code(const String &p_code) {
	//compile

	code = p_code;
	valid = false;
	ubo_size = 0;
	uniforms.clear();

	if (code.is_empty()) {
		return; //just invalid, but no error
	}

	ShaderCompiler::GeneratedCode gen_code;
	ShaderCompiler::IdentifierActions actions;

	actions.entry_point_stages["vertex"] = ShaderCompiler::STAGE_VERTEX;
	actions.entry_point_stages["fragment"] = ShaderCompiler::STAGE_FRAGMENT;
	actions.render_mode_values["cull_disabled"] = Pair<int *, int>(&cull_modei, RS::CULL_MODE_DISABLED);
	actions.render_mode_values["cull_front"] = Pair<int *, int>(&cull_modei, RS::CULL_MODE_FRONT);
	actions.render_mode_values["cull_back"] = Pair<int *, int>(&cull_modei, RS::CULL_MODE_BACK);

	actions.uniforms = &uniforms;

	Error err = singleton->compiler.compile(RS::SHADER_MESH_RASTERIZER, code, &actions, path, gen_code);
	ERR_FAIL_COND_MSG(err != OK, "Shader compilation failed.");

	if (version.is_null()) {
		version = singleton->shader_file_rd.version_create();
	}

	singleton->shader_file_rd.version_set_code(version, gen_code.code, gen_code.uniforms, gen_code.stage_globals[ShaderCompiler::STAGE_VERTEX], gen_code.stage_globals[ShaderCompiler::STAGE_FRAGMENT], gen_code.defines);

	ubo_size = gen_code.uniform_total_size;
	ubo_offsets = gen_code.uniform_offsets;
	texture_uniforms = gen_code.texture_uniforms;

	{
		// Global shader uniforms.
		RD::Uniform u;
		u.uniform_type = RD::UNIFORM_TYPE_STORAGE_BUFFER;
		u.binding = 0;
		u.append_id(RendererRD::MaterialStorage::get_singleton()->global_shader_uniforms_get_storage_buffer());

		Vector<RD::Uniform> us = { u };
		MaterialStorage::get_singleton()->samplers_rd_get_default().append_uniforms(us, SAMPLERS_BINDING_FIRST_INDEX);
		base_uniforms = RD::get_singleton()->uniform_set_create(us, singleton->shader_file_rd.version_get_shader(version, 0), BASE_UNIFORM_SET);
	}

	shader_rd = singleton->shader_file_rd.version_get_shader(version, 0);

	valid = true;
}

bool MeshRasterizerRD::RasterizeMeshShaderData::is_animated() const {
	return false;
}

bool MeshRasterizerRD::RasterizeMeshShaderData::casts_shadows() const {
	return false;
}

RS::ShaderNativeSourceCode MeshRasterizerRD::RasterizeMeshShaderData::get_native_source_code() const {
	return singleton->shader_file_rd.version_get_native_source_code(version);
}

Pair<ShaderRD *, RID> MeshRasterizerRD::RasterizeMeshShaderData::get_native_shader_and_version() const {
	return { &singleton->shader_file_rd, version };
}

uint64_t MeshRasterizerRD::RasterizeMeshShaderData::get_vertex_input_mask() {
	return RD::get_singleton()->shader_get_vertex_input_attribute_mask(shader_rd);
}

MeshRasterizerRD::RasterizeMeshShaderData::~RasterizeMeshShaderData() {
	MeshRasterizerRD *rasterizer = MeshRasterizerRD::get_singleton();
	rasterizer->shader_file_rd.version_free(version);
}

bool MeshRasterizerRD::RasterizeMeshMaterialData::update_parameters(const HashMap<StringName, Variant> &p_parameters, bool p_uniform_dirty, bool p_textures_dirty) {
	MeshRasterizerRD *rasterizer = MeshRasterizerRD::get_singleton();
	return update_parameters_uniform_set(p_parameters, p_uniform_dirty, p_textures_dirty, shader_data->uniforms, shader_data->ubo_offsets.ptr(), shader_data->texture_uniforms, shader_data->default_texture_params, shader_data->ubo_size, material_uniforms, rasterizer->shader_file_rd.version_get_shader(shader_data->version, 0), MATERIAL_UNIFORM_SET, false, false);
}

RendererRD::MaterialStorage::ShaderData *MeshRasterizerRD::_create_mesh_rasterizer_shader_funcs() {
	RasterizeMeshShaderData *shader_data = memnew(RasterizeMeshShaderData);
	return shader_data;
}

RendererRD::MaterialStorage::MaterialData *MeshRasterizerRD::_create_mesh_rasterizer_material_funcs(RendererRD::MaterialStorage::ShaderData *p_shader) {
	RasterizeMeshMaterialData *material_data = memnew(RasterizeMeshMaterialData);
	material_data->shader_data = static_cast<RasterizeMeshShaderData *>(p_shader);
	return material_data;
}

RID MeshRasterizerRD::mesh_rasterizer_allocate() {
	return mesh_rasterizer_owner.allocate_rid();
}

void MeshRasterizerRD::mesh_rasterizer_initialize(RID p_mesh_rasterizer, RID p_mesh, RID p_material, uint32_t p_surface_index) {
	mesh_rasterizer_owner.initialize_rid(p_mesh_rasterizer);
	MeshRasterizerData *mesh_rasterizer = mesh_rasterizer_owner.get_or_null(p_mesh_rasterizer);

	MaterialStorage *material_storage = MaterialStorage::get_singleton();
	MaterialStorage::MaterialData *md = material_storage->material_get_data(p_material, MaterialStorage::SHADER_TYPE_MESH_RASTERIZER);
	ERR_FAIL_NULL(md);
	mesh_rasterizer->material_data = static_cast<RasterizeMeshMaterialData *>(md);
	mesh_rasterizer->shader_data = static_cast<RasterizeMeshShaderData *>(material_storage->material_get_shader_data(p_material));
	ERR_FAIL_COND(mesh_rasterizer->shader_data->shader_rd.is_null());
	mesh_rasterizer->mesh = p_mesh;
	mesh_rasterizer->surface_index = p_surface_index;
	mesh_rasterizer->update_mesh();
	if (p_mesh.is_valid()) {
		Utilities::get_singleton()->base_update_dependency(p_mesh, &mesh_rasterizer->dependency_tracker);
	}
}

void MeshRasterizerRD::mesh_rasterizer_draw(RID p_mesh_rasterizer, RID p_texture_drawable, Ref<RasterizerBlendState> p_blend_state, const Color &p_bg_color, RD::TextureSamples p_multisample) {
	MeshRasterizerData *mesh_rasterizer = mesh_rasterizer_owner.get_or_null(p_mesh_rasterizer);
	ERR_FAIL_COND(p_mesh_rasterizer.is_null());
	ERR_FAIL_COND(mesh_rasterizer->vertex_array_rid.is_null());

	MaterialStorage::get_singleton()->_update_global_shader_uniforms(); //must do before materials, so it can queue them for update
	MaterialStorage::get_singleton()->_update_queued_materials();

	TextureStorage *texture_storage = TextureStorage::get_singleton();
	RID rd_texture = texture_storage->texture_get_rd_texture(p_texture_drawable, false);
	RD::TextureFormat tex_fmt = RD::get_singleton()->texture_get_format(rd_texture);

	// MSAA.
	RID rd_texture_samples;
	bool is_msaa = p_multisample > RD::TEXTURE_SAMPLES_1;
	if (is_msaa) {
		if (mesh_rasterizer->rd_texture_samples_cache.second.is_valid() && rd_texture == mesh_rasterizer->rd_texture_samples_cache.first) {
			rd_texture_samples = mesh_rasterizer->rd_texture_samples_cache.second;
		} else {
			tex_fmt.samples = p_multisample;
			tex_fmt.mipmaps = 1;
			tex_fmt.usage_bits = RD::TEXTURE_USAGE_COLOR_ATTACHMENT_BIT | RD::TEXTURE_USAGE_CAN_COPY_FROM_BIT;
			if (mesh_rasterizer->rd_texture_samples_cache.second.is_valid()) {
				RD::get_singleton()->free(mesh_rasterizer->rd_texture_samples_cache.second);
			}
			rd_texture_samples = RD::get_singleton()->texture_create(tex_fmt, {});

			mesh_rasterizer->rd_texture_samples_cache = { rd_texture, rd_texture_samples };
		}
	}

	RID framebuffer_rid;
	if (is_msaa) {
		framebuffer_rid = FramebufferCacheRD::get_singleton()->get_cache(rd_texture_samples);
	} else {
		framebuffer_rid = FramebufferCacheRD::get_singleton()->get_cache(rd_texture);
	}

	RD::PipelineRasterizationState rasterization_state;
	rasterization_state.cull_mode = (RD::PolygonCullMode)mesh_rasterizer->shader_data->cull_modei;
	RD::PipelineMultisampleState pipline_multisample_state;
	pipline_multisample_state.sample_count = p_multisample;
	RD::FramebufferFormatID fb_fmt = RD::get_singleton()->framebuffer_get_format(framebuffer_rid);

	RD::PipelineColorBlendState blend_state;
	blend_state.attachments.push_back({});
	if (p_blend_state.is_valid()) {
		p_blend_state->get_rd_blend_state(blend_state);
	}

	RID pipeline = RD::get_singleton()->render_pipeline_create(mesh_rasterizer->shader_data->shader_rd, fb_fmt, mesh_rasterizer->vertex_format, mesh_rasterizer->primitive, rasterization_state, pipline_multisample_state, {}, blend_state);

	LocalVector<Color> clear_colors = { p_bg_color };
	RD::DrawListID draw_list = RD::get_singleton()->draw_list_begin(framebuffer_rid, p_blend_state.is_null() ? RD::DRAW_CLEAR_ALL : RD::DRAW_DEFAULT_ALL, clear_colors);
	RD::get_singleton()->draw_list_bind_render_pipeline(draw_list, pipeline);

	// Vertex
	RD::get_singleton()->draw_list_bind_vertex_array(draw_list, mesh_rasterizer->vertex_array_rid);
	if (mesh_rasterizer->index_array_rid.is_valid()) {
		RD::get_singleton()->draw_list_bind_index_array(draw_list, mesh_rasterizer->index_array_rid);
	}

	// Uniforms
	RD::get_singleton()->draw_list_bind_uniform_set(draw_list, mesh_rasterizer->shader_data->base_uniforms, BASE_UNIFORM_SET);
	if (mesh_rasterizer->material_data->material_uniforms.is_valid()) {
		RD::get_singleton()->draw_list_bind_uniform_set(draw_list, mesh_rasterizer->material_data->material_uniforms, MATERIAL_UNIFORM_SET);
	}

	RD::get_singleton()->draw_list_draw(draw_list, mesh_rasterizer->index_array_rid.is_valid(), 1);
	RD::get_singleton()->draw_list_end();

	if (is_msaa) {
		Error err = RD::get_singleton()->texture_resolve_multisample(rd_texture_samples, rd_texture);
		ERR_FAIL_COND_MSG(err != OK, vformat("Resolve multisample texture fails: %s", error_names[err]));
	}
}

static RD::RenderPrimitive _primitive_type_to_render_primitive(RS::PrimitiveType p_primitive) {
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

void MeshRasterizerRD::MeshRasterizerData::update_mesh() {
	MeshStorage *mesh_storage = MeshStorage::get_singleton();
	bool is_valid = false;
	if (mesh.is_valid() && surface_index < (uint32_t)mesh_storage->mesh_get_surface_count(mesh)) {
		void *surface = mesh_storage->mesh_get_surface(mesh, surface_index);
		primitive = _primitive_type_to_render_primitive(mesh_storage->mesh_surface_get_primitive(surface));
		if (primitive != RD::RENDER_PRIMITIVE_MAX) {
			index_array_rid = mesh_storage->mesh_surface_get_index_array(&surface, 0);
			uint64_t input_mask = shader_data->get_vertex_input_mask();
			mesh_storage->mesh_surface_get_vertex_arrays_and_format(&surface, input_mask, false, vertex_array_rid, vertex_format);
			is_valid = true;
		}
	}
	if (!is_valid) {
		index_array_rid = RID();
		vertex_array_rid = RID();
	}
}

bool MeshRasterizerRD::free(RID p_mesh_rasterizer) {
	MeshRasterizerData *mesh_rasterizer = mesh_rasterizer_owner.get_or_null(p_mesh_rasterizer);
	if (mesh_rasterizer == nullptr) {
		return false;
	}
	if (mesh_rasterizer->rd_texture_samples_cache.second.is_valid()) {
		RD::get_singleton()->free(mesh_rasterizer->rd_texture_samples_cache.second);
	}
	mesh_rasterizer_owner.free(p_mesh_rasterizer);
	return true;
}

void MeshRasterizerRD::_dependency_changed(Dependency::DependencyChangedNotification p_notification, DependencyTracker *p_tracker) {
	MeshRasterizerData *mesh_rasterizer = (MeshRasterizerData *)p_tracker->userdata;
	switch (p_notification) {
		case Dependency::DEPENDENCY_CHANGED_MESH: {
			mesh_rasterizer->update_mesh();
		} break;
		default: {
		}
	}
}

void MeshRasterizerRD::_dependency_deleted(const RID &p_dependency, DependencyTracker *p_tracker) {
	MeshRasterizerData *mesh_rasterizer = (MeshRasterizerData *)p_tracker->userdata;
	if (p_dependency == mesh_rasterizer->mesh) {
		mesh_rasterizer->mesh = RID();
		mesh_rasterizer->update_mesh();
	}
}

MeshRasterizerRD::MeshRasterizerData::MeshRasterizerData() {
	dependency_tracker.userdata = this;
	dependency_tracker.changed_callback = &MeshRasterizerRD::_dependency_changed;
	dependency_tracker.deleted_callback = &MeshRasterizerRD::_dependency_deleted;
}

MeshRasterizerRD::MeshRasterizerRD() {
	singleton = this;

	MaterialStorage *material_storage = MaterialStorage::get_singleton();

	// register our shader funds
	material_storage->shader_set_data_request_function(MaterialStorage::SHADER_TYPE_MESH_RASTERIZER, _create_mesh_rasterizer_shader_funcs);
	material_storage->material_set_data_request_function(MaterialStorage::SHADER_TYPE_MESH_RASTERIZER, _create_mesh_rasterizer_material_funcs);

	{
		//shader compiler
		ShaderCompiler::DefaultIdentifierActions actions;

		actions.renames["VERTEX"] = "vertex_interp";
		actions.renames["NORMAL"] = "normal_interp";
		actions.renames["TANGENT"] = "tangent_interp";
		actions.renames["BINORMAL"] = "binormal_interp";
		actions.renames["POSITION"] = "position";
		actions.renames["UV"] = "uv_interp";
		actions.renames["UV2"] = "uv2_interp";
		actions.renames["COLOR"] = "color";

		actions.renames["POINT_SIZE"] = "gl_PointSize";
		actions.renames["VERTEX_ID"] = "gl_VertexIndex";
		actions.renames["FRAGCOORD"] = "gl_FragCoord";
		actions.renames["POINT_COORD"] = "gl_PointCoord";
		actions.renames["FRONT_FACING"] = "gl_FrontFacing";

		actions.renames["PI"] = String::num(Math::PI);
		actions.renames["TAU"] = String::num(Math::TAU);
		actions.renames["E"] = String::num(Math::E);

		actions.usage_defines["NORMAL"] = "#define NORMAL_USED\n";
		actions.usage_defines["TANGENT"] = "#define TANGENT_USED\n";
		actions.usage_defines["BINORMAL"] = "@TANGENT";
		actions.usage_defines["UV"] = "#define UV_USED\n";
		actions.usage_defines["UV2"] = "#define UV2_USED\n";
		actions.usage_defines["BONE_INDICES"] = "#define BONES_USED\n";
		actions.usage_defines["BONE_WEIGHTS"] = "#define WEIGHTS_USED\n";
		actions.usage_defines["CUSTOM0"] = "#define CUSTOM0_USED\n";
		actions.usage_defines["CUSTOM1"] = "#define CUSTOM1_USED\n";
		actions.usage_defines["CUSTOM2"] = "#define CUSTOM2_USED\n";
		actions.usage_defines["CUSTOM3"] = "#define CUSTOM3_USED\n";
		actions.usage_defines["COLOR"] = "#define COLOR_USED\n";
		actions.usage_defines["POSITION"] = "#define OVERRIDE_POSITION\n";

		actions.base_texture_binding_index = 1;
		actions.texture_layout_set = MATERIAL_UNIFORM_SET;
		actions.base_uniform_string = "material.";
		actions.default_filter = ShaderLanguage::FILTER_LINEAR;
		actions.default_repeat = ShaderLanguage::REPEAT_DISABLE;
		actions.base_varying_index = 7;

		actions.global_buffer_array_variable = "global_shader_uniforms.data";

		compiler.initialize(actions);
	}

	String defines = "\n#define SAMPLERS_BINDING_FIRST_INDEX " + itos(SAMPLERS_BINDING_FIRST_INDEX) + "\n";
	shader_file_rd.initialize({ "" }, defines);

	RD::FramebufferPass pass;
	pass.resolve_attachments.append(0);
	pass.color_attachments.append(1);
	render_passes = { pass };
}

} //namespace RendererRD
