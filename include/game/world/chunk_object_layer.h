// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#ifndef JACTORIO_INCLUDE_GAME_WORLD_CHUNK_OBJECT_LAYER_H
#define JACTORIO_INCLUDE_GAME_WORLD_CHUNK_OBJECT_LAYER_H
#pragma once

#include "game/world/chunk_layer.h"

namespace jactorio::game
{
	///
	/// \brief Holds items which do not adhere to the tiles for rendering
	struct ChunkObjectLayer : ChunkLayer
	{
		ChunkObjectLayer() = default;

		explicit ChunkObjectLayer(const data::PrototypeBase* proto)
			: ChunkLayer(proto) {
		}

		explicit ChunkObjectLayer(const data::PrototypeBase* proto,
		                          const float position_x, const float position_y,
		                          const float size_x, const float size_y)
			: ChunkLayer(proto),
			  positionX(position_x), positionY(position_y), sizeX(size_x), sizeY(size_y) {
		}

		// Distance (tiles) from top left of chunk to top left of object
		float positionX = 0;
		float positionY = 0;

		float sizeX = 0;
		float sizeY = 0;
	};
}

#endif //JACTORIO_INCLUDE_GAME_WORLD_CHUNK_OBJECT_LAYER_H
