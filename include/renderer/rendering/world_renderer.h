#ifndef RENDERER_RENDERING_TILE_RENDERER_H
#define RENDERER_RENDERING_TILE_RENDERER_H

#include "renderer/rendering/renderer.h"

/**
 * Renders items utilizing the grid system of the map
 * For items which exists across tiles, use position_renderer
 */
namespace jactorio::renderer::world_renderer
{
	/**
	 * Draws chunks to the screen
	 * Attempting to draw chunks which do not exist will result in the chunk being queued for generation
	 * @param layer_index Index of layer to render
	 * @param is_tile_layer Is the provided layer_index for a tile layer? If false is object layer
	 * @param render_offset_x Tiles from window left to offset rendering
	 * @param render_offset_y Tiles from window top to offset rendering
	 * @param chunk_start_x Chunk to begin rendering
	 * @param chunk_start_y Chunk to begin rendering
	 * @param chunk_amount_x Number of chunks on X axis after chunk_start_x to draw
	 * @param chunk_amount_y Number of chunks on Y axis after chunk_start_y to draw
	 * @param layer Layer on which vertex and UV draw data will be placed
	 */
	void prepare_chunk_draw_data(int layer_index, bool is_tile_layer, 
	                             int render_offset_x, int render_offset_y, 
	                             int chunk_start_x, int chunk_start_y, 
	                             int chunk_amount_x, int chunk_amount_y, 
	                             Renderer_layer* layer);

	/**
	 * Moves the world to match player_position_ <br>
	 * This is achieved by offsetting the rendered chunks, for decimal numbers, the view matrix is used
	 * @param renderer The renderer on which the world is drawn
	 * @param player_x X Position of the player in tiles
	 * @param player_y Y Position of the player in tiles
	 */
	void render_player_position(Renderer* renderer, float player_x, float player_y);

	// Within game::Chunk, store a vector for positioned items (trees, items, etc)
	// The tile renderer is obviously faster since everything belongs to a known count of tiles
	//
	// The position stored by each element within the vector should be a float offset from the top left of the chunk
	// where 1 = 1 tile
	// Each element is behaviorally identical to Chunk_tile_layer without multi tile obviously,
	// make another class to use which the current Chunk_tile_layer will inherit from (without the multi tile stuff)
	//
	// I also need to store the size of these sprites, they will store their tile span X, Y

	// WHen items are on the ground, they exist as entities within their respective chunk
	// Items on transport belts however:
	//		Transport belt connected together are group by "segments" assigned by an id
	//		The segment holds all the items within it (Therefore no need to move item entities between chunks)

}

#endif // RENDERER_RENDERING_TILE_RENDERER_H
