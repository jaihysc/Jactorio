#version 410 core

layout(quads, equal_spacing, cw) in;

uniform mat4 u_model_view_projection_matrix;

in vec4 base_ES_in[];

out vec2 tex_coord_FS_in;

void main() {
	tex_coord_FS_in = vec2(
		mix(0, 1, gl_TessCoord.x),
		mix(0, 1, gl_TessCoord.y)
	);

	gl_Position = base_ES_in[0];
	gl_Position.xy += gl_TessCoord.xy;
	gl_Position = u_model_view_projection_matrix * gl_Position;
}

