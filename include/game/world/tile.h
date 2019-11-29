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
		// Data at pointers does not need to be freed, it is managed by data_manager

		// Order in prototypes determines render order, last item is on top
		std::vector<data::Tile*> tile_prototypes;
		
		// If this tile has a resource, this points to the the prototype for the resource
		data::Tile* tile_resource_prototype = nullptr;
	};
}

#endif // GAME_WORLD_TILE_H
