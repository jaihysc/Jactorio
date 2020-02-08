#ifndef RENDERER_RENDERING_TILE_RENDERER_H
#define RENDERER_RENDERING_TILE_RENDERER_H

#include "renderer/rendering/renderer.h"
#include "game/world/chunk_tile.h"

/**
 * Renders items utilizing the grid system of the map
 * For items which exists across tiles, use position_renderer
 */
namespace jactorio::renderer::tile_renderer
{
	using tile_draw_func = unsigned int (*)(const game::Chunk_tile&);

	/**
	 * Draws chunks to the screen
	 * Attempting to draw chunks which do not exist will result in undefined behavior
	 *
	 * @param layer_index Index of layer to render
	 * @param window_start_x Position on window to begin drawing upper left corner of chunks
	 * @param window_start_y Position on window to begin drawing upper left corner of chunks
	 * @param chunk_start_x Position in chunk grid to begin drawing
	 * @param chunk_start_y Position in chunk grid to begin drawing
	 * @param chunk_amount_x Number of chunks on X axis to draw
	 * @param chunk_amount_y Number of chunks on Y axis to draw
	 * @param buffer UV draw data will be placed in this
	 * @param buffer_span Span of the buffer, in element count
	 * @param buffer_height Height of the buffer, in element count
	 */
	void draw_chunks(
		unsigned layer_index, 
		int window_start_x, int window_start_y, 
		int chunk_start_x, int chunk_start_y,
		int chunk_amount_x, int chunk_amount_y,
		float* buffer, unsigned short buffer_span, unsigned short buffer_height);

	/**
	 * Moves the world to match player_position_ <br>
	 * This is achieved by offsetting the rendered chunks, for decimal numbers, the view matrix is used
	 * @param renderer The renderer on which the world is drawn
	 * @param player_x X Position of the player in tiles
	 * @param player_y Y Position of the player in tiles
	 */
	void render_player_position(Renderer* renderer, float player_x, float player_y);
}

#endif // RENDERER_RENDERING_TILE_RENDERER_H
