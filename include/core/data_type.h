// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_CORE_DATA_TYPE_H
#define JACTORIO_INCLUDE_CORE_DATA_TYPE_H
#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

#include <decimal.h>

namespace jactorio::core
{
    template <typename TVal>
    struct Position2;

    template <typename TPosition>
    struct QuadPosition;
} // namespace jactorio::core

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

    using SpriteSetT   = uint16_t;
    using SpriteFrameT = uint16_t;
    using SpriteTrimT  = uint16_t;

    using ResourceEntityResourceCount = uint32_t;


    // Game

    using GameTickT          = uint64_t;
    constexpr int kGameHertz = 60; // 60 updates per second


    /// Forward declaration only, game::World must be included
    using GameWorlds = std::vector<game::World>;

    /// Tiles in the world
    using WorldCoordAxis = int32_t;
    using WorldCoord     = core::Position2<WorldCoordAxis>;
    /// World currently in
    using WorldId = std::size_t;

    /// Chunks in the world
    using ChunkCoordAxis = int32_t;
    using ChunkCoord     = core::Position2<ChunkCoordAxis>;

    /// Tiles within a chunk
    using ChunkTileCoordAxis = uint8_t;
    using ChunkTileCoord     = core::Position2<ChunkTileCoordAxis>;

    /// Offset from top left of chunk
    using OverlayOffsetAxis = float;


    // Rendering

    using UvPositionT = core::QuadPosition<core::Position2<float>>;
    /// Internal id of sprite to uv coordinates
    using SpriteUvCoordsT = std::unordered_map<unsigned int, UvPositionT>;

} // namespace jactorio

#endif // JACTORIO_INCLUDE_CORE_DATA_TYPE_H
