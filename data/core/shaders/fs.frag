#version 300 es
precision mediump float;

layout(location = 0) out vec4 color;

uniform vec4 u_color;
uniform sampler2D u_texture;

in vec2 v_tex_coord;

void main() {
	if (v_tex_coord.x != -1.f && v_tex_coord.y != -1.f) {
		color = texture(u_texture, v_tex_coord);
	}
	else {
		color =  vec4(0.0, 0.0, 0.0, 0.0);
	}
}
