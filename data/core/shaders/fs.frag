#version 410 core
precision mediump float;

layout(location = 0) out vec4 color;

uniform sampler2D u_texture;

in vec2 tex_coord_FS_in;

void main() {
	color = texture(u_texture, tex_coord_FS_in);
}
