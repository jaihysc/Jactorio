#version 410 core

layout(quads, equal_spacing, cw) in;

uniform mat4 u_model_view_projection_matrix;
uniform vec4 u_tex_coords[__terrain_tex_coords_size]; // x1, y1, x2, y2 // Replaced with constant during Jactorio shader compilation

in uvec4 data_ES_in[];

out vec2 tex_coord_FS_in;

void main() {
	tex_coord_FS_in = vec2(
		// x1, x2
		// y1, y2
		mix(u_tex_coords[data_ES_in[0].w].x, u_tex_coords[data_ES_in[0].w].z, gl_TessCoord.x),
		mix(u_tex_coords[data_ES_in[0].w].y, u_tex_coords[data_ES_in[0].w].w, gl_TessCoord.y)
	);

	gl_Position = vec4(vec3(data_ES_in[0]), 1);
	gl_Position.xy += gl_TessCoord.xy;
	gl_Position = u_model_view_projection_matrix * gl_Position;
}

