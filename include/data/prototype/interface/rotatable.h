// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_DATA_PROTOTYPE_INTERFACE_ROTATABLE_H
#define JACTORIO_DATA_PROTOTYPE_INTERFACE_ROTATABLE_H
#pragma once

#include "jactorio.h"

#include <utility>

#include "data/prototype/sprite.h"
#include "data/prototype/type.h"
#include "game/world/world_data.h"

namespace jactorio::data
{
	///
	/// \brief Entities which can be rotated by the player
	class IRotatable
	{
	protected:
		IRotatable()          = default;
		virtual ~IRotatable() = default;

		IRotatable(const IRotatable& other)                = default;
		IRotatable(IRotatable&& other) noexcept            = default;
		IRotatable& operator=(const IRotatable& other)     = default;
		IRotatable& operator=(IRotatable&& other) noexcept = default;


	public:
		// The single sprite from entity serves as north
		PYTHON_PROP_I(IRotatable, Sprite*, spriteE, nullptr);
		PYTHON_PROP_I(IRotatable, Sprite*, spriteS, nullptr);
		PYTHON_PROP_I(IRotatable, Sprite*, spriteW, nullptr);

		///
		/// \brief Maps a placementOrientation to a <set, frame>
		/// Chunk_tile_layer* are the neighbors of the destination Chunk_tile_layer
		virtual Sprite::SetT OnRGetSet(Orientation orientation,
		                               game::WorldData& world_data,
		                               const game::WorldData::WorldPair& world_coords) const = 0;
	};
}

#endif // JACTORIO_DATA_PROTOTYPE_INTERFACE_ROTATABLE_H
