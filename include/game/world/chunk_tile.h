// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_WORLD_CHUNK_TILE_H
#define JACTORIO_INCLUDE_GAME_WORLD_CHUNK_TILE_H
#pragma once

#include "jactorio.h"

#include "chunk_tile_layer.h"

// Cannot include the data headers
namespace jactorio::data
{
	class Tile;
	class Entity;
	class Sprite;
}

namespace jactorio::game
{
	///
	/// \brief A tile in the world
	/// ! Be careful when adding members to this class, its size should be minimized ! This is created for every chunk
	class ChunkTile
	{
	public:
		ChunkTile() = default;

		enum class ChunkLayer
		{
			base = 0,
			resource,
			entity,
			count_
		};

		static constexpr int kTileLayerCount = static_cast<int>(ChunkLayer::count_);


		J_NODISCARD ChunkTileLayer& GetLayer(const ChunkLayer layer) const {
			return layers[GetLayerIndex(layer)];
		}

		J_NODISCARD ChunkTileLayer& GetLayer(const uint8_t layer_index) const {
			return layers[layer_index];
		}

		static unsigned int GetLayerIndex(ChunkLayer category) {
			return static_cast<unsigned int>(category);
		}


		// ============================================================================================
		// Methods below are type checked to avoid getting / setting the wrong data

		// chunk_layer::base only
		J_NODISCARD const data::Tile* GetTilePrototype(ChunkLayer category = ChunkLayer::base) const;
		void SetTilePrototype(const data::Tile* tile_prototype, ChunkLayer category = ChunkLayer::base) const;


		// chunk_layer::resource, chunk_layer::entity only
		J_NODISCARD const data::Entity* GetEntityPrototype(ChunkLayer category = ChunkLayer::entity) const;
		void SetEntityPrototype(const data::Entity* tile_prototype, ChunkLayer category = ChunkLayer::entity) const;


		// ============================================================================================
		// Minimize the variables below vvvv

		/// \remark To access prototype at each location, cast desired prototype_category to int and index tile_prototypes
		mutable ChunkTileLayer layers[kTileLayerCount];
	};
}

#endif //JACTORIO_INCLUDE_GAME_WORLD_CHUNK_TILE_H
