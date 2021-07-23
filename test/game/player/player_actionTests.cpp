// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "game/game_controller.h"
#include "game/player/player_action.h"

namespace jactorio::game
{
    TEST(PlayerAction, GetExecutor) {
        GameController game_controller{nullptr};

        PlayerAction::GetExecutor(PlayerAction::Type::test)({game_controller});

        EXPECT_FLOAT_EQ(game_controller.player.world.GetPosition().x, -100.f);
        EXPECT_FLOAT_EQ(game_controller.player.world.GetPosition().y, 120.f);
    }
} // namespace jactorio::game
