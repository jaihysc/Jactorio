// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "game/game_data.h"

#include "proto/container_entity.h"
#include "proto/sprite.h"

namespace jactorio::game
{
    TEST(GameData, ClearRefsToWorld) {
        GameDataGlobal global;
        GameDataLocal local;


        const data::Sprite sprite;
        const data::ContainerEntity container;


        global.worlds[0].EmplaceChunk(0, 0);


        // 1 Should not attempt to remove cursor overlays
        local.input.mouse.DrawOverlay(global.worlds[0], {0, 0}, data::Orientation::up, &container, sprite);

        // 2 Should not hold pointer to any tile layer (as they will be destroyed)
        ChunkTileLayer tile_layer;
        global.player.placement.SetActivatedLayer(&tile_layer);


        global.ClearRefsToWorld(local);


        // 1 Fails if attempted to erase last overlay
        local.input.mouse.DrawOverlay(global.worlds[0], {0, 1}, data::Orientation::up, &container, sprite);

        // 2
        EXPECT_EQ(global.player.placement.GetActivatedLayer(), nullptr);
    }
} // namespace jactorio::game