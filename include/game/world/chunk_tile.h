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

		// Prototype data must be deleted after chunk data
		~Chunk_tile_layer() {
			if (unique_data != nullptr) {
				assert(prototype_data_ != nullptr);  // Unique_data_ was defined, but no prototype_data_ is available to delete it
				prototype_data_->delete_unique_data(unique_data);
			}
		}

		Chunk_tile_layer(const Chunk_tile_layer& other);
		Chunk_tile_layer(Chunk_tile_layer&& other) noexcept;

		Chunk_tile_layer& operator=(const Chunk_tile_layer& other);
		Chunk_tile_layer& operator=(Chunk_tile_layer&& other) noexcept;

		
		explicit Chunk_tile_layer(data::Tile* data) {
			set_data(data);
		}

		explicit Chunk_tile_layer(data::Entity* data) {
			set_data(data);
		}

		explicit Chunk_tile_layer(data::Sprite* data) {
			set_data(data);
		}

		J_NODISCARD data::Tile* get_tile_prototype() const {
			return static_cast<data::Tile*>(prototype_data_);
		}

		J_NODISCARD data::Entity* get_entity_prototype() const {
			return static_cast<data::Entity*>(prototype_data_);
		}

		J_NODISCARD data::Sprite* get_sprite_prototype() const {
			return reinterpret_cast<data::Sprite*>(prototype_data_);
		}
		
		void set_data(data::Tile* data) {
			prototype_data_ = data;
		}

		void set_data(data::Entity* data) {
			prototype_data_ = data;
		}

		void set_data(data::Sprite* data) {
			prototype_data_ = data;
		}

		void set_data(std::nullptr_t) {
			prototype_data_ = nullptr;
		}

		// ============================================================================================
		// Minimize the variables below vvvvvv
	private:
		/*
		 * A layer may point to a tile prototype to provide additional data (collisions, world gen) <br>
		 * The sprite is stored within the tile prototype instead <br>
		 * If not provided, it is nullptr <br>
		 */

		/**
		 * Entities also possesses a sprite pointer within their prototype
		 */
		
		/**
		 * Depending on the layer, this will be either a data::Tile*, data::Entity* or a data::Sprite* <br>
		 */
		data::Prototype_base* prototype_data_ = nullptr;

	public:
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

		// Methods below are type checked to avoid getting / setting the wrong data
		
		// chunk_layer::base only
		J_NODISCARD data::Tile* get_layer_tile_prototype(chunk_layer category) const;
		void set_layer_tile_prototype(chunk_layer category, data::Tile* tile_prototype);

		// chunk_layer::resource, chunk_layer::entity only
		J_NODISCARD data::Entity* get_layer_entity_prototype(chunk_layer category) const;
		void set_layer_entity_prototype(chunk_layer category, data::Entity* tile_prototype);

		// chunk_layer::overlay only
		J_NODISCARD data::Sprite* get_layer_sprite_prototype(chunk_layer category) const;
		void set_layer_sprite_prototype(chunk_layer category, data::Sprite* tile_prototype);
		
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
