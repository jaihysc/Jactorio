#ifndef DATA_PROTOTYPE_TILE_RESOURCE_TILE_H
#define DATA_PROTOTYPE_TILE_RESOURCE_TILE_H

#include "tile.h"

#include "jactorio.h"

namespace jactorio::data
{
	class Resource_tile : public Tile
	{
		using Tile::Tile;
	public:
		Resource_tile()
			: product(nullptr), resource_amount(1) {
		}

		~Resource_tile() override = default;

		/**
		 * Item given when this resource is extracted
		 */
		PYTHON_PROP(Resource_tile, Item*, product)

		
		// Unique per tile

		/**
		 * Amount of product which can still be extracted from this tile
		 */
		PYTHON_PROP(Resource_tile, uint16_t, resource_amount)
	};
}

#endif // DATA_PROTOTYPE_TILE_RESOURCE_TILE_H
