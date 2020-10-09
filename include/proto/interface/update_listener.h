// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_PROTO_INTERFACE_UPDATE_LISTENER_H
#define JACTORIO_INCLUDE_PROTO_INTERFACE_UPDATE_LISTENER_H
#pragma once

#include "core/data_type.h"

namespace jactorio::game
{
    class WorldData;
}

namespace jactorio::proto
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
} // namespace jactorio::proto

#endif // JACTORIO_INCLUDE_PROTO_INTERFACE_UPDATE_LISTENER_H
