/**************************************************************************/
/*  rasterize_mesh.h                                                      */
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

#pragma once

#include "servers/rendering/renderer_rd/shaders/effects/rasterize_mesh.glsl.gen.h"
#include "servers/rendering/renderer_rd/storage_rd/material_storage.h"

namespace RendererRD {

class RasterizeMeshRD {
	static RasterizeMeshRD *singleton;

	ShaderCompiler compiler;

	static RendererRD::MaterialStorage::ShaderData *_create_rasterize_mesh_shader_funcs();
	static RendererRD::MaterialStorage::MaterialData *_create_rasterize_mesh_material_funcs(RendererRD::MaterialStorage::ShaderData *p_shader);

public:
	struct RasterizeMeshShaderData : public RendererRD::MaterialStorage::ShaderData {
		RasterizeMeshShaderRD shader_rd;
		RID version;
		bool valid = false;
		Vector<ShaderCompiler::GeneratedCode::Texture> texture_uniforms;
		Vector<uint32_t> ubo_offsets;
		uint32_t ubo_size = 0;

		String code;

		virtual void set_code(const String &p_Code);
		virtual bool is_animated() const;
		virtual bool casts_shadows() const;
	};

	struct RasterizeMeshMaterialData : public RendererRD::MaterialStorage::MaterialData {
		RasterizeMeshShaderData *shader_data = nullptr;
		RID uniform_set;

		virtual void set_render_priority(int p_priority);
		virtual void set_next_pass(RID p_pass);
		virtual bool update_parameters(const HashMap<StringName, Variant> &p_parameters, bool p_uniform_dirty, bool p_textures_dirty);
	};

	static RasterizeMeshRD *get_singleton();
	void init();
	RasterizeMeshRD();
};
} //namespace RendererRD
