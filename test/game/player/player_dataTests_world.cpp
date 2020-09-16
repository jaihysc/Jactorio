// This file is subject to the terms and conditions defined in 'LICENSE' included in the source code package

#include <gtest/gtest.h>

#include "game/player/player_data.h"

#include "jactorioTests.h"

namespace jactorio::game
{
    class PlayerDataWorldTest : public testing::Test
    {
    protected:
        PlayerData::World playerWorld_;
    };

    TEST_F(PlayerDataWorldTest, Serialize) {
        playerWorld_.SetId(32);
        playerWorld_.SetPlayerX(32.41);
        playerWorld_.SetPlayerY(16.92);

        const auto result = TestSerializeDeserialize(playerWorld_);

        EXPECT_EQ(result.GetId(), 32);
        EXPECT_FLOAT_EQ(result.GetPositionX(), 32.41);
        EXPECT_FLOAT_EQ(result.GetPositionY(), 16.92);
    }
} // namespace jactorio::game
