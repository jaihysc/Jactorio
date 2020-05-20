#include <gtest/gtest.h>

#include "game/logic/inserter_controller.h"

namespace game
{
	class InserterControllerTest : public testing::Test
	{
	protected:
		
	};

	TEST_F(InserterControllerTest, GetInserterArmOffset) {
		EXPECT_FLOAT_EQ(jactorio::game::GetInserterArmOffset(160, 1), 0.436764281);
	}
}