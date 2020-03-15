// 
// renderer_grid.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 10/22/2019
// Last modified: 03/14/2020
// 

#include "renderer/rendering/renderer_grid.h"

void jactorio::renderer::renderer_grid::gen_render_grid(Renderer_layer* r_layer,
                                                        const uint16_t tiles_x, const uint16_t tiles_y,
                                                        const uint16_t tile_width) {
	const auto required_size = static_cast<unsigned long long>(tiles_x) * static_cast<unsigned long long>(tiles_y);

	// Resize up if necessary
	if (r_layer->get_capacity() < required_size)
		r_layer->resize(required_size);


	unsigned int index = 0;
	for (unsigned short y = 0; y < tiles_y; ++y) {
		// Multiply to create specified tile width
		const auto local_y = y * tile_width;

		for (unsigned short x = 0; x < tiles_x; ++x) {
			const auto local_x = x * tile_width;

			r_layer->set_vertex(
				index++,
				core::Quad_position(
					core::Position2<float>(local_x, local_y),
					core::Position2<float>(local_x + tile_width, local_y + tile_width)
				)
			);
		}
	}
}

float* jactorio::renderer::renderer_grid::gen_render_grid(
	const unsigned short vertices_x,
	const unsigned short vertices_y) {

	const auto grid = new float[
		static_cast<unsigned long long>(vertices_x) * vertices_y * 2];

	unsigned int index = 0;
	for (unsigned int y = 0; y < vertices_x; ++y) {
		for (unsigned int x = 0; x < vertices_y; ++x) {
			// 1 coordinate position (vertex)
			grid[index++] = static_cast<float>(x);
			grid[index++] = static_cast<float>(y);
		}
	}

	return grid;
}

float* jactorio::renderer::renderer_grid::gen_render_tile_grid(
	const unsigned short tiles_x, const unsigned short tiles_y, const unsigned short tile_width) {

	const auto grid = new float[
		static_cast<unsigned long long>(tiles_x) * static_cast<unsigned long long>(tiles_y) * 8];

	unsigned int index = 0;
	for (unsigned short y = 0; y < tiles_y; ++y) {
		// Multiply to create specified tile width
		const auto local_y = y * tile_width;

		for (unsigned short x = 0; x < tiles_x; ++x) {
			const auto local_x = x * tile_width;

			// Top left
			grid[index++] = static_cast<float>(local_x);
			grid[index++] = static_cast<float>(local_y);

			// Top right
			grid[index++] = static_cast<float>(local_x + tile_width); // 1 tile right
			grid[index++] = static_cast<float>(local_y);


			// Bottom right
			grid[index++] = static_cast<float>(local_x + tile_width); // 1 tile right
			grid[index++] = static_cast<float>(local_y + tile_width); // 1 tile down

			// Bottom left
			grid[index++] = static_cast<float>(local_x);
			grid[index++] = static_cast<float>(local_y + tile_width); // 1 tile down
		}
	}

	return grid;
}

float* jactorio::renderer::renderer_grid::gen_texture_grid(
	const unsigned int elements_count) {
	const auto tex_coords = new float[static_cast<unsigned long long>(
		elements_count) * 4 * 2];

	unsigned short index = 0;
	for (unsigned int i = 0; i < elements_count; ++i) {
		// tex_coords[index++] = 0.f;
		// tex_coords[index++] = 1.f, // bottom left
		// tex_coords[index++] = 1.f;
		// tex_coords[index++] = 1.f; // bottom right
		// tex_coords[index++] = 1.f;
		// tex_coords[index++] = 0.f; // upper right
		// tex_coords[index++] = 0.f;
		// tex_coords[index++] = 0.f; // upper left

		tex_coords[index++] = 0.f;
		tex_coords[index++] = 0.f, // bottom left
			tex_coords[index++] = 0.f;
		tex_coords[index++] = 0.f; // bottom right
		tex_coords[index++] = 0.f;
		tex_coords[index++] = 0.f; // upper right
		tex_coords[index++] = 0.f;
		tex_coords[index++] = 0.f; // upper left
	}

	return tex_coords;
}

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
