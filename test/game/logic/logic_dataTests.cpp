// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "game/logic/logic_data.h"
#include "game/world/world_data.h"

namespace jactorio::game
{
	class LogicDataTest : public testing::Test
	{
	protected:
		LogicData logicData_{};
	};

	TEST_F(LogicDataTest, OnTickAdvance) {
		// Should move the game_tick forward
		EXPECT_EQ(logicData_.GameTick(), 0);

		logicData_.GameTickAdvance();
		EXPECT_EQ(logicData_.GameTick(), 1);

		logicData_.GameTickAdvance();
		EXPECT_EQ(logicData_.GameTick(), 2);

		logicData_.GameTickAdvance();
		EXPECT_EQ(logicData_.GameTick(), 3);
	}
}
