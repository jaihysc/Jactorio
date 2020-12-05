// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "game/logic/conveyor_connection.h"

#include "jactorioTests.h"

#include "game/world/world_data.h"
#include "proto/transport_belt.h"

namespace jactorio::game
{

    class ConveyorConnectionTest : public testing::Test
    {
    protected:
        WorldData worldData_;
        proto::TransportBelt transBelt_;

        void SetUp() override {
            worldData_.EmplaceChunk(0, 0);
        }

        ///
        /// Creates transport belt using provided conveyor structure
        auto& BuildStruct(WorldData& world, const WorldCoord& coord, std::shared_ptr<ConveyorStruct>& conveyor_struct) {
            auto& layer         = world.GetTile(coord)->GetLayer(TileLayer::entity);
            layer.prototypeData = &transBelt_;

            auto* ud = layer.MakeUniqueData<proto::ConveyorData>(conveyor_struct);
            assert(ud != nullptr);

            return *ud;
        }

        ///
        /// Creates a transport belt with its own conveyor structure
        auto& BuildStruct(WorldData& world,
                          const WorldCoord& coord,
                          const proto::Orientation orien,
                          const ConveyorStruct::TerminationType ttype = ConveyorStruct::TerminationType::straight,
                          const std::uint8_t len                      = 1) {
            auto con_struct = std::make_shared<ConveyorStruct>(orien, ttype, len);

            return BuildStruct(world, coord, con_struct);
        }
    };

    ///
    /// Should gracefully handle no struct above
    TEST_F(ConveyorConnectionTest, ConnectUpNoStructAbove) {
        BuildStruct(worldData_, {0, 1}, proto::Orientation::up);

        ConveyorConnectUp(worldData_, {0, 1});
    }

    ///
    /// Should gracefully handle entity not a conveyor struct
    TEST_F(ConveyorConnectionTest, ConnectUpNonStruct) {
        const proto::ContainerEntity container_proto;

        TestSetupContainer(worldData_, {0, 0}, container_proto);
        BuildStruct(worldData_, {0, 1}, proto::Orientation::up);

        ConveyorConnectUp(worldData_, {0, 1});
    }

    ///
    /// Do not connect to itself if the struct spans multiple tiles
    TEST_F(ConveyorConnectionTest, ConnectUpNoConnectSelf) {
        auto& structure = BuildStruct(worldData_, {0, 0}, proto::Orientation::up).structure;
        BuildStruct(worldData_, {0, 1}, structure);

        ConveyorConnectUp(worldData_, {0, 1});

        EXPECT_EQ(structure->target, nullptr);
    }

    ///
    /// A conveyor pointing to another one will set the target of the former to the latter
    TEST_F(ConveyorConnectionTest, ConnectUpLeading) {
        // ^
        // ^

        auto& con_struct_ahead = *BuildStruct(worldData_, {0, 0}, proto::Orientation::up).structure;
        auto& con_struct       = *BuildStruct(worldData_, {0, 1}, proto::Orientation::up).structure;

        ConveyorConnectUp(worldData_, {0, 1});

        EXPECT_EQ(con_struct.target, &con_struct_ahead);
    }

    ///
    /// A conveyor placed in front of another one will set the target of the neighbor
    TEST_F(ConveyorConnectionTest, ConnectUpTrailing) {
        //  v
        //  >

        auto& con_struct_d = *BuildStruct(worldData_, {0, 0}, proto::Orientation::down).structure;
        auto& con_struct_r = *BuildStruct(worldData_, {0, 1}, proto::Orientation::right).structure;

        ConveyorConnectUp(worldData_, {0, 1});

        EXPECT_EQ(con_struct_d.target, &con_struct_r);
    }

    ///
    /// Do not connect conveyors with orientations pointed at each other
    TEST_F(ConveyorConnectionTest, ConnectUpPointedTowardsEachOther) {
        // v
        // ^

        auto& con_struct_d = *BuildStruct(worldData_, {0, 0}, proto::Orientation::down).structure;
        auto& con_struct_u = *BuildStruct(worldData_, {0, 1}, proto::Orientation::up).structure;

        ConveyorConnectUp(worldData_, {0, 1});

        EXPECT_EQ(con_struct_d.target, nullptr);
        EXPECT_EQ(con_struct_u.target, nullptr);
    }

    ///
    /// When connecting to a conveyor, it should store the target's structIndex as targetInsertOffset
    TEST_F(ConveyorConnectionTest, ConnectUpSetStructIndex) {
        // >
        // ^

        auto& con_data_r   = BuildStruct(worldData_, {0, 0}, proto::Orientation::right);
        auto& con_struct_u = *BuildStruct(worldData_, {0, 1}, proto::Orientation::up).structure;

        con_data_r.structIndex = 10;

        ConveyorConnectUp(worldData_, {0, 1});

        EXPECT_EQ(con_struct_u.targetInsertOffset, 10);
    }

} // namespace jactorio::game