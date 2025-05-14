#[vertex]

#version 450

layout(location = 0) in vec3 vertex_attrib;
layout(location = 1) in vec2 uv_attrib;

layout(location = 0) out vec2 uv_interp;

#ifdef MATERIAL_UNIFORMS_USED
/* clang-format off */
layout(set = 1, binding = 0, std140) uniform MaterialUniforms {
#MATERIAL_UNIFORMS
} material;
/* clang-format on */
#endif

#GLOBALS

void main() {
	vec3 vertex = vertex_attrib;
	vec2 uv = uv_attrib;
	{
#CODE : VERTEX
	} uv_interp = uv;
	gl_Position = vec4(vertex, 1.0);
}

#[fragment]

#version 450

layout(location = 0) in vec2 uv_interp;

layout(location = 0) out vec4 frag_color;

#ifdef MATERIAL_UNIFORMS_USED
/* clang-format off */
layout(set = 1, binding = 0, std140) uniform MaterialUniforms {
#MATERIAL_UNIFORMS
} material;
/* clang-format on */
#endif

#GLOBALS

void main() {
	vec2 uv = uv_interp;
	vec4 color = vec4(1.0);
	{
#CODE : FRAGMENT
	} frag_color = color;
}
