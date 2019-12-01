#ifndef GAME_WORLD_TILE_H
#define GAME_WORLD_TILE_H

#include <vector>

#include "data/prototype/tile/tile.h"

namespace jactorio::game
{
	// World tile
	class Chunk_tile
	{
	public:
		/**
		 * Is this tile a water tile?
		 * If so, resources will not be placed on this tile, navigation on this tile will be disallowed
		 */
		bool is_water = false;
		
		// Data at pointers does not need to be freed, it is managed by data_manager

		// Order in prototypes determines render order, last item is on top
		std::vector<data::Tile*> tile_prototypes;
	};
}

#endif // GAME_WORLD_TILE_H
