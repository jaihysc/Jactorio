#ifndef GAME_WORLD_TILE_H
#define GAME_WORLD_TILE_H

#include <string>

#include "data/prototype/tile.h"

namespace jactorio::game
{
	// World tile
	class Chunk_tile
	{
	public:
		data::Tile* tile_prototype = nullptr;
		// If this tile has a resource, this points to the the prototype for the resource
		data::Tile* tile_resource_prototype = nullptr;
	};
}

#endif // GAME_WORLD_TILE_H
