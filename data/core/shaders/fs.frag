#version 330 core

layout(location = 0) out vec4 color;

uniform vec4 u_color;
uniform sampler2D u_texture;

in vec2 v_tex_coord;

void main() {
	vec4 tex_color = texture(u_texture, v_tex_coord);
	color = tex_color;
}