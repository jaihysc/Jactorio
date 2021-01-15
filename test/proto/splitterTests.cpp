// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "proto/splitter.h"

#include "jactorioTests.h"

#include "game/world/world_data.h"
#include "proto/transport_belt.h"

namespace jactorio::proto
{
    class SplitterTest : public testing::Test
    {
    protected:
        void SetUp() override {
            world_.EmplaceChunk(0, 0);
        }

        game::WorldData world_;
        game::LogicData logic_;
        TransportBelt transBelt_;
    };

    ///
    /// Splitter creates conveyor at its 2 tiles, then connects to neighboring conveyors
    ///
    /// v v
    /// C C Orientation down
    /// < >
    TEST_F(SplitterTest, BuildCreateConnectConveyor) {
        auto& con_data_tl = TestSetupConveyor(world_, {0, 0}, Orientation::down, transBelt_);
        auto& con_data_tr = TestSetupConveyor(world_, {1, 0}, Orientation::down, transBelt_);

        auto& con_data_bl = TestSetupConveyor(world_, {0, 2}, Orientation::left, transBelt_);
        auto& con_data_br = TestSetupConveyor(world_, {1, 2}, Orientation::right, transBelt_);

        Splitter splitter;
        splitter.SetWidth(2);

        TestSetupMultiTile(world_, {0, 1}, game::TileLayer::entity, Orientation::down, splitter);

        splitter.OnBuild(
            world_, logic_, {0, 1}, world_.GetTile(0, 1)->GetLayer(game::TileLayer::entity), Orientation::down);

        auto* splitter_data = world_.GetTile(0, 1)->GetLayer(game::TileLayer::entity).GetUniqueData<SplitterData>();
        ASSERT_NE(splitter_data, nullptr);

        // Top conveyor grouped with splitter
        EXPECT_EQ(con_data_tl.structure->target, con_data_bl.structure.get());
        EXPECT_EQ(con_data_tr.structure->target, con_data_br.structure.get());

        EXPECT_EQ(splitter_data->right.structure->target, con_data_bl.structure.get());
        EXPECT_EQ(splitter_data->left.structure->target, con_data_br.structure.get());
    }
} // namespace jactorio::proto