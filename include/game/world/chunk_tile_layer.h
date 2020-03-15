// 
// chunk_tile_layer.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 02/07/2020
// Last modified: 03/14/2020
// 

#ifndef JACTORIO_INCLUDE_GAME_WORLD_CHUNK_TILE_LAYER_H
#define JACTORIO_INCLUDE_GAME_WORLD_CHUNK_TILE_LAYER_H
#pragma once

#include "data/prototype/prototype_base.h"
#include "game/world/chunk_layer.h"

namespace jactorio::game
{
	/**
	 * A Chunk_tile has many layers
	 */
	struct Chunk_tile_layer : Chunk_layer
	{
		Chunk_tile_layer() = default;

		explicit Chunk_tile_layer(data::Prototype_base* proto)
			: Chunk_layer(proto) {
		}

		/**
		 * Deletes unique_data, resets pointers to nullptr, resets multi_tile members
		 */
		void clear();

		// ============================================================================================
		// Minimize the variables below vvvvvv
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

	inline void Chunk_tile_layer::clear() {
		delete_unique_data();
		unique_data = nullptr;
		prototype_data = nullptr;

		multi_tile_index = 0;
		multi_tile_span = 1;
		multi_tile_height = 1;
	}
}

#endif //JACTORIO_INCLUDE_GAME_WORLD_CHUNK_TILE_LAYER_H
