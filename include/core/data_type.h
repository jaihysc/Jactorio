// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_CORE_DATA_TYPE_H
#define JACTORIO_INCLUDE_CORE_DATA_TYPE_H
#pragma once

#include <unordered_map>
#include <vector>

#include <decimal.h>

#include "jactorio.h"

#include "core/coordinate_tuple.h"

namespace jactorio::game
{
    class World;
}

namespace jactorio
{
    // Data types of the various components within Jactorio
    // Defined here to solve circular includes

    using Decimal3T = dec::decimal<3>;


    // Prototypes

    using PrototypeIdT  = uint32_t;
    using UniqueDataIdT = uint32_t;

    using SpriteSetT           = uint16_t;
    using SpriteFrameT         = uint16_t;
    using SpriteTrimT          = uint16_t;
    using SpriteTexCoordIndexT = uint16_t;

    using ResourceEntityResourceCount = uint32_t;


    // Game

    using GameTickT          = uint64_t;
    constexpr int kGameHertz = 60; // 60 updates per second


    /// Forward declaration only, game::World must be included
    using GameWorlds = std::vector<game::World>;

    /// Tiles in the world
    using WorldCoordAxis = int32_t;

    class WorldCoord : public Position2<WorldCoordAxis>
    {
    public:
        using Position2<WorldCoordAxis>::Position2;

        /// Increments coordinate
        /// \param val Used to select specialization of Position2Increment to increment coordinate
        /// \param increment Amount to increment by, negative to decrement
        template <typename T, typename TInc = int>
        void Increment(T&& val, const TInc increment = 1) {
            Position2Increment(std::forward<T>(val), *this, increment);
        }

        /// \return Incremented form of current coordinate
        /// \param val Used to select specialization of Position2Increment to increment coordinate
        /// \param increment Amount to increment by, negative to decrement
        template <typename T, typename TInc = int>
        J_NODISCARD WorldCoord Incremented(T&& val, const TInc increment = 1) const {
            auto coord = *this;
            Position2Increment(std::forward<T>(val), coord, increment);
            return coord;
        }
    };

    /// World currently in
    using WorldId = std::size_t;

    /// Chunks in the world
    using ChunkCoordAxis = int32_t;
    using ChunkCoord     = Position2<ChunkCoordAxis>;

    /// Tiles within a chunk
    using ChunkTileCoordAxis = uint8_t;
    using ChunkTileCoord     = Position2<ChunkTileCoordAxis>;

    /// Offset from top left of chunk
    using OverlayOffsetAxis = float;


    // Rendering

    using UvPositionT = QuadPosition<Position2<float>>;
    /// Internal id of sprite to uv coordinates
    using SpriteUvCoordsT = std::unordered_map<unsigned int, UvPositionT>;

} // namespace jactorio

#endif // JACTORIO_INCLUDE_CORE_DATA_TYPE_H
