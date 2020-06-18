// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 06/10/2020

#ifndef JACTORIO_DATA_PROTOTYPE_INTERFACE_UPDATE_LISTENER_H
#define JACTORIO_DATA_PROTOTYPE_INTERFACE_UPDATE_LISTENER_H
#pragma once

#include "game/world/world_data.h"

namespace jactorio::data
{
	enum class UpdateType
	{
		place,
		remove
	};

	class IUpdateListener
	{
	public:
		virtual ~IUpdateListener() = default;

		virtual void OnTileUpdate(game::WorldData& world_data,
		                          const game::WorldData::WorldPair& emit_coords,
		                          const game::WorldData::WorldPair& receive_coords,
		                          UpdateType type) const = 0;
	};
}

#endif // JACTORIO_DATA_PROTOTYPE_INTERFACE_UPDATE_LISTENER_H
