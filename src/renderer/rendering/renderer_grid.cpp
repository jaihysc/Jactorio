// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/22/2019

#include "renderer/rendering/renderer_grid.h"

unsigned* jactorio::renderer::renderer_grid::gen_render_grid_indices(uint32_t tile_count) {

	// Indices generation pattern:
	// top left
	// top right
	// bottom right
	// bottom left

	const auto positions = new unsigned int[tile_count * 6];

	unsigned int positions_index = 0;
	unsigned int index_buffer_index = 0; // Index to be saved into positions

	for (auto i = 0; i < tile_count; ++i) {
		positions[positions_index++] = index_buffer_index;
		positions[positions_index++] = index_buffer_index + 1;
		positions[positions_index++] = index_buffer_index + 2;

		positions[positions_index++] = index_buffer_index + 2;
		positions[positions_index++] = index_buffer_index + 3;
		positions[positions_index++] = index_buffer_index;

		index_buffer_index += 4;

		// const unsigned int start = x + y * tiles_x;
		//
		// positions[index++] = start;
		// positions[index++] = start + 1;
		// positions[index++] = start + 1 + tiles_x;
		//
		// positions[index++] = start + 1 + tiles_x;
		// positions[index++] = start + tiles_x;
		// positions[index++] = start;
	}

	return positions;
}
