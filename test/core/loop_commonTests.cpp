// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "core/loop_common.h"

namespace jactorio::core
{
    TEST(LoopCommon, Construct) {
        ThreadedLoopCommon common;

        EXPECT_FLOAT_EQ(common.GetDataGlobal().player.world.GetPositionX(), 0);
    }

    TEST(LoopCommon, Reset) {
        ThreadedLoopCommon common;

        common.GetDataGlobal().player.world.SetPlayerX(1234);

        common.ResetGlobalData();

        EXPECT_FLOAT_EQ(common.GetDataGlobal().player.world.GetPositionX(), 0);
    }
} // namespace jactorio::core