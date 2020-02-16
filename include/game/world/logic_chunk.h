#ifndef GAME_WORLD_LOGIC_CHUNK_H
#define GAME_WORLD_LOGIC_CHUNK_H

#include "game/world/chunk.h"
#include "data/prototype/item/item.h"

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
		Logic_chunk() {
			assert(false);  // A chunk pointer needs to be provided - Was there a logic chunk created when it was accessed?
		}
		
		explicit Logic_chunk(Chunk* chunk)
			: chunk(chunk){
			assert(chunk != nullptr);  // Logic chunks should always be associated with a chunk
		}

		// The chunk which this logic chunk is associated with
		const Chunk* chunk;

		
		// If an item's positions matches against an entry within this map, its movement direction should
		// be updated to the one provided
		std::map<std::pair<float, float>, data::Transport_line_item_data::move_dir> transport_line_updates{};
	};
}

#endif // GAME_WORLD_LOGIC_CHUNK_H