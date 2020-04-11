// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 12/21/2019

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
	class Chunk_tile
	{
	public:
		Chunk_tile() = default;

		enum class chunkLayer
		{
			base = 0,
			resource,
			entity,
			overlay,  // Cursor highlights, inserter arrows, etc
			count_
		};

		static constexpr int tile_layer_count = static_cast<int>(chunkLayer::count_);


		J_NODISCARD Chunk_tile_layer& get_layer(const chunkLayer layer) const {
			return layers[get_layer_index(layer)];
		}

		J_NODISCARD Chunk_tile_layer& get_layer(const uint8_t layer_index) const {
			return layers[layer_index];
		}

		static unsigned short get_layer_index(chunkLayer category) {
			return static_cast<unsigned short>(category);
		}


		// ============================================================================================
		// Methods below are type checked to avoid getting / setting the wrong data

		// chunk_layer::base only
		J_NODISCARD const data::Tile* get_tile_prototype(chunkLayer category) const;
		void set_tile_prototype(chunkLayer category, const data::Tile* tile_prototype) const;


		// chunk_layer::resource, chunk_layer::entity only
		J_NODISCARD const data::Entity* get_entity_prototype(chunkLayer category) const;
		void set_entity_prototype(chunkLayer category, const data::Entity* tile_prototype) const;


		// chunk_layer::overlay only
		J_NODISCARD const data::Sprite* get_sprite_prototype(chunkLayer category) const;
		void set_sprite_prototype(chunkLayer category, const data::Sprite* tile_prototype) const;

	public:
		// ============================================================================================
		// Minimize the variables below vvvv

		/**
		 * To access prototype at each location, cast desired prototype_category to int and index tile_prototypes
		 */
		mutable Chunk_tile_layer layers[tile_layer_count];
	};
}

#endif //JACTORIO_INCLUDE_GAME_WORLD_CHUNK_TILE_H
