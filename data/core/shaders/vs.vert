#version 410 core

layout(location = 0) in vec4 base_VS_in;

out vec4 base_ES_in;

void main() {
	base_ES_in = base_VS_in;
}
