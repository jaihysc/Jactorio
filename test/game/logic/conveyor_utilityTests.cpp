// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "game/logic/conveyor_utility.h"

#include <functional>
#include <utility>

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
        /// Creates transport belt, provided parameters forwarded to ConveyorData constructor
        template <typename... TParams>
        auto& BuildConveyor(WorldData& world, const WorldCoord& coord, TParams&&... params) {
            auto& layer         = world.GetTile(coord)->GetLayer(TileLayer::entity);
            layer.prototypeData = &transBelt_;

            return layer.MakeUniqueData<proto::ConveyorData>(std::forward<TParams>(params)...);
        }

        ///
        /// Creates a transport belt with its own conveyor structure
        auto& CreateConveyor(WorldData& world,
                             const WorldCoord& coord,
                             const proto::Orientation orien,
                             const ConveyorStruct::TerminationType ttype = ConveyorStruct::TerminationType::straight,
                             const std::uint8_t len                      = 1) {
            auto con_struct = std::make_shared<ConveyorStruct>(orien, ttype, len);

            return BuildConveyor(world, coord, con_struct);
        }

        ///
        /// Checks if conveyor at current coords with current_direction
        /// grouped with other conveyor at other_coord with other_direction
        /// \param compare Function which can be used to do additional comparisons
        /// \return true if grouped
        J_NODISCARD bool TestGrouping(
            const WorldCoord other_coord,
            const proto::Orientation other_direction,
            const WorldCoord current_coord,
            const proto::Orientation direction,
            const std::function<
                void(const WorldData& world, proto::ConveyorData& current, const proto::ConveyorData& other)>& compare =
                [](auto&, auto&, auto&) {}) {

            WorldData world; // Cannot use test's world since this is building at the same tile multiple times
            world.EmplaceChunk(0, 0);

            // For testing grouping across chunk boundaries
            world.EmplaceChunk(0, -1);
            world.EmplaceChunk(0, 1);
            world.EmplaceChunk(-1, 0);
            world.EmplaceChunk(1, 0);

            auto& other_con_data = BuildConveyor(world, other_coord);
            ConveyorCreate(world, other_coord, other_con_data, other_direction);

            auto& con_data = BuildConveyor(world, current_coord);
            ConveyorCreate(world, current_coord, con_data, direction);

            compare(world, con_data, other_con_data);

            return con_data.structure == other_con_data.structure;
        }
    };

    ///
    /// Should gracefully handle no tile above
    TEST_F(ConveyorConnectionTest, ConnectUpNoTileAbove) {
        CreateConveyor(worldData_, {0, 0}, proto::Orientation::up);

        ConveyorConnectUp(worldData_, {0, 0});
    }

    ///
    /// Should gracefully handle no struct above
    TEST_F(ConveyorConnectionTest, ConnectUpNoStructAbove) {
        CreateConveyor(worldData_, {0, 1}, proto::Orientation::up);

        ConveyorConnectUp(worldData_, {0, 1});
    }

    ///
    /// Should gracefully handle entity not a conveyor struct
    TEST_F(ConveyorConnectionTest, ConnectUpNonStruct) {
        const proto::ContainerEntity container_proto;

        TestSetupContainer(worldData_, {0, 0}, container_proto);
        CreateConveyor(worldData_, {0, 1}, proto::Orientation::up);

        ConveyorConnectUp(worldData_, {0, 1});
    }

    ///
    /// Do not connect to itself if the struct spans multiple tiles
    TEST_F(ConveyorConnectionTest, ConnectUpNoConnectSelf) {
        auto& structure = CreateConveyor(worldData_, {0, 0}, proto::Orientation::up).structure;
        BuildConveyor(worldData_, {0, 1}, structure);

        ConveyorConnectUp(worldData_, {0, 1});

        EXPECT_EQ(structure->target, nullptr);
    }

    ///
    /// A conveyor pointing to another one will set the target of the former to the latter
    TEST_F(ConveyorConnectionTest, ConnectUpLeading) {
        // ^
        // ^

        auto& con_struct_ahead = *CreateConveyor(worldData_, {0, 0}, proto::Orientation::up).structure;
        auto& con_struct       = *CreateConveyor(worldData_, {0, 1}, proto::Orientation::up).structure;

        ConveyorConnectUp(worldData_, {0, 1});

        EXPECT_EQ(con_struct.target, &con_struct_ahead);
    }

    ///
    /// A conveyor placed in front of another one will set the target of the neighbor
    TEST_F(ConveyorConnectionTest, ConnectUpTrailing) {
        //  v
        //  >

        auto& con_struct_d = *CreateConveyor(worldData_, {0, 0}, proto::Orientation::down).structure;
        auto& con_struct_r = *CreateConveyor(worldData_, {0, 1}, proto::Orientation::right).structure;

        ConveyorConnectUp(worldData_, {0, 1});

        EXPECT_EQ(con_struct_d.target, &con_struct_r);
    }

    ///
    /// Do not connect conveyors with orientations pointed at each other
    TEST_F(ConveyorConnectionTest, ConnectUpPointedTowardsEachOther) {
        // v
        // ^

        auto& con_struct_d = *CreateConveyor(worldData_, {0, 0}, proto::Orientation::down).structure;
        auto& con_struct_u = *CreateConveyor(worldData_, {0, 1}, proto::Orientation::up).structure;

        ConveyorConnectUp(worldData_, {0, 1});

        EXPECT_EQ(con_struct_d.target, nullptr);
        EXPECT_EQ(con_struct_u.target, nullptr);
    }

    ///
    /// When connecting to a conveyor, it should store the target's structIndex as targetInsertOffset
    TEST_F(ConveyorConnectionTest, ConnectUpSetStructIndex) {
        // >
        // ^

        auto& con_data_r   = CreateConveyor(worldData_, {0, 0}, proto::Orientation::right);
        auto& con_struct_u = *CreateConveyor(worldData_, {0, 1}, proto::Orientation::up).structure;

        con_data_r.structIndex = 10;

        ConveyorConnectUp(worldData_, {0, 1});

        EXPECT_EQ(con_struct_u.targetInsertOffset, 10);
    }

    ///
    /// A conveyor pointing to another one will set the target of the former to the latter
    TEST_F(ConveyorConnectionTest, ConnectRightLeading) {
        // > >

        auto& con_struct       = *CreateConveyor(worldData_, {0, 0}, proto::Orientation::right).structure;
        auto& con_struct_ahead = *CreateConveyor(worldData_, {1, 0}, proto::Orientation::right).structure;

        ConveyorConnectRight(worldData_, {0, 0});

        EXPECT_EQ(con_struct.target, &con_struct_ahead);
    }

    ///
    /// A conveyor pointing to another one will set the target of the former to the latter
    TEST_F(ConveyorConnectionTest, ConnectDownLeading) {
        // v
        // v

        auto& con_struct       = *CreateConveyor(worldData_, {0, 0}, proto::Orientation::down).structure;
        auto& con_struct_ahead = *CreateConveyor(worldData_, {0, 1}, proto::Orientation::down).structure;

        ConveyorConnectDown(worldData_, {0, 0});

        EXPECT_EQ(con_struct.target, &con_struct_ahead);
    }

    ///
    /// A conveyor pointing to another one will set the target of the former to the latter
    TEST_F(ConveyorConnectionTest, ConnectLeftLeading) {
        // < <

        auto& con_struct_ahead = *CreateConveyor(worldData_, {0, 0}, proto::Orientation::left).structure;
        auto& con_struct       = *CreateConveyor(worldData_, {1, 0}, proto::Orientation::left).structure;

        ConveyorConnectLeft(worldData_, {1, 0});

        EXPECT_EQ(con_struct.target, &con_struct_ahead);
    }

    //
    //
    //
    //
    //

    TEST_F(ConveyorConnectionTest, DisconnectUpToNeighbor) {
        auto& con_struct        = *CreateConveyor(worldData_, {0, 1}, proto::Orientation::down).structure;
        auto& con_struct_behind = *CreateConveyor(worldData_, {0, 0}, proto::Orientation::down).structure;

        con_struct_behind.target = &con_struct;

        ConveyorDisconnectUp(worldData_, {0, 1});

        EXPECT_EQ(con_struct_behind.target, nullptr);
    }

    TEST_F(ConveyorConnectionTest, DisconnectUpNoDisconnectSelf) {
        auto& con_struct_ahead = *CreateConveyor(worldData_, {0, 0}, proto::Orientation::up).structure;
        auto& con_struct       = *CreateConveyor(worldData_, {0, 1}, proto::Orientation::up).structure;

        con_struct.target = &con_struct_ahead;

        ConveyorDisconnectUp(worldData_, {0, 1});

        EXPECT_EQ(con_struct.target, &con_struct_ahead);
    }

    ///
    /// If neighbor segment connects to current and bends, the bend must be removed after disconnecting
    TEST_F(ConveyorConnectionTest, DisconnectUpFromNeighborBending) {
        auto& con_struct_ahead = *CreateConveyor(worldData_, {0, 0}, proto::Orientation::down).structure;
        auto& con_struct       = *CreateConveyor(worldData_, {0, 1}, proto::Orientation::right).structure;

        con_struct_ahead.target          = &con_struct;
        con_struct_ahead.terminationType = ConveyorStruct::TerminationType::bend_left;

        con_struct_ahead.length     = 2;
        con_struct_ahead.itemOffset = 1;


        ConveyorDisconnectUp(worldData_, {0, 1});

        EXPECT_EQ(con_struct_ahead.target, nullptr);
        EXPECT_EQ(con_struct_ahead.terminationType, ConveyorStruct::TerminationType::straight);
        EXPECT_EQ(con_struct_ahead.length, 1);
        EXPECT_EQ(con_struct_ahead.itemOffset, 0);
    }

    TEST_F(ConveyorConnectionTest, DisconnectRightToNeighbor) {
        auto& con_struct        = *CreateConveyor(worldData_, {0, 0}, proto::Orientation::left).structure;
        auto& con_struct_behind = *CreateConveyor(worldData_, {1, 0}, proto::Orientation::left).structure;

        con_struct_behind.target = &con_struct;

        ConveyorDisconnectRight(worldData_, {0, 0});

        EXPECT_EQ(con_struct_behind.target, nullptr);
    }

    TEST_F(ConveyorConnectionTest, DisconnectDownToNeighbor) {
        auto& con_struct        = *CreateConveyor(worldData_, {0, 0}, proto::Orientation::up).structure;
        auto& con_struct_behind = *CreateConveyor(worldData_, {0, 1}, proto::Orientation::up).structure;

        con_struct_behind.target = &con_struct;

        ConveyorDisconnectDown(worldData_, {0, 0});

        EXPECT_EQ(con_struct_behind.target, nullptr);
    }

    TEST_F(ConveyorConnectionTest, DisconnectLeftToNeighbor) {
        auto& con_struct        = *CreateConveyor(worldData_, {1, 0}, proto::Orientation::right).structure;
        auto& con_struct_behind = *CreateConveyor(worldData_, {0, 0}, proto::Orientation::right).structure;

        con_struct_behind.target = &con_struct;

        ConveyorDisconnectLeft(worldData_, {1, 0});

        EXPECT_EQ(con_struct_behind.target, nullptr);
    }

    //
    //
    //
    //
    //

    ///
    /// Using ahead conveyor structure in same direction is prioritized over creating a new conveyor structure
    TEST_F(ConveyorConnectionTest, ConveyorCreateGroupAhead) {
        auto compare_func =
            [](const WorldData& /*world*/, const proto::ConveyorData& current, const proto::ConveyorData& other) {
                EXPECT_EQ(current.structure->length, 2);

                EXPECT_EQ(other.structIndex, 0);
                EXPECT_EQ(current.structIndex, 1);
            };

        EXPECT_TRUE(TestGrouping({0, 0}, proto::Orientation::up, {0, 1}, proto::Orientation::up, compare_func));
        EXPECT_TRUE(TestGrouping({1, 0}, proto::Orientation::right, {0, 0}, proto::Orientation::right, compare_func));
        EXPECT_TRUE(TestGrouping({0, 1}, proto::Orientation::down, {0, 0}, proto::Orientation::down, compare_func));
        EXPECT_TRUE(TestGrouping({0, 0}, proto::Orientation::left, {1, 0}, proto::Orientation::left, compare_func));
    }

    ///
    /// Cannot use ahead conveyor if it is in different direction
    TEST_F(ConveyorConnectionTest, ConveyorCreateGroupAheadDifferentDirection) {
        EXPECT_FALSE(TestGrouping({0, 0}, proto::Orientation::right, {0, 1}, proto::Orientation::up));
        EXPECT_FALSE(TestGrouping({0, 0}, proto::Orientation::up, {0, 1}, proto::Orientation::down));
    }

    ///
    /// Grouping with behind conveyor prioritized over creating a new conveyor structure
    TEST_F(ConveyorConnectionTest, ConveyorCreateGroupBehind) {
        auto compare_func =
            [](const WorldData& world, const proto::ConveyorData& current, const proto::ConveyorData& other) {
                EXPECT_EQ(world.LogicGetChunks().at(0)->GetLogicGroup(Chunk::LogicGroup::conveyor).size(), 1);

                EXPECT_EQ(current.structure->length, 2);

                EXPECT_EQ(current.structIndex, 0);
                EXPECT_EQ(other.structIndex, 1);
            };

        EXPECT_TRUE(TestGrouping({0, 1}, proto::Orientation::up, {0, 0}, proto::Orientation::up, compare_func));
        EXPECT_TRUE(TestGrouping({0, 0}, proto::Orientation::right, {1, 0}, proto::Orientation::right, compare_func));
        EXPECT_TRUE(TestGrouping({0, 0}, proto::Orientation::down, {0, 1}, proto::Orientation::down, compare_func));
        EXPECT_TRUE(TestGrouping({1, 0}, proto::Orientation::left, {0, 0}, proto::Orientation::left, compare_func));
    }

    ///
    /// Conveyors will not group across chunk boundaries
    TEST_F(ConveyorConnectionTest, ConveyorCreateNoGroupCrossChunk) {
        EXPECT_FALSE(TestGrouping({0, -1}, proto::Orientation::up, {0, 0}, proto::Orientation::up));
        EXPECT_FALSE(TestGrouping({31, 0}, proto::Orientation::right, {32, 0}, proto::Orientation::right));
        EXPECT_FALSE(TestGrouping({0, 31}, proto::Orientation::down, {0, 32}, proto::Orientation::down));
        EXPECT_FALSE(TestGrouping({-1, 0}, proto::Orientation::left, {0, 0}, proto::Orientation::left));
    }

    //
    //
    //
    //
    //

    TEST_F(ConveyorConnectionTest, ConveyorRenumber) {
        auto& con_data_1             = CreateConveyor(worldData_, {0, 0}, proto::Orientation::up);
        con_data_1.structure->length = 3;
        con_data_1.structIndex       = 5;

        auto& con_data_2       = CreateConveyor(worldData_, {0, 1}, proto::Orientation::up);
        con_data_2.structIndex = 6;

        auto& con_data_3       = CreateConveyor(worldData_, {0, 2}, proto::Orientation::up);
        con_data_3.structIndex = 7;

        ConveyorRenumber(worldData_, {0, 0}, 1);
        EXPECT_EQ(con_data_1.structIndex, 1);
        EXPECT_EQ(con_data_2.structIndex, 2);
        EXPECT_EQ(con_data_3.structIndex, 7);
    }

    TEST_F(ConveyorConnectionTest, ChangeStructure) {
        auto& new_con_struct   = CreateConveyor(worldData_, {5, 5}, proto::Orientation::up).structure;
        new_con_struct->length = 3;


        auto& con_data_h             = CreateConveyor(worldData_, {0, 0}, proto::Orientation::up);
        con_data_h.structure->length = 3;

        auto& con_data_2 = BuildConveyor(worldData_, {0, 1}, con_data_h.structure);
        auto& con_data_3 = BuildConveyor(worldData_, {0, 2}, con_data_h.structure);

        ConveyorChangeStructure(worldData_, {0, 0}, new_con_struct);
        EXPECT_EQ(con_data_h.structure, new_con_struct);
        EXPECT_EQ(con_data_2.structure, new_con_struct);
        EXPECT_EQ(con_data_3.structure, new_con_struct);
    }

    ///
    /// When changing structure, other structures which has the old structure as a target must be updated
    /// to use the new structure
    TEST_F(ConveyorConnectionTest, ChangeStructureUpdateTargets) {
        worldData_.EmplaceChunk(0, -1);

        auto& new_con_struct   = CreateConveyor(worldData_, {5, 5}, proto::Orientation::left).structure;
        new_con_struct->length = 3;


        auto& con_data_h             = CreateConveyor(worldData_, {0, 0}, proto::Orientation::left);
        con_data_h.structure->length = 999; // Should use length of new con struct, not old

        BuildConveyor(worldData_, {1, 0}, con_data_h.structure);
        BuildConveyor(worldData_, {2, 0}, con_data_h.structure);


        auto create_dependee_conveyor = [&](const WorldCoord& coord) -> proto::ConveyorData& {
            auto& dependee = CreateConveyor(worldData_,
                                            coord,
                                            // Orientation does not matter, it determines if is dependee based on target
                                            proto::Orientation::up,
                                            ConveyorStruct::TerminationType::straight);

            dependee.structure->target = con_data_h.structure.get();

            return dependee;
        };

        // Dependee conveyors
        auto& d_con_data_1 = create_dependee_conveyor({0, -1});
        auto& d_con_data_2 = create_dependee_conveyor({1, 1});
        auto& d_con_data_3 = create_dependee_conveyor({2, -1});
        auto& d_con_data_4 = create_dependee_conveyor({3, 0});


        ConveyorChangeStructure(worldData_, {0, 0}, new_con_struct);

        EXPECT_EQ(d_con_data_1.structure->target, new_con_struct.get());
        EXPECT_EQ(d_con_data_2.structure->target, new_con_struct.get());
        EXPECT_EQ(d_con_data_3.structure->target, new_con_struct.get());
        EXPECT_EQ(d_con_data_4.structure->target, new_con_struct.get());
    }
} // namespace jactorio::game