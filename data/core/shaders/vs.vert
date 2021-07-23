#version 410 core

layout(location = 0) in uvec4 data_VS_in; // Data prepared by renderer

out uvec4 data_ES_in;

void main() {
	data_ES_in = data_VS_in;
}
