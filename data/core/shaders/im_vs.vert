#version 410 core

layout (location = 0) in vec2 Position;
layout (location = 1) in vec2 UV;
layout (location = 2) in vec4 Color;

uniform mat4 u_proj_mtx;

out vec2 tex_coord_FS_in;
out vec4 frag_color_FS_in;

void main() {
    tex_coord_FS_in = UV;
    frag_color_FS_in = Color;
    gl_Position = u_proj_mtx * vec4(Position.xy, 0, 1);
}

