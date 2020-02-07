#ifndef GAME_WORLD_CHUNK_TILE_LAYER_H
#define GAME_WORLD_CHUNK_TILE_LAYER_H

#include "data/prototype/prototype_base.h"

namespace jactorio::game
{
	/**
	 * A Chunk_tile has many layers <br>
	 * This will NOT delete the provided pointers, it is managed by data_manager
	 */
	struct Chunk_tile_layer
	{
		Chunk_tile_layer() = default;

		explicit Chunk_tile_layer(data::Prototype_base* proto)
			: prototype_data(proto) {
		}

		// Prototype data must be deleted after chunk data
		~Chunk_tile_layer() {
			delete_unique_data();
		}

		Chunk_tile_layer(const Chunk_tile_layer& other);
		Chunk_tile_layer(Chunk_tile_layer&& other) noexcept;

		Chunk_tile_layer& operator=(const Chunk_tile_layer& other);
		Chunk_tile_layer& operator=(Chunk_tile_layer&& other) noexcept;

		/**
		 * Deletes unique_data, resets pointers to nullptr, resets multi_tile members
		 */
		void clear();

	private:
		void delete_unique_data() const;
		
	public:
		// ============================================================================================
		// Minimize the variables below vvvvvv
		/*
		 * A layer may point to a tile prototype to provide additional data (collisions, world gen) <br>
		 * The sprite is stored within the tile prototype instead <br>
		 * If not provided, it is nullptr <br>
		 */

		/*
		 * Entities also possesses a sprite pointer within their prototype
		 */

		/**
		 * Depending on the layer, this will be either a data::Tile*, data::Entity* or a data::Sprite* <br>
		 */
		data::Prototype_base* prototype_data = nullptr;

		/**
		 * Data for the prototype which is unique per tile and layer <br>
		 * When this layer is deleted, unique_data_ will be deleted with delete method in prototype_data_
		 */
		void* unique_data = nullptr;

		/**
		 * If the layer is multi-tile, eg: 3 x 3, this holds the index of this tile (see below) <br>
		 * Non multi tile has 0 for index, but it can also exist on multi tile tiles <br>
		 * 0 1 2 <br>
		 * 3 4 5 <br>
		 * 6 7 8 <br>
		 */
		uint8_t multi_tile_index = 0;

		// TODO, somehow store these variables elsewhere instead of having a copy for each tile
		/**
		 * How many tiles does this sprite span? <br>
		 * Only the width is needed for most calculations
		 */
		uint8_t multi_tile_span = 1;
		uint8_t multi_tile_height = 1;
	};
}

#endif // GAME_WORLD_CHUNK_TILE_LAYER_H
