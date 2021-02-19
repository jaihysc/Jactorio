// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "game/game_controller.h"

#include "proto/container_entity.h"
#include "proto/sprite.h"

namespace jactorio::game
{
    TEST(GameController, ResetGame) {
        GameController game_controller;

        game_controller.worlds[0].SetWorldGeneratorSeed(1234);
        game_controller.logic.GameTickAdvance();
        game_controller.player.inventory.inventory.Resize(1);

        // Should only affect world, logic, player
        game_controller.proto.Make<proto::ContainerEntity>();


        game_controller.ResetGame();


        EXPECT_NE(game_controller.worlds[0].GetWorldGeneratorSeed(), 1234);
        EXPECT_EQ(game_controller.logic.GameTick(), 0);
        EXPECT_EQ(game_controller.player.inventory.inventory.Size(),
                  game_controller.player.inventory.kDefaultInventorySize);

        EXPECT_FALSE(game_controller.proto.GetAll<proto::ContainerEntity>().empty());
    }


    TEST(GameController, ClearRefsToWorld) {
        GameController game_controller;

        const proto::Sprite sprite;
        const proto::ContainerEntity container;


        game_controller.worlds[0].EmplaceChunk({0, 0});


        // 1 Should not attempt to remove cursor overlays
        game_controller.input.mouse.DrawOverlay(game_controller.worlds[0], {0, 0}, Orientation::up, &container, sprite);

        // 2 Should not hold pointer to any tile (as they will be destroyed)
        ChunkTile tile;
        game_controller.player.placement.SetActivatedTile(&tile);


        game_controller.ClearRefsToWorld();


        // 1 Fails if attempted to erase last overlay
        game_controller.input.mouse.DrawOverlay(game_controller.worlds[0], {0, 1}, Orientation::up, &container, sprite);

        // 2
        EXPECT_EQ(game_controller.player.placement.GetActivatedTile(), nullptr);
    }
} // namespace jactorio::game