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
			overlay,  // Cursor highlights, inserter arrows, etc
			count_
		};

		static constexpr int kTileLayerCount = static_cast<int>(ChunkLayer::count_);


		J_NODISCARD ChunkTileLayer& GetLayer(const ChunkLayer layer) const {
			return layers[GetLayerIndex(layer)];
		}

		J_NODISCARD ChunkTileLayer& GetLayer(const uint8_t layer_index) const {
			return layers[layer_index];
		}

		static unsigned short GetLayerIndex(ChunkLayer category) {
			return static_cast<unsigned short>(category);
		}


		// ============================================================================================
		// Methods below are type checked to avoid getting / setting the wrong data

		// chunk_layer::base only
		J_NODISCARD const data::Tile* GetTilePrototype(ChunkLayer category) const;
		void SetTilePrototype(ChunkLayer category, const data::Tile* tile_prototype) const;


		// chunk_layer::resource, chunk_layer::entity only
		J_NODISCARD const data::Entity* GetEntityPrototype(ChunkLayer category) const;
		void SetEntityPrototype(ChunkLayer category, const data::Entity* tile_prototype) const;


		// chunk_layer::overlay only
		J_NODISCARD const data::Sprite* GetSpritePrototype(ChunkLayer category) const;
		void SetSpritePrototype(ChunkLayer category, const data::Sprite* tile_prototype) const;

	public:
		// ============================================================================================
		// Minimize the variables below vvvv

		/// \remark To access prototype at each location, cast desired prototype_category to int and index tile_prototypes
		mutable ChunkTileLayer layers[kTileLayerCount];
	};
}

#endif //JACTORIO_INCLUDE_GAME_WORLD_CHUNK_TILE_H
