/**************************************************************************/
/*  rasterize_mesh.cpp                                                    */
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

#include "rasterize_mesh.h"

namespace RendererRD {

RasterizeMeshRD *RasterizeMeshRD::singleton = nullptr;

RasterizeMeshRD *RasterizeMeshRD::get_singleton() {
	return singleton;
}

void RasterizeMeshRD::RasterizeMeshShaderData::set_code(const String &p_code) {
	//compile

	code = p_code;
	valid = false;
	ubo_size = 0;
	uniforms.clear();

	if (code.is_empty()) {
		return; //just invalid, but no error
	}
	ShaderCompiler compiler = RasterizeMeshRD::get_singleton()->compiler;

	ShaderCompiler::GeneratedCode gen_code;
	ShaderCompiler::IdentifierActions actions;

	actions.entry_point_stages["vertex"] = ShaderCompiler::STAGE_VERTEX;
	actions.entry_point_stages["fragment"] = ShaderCompiler::STAGE_FRAGMENT;

	actions.uniforms = &uniforms;

	Error err = compiler.compile(RS::SHADER_RASTERIZE_MESH, code, &actions, path, gen_code);
	ERR_FAIL_COND_MSG(err != OK, "Shader compilation failed.");

	if (version.is_null()) {
		version = singleton->shader_file_rd.version_create();
	}

	singleton->shader_file_rd.version_set_code(version, gen_code.code, gen_code.uniforms, gen_code.stage_globals[ShaderCompiler::STAGE_VERTEX], gen_code.stage_globals[ShaderCompiler::STAGE_FRAGMENT], gen_code.defines);
	ERR_FAIL_COND(!singleton->shader_file_rd.version_is_valid(version));

	ubo_size = gen_code.uniform_total_size;
	ubo_offsets = gen_code.uniform_offsets;
	texture_uniforms = gen_code.texture_uniforms;

	Vector<RD::Uniform> sampler_uniforms;
	MaterialStorage::get_singleton()->samplers_rd_get_default().append_uniforms(sampler_uniforms, SAMPLERS_BINDING_FIRST_INDEX);
	base_uniforms = RD::get_singleton()->uniform_set_create(sampler_uniforms, singleton->shader_file_rd.version_get_shader(version, 0), BASE_UNIFORM_SET);
	shader_rd = singleton->shader_file_rd.version_get_shader(version, 0);

	valid = true;
}

bool RasterizeMeshRD::RasterizeMeshShaderData::is_animated() const {
	return false;
}

bool RasterizeMeshRD::RasterizeMeshShaderData::casts_shadows() const {
	return false;
}

RasterizeMeshRD::RasterizeMeshShaderData::~RasterizeMeshShaderData() {
	singleton->shader_file_rd.version_free(version);
}

void RasterizeMeshRD::RasterizeMeshMaterialData::set_render_priority(int p_priority) {
}

void RasterizeMeshRD::RasterizeMeshMaterialData::set_next_pass(RID p_pass) {
}

bool RasterizeMeshRD::RasterizeMeshMaterialData::update_parameters(const HashMap<StringName, Variant> &p_parameters, bool p_uniform_dirty, bool p_textures_dirty) {
	return update_parameters_uniform_set(p_parameters, p_uniform_dirty, p_textures_dirty, shader_data->uniforms, shader_data->ubo_offsets.ptr(), shader_data->texture_uniforms, shader_data->default_texture_params, shader_data->ubo_size, material_uniforms, singleton->shader_file_rd.version_get_shader(shader_data->version, 0), MATERIAL_UNIFORM_SET, false, false);
}

RendererRD::MaterialStorage::ShaderData *RasterizeMeshRD::_create_rasterize_mesh_shader_funcs() {
	RasterizeMeshShaderData *shader_data = memnew(RasterizeMeshShaderData);
	return shader_data;
}

RendererRD::MaterialStorage::MaterialData *RasterizeMeshRD::_create_rasterize_mesh_material_funcs(RendererRD::MaterialStorage::ShaderData *p_shader) {
	RasterizeMeshMaterialData *material_data = memnew(RasterizeMeshMaterialData);
	material_data->shader_data = static_cast<RasterizeMeshShaderData *>(p_shader);
	return material_data;
}

void RasterizeMeshRD::init() {
	RendererRD::MaterialStorage *material_storage = RendererRD::MaterialStorage::get_singleton();
	// register our shader funds
	material_storage->shader_set_data_request_function(RendererRD::MaterialStorage::SHADER_TYPE_RASTERIZE_MESH, _create_rasterize_mesh_shader_funcs);
	material_storage->material_set_data_request_function(RendererRD::MaterialStorage::SHADER_TYPE_RASTERIZE_MESH, _create_rasterize_mesh_material_funcs);

	{
		//shader compiler
		ShaderCompiler::DefaultIdentifierActions actions;
		actions.renames["VERTEX"] = "vertex";
		actions.renames["UV"] = "uv";
		actions.renames["COLOR"] = "color";

		actions.renames["PI"] = String::num(Math::PI);
		actions.renames["TAU"] = String::num(Math::TAU);
		actions.renames["E"] = String::num(Math::E);
		actions.renames["TIME"] = "time";

		actions.base_texture_binding_index = 1;
		actions.texture_layout_set = MATERIAL_UNIFORM_SET;
		actions.base_uniform_string = "material.";
		actions.default_filter = ShaderLanguage::FILTER_LINEAR;
		actions.default_repeat = ShaderLanguage::REPEAT_DISABLE;
		actions.base_varying_index = 2;

		compiler.initialize(actions);
	}

	String defines = "\n#define SAMPLERS_BINDING_FIRST_INDEX " + itos(SAMPLERS_BINDING_FIRST_INDEX) + "\n";
	shader_file_rd.initialize({ "" }, defines);

	default_shader = material_storage->shader_allocate();
	default_material = material_storage->material_allocate();
	material_storage->shader_initialize(default_shader);
	material_storage->material_initialize(default_material);
	material_storage->shader_set_code(default_shader, "shader_type rasterize_mesh;");
	material_storage->material_set_shader(default_material, default_shader);
}

RID RasterizeMeshRD::get_default_material() const {
	return default_material;
}

RID RasterizeMeshRD::get_default_shader_rd() const {
	return static_cast<RasterizeMeshShaderData *>(MaterialStorage::get_singleton()->material_get_shader_data(default_material))->shader_rd;
}

RasterizeMeshRD::RasterizeMeshRD() {
	singleton = this;
}

RasterizeMeshRD::~RasterizeMeshRD() {
	MaterialStorage::get_singleton()->shader_free(default_shader);
	MaterialStorage::get_singleton()->material_free(default_material);
}

} //namespace RendererRD
