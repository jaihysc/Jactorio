#ifndef GAME_WORLD_CHUNK_LAYER_H
#define GAME_WORLD_CHUNK_LAYER_H

#include "data/prototype/prototype_base.h"

namespace jactorio::game
{
	/**
	 * Abstract class for Chunk_layer and Chunk_object_layer <br>
	 * Will only delete unique_data, others must be manually deleted
	 */
	class Chunk_layer
	{
	protected:
		Chunk_layer() = default;

		explicit Chunk_layer(data::Prototype_base* proto)
			: prototype_data(proto) {
		}

		// Prototype data must be deleted after chunk data
		~Chunk_layer() {
			delete_unique_data();
		}

		void delete_unique_data() const {
			if (unique_data != nullptr) {
				assert(prototype_data != nullptr);  // Unique_data_ was defined, but no prototype_data_ is available to delete it
				prototype_data->delete_unique_data(unique_data);
			}
		}

		Chunk_layer(const Chunk_layer& other);
		Chunk_layer(Chunk_layer&& other) noexcept;

		Chunk_layer& operator=(const Chunk_layer& other);
		Chunk_layer& operator=(Chunk_layer&& other) noexcept;

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
	};

	inline Chunk_layer::Chunk_layer(const Chunk_layer& other)
		: prototype_data(other.prototype_data) {

		// Use prototype defined method for copying unique_data_ if other has data to copy
		if (other.unique_data != nullptr) {
			assert(other.prototype_data != nullptr);  // No prototype_data_ available for copying unique_data_
			unique_data = other.prototype_data->copy_unique_data(other.unique_data);
		}
	}

	inline Chunk_layer::Chunk_layer(Chunk_layer&& other) noexcept
		: prototype_data(other.prototype_data),
		  unique_data(other.unique_data) {
		// After moving data away, set unique_data to nullptr so it is not deleted
		other.unique_data = nullptr;
	}

	inline Chunk_layer& Chunk_layer::operator=(const Chunk_layer& other) {
		if (this == &other)
			return *this;
		
		prototype_data = other.prototype_data;
		
		// Use prototype defined method for copying unique_data_
		if (unique_data != nullptr) {
			assert(prototype_data != nullptr);  // No prototype_data_ available for copying unique_data_
			unique_data = prototype_data->copy_unique_data(other.unique_data);
		}
		return *this;
	}

	inline Chunk_layer& Chunk_layer::operator=(Chunk_layer&& other) noexcept {
		prototype_data = other.prototype_data;
		unique_data = other.unique_data;
		other.unique_data = nullptr;

		return *this;
	}
}


#endif // GAME_WORLD_CHUNK_LAYER_H
