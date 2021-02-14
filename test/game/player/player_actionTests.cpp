// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "game/game_controller.h"
#include "game/player/player_action.h"

namespace jactorio::game
{
    TEST(PlayerAction, GetExecutor) {
        GameController game_controller;

        PlayerAction::GetExecutor(PlayerAction::Type::test)(game_controller);

        EXPECT_FLOAT_EQ(game_controller.player.world.GetPositionX(), -100);
        EXPECT_FLOAT_EQ(game_controller.player.world.GetPositionY(), 120);
    }
} // namespace jactorio::game
