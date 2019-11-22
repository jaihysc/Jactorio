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
}

#endif // GAME_WORLD_WORLD_MANAGER_H
