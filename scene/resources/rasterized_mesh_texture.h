/**************************************************************************/
/*  rasterized_mesh_texture.h                                             */
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

#include "scene/resources/texture.h"

class Mesh;

class RasterizedMeshTexture : public Texture2D {
	GDCLASS(RasterizedMeshTexture, Texture2D);

	static const String _vertex_code;
	static const String _fragment_code;

	Size2i size = Size2i(256, 256);
	Ref<Texture2D> base_texture;
	Ref<Mesh> mesh;
	Color bg_color = Color(0, 0, 0, 0);
	Projection projection;
	bool generate_mipmaps = false;

	RID texture;

	RID framebuffer_texture_id;
	RID framebuffer_id;
	RID vertex_array_id;
	RID index_array_id;
	RID pipeline_id;
	RID sampler_id;
	RID uniform_set_id;
	RID index_buffer_id;
	RID vertex_buffer_pos_id;
	RID vertex_buffer_uv_id;

	long vertex_format;
	bool is_2d_mesh = false;
	RD::RenderPrimitive primitive_type = RD::RENDER_PRIMITIVE_TRIANGLES;

	bool pipeline_dirty = false;
	bool mesh_dirty = false;
	bool uniform_set_dirty = false;

	bool update_queued = false;

	RD::PipelineRasterizationState pipeline_rasterization_state;
	RD::TextureFormat tex_format;

	RD::Uniform uniform_tex;
	Vector<RD::VertexAttribute> vertex_attrs;
	RD::TextureView tex_view;
	RD::PipelineMultisampleState pipeline_multisample_state;
	RD::PipelineDepthStencilState pipeline_depth_stencil_state;
	RD::PipelineColorBlendState pipeline_color_blend_state;
	RD::SamplerState sampler_state;

	static Mutex shader_mutex;
	static RID shader_cache;

	static void _update_shader();

protected:
	static void _bind_methods();

public:
	static void cleanup_shader();

	int get_width() const override;
	int get_height() const override;
	bool has_alpha() const override;
	RID get_rid() const override;

	Ref<Image> get_image() const override;

	void set_width(int p_width);
	void set_height(int p_height);

	void set_mesh(const Ref<Mesh> &p_mesh);
	Ref<Mesh> get_mesh() const;

	void set_base_texture(const Ref<Texture2D> &p_base_texture);
	Ref<Texture2D> get_base_texture() const;

	void set_bg_color(const Color &p_color);
	Color get_bg_color() const;

	void set_projection(const Projection &p_projection);
	Projection get_projection() const;

	void set_generate_mipmaps(bool p_generate_mipmaps);
	bool is_generating_mipmaps() const;

	RasterizedMeshTexture();
	~RasterizedMeshTexture();

private:
	void update();

	void queue_update();
	void queue_update_pipeline();
	void queue_update_uniform_set();
	void queue_update_mesh();

	void reset_vertex();
	void reset_pipeline();
	void reset_uniform();
	void draw_list_draw();

	void create_mipmaps();
};
