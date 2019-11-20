#ifndef GAME_WORLD_WORLD_MANAGER_H
#define GAME_WORLD_WORLD_MANAGER_H

#include "game/world/chunk.h"
#include "renderer/rendering/renderer.h"

// Manages the game world, the tiles and the entities on it
// Handles saving and loading the world
namespace jactorio::game::world_manager
{
	// The world is make up of chunks
	// Each chunk contains 32 x 32 tiles
	// 
	// Chunks increment heading right and down

	/**
	 * Adds a chunk into the game world
	 * @param chunk Chunk to be added to the world
	 * @return Pointer to added chunk
	 */
	Chunk* add_chunk(Chunk* chunk);

	/**
	 * Retrieves a chunk in game world
	 * @param x X position of CHUNK
	 * @param y Y position of CHUNK
	 */
	Chunk* get_chunk(int x, int y);
	
	/**
	 * Erases, frees memory from all stored chunk data + its subsequent contents
	 */
	void clear_chunk_data();

	
	// Rendering

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
	void draw_chunks(const renderer::Renderer& renderer, const int window_start_x,
	                 const int window_start_y,
	                 int chunk_start_x, int chunk_start_y, unsigned chunk_amount_x, unsigned
	                 chunk_amount_y);
}

#endif // GAME_WORLD_WORLD_MANAGER_H
