// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 04/12/2020

#ifndef JACTORIO_DATA_PROTOTYPE_PROTOTYPE_TYPE_H
#define JACTORIO_DATA_PROTOTYPE_PROTOTYPE_TYPE_H
#pragma once

#include <utility>

#include "game/world/world_data.h"

namespace jactorio::data
{
	///
	/// \brief Represents an output location for an entity on 4 orientation
	struct Tile_4_way
	{
		Tile_4_way() = default;

		Tile_4_way(game::World_data::world_pair up,
		           game::World_data::world_pair right,
		           game::World_data::world_pair down,
		           game::World_data::world_pair left)
			: up(std::move(up)), right(std::move(right)), down(std::move(down)), left(std::move(left)) {
		}

		game::World_data::world_pair up;
		game::World_data::world_pair right;
		game::World_data::world_pair down;
		game::World_data::world_pair left;

		J_NODISCARD const game::World_data::world_pair& get(const placementOrientation orientation) const {
			switch (orientation) {

			case placementOrientation::up:
				return up;
			case placementOrientation::right:
				return right;
			case placementOrientation::down:
				return down;
			case placementOrientation::left:
				return left;

			default:
				assert(false);
				return up;
			}
		}
	};
}

#endif // JACTORIO_DATA_PROTOTYPE_PROTOTYPE_TYPE_H
