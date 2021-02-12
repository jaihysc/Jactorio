// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_WORLD_TILE_LAYER_H
#define JACTORIO_INCLUDE_GAME_WORLD_TILE_LAYER_H
#pragma once

namespace jactorio::game
{
    enum class TileLayer
    {
        base = 0,
        resource,
        entity,
        count_
    };

    static constexpr auto kTileLayerCount = static_cast<int>(TileLayer::count_);
} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_WORLD_TILE_LAYER_H
