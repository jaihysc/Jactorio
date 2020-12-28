// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "core/loop_common.h"

#include "proto/container_entity.h"
#include "proto/sprite.h"

namespace jactorio::core
{
    TEST(LoopCommon, Construct) {
        ThreadedLoopCommon common;

        EXPECT_FLOAT_EQ(common.GetDataGlobal().player.world.GetPositionX(), 0);
    }

    TEST(LoopCommon, ResetDataGlobal) {
        ThreadedLoopCommon common;

        common.GetDataGlobal().player.world.SetPlayerX(1234);

        common.ResetGlobalData();

        EXPECT_FLOAT_EQ(common.GetDataGlobal().player.world.GetPositionX(), 0);
    }

    TEST(LoopCommon, ClearRefsToWorld) {
        ThreadedLoopCommon common;


        const proto::Sprite sprite;
        const proto::ContainerEntity container;

        auto& local = common.gameDataLocal;

        {
            auto& global = common.GetDataGlobal();

            global.worlds[0].EmplaceChunk(0, 0);

            local.input.mouse.DrawOverlay(global.worlds[0], {0, 0}, Orientation::up, &container, sprite);
        }


        common.ResetGlobalData();


        {
            auto& global = common.GetDataGlobal(); // global is reconstructed

            global.worlds[0].EmplaceChunk(0, 0);

            local.input.mouse.DrawOverlay(global.worlds[0], {0, 1}, Orientation::up, &container, sprite);
        }
    }
} // namespace jactorio::core