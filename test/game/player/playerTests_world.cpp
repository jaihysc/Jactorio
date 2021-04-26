// This file is subject to the terms and conditions defined in 'LICENSE' included in the source code package

#include <gtest/gtest.h>

#include "game/player/player.h"

#include "jactorioTests.h"

namespace jactorio::game
{
    class PlayerWorldTest : public testing::Test
    {
    protected:
        Player::World playerWorld_;
    };

    TEST_F(PlayerWorldTest, Serialize) {
        playerWorld_.SetId(32);
        playerWorld_.SetPlayerX(32.41);
        playerWorld_.SetPlayerY(16.92);

        const auto result = TestSerializeDeserialize(playerWorld_);

        EXPECT_EQ(result.GetId(), 32);
        EXPECT_FLOAT_EQ(result.GetPosition().x, 32.41f);
        EXPECT_FLOAT_EQ(result.GetPosition().y, 16.92f);
    }
} // namespace jactorio::game
