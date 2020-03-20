// 
// rotatable.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 03/19/2020
// Last modified: 03/20/2020
// 

#ifndef JACTORIO_DATA_PROTOTYPE_INTERFACE_ROTATABLE_H
#define JACTORIO_DATA_PROTOTYPE_INTERFACE_ROTATABLE_H
#pragma once

#include "jactorio.h"
#include "game/world/chunk_tile_layer.h"

#include <cstdint>
#include <utility>

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
	class Rotatable_entity
	{
	protected:
		Rotatable_entity() = default;
		virtual ~Rotatable_entity() = default;

		Rotatable_entity(const Rotatable_entity& other) = default;
		Rotatable_entity(Rotatable_entity&& other) noexcept = default;
		Rotatable_entity& operator=(const Rotatable_entity& other) = default;
		Rotatable_entity& operator=(Rotatable_entity&& other) noexcept = default;


	public:
		///
		/// \brief Maps a placementOrientation to a <set, frame>
		/// Chunk_tile_layer* are the neighbors of the destination Chunk_tile_layer
		J_NODISCARD virtual std::pair<uint16_t, uint16_t> map_placement_orientation(placementOrientation orientation,
		                                                                            game::Chunk_tile_layer* up,
		                                                                            game::Chunk_tile_layer* right,
		                                                                            game::Chunk_tile_layer* down,
		                                                                            game::Chunk_tile_layer* left) const = 0;
	};
}

#endif // JACTORIO_DATA_PROTOTYPE_INTERFACE_ROTATABLE_H
