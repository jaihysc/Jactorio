// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#ifndef JACTORIO_INCLUDE_GAME_WORLD_CHUNK_STRUCT_LAYER_H
#define JACTORIO_INCLUDE_GAME_WORLD_CHUNK_STRUCT_LAYER_H
#pragma once

#include "game/world/chunk_layer.h"

namespace jactorio::game
{
	struct UniqueDataStruct
	{
		virtual ~UniqueDataStruct() = default;
	};

	///
	/// \brief For a Logic_chunk, stores chunk structures (transport line segments (Not the entities), etc)
	class ChunkStructLayer : public ChunkLayer<UniqueDataStruct>
	{
	public:
		using StructCoord = float;

		ChunkStructLayer() = default;

		explicit ChunkStructLayer(const data::PrototypeBase* proto)
			: ChunkLayer(proto) {
		}

		explicit ChunkStructLayer(const data::PrototypeBase* proto,
		                          const StructCoord position_x, const StructCoord position_y)
			: ChunkLayer(proto), positionX(position_x), positionY(position_y) {
		}

		// Distance (tiles) from top left of chunk to top left of object
		StructCoord positionX = 0;
		StructCoord positionY = 0;
	};
}


#endif //JACTORIO_INCLUDE_GAME_WORLD_CHUNK_STRUCT_LAYER_H
