// 
// chunk_tile.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 12/21/2019
// Last modified: 03/12/2020
// 

#ifndef JACTORIO_INCLUDE_GAME_WORLD_CHUNK_TILE_H
#define JACTORIO_INCLUDE_GAME_WORLD_CHUNK_TILE_H
#pragma once

#include "jactorio.h"

#include "chunk_tile_layer.h"

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
