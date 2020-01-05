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
	private:
		/**
		 * A layer may point to a tile prototype to provide additional data (collisions, world gen)
		 * otherwise if not provided, it is nullptr <br>
		 * Only accessible via getter setter because setting the tile_prototype needs to also set the sprite pointer
		 */
		data::Tile* tile_prototype_ = nullptr;
		
	public:
		Chunk_tile_layer() = default;

		explicit Chunk_tile_layer(data::Tile* tile_proto) {
			set_tile_prototype(tile_proto);
		}

		
		data::Sprite* sprite = nullptr;

		J_NODISCARD data::Tile* get_tile_prototype() const {
			return tile_prototype_;
		}

		/**
		 * Setting the tile_prototype will also set sprite IF tile_prototype is not nullptr <br>
		 * If tile_prototype is nullptr, sprite will remain untouched
		 */
		void set_tile_prototype(data::Tile* tile_prototype);
	};

	
	/**
	 * A tile in the world
	 */
	class Chunk_tile
	{
	public:
		enum class chunk_layer
		{
			base = 0,
			resource,
			entity,
			overlay  // Cursor highlights, inserter arrows, etc
		};
		
	private:
		static unsigned short get_layer_index(chunk_layer category);
		
	public:
		Chunk_tile() = default;
		
		/**
		 * Maximum number of layers a chunk_tile will have
		 */
		static constexpr int layers_count = sizeof(chunk_layer);

		/**
		 * To access prototype at each location, cast desired prototype_category to int and index
		 * tile_prototypes
		 */
		Chunk_tile_layer layers[layers_count];

		// The entity which exists on this tile
		data::Entity* entity = nullptr;
		
		J_NODISCARD data::Tile* get_tile_layer_tile_prototype(chunk_layer category) const;
		void set_tile_layer_tile_prototype(chunk_layer category, data::Tile* tile_prototype);

		J_NODISCARD data::Sprite* get_tile_layer_sprite_prototype(chunk_layer category) const;
		void set_tile_layer_sprite_prototype(chunk_layer category, data::Sprite* sprite_prototype);
	};
}

#endif // GAME_WORLD_TILE_H
