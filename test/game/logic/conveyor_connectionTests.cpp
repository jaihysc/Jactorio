// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "game/logic/conveyor_connection.h"

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

        // ======================================================================
        // Connections

        auto& BuildStruct(WorldData& world,
                          const WorldCoord& coord,
                          const proto::Orientation orien,
                          const ConveyorStruct::TerminationType ttype = ConveyorStruct::TerminationType::straight,
                          const std::uint8_t len                      = 1) {
            auto& layer         = world.GetTile(coord)->GetLayer(TileLayer::entity);
            layer.prototypeData = &transBelt_;

            auto con_struct = std::make_shared<ConveyorStruct>(orien, ttype, len);

            auto* ud = layer.MakeUniqueData<proto::ConveyorData>(con_struct);
            assert(ud != nullptr);

            return *ud->structure;
        }
    };
    ///
    /// A conveyor pointing to another one will set the target of the former to the latter
    TEST_F(ConveyorConnectionTest, ConnectUpLeading) {
        // ^
        // ^

        auto& con_struct_ahead = BuildStruct(worldData_, {0, 0}, proto::Orientation::up);
        auto& con_struct       = BuildStruct(worldData_, {0, 1}, proto::Orientation::up);

        ConnectUp(worldData_, {0, 1});

        EXPECT_EQ(con_struct.target, &con_struct_ahead);
    }


    ///
    /// A conveyor placed in front of another one will set the target of the neighbor
    TEST_F(ConveyorConnectionTest, ConnectUpTrailing) {
        //  v
        //  >

        auto& con_struct_above = BuildStruct(worldData_, {0, 0}, proto::Orientation::down);
        auto& con_struct       = BuildStruct(worldData_, {0, 1}, proto::Orientation::right);

        ConnectUp(worldData_, {0, 1});

        EXPECT_EQ(con_struct_above.target, &con_struct);
    }

    ///
    /// Do not connect conveyors with orientations pointed at each other
    TEST_F(ConveyorConnectionTest, ConnectUpPointedTowardsEachOther) {
        // v
        // ^

        auto& con_struct_u = BuildStruct(worldData_, {0, 0}, proto::Orientation::down);
        auto& con_struct_d = BuildStruct(worldData_, {0, 1}, proto::Orientation::up);

        ConnectUp(worldData_, {0, 1});

        EXPECT_EQ(con_struct_u.target, nullptr);
        EXPECT_EQ(con_struct_d.target, nullptr);
    }
} // namespace jactorio::game