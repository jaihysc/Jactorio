// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#ifndef JACTORIO_INCLUDE_GAME_WORLD_CHUNK_STRUCT_LAYER_H
#define JACTORIO_INCLUDE_GAME_WORLD_CHUNK_STRUCT_LAYER_H
#pragma once

#include "game/world/chunk_layer.h"

namespace jactorio::game
{
	///
	/// \brief For a Logic_chunk, stores chunk structures (transport line segments (Not the entities), etc)
	class Chunk_struct_layer : public Chunk_layer
	{
	public:
		using struct_coord = float;
		
		Chunk_struct_layer() = default;

		explicit Chunk_struct_layer(const data::Prototype_base* proto)
			: Chunk_layer(proto) {
		}

		explicit Chunk_struct_layer(const data::Prototype_base* proto,
		                            const struct_coord position_x, const struct_coord position_y)
			: Chunk_layer(proto), position_x(position_x), position_y(position_y) {
		}

		// Distance (tiles) from top left of chunk to top left of object
		struct_coord position_x = 0;
		struct_coord position_y = 0;
	};
}


#endif //JACTORIO_INCLUDE_GAME_WORLD_CHUNK_STRUCT_LAYER_H
