#version 300 es
precision mediump float;

layout(location = 0) out vec4 color;

uniform sampler2D u_texture;

in vec2 v_tex_coord;

void main() {
	color = texture(u_texture, v_tex_coord);
}
