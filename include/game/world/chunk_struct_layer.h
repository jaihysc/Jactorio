// 
// chunk_struct_layer.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 02/22/2020
// Last modified: 03/24/2020
// 

#ifndef JACTORIO_INCLUDE_GAME_WORLD_CHUNK_STRUCT_LAYER_H
#define JACTORIO_INCLUDE_GAME_WORLD_CHUNK_STRUCT_LAYER_H
#pragma once

#include <cmath>

#include "game/world/chunk_layer.h"

namespace jactorio::game
{
	///
	/// \brief For a Logic_chunk, stores chunk structures (transport line segments (Not the entities), etc)
	class Chunk_struct_layer : public Chunk_layer
	{
	public:
		Chunk_struct_layer() = default;

		explicit Chunk_struct_layer(const data::Prototype_base* proto)
			: Chunk_layer(proto) {
		}

		explicit Chunk_struct_layer(const data::Prototype_base* proto,
		                            const float position_x, const float position_y)
			: Chunk_layer(proto), position_x(position_x), position_y(position_y) {
		}

		// Distance (tiles) from top left of chunk to top left of object
		float position_x = 0;
		float position_y = 0;

		static double to_position(const int32_t chunk_coordinate, const int32_t world_coordinate) {
			return fabs(chunk_coordinate * 32 - world_coordinate);
		}
	};
}


#endif //JACTORIO_INCLUDE_GAME_WORLD_CHUNK_STRUCT_LAYER_H
