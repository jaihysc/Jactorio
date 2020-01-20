#ifndef GAME_WORLD_WORLD_MANAGER_H
#define GAME_WORLD_WORLD_MANAGER_H

#include "game/world/chunk.h"

// Manages the game world, the tiles and the entities on it
// Handles saving and loading the world
namespace jactorio::game::world_manager
{
	// The world is make up of chunks
	// Each chunk contains 32 x 32 tiles
	// 
	// Chunks increment heading right and down

	/**
	 * Adds a chunk into the game world <br>
	 * Will overwrite existing chunks if they occupy the same position, the overriden chunk's
	 * destructor will be called <br>
	 * Do NOT delete the provided chunk pointer, it will be automatically deleted
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
	 * Gets the tile at the specified world coordinate
	 * @return nullptr if no tile exists
	 */
	Chunk_tile* get_tile_world_coords(int world_x, int world_y);

	/**
	 * Gets the tile the mouse is hovered over
	 */
	Chunk_tile* get_mouse_selected_tile();
	
	/**
	 * Erases, frees memory from all stored chunk data + its subsequent contents
	 */
	void clear_chunk_data();
}

#endif // GAME_WORLD_WORLD_MANAGER_H
