#include "renderer/rendering/renderer_grid.h"

float* jactorio_renderer_rendering::Renderer_grid::gen_render_grid(const unsigned short vertices_x,
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

float* jactorio_renderer_rendering::Renderer_grid::gen_render_tile_grid(const unsigned short tiles_x,
                                                                        const unsigned short tiles_y) {
	const auto grid = new float[
		static_cast<unsigned long long>(tiles_x) * static_cast<unsigned long long>(tiles_y) * 8];

	unsigned int index = 0;
	for (unsigned short y = 0; y < tiles_y; ++y) {
		for (unsigned short x = 0; x < tiles_x; ++x) {
			// Top left
			grid[index++] = static_cast<float>(x);
			grid[index++] = static_cast<float>(y);

			// Top right
			grid[index++] = static_cast<float>(x + 1);  // 1 tile right
			grid[index++] = static_cast<float>(y);


			// Bottom right
			grid[index++] = static_cast<float>(x + 1);  // 1 tile right
			grid[index++] = static_cast<float>(y + 1);  // 1 tile down

			// Bottom left
			grid[index++] = static_cast<float>(x);
			grid[index++] = static_cast<float>(y + 1);  // 1 tile down
		}
	}

	return grid;
}

float* jactorio_renderer_rendering::Renderer_grid::gen_texture_grid(const unsigned int elements_count) {
	const auto tex_coords = new float[static_cast<unsigned long long>(elements_count) * 4 * 2];

	unsigned short index = 0;
	for (unsigned int i = 0; i < elements_count; ++i) {
		tex_coords[index++] = 0.f; tex_coords[index++] = 1.f,  // bottom left
		tex_coords[index++] = 1.f; tex_coords[index++] = 1.f;  // bottom right
		tex_coords[index++] = 1.f; tex_coords[index++] = 0.f;  // upper right
		tex_coords[index++] = 0.f; tex_coords[index++] = 0.f;  // upper left
	}

	return tex_coords;
}

unsigned* jactorio_renderer_rendering::Renderer_grid::gen_render_grid_indices(const unsigned short tiles_x,
                                                                              const unsigned short tiles_y) {

	// Indices generation pattern:
	// top left
	// top right
	// bottom right
	// bottom left

	const auto positions = new unsigned int[
		static_cast<unsigned long long>(tiles_x) * static_cast<unsigned long long>(tiles_y) * 6];

	unsigned int positions_index = 0;
	unsigned int index_buffer_index = 0;  // Index to be saved into positions
	for (auto y = 0; y < tiles_y; ++y) {
		for (auto x = 0; x < tiles_x; ++x) {
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
	}
	
	return positions;
}
