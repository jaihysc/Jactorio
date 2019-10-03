#ifndef RENDERER_RENDERING_RENDERER_GRID_H
#define RENDERER_RENDERING_RENDERER_GRID_H

// Maintains the grid layout used by the renderer
// Examples include:
//		Tile grid based on screen resolution + zoom for rendering the world
//		TODO Texture spritemap positions (Is currently constant)
//		Indices for tile and texture grid

namespace jactorio_renderer_rendering
{
	class Renderer_grid
	{
	public:
		/*!
		 * Generates grid positions with the specified width and height \n
		 * Starts with 0 being top left, ascends going right, then down \n
		 * @return array, span of 2
		 */
		static float* gen_render_grid(unsigned short vertices_x, unsigned short vertices_y);

		/*!
		 * Generates a set of grid positions for each tile \n
		 * Each tile will index its own 4 positions
		 * Starts with 0 being top left, ascends going right, then down \n
		 * @return array, span of 2
		 */
		static float* gen_render_tile_grid(unsigned short tiles_x, unsigned short tiles_y);

		
		/*!
		 * Generates texture positions using the tile positions specified from \n
		 * gen_render_grid_indices
		 * @return array, span of 2
		 */
		static float* gen_texture_grid(unsigned int elements_count);

		
		/*!
		 * Generates indices to draw tiles using the grid from gen_render_grid
		 * @returns Indices, feed into Index_buffer
		 */
		static unsigned int* gen_render_grid_indices(unsigned short tiles_x, unsigned short tiles_y);
		
	};	
}

#endif // RENDERER_RENDERING_RENDERER_GRID_H
