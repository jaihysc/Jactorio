// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#ifndef JACTORIO_INCLUDE_GAME_WORLD_CHUNK_LAYER_H
#define JACTORIO_INCLUDE_GAME_WORLD_CHUNK_LAYER_H
#pragma once

#include "data/prototype/prototype_base.h"

namespace jactorio::game
{
	///
	/// \brief Abstract class for Chunk_tile_layer, Chunk_object_layer, Chunk_struct_layer
	/// \remark Will only delete unique_data, others must be manually deleted
	template <typename TUniqueData = data::UniqueDataBase>
	class ChunkLayer
	{
	protected:
		ChunkLayer() = default;

	public:
		explicit ChunkLayer(const data::PrototypeBase* proto)
			: prototypeData(proto) {
		}

		// Prototype data must be deleted after chunk data
		~ChunkLayer() {
			delete uniqueData;
		}

		ChunkLayer(const ChunkLayer& other);
		ChunkLayer(ChunkLayer&& other) noexcept;

		ChunkLayer& operator=(ChunkLayer other) {
			swap(*this, other);
			return *this;
		}

		friend void swap(ChunkLayer& lhs, ChunkLayer& rhs) noexcept {
			using std::swap;
			swap(lhs.prototypeData, rhs.prototypeData);
			swap(lhs.uniqueData, rhs.uniqueData);
		}

		// ======================================================================
		// Minimize the variables below 

		// A layer may point to a tile prototype to provide additional data (collisions, world gen) <br>
		// The sprite is stored within the tile prototype instead <br>
		// If not provided, it is nullptr <br>

		// Entities also possesses a sprite pointer within their prototype

		/// Depending on the layer, this will be either a data::Tile*, data::Entity* or a data::Sprite* <br>
		const data::PrototypeBase* prototypeData = nullptr;

		/// Data for the prototype which is unique per tile and layer <br>
		/// When this layer is deleted, unique_data_ will be deleted with delete method in prototype_data_
		TUniqueData* uniqueData = nullptr;
	};

	template <typename TUniqueData>
	ChunkLayer<TUniqueData>::ChunkLayer(const ChunkLayer& other)
		: prototypeData(other.prototypeData) {

		// Use prototype defined method for copying unique_data_ if other has data to copy
		if (other.uniqueData != nullptr) {
			assert(other.prototypeData != nullptr);  // No prototype_data_ available for copying unique_data_
			uniqueData = other.prototypeData->CopyUniqueData(other.uniqueData);
		}
	}

	template <typename TUniqueData>
	ChunkLayer<TUniqueData>::ChunkLayer(ChunkLayer&& other) noexcept
		: prototypeData(other.prototypeData),
		  uniqueData(other.uniqueData) {
		// After moving data away, set unique_data to nullptr so it is not deleted
		other.uniqueData = nullptr;
	}
}


#endif //JACTORIO_INCLUDE_GAME_WORLD_CHUNK_LAYER_H
