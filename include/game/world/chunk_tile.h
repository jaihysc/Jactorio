#ifndef GAME_WORLD_TILE_H
#define GAME_WORLD_TILE_H

#include "jactorio.h"
#include "data/prototype/tile/tile.h"
#include "data/prototype/entity/entity.h"

#include "chunk_tile_layer.h"

namespace jactorio::game
{	
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

		
		J_NODISCARD Chunk_tile_layer& get_layer(const chunk_layer layer) const {
			return layers[get_layer_index(layer)];
		}

		J_NODISCARD Chunk_tile_layer& get_layer(const uint8_t layer_index) const {
			return layers[layer_index];
		}

		// ============================================================================================
		// Methods below are type checked to avoid getting / setting the wrong data
		
		// chunk_layer::base only
		J_NODISCARD data::Tile* get_layer_tile_prototype(chunk_layer category) const;
		void set_layer_tile_prototype(chunk_layer category, data::Tile* tile_prototype) const;

		// chunk_layer::resource, chunk_layer::entity only
		J_NODISCARD data::Entity* get_layer_entity_prototype(chunk_layer category) const;
		void set_layer_entity_prototype(chunk_layer category, data::Entity* tile_prototype) const;

		// chunk_layer::overlay only
		J_NODISCARD data::Sprite* get_layer_sprite_prototype(chunk_layer category) const;
		void set_layer_sprite_prototype(chunk_layer category, data::Sprite* tile_prototype) const;
		
	private:
		static unsigned short get_layer_index(chunk_layer category);

	public:
		// ============================================================================================
		// Minimize the variables below vvvv
		
		/**
		 * To access prototype at each location, cast desired prototype_category to int and index tile_prototypes
		 */
		mutable Chunk_tile_layer layers[layer_count];
	};
}

#endif // GAME_WORLD_TILE_H
