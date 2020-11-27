// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "game/game_data.h"
#include "game/player/player_action.h"

namespace jactorio::game
{
    TEST(PlayerAction, GetExecutor) {
        GameDataGlobal data_global;

        PlayerAction::GetExecutor(PlayerAction::Type::test)(data_global);

        EXPECT_FLOAT_EQ(data_global.player.world.GetPositionX(), -100);
        EXPECT_FLOAT_EQ(data_global.player.world.GetPositionY(), 120);
    }
} // namespace jactorio::game
