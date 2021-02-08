// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "game/logic/logic.h"
#include "game/world/world.h"

namespace jactorio::game
{
    class LogicTest : public testing::Test
    {
    protected:
        Logic logic_;
    };

    TEST_F(LogicTest, OnTickAdvance) {
        // Should move the game_tick forward
        EXPECT_EQ(logic_.GameTick(), 0);

        logic_.GameTickAdvance();
        EXPECT_EQ(logic_.GameTick(), 1);

        logic_.GameTickAdvance();
        EXPECT_EQ(logic_.GameTick(), 2);

        logic_.GameTickAdvance();
        EXPECT_EQ(logic_.GameTick(), 3);
    }
} // namespace jactorio::game
