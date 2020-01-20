#ifndef RENDERER_RENDERING_RENDERER_GRID_H
#define RENDERER_RENDERING_RENDERER_GRID_H

#include "jactorio.h"

#include "renderer/rendering/renderer_layer.h"

// Maintains the grid layout used by the renderer
// Examples include:
//		Tile grid based on screen resolution + zoom for rendering the world
//		Default Texture spritemap positions
//		Indices for tile and texture grid

/**
 * Maintains the grid layout used by the renderer
 * Pointers returned by the functions MUST BE manually freed
 */
namespace jactorio::renderer::renderer_grid
{
	void gen_render_grid(Renderer_layer* r_layer, uint16_t tiles_x, uint16_t tiles_y, uint16_t tile_width);

	// Functions below are DEPRECATED, they are unsafe in memory allocation
	
	/**
	 * Generates grid positions with the specified width and height <br>
	 * Starts with 0 being top left, ascends going right, then down <br>
	 * 
	 * Use gen_render_tile_grid(#, #, 1) instead
	 * @return array, span of 2
	 */
	J_DEPRECATED float* gen_render_grid(unsigned short vertices_x, unsigned short vertices_y);

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
	J_DEPRECATED float* gen_render_tile_grid(unsigned short tiles_x,
	                                         unsigned short tiles_y,
	                                         unsigned short tile_width);


	/**
	 * Generates texture positions using  tile positions from gen_render_grid_indices
	 * @return array, span of 2
	 */
	J_DEPRECATED float* gen_texture_grid(unsigned int elements_count);


	/**
	 * Generates indices to draw tiles using the grid from gen_render_grid
	 * @returns Indices to be feed into Index_buffer
	 */
	unsigned int* gen_render_grid_indices(uint32_t tile_count);
}

#endif // RENDERER_RENDERING_RENDERER_GRID_H
