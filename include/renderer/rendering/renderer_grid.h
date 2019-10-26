#ifndef RENDERER_RENDERING_RENDERER_GRID_H
#define RENDERER_RENDERING_RENDERER_GRID_H

// Maintains the grid layout used by the renderer
// Examples include:
//		Tile grid based on screen resolution + zoom for rendering the world
//		TODO Texture spritemap positions (Is currently constant)
//		Indices for tile and texture grid

namespace jactorio::renderer::renderer_grid
{
	/**
	 * Generates grid positions with the specified width and height <br>
	 * Starts with 0 being top left, ascends going right, then down <br>
	 * @return array, span of 2
	 */
	float* gen_render_grid(unsigned short vertices_x,
	                       unsigned short vertices_y);

	/**
	 * Generates a set of grid positions for each tile <br>
	 * Each tile will index its own 4 positions <br>
	 *
	 * Starts with 0 being top left, ascends going right, then down
	 * @param tiles_x count of tiles on X axis
	 * @param tiles_y count of tiles on Y axis
	 * @param tile_width Width of a tile in pixels
	 * @return array, span of 2
	 */
	float* gen_render_tile_grid(unsigned short tiles_x,
	                            unsigned short tiles_y,
	                            unsigned short tile_width);


	/**
	 * Generates texture positions using  tile positions from gen_render_grid_indices
	 * @return array, span of 2
	 */
	float* gen_texture_grid(unsigned int elements_count);


	/**
	 * Generates indices to draw tiles using the grid from gen_render_grid
	 * @returns Indices to be feed into Index_buffer
	 */
	unsigned int* gen_render_grid_indices(
		unsigned short tiles_x, unsigned short tiles_y);
}

#endif // RENDERER_RENDERING_RENDERER_GRID_H
