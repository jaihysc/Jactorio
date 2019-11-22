#ifndef RENDERER_RENDERING_WORLD_RENDERER_H
#define RENDERER_RENDERING_WORLD_RENDERER_H

#include "renderer/rendering/renderer.h"

namespace jactorio::renderer::world_renderer
{
	inline long long player_position_x = 0;
	inline long long player_position_y = 0;

	/**
	 * Draws chunks to the screen
	 * Attempting to draw chunks which do not exist will result in undefined behavior
	 *
	 * @param renderer Renderer on which to draw the chunks
	 * @param window_start_x Position on window to begin drawing upper left corner of chunks
	 * @param window_start_y Position on window to begin drawing upper left corner of chunks
	 * @param chunk_start_x Position in chunk grid to begin drawing
	 * @param chunk_start_y Position in chunk grid to begin drawing
	 * @param chunk_amount_x How many chunks to draw on x axis to the right ->
	 * @param chunk_amount_y How many chunks to draw on y axis downwards \/
	 */
	void draw_chunks(const Renderer& renderer, int window_start_x,
	                 int window_start_y,
	                 int chunk_start_x, int chunk_start_y, unsigned chunk_amount_x, unsigned
	                 chunk_amount_y);
	
	/**
	 * Moves the world to match player_position_ <br>
	 * This is achieved by offsetting the rendered chunks, for decimal numbers, the view matrix is used
	 * @param renderer The renderer on which the world is drawn
	 */
	void render_player_position(Renderer* renderer);
}

#endif // RENDERER_RENDERING_WORLD_RENDERER_H
