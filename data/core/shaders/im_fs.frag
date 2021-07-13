#version 410 core

layout (location = 0) out vec4 color;

in vec2 tex_coord_FS_in;
in vec4 frag_color_FS_in;

uniform sampler2D u_texture;

void main() {
    color = frag_color_FS_in * texture(u_texture, tex_coord_FS_in.st);
}

