#version 410 core

layout(quads, equal_spacing, cw) in;

uniform mat4 u_model_view_projection_matrix;
uniform vec4 u_tex_coords[1]; // TODO add replaceable constant

in vec4 base_ES_in[];

out vec2 tex_coord_FS_in;

void main() {
	tex_coord_FS_in = vec2(
		mix(u_tex_coords[floatBitsToUint(base_ES_in[0].w)].x, u_tex_coords[floatBitsToUint(base_ES_in[0].w)].y, gl_TessCoord.x),
		mix(u_tex_coords[floatBitsToUint(base_ES_in[0].w)].z, u_tex_coords[floatBitsToUint(base_ES_in[0].w)].w, gl_TessCoord.y)
	);

	gl_Position = vec4(vec3(base_ES_in[0]), 1);
	gl_Position.xy += gl_TessCoord.xy;
	gl_Position = u_model_view_projection_matrix * gl_Position;
}

