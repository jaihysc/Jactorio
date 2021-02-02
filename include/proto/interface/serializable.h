// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_PROTO_INTERFACE_SERIALIZABLE_H
#define JACTORIO_INCLUDE_PROTO_INTERFACE_SERIALIZABLE_H
#pragma once

#include "core/data_type.h"

namespace jactorio::game
{
    class World;
    class ChunkTileLayer;
} // namespace jactorio::game

namespace jactorio::proto
{
    class ISerializable
    {
    public:
        ISerializable()          = default;
        virtual ~ISerializable() = default;

        ISerializable(const ISerializable& other) = default;
        ISerializable(ISerializable&& other)      = default;

        ISerializable& operator=(const ISerializable& other) = default;
        ISerializable& operator=(ISerializable&& other) = default;

        virtual void OnDeserialize(game::World& world,
                                   const WorldCoord& world_coord,
                                   game::ChunkTileLayer& tile_layer) const = 0;
    };
} // namespace jactorio::proto

#endif // JACTORIO_INCLUDE_PROTO_INTERFACE_SERIALIZABLE_H
