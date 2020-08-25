// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_DATA_PROTOTYPE_INTERFACE_SERIALIZABLE_H
#define JACTORIO_DATA_PROTOTYPE_INTERFACE_SERIALIZABLE_H
#pragma once

#include "core/data_type.h"

namespace jactorio::game
{
	class WorldData;
	class ChunkTileLayer;
}

namespace jactorio::data
{
	class ISerializable
	{
	public:
		ISerializable() = default;
		virtual ~ISerializable() = default;

		virtual void OnDeserialize(game::WorldData& world_data,
								   const WorldCoord& world_coord, game::ChunkTileLayer& tile_layer) const = 0;
	};
}

#endif // JACTORIO_DATA_PROTOTYPE_INTERFACE_SERIALIZABLE_H
