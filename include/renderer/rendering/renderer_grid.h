#ifndef RENDERER_RENDERING_RENDERER_GRID_H
#define RENDERER_RENDERING_RENDERER_GRID_H

// Maintains the grid layout used by the renderer
// Examples include:
//		Tile grid based on screen resolution + zoom for rendering the world
//		

namespace jactorio_renderer_rendering
{
	class Renderer_grid
	{
	public:
		/*!
		 * Generates grid positions covering the display area \n
		 * Starts with 0 being top left, ascends going right, then down
		 * @return Array of positions, span of 2
		 */
		static float* gen_render_grid(unsigned short x_max, unsigned short y_max);
	};	
}

#endif // RENDERER_RENDERING_RENDERER_GRID_H
