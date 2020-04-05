// 
// rotatable.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 04/05/2020
// Last modified: 04/05/2020
// 

#ifndef JACTORIO_DATA_PROTOTYPE_INTERFACE_ROTATABLE_H
#define JACTORIO_DATA_PROTOTYPE_INTERFACE_ROTATABLE_H
#pragma once

#include "jactorio.h"

#include <cstdint>
#include <utility>

#include "data/prototype/sprite.h"
#include "game/world/world_data.h"

namespace jactorio::data
{
	enum class placementOrientation
	{
		up,
		right,
		down,
		left
	};

	///
	/// \brief Entities which can be rotated by the player
	class Rotatable
	{
	protected:
		Rotatable() = default;
		virtual ~Rotatable() = default;

		Rotatable(const Rotatable& other) = default;
		Rotatable(Rotatable&& other) noexcept = default;
		Rotatable& operator=(const Rotatable& other) = default;
		Rotatable& operator=(Rotatable&& other) noexcept = default;


	public:
		// The single sprite from entity serves as north
		PYTHON_PROP_I(Rotatable, Sprite*, sprite_e, nullptr);
		PYTHON_PROP_I(Rotatable, Sprite*, sprite_s, nullptr);
		PYTHON_PROP_I(Rotatable, Sprite*, sprite_w, nullptr);

		///
		/// \brief Maps a placementOrientation to a <set, frame>
		/// Chunk_tile_layer* are the neighbors of the destination Chunk_tile_layer
		J_NODISCARD virtual std::pair<uint16_t, uint16_t> map_placement_orientation(placementOrientation orientation,
		                                                                            game::World_data& world_data,
		                                                                            std::pair<int, int> world_coords) const = 0;
	};
}

#endif // JACTORIO_DATA_PROTOTYPE_INTERFACE_ROTATABLE_H
