#ifndef GAME_WORLD_TILE_H
#define GAME_WORLD_TILE_H

#include <string>

#include "data/prototype/tile.h"

namespace jactorio::game
{
	// World tile
	class Tile
	{
	public:
		data::Tile* tile_prototype = nullptr;
	};
}

#endif // GAME_WORLD_TILE_H
