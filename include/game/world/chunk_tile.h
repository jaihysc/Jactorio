#ifndef GAME_WORLD_TILE_H
#define GAME_WORLD_TILE_H

#include "data/prototype/tile/tile.h"

namespace jactorio::game
{	
	// World tile
	class Chunk_tile
	{
	public:
		Chunk_tile();

		/**
		 * Is this tile a water tile?
		 * If so, resources will not be placed on this tile, navigation on this tile will be disallowed
		 */
		bool is_water = false;
		
		/**
		 * Maximum number of layers a chunk_tile will have
		 */
		static constexpr int tile_prototypes_count = 3;
		
		// Data at pointers does not need to be freed, it is managed by data_manager

		enum class prototype_category
		{
			base = 0,
			resource = 1,
			// Cursor highlights, inserter arrows, etc
			overlay = 2
		};

		/**
		 * To access prototype at each location, cast desired prototype_category to int and index
		 * tile_prototypes
		 */
		data::Tile* tile_prototypes[tile_prototypes_count];

		[[nodiscard]] data::Tile* get_tile_prototype(prototype_category category) const;
		void set_tile_prototype(prototype_category category, data::Tile* tile_prototype);
	};
}

#endif // GAME_WORLD_TILE_H
