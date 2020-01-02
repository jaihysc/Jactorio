#ifndef DATA_PROTOTYPE_TILE_RESOURCE_TILE_H
#define DATA_PROTOTYPE_TILE_RESOURCE_TILE_H

#include "tile.h"

namespace jactorio::data
{
	class Resource_tile : public Tile
	{
		using Tile::Tile;
	public:
		Resource_tile() = default;

		~Resource_tile() override = default;
	};
}

#endif // DATA_PROTOTYPE_TILE_RESOURCE_TILE_H
