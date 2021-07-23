// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "game/game_controller.h"

#include "proto/container_entity.h"
#include "proto/sprite.h"

namespace jactorio::game
{
    TEST(GameController, ResetGame) {
        GameController game_controller{nullptr};

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
} // namespace jactorio::game