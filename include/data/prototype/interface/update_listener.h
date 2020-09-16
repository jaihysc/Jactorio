// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_INTERFACE_UPDATE_LISTENER_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_INTERFACE_UPDATE_LISTENER_H
#pragma once

#include "core/data_type.h"

namespace jactorio::game
{
    class WorldData;
}

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
                                  const WorldCoord& emit_coords,
                                  const WorldCoord& receive_coords,
                                  UpdateType type) const = 0;
    };
} // namespace jactorio::data

#endif // JACTORIO_INCLUDE_DATA_PROTOTYPE_INTERFACE_UPDATE_LISTENER_H
