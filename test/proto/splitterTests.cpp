// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "proto/splitter.h"

#include "jactorioTests.h"

#include "game/logic/conveyor_utility.h"
#include "game/world/world.h"
#include "proto/transport_belt.h"

namespace jactorio::proto
{
    class SplitterTest : public testing::Test
    {
    protected:
        void SetUp() override {
            world_.EmplaceChunk({0, 0});

            // Conveyor utility requires tex coord id when setting up conveyor
            transBelt_.sprite  = &sprite_;
            splitter_.sprite   = &sprite_;
            sprite_.texCoordId = 1234;
        }

        game::World world_;
        game::Logic logic_;

        TransportBelt transBelt_;
        Splitter splitter_;
        Sprite sprite_;
    };

    /// Splitter creates conveyor at its 2 tiles, then connects to neighboring conveyors
    /// v v
    /// C C Orientation down
    /// < >
    TEST_F(SplitterTest, BuildConnectConveyor) {
        auto& con_data_tl = TestSetupConveyor(world_, {0, 0}, Orientation::down, transBelt_);
        auto& con_data_tr = TestSetupConveyor(world_, {1, 0}, Orientation::down, transBelt_);

        auto& con_data_bl = TestSetupConveyor(world_, {0, 2}, Orientation::left, transBelt_);
        auto& con_data_br = TestSetupConveyor(world_, {1, 2}, Orientation::right, transBelt_);

        splitter_.SetWidth(2);

        TestSetupMultiTile(world_, {0, 1}, game::TileLayer::entity, Orientation::down, splitter_);

        splitter_.OnBuild(world_, logic_, {0, 1}, Orientation::down);

        auto* splitter_data = world_.GetTile({0, 1}, game::TileLayer::entity)->GetUniqueData<SplitterData>();
        ASSERT_NE(splitter_data, nullptr);

        EXPECT_EQ(con_data_tl.structure->target, splitter_data->right.structure.get());
        EXPECT_EQ(con_data_tr.structure->target, splitter_data->structure.get()); // Left

        EXPECT_EQ(splitter_data->right.structure->target, con_data_bl.structure.get());
        EXPECT_EQ(splitter_data->structure->target, con_data_br.structure.get()); // Left
    }

    /// Removing should disconnect from neighboring conveyors
    ///   Left
    /// < C <
    /// v C <
    TEST_F(SplitterTest, RemoveDisconnectConveyor) {
        auto& con_data_lt = TestSetupConveyor(world_, {0, 0}, Orientation::left, transBelt_);
        auto& con_data_lb = TestSetupConveyor(world_, {0, 1}, Orientation::down, transBelt_);

        auto& con_data_rt = TestSetupConveyor(world_, {2, 0}, Orientation::left, transBelt_);
        auto& con_data_rb = TestSetupConveyor(world_, {2, 1}, Orientation::left, transBelt_);

        splitter_.SetWidth(2);
        auto& splitter_data = TestSetupSplitter(world_, {1, 0}, Orientation::left, splitter_);


        splitter_data.structure->target       = con_data_lb.structure.get(); // Left
        splitter_data.right.structure->target = con_data_lt.structure.get();

        con_data_rb.structure->target = splitter_data.structure.get(); // Left
        con_data_rt.structure->target = splitter_data.right.structure.get();


        splitter_.OnRemove(world_, logic_, {1, 0});


        EXPECT_EQ(splitter_data.structure.get(), nullptr); // Left
        EXPECT_EQ(splitter_data.right.structure.get(), nullptr);

        EXPECT_EQ(con_data_rb.structure->target, nullptr);
        EXPECT_EQ(con_data_rt.structure->target, nullptr);
    }

    /// Both sides of splitter finds its targets
    TEST_F(SplitterTest, Serialize) {
        //
        // S >
        // S >
        data::PrototypeManager proto;
        data::UniqueDataManager unique;

        auto& belt     = proto.Make<TransportBelt>();
        auto& splitter = proto.Make<Splitter>();
        splitter.SetWidth(2);

        TestSetupConveyor(world_, {2, 1}, Orientation::right, belt);
        TestSetupConveyor(world_, {2, 2}, Orientation::right, belt);

        TestSetupSplitter(world_, {1, 1}, Orientation::right, splitter);

        data::active_prototype_manager   = &proto;
        data::active_unique_data_manager = &unique;


        //
        proto.GenerateRelocationTable();
        auto result = TestSerializeDeserialize(world_);
        result.DeserializePostProcess();

        auto [proto_splitter_top, data_splitter_top]       = game::GetConveyorInfo(result, {1, 1});
        auto [proto_splitter_bottom, data_splitter_bottom] = game::GetConveyorInfo(result, {1, 2});

        auto [proto_top, data_top] = game::GetConveyorInfo(result, {2, 1});
        EXPECT_EQ(data_splitter_top->structure->target, data_top->structure.get());

        auto [proto_bottom, data_bottom] = game::GetConveyorInfo(result, {2, 2});
        EXPECT_EQ(data_splitter_bottom->structure->target, data_bottom->structure.get());
    }
} // namespace jactorio::proto