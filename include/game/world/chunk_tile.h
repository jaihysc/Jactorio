#ifndef GAME_WORLD_TILE_H
#define GAME_WORLD_TILE_H

#include "jactorio.h"
#include "data/prototype/tile/tile.h"
#include "data/prototype/entity/entity.h"

namespace jactorio::game
{
	/**
	 * A Chunk_tile has many layers <br>
	 * This will NOT delete the provided pointers, it is managed by data_manager
	 */
	struct Chunk_tile_layer
	{
		Chunk_tile_layer() = default;

		explicit Chunk_tile_layer(data::Tile* tile_proto) {
			set_tile_prototype(tile_proto);
		}
		
		J_NODISCARD data::Tile* get_tile_prototype() const {
			return tile_prototype_;
		}

		/**
		 * Setting the tile_prototype will also set sprite IF tile_prototype is not nullptr <br>
		 * If tile_prototype is nullptr, sprite will remain untouched
		 */
		void set_tile_prototype(data::Tile* tile_prototype);


		//
		// Minimize the variables below vvvvvv
	private:
		/**
		 * A layer may point to a tile prototype to provide additional data (collisions, world gen)
		 * otherwise if not provided, it is nullptr <br>
		 * Only accessible via getter setter because setting the tile_prototype needs to also set the sprite pointer
		 */
		data::Tile* tile_prototype_ = nullptr;
		// TODO join tile_prototype_ and sprite, make it difference between the 2 in the getter
	public:
		data::Sprite* sprite = nullptr;

		/**
		 * If the layer is multi-tile, eg: 3 x 3, this holds the index of this tile (see below) <br>
		 * Non multi tile has 0 for index, but it can also exist on multi tile tiles <br>
		 * 0 1 2 <br>
		 * 3 4 5 <br>
		 * 6 7 8 <br>
		 */
		uint8_t multi_tile_index = 0;

		// TODO, somehow store these variables elsewhere instead of having a copy for each multi-tile
		/**
		 * How many tiles does this sprite span? <br>
		 * Only the width is needed for calculations
		 */
		uint8_t multi_tile_span = 1;
		uint8_t multi_tile_height = 1;
	};

	
	/**
	 * A tile in the world <br>
	 * ! Be careful when adding members to this class, its size should be minimized ! This is created for every chunk
	 */
	class Chunk_tile
	{
	public:
		Chunk_tile() = default;

		enum class chunk_layer
		{
			base = 0,
			resource,
			entity,
			overlay,  // Cursor highlights, inserter arrows, etc
			count_
		};

		/**
		 * Maximum number of layers a chunk_tile will have
		 */
		static constexpr int layer_count = static_cast<int>(chunk_layer::count_);


		// Layer getters setters
		J_NODISCARD data::Tile* get_tile_layer_tile_prototype(chunk_layer category) const;
		void set_tile_layer_tile_prototype(chunk_layer category, data::Tile* tile_prototype);

		J_NODISCARD data::Sprite* get_tile_layer_sprite_prototype(chunk_layer category) const;
		void set_tile_layer_sprite_prototype(chunk_layer category, data::Sprite* sprite_prototype);

		
		J_NODISCARD Chunk_tile_layer& get_layer(const chunk_layer layer) {
			return layers[get_layer_index(layer)];
		}

		J_NODISCARD Chunk_tile_layer& get_layer(const uint8_t layer_index) {
			return layers[layer_index];
		}
		
	private:
		static unsigned short get_layer_index(chunk_layer category);

	public:
		// Minimize the variables below vvvv
		//
		
		/**
		 * To access prototype at each location, cast desired prototype_category to int and index tile_prototypes
		 */
		Chunk_tile_layer layers[layer_count];

		// The entity which exists on this tile
		data::Entity* entity = nullptr;
	};
}

#endif // GAME_WORLD_TILE_H
