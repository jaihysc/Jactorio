#ifndef GAME_WORLD_LOGIC_CHUNK_H
#define GAME_WORLD_LOGIC_CHUNK_H

#include "game/world/chunk.h"

#include <map>

namespace jactorio::game
{
	/**
	 * Contains additional data for chunks that require logic updates <br>
	 * To be stored alongside a game::Chunk
	 */
	class Logic_chunk
	{
	public:
		explicit Logic_chunk(Chunk* chunk)
			: chunk(chunk){
			assert(chunk != nullptr);  // Logic chunks should always be associated with a chunk
		}

		// The chunk which this logic chunk is associated with
		const Chunk* chunk;

		
		// The directions for items on belts stored on this chunk
		/**
		 * Directions:
		 * 0 = none,
		 * 1 = up,
		 * 2 = right,
		 * 3 = down,
		 * 4 = left
		 */
		// <Index for item in Chunk_object_layer vector, Direction>
		std::map<uint32_t, uint8_t> item_direction;
	};
}

#endif // GAME_WORLD_LOGIC_CHUNK_H