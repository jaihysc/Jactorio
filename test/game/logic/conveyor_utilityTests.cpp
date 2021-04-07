// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "game/logic/conveyor_utility.h"

#include <functional>
#include <utility>

#include "jactorioTests.h"

#include "game/world/world.h"
#include "proto/transport_belt.h"

namespace jactorio::game
{
    class ConveyorUtilityTest : public testing::Test
    {
    protected:
        /// Logic group chosen for the tests
        static constexpr LogicGroup kLogicGroup_ = LogicGroup::splitter;

        World world_;
        proto::TransportBelt transBelt_;

        void SetUp() override {
            world_.EmplaceChunk({0, 0});
        }
    };

    TEST_F(ConveyorUtilityTest, GetConveyorDataConveyor) {
        auto& con_data = TestSetupConveyor(world_, {0, 0}, Orientation::up, transBelt_);

        EXPECT_EQ(GetConData(world_, {0, 0}), &con_data);
    }

    TEST_F(ConveyorUtilityTest, GetConveyorDataSplitter) {
        proto::Splitter splitter;
        splitter.SetWidth(2);

        auto& splitter_data = TestSetupBlankSplitter(world_, {0, 0}, Orientation::up, splitter);

        EXPECT_EQ(GetConData(world_, {0, 0}), &splitter_data.left);
        EXPECT_EQ(GetConData(world_, {1, 0}), &splitter_data.right);
    }

    TEST_F(ConveyorUtilityTest, GetConveyorDataSplitterInverted) {
        proto::Splitter splitter;
        splitter.SetWidth(2);

        auto& splitter_data = TestSetupBlankSplitter(world_, {0, 0}, Orientation::left, splitter);

        EXPECT_EQ(GetConData(world_, {0, 1}), &splitter_data.left);
        EXPECT_EQ(GetConData(world_, {0, 0}), &splitter_data.right);
    }


    /// Should gracefully handle no tile above
    TEST_F(ConveyorUtilityTest, ConnectUpNoTileAbove) {
        TestSetupConveyor(world_, {0, 0}, Orientation::up, transBelt_);

        ConveyorConnectUp(world_, {0, 0});
    }

    /// Should gracefully handle no struct above
    TEST_F(ConveyorUtilityTest, ConnectUpNoStructAbove) {
        TestSetupConveyor(world_, {0, 1}, Orientation::up, transBelt_);

        ConveyorConnectUp(world_, {0, 1});
    }

    /// Should gracefully handle entity not a conveyor struct
    TEST_F(ConveyorUtilityTest, ConnectUpNonStruct) {
        const proto::ContainerEntity container_proto;

        TestSetupContainer(world_, {0, 0}, Orientation::up, container_proto);
        TestSetupConveyor(world_, {0, 1}, Orientation::up, transBelt_);

        ConveyorConnectUp(world_, {0, 1});
    }

    /// Do not connect to itself if the struct spans multiple tiles
    TEST_F(ConveyorUtilityTest, ConnectUpNoConnectSelf) {
        auto& structure = TestSetupConveyor(world_, {0, 0}, Orientation::up, transBelt_).structure;
        TestSetupConveyor(world_, {0, 1}, transBelt_, structure);

        ConveyorConnectUp(world_, {0, 1});

        EXPECT_EQ(structure->target, nullptr);
    }

    /// A conveyor pointing to another one will set the target of the former to the latter
    TEST_F(ConveyorUtilityTest, ConnectUpLeading) {
        // ^
        // ^

        auto& con_struct_ahead = *TestSetupConveyor(world_, {0, 0}, Orientation::up, transBelt_).structure;
        auto& con_struct       = *TestSetupConveyor(world_, {0, 1}, Orientation::up, transBelt_).structure;

        ConveyorConnectUp(world_, {0, 1});

        EXPECT_EQ(con_struct.target, &con_struct_ahead);
    }

    /// A conveyor placed in front of another one will set the target of the neighbor
    TEST_F(ConveyorUtilityTest, ConnectUpTrailing) {
        //  v
        //  >

        auto& con_struct_d = *TestSetupConveyor(world_, {0, 0}, Orientation::down, transBelt_).structure;
        auto& con_struct_r = *TestSetupConveyor(world_, {0, 1}, Orientation::right, transBelt_).structure;

        ConveyorConnectUp(world_, {0, 1});

        EXPECT_EQ(con_struct_d.target, &con_struct_r);
    }

    /// Do not connect conveyors with orientations pointed at each other
    TEST_F(ConveyorUtilityTest, ConnectUpPointedTowardsEachOther) {
        // v
        // ^

        auto& con_struct_d = *TestSetupConveyor(world_, {0, 0}, Orientation::down, transBelt_).structure;
        auto& con_struct_u = *TestSetupConveyor(world_, {0, 1}, Orientation::up, transBelt_).structure;

        ConveyorConnectUp(world_, {0, 1});

        EXPECT_EQ(con_struct_d.target, nullptr);
        EXPECT_EQ(con_struct_u.target, nullptr);
    }

    /// When connecting to a conveyor, it should store the target's structIndex as sideInsertIndex
    TEST_F(ConveyorUtilityTest, ConnectUpSetStructIndex) {
        // >
        // ^

        auto& con_data_r   = TestSetupConveyor(world_, {0, 0}, Orientation::right, transBelt_);
        auto& con_struct_u = *TestSetupConveyor(world_, {0, 1}, Orientation::up, transBelt_).structure;

        con_data_r.structIndex = 10;

        ConveyorConnectUp(world_, {0, 1});

        EXPECT_EQ(con_struct_u.sideInsertIndex, 10);
    }

    /// A conveyor pointing to another one will set the target of the former to the latter
    TEST_F(ConveyorUtilityTest, ConnectRightLeading) {
        // > >

        auto& con_struct       = *TestSetupConveyor(world_, {0, 0}, Orientation::right, transBelt_).structure;
        auto& con_struct_ahead = *TestSetupConveyor(world_, {1, 0}, Orientation::right, transBelt_).structure;

        ConveyorConnectRight(world_, {0, 0});

        EXPECT_EQ(con_struct.target, &con_struct_ahead);
    }

    /// A conveyor pointing to another one will set the target of the former to the latter
    TEST_F(ConveyorUtilityTest, ConnectDownLeading) {
        // v
        // v

        auto& con_struct       = *TestSetupConveyor(world_, {0, 0}, Orientation::down, transBelt_).structure;
        auto& con_struct_ahead = *TestSetupConveyor(world_, {0, 1}, Orientation::down, transBelt_).structure;

        ConveyorConnectDown(world_, {0, 0});

        EXPECT_EQ(con_struct.target, &con_struct_ahead);
    }

    /// A conveyor pointing to another one will set the target of the former to the latter
    TEST_F(ConveyorUtilityTest, ConnectLeftLeading) {
        // < <

        auto& con_struct_ahead = *TestSetupConveyor(world_, {0, 0}, Orientation::left, transBelt_).structure;
        auto& con_struct       = *TestSetupConveyor(world_, {1, 0}, Orientation::left, transBelt_).structure;

        ConveyorConnectLeft(world_, {1, 0});

        EXPECT_EQ(con_struct.target, &con_struct_ahead);
    }

    //
    //
    //
    //
    //

    TEST_F(ConveyorUtilityTest, DisconnectUpToNeighbor) {
        auto& con_struct        = *TestSetupConveyor(world_, {0, 1}, Orientation::down, transBelt_).structure;
        auto& con_struct_behind = *TestSetupConveyor(world_, {0, 0}, Orientation::down, transBelt_).structure;

        con_struct_behind.target = &con_struct;

        ConveyorDisconnectUp(world_, {0, 1});

        EXPECT_EQ(con_struct_behind.target, nullptr);
    }

    /// Only disconnects the conveyor above, not itself
    TEST_F(ConveyorUtilityTest, DisconnectUpNoDisconnectSelf) {
        auto& con_struct_ahead = *TestSetupConveyor(world_, {0, 0}, Orientation::up, transBelt_).structure;
        auto& con_struct       = *TestSetupConveyor(world_, {0, 1}, Orientation::up, transBelt_).structure;

        con_struct.target = &con_struct_ahead;

        ConveyorDisconnectUp(world_, {0, 1});

        EXPECT_EQ(con_struct.target, &con_struct_ahead);
    }

    /// If neighbor segment connects to current and bends, the bend must be removed after disconnecting
    TEST_F(ConveyorUtilityTest, DisconnectUpFromNeighborBending) {
        auto& con_data_above = TestSetupConveyor(world_, {0, 0}, Orientation::down, transBelt_);
        auto& con_struct     = *TestSetupConveyor(world_, {0, 1}, Orientation::right, transBelt_).structure;

        auto& con_struct_ahead = *con_data_above.structure;

        con_data_above.structIndex = 1; // Should subtract 1

        con_struct_ahead.target          = &con_struct;
        con_struct_ahead.terminationType = ConveyorStruct::TerminationType::bend_left;

        con_struct_ahead.length     = 2;
        con_struct_ahead.headOffset = 1;


        ConveyorDisconnectUp(world_, {0, 1});

        EXPECT_EQ(con_data_above.structIndex, 0);
        EXPECT_EQ(con_struct_ahead.target, nullptr);
        EXPECT_EQ(con_struct_ahead.terminationType, ConveyorStruct::TerminationType::straight);
        EXPECT_EQ(con_struct_ahead.length, 1);
        EXPECT_EQ(con_struct_ahead.headOffset, 0);
    }

    TEST_F(ConveyorUtilityTest, DisconnectRightToNeighbor) {
        auto& con_struct        = *TestSetupConveyor(world_, {0, 0}, Orientation::left, transBelt_).structure;
        auto& con_struct_behind = *TestSetupConveyor(world_, {1, 0}, Orientation::left, transBelt_).structure;

        con_struct_behind.target = &con_struct;

        ConveyorDisconnectRight(world_, {0, 0});

        EXPECT_EQ(con_struct_behind.target, nullptr);
    }

    TEST_F(ConveyorUtilityTest, DisconnectDownToNeighbor) {
        auto& con_struct        = *TestSetupConveyor(world_, {0, 0}, Orientation::up, transBelt_).structure;
        auto& con_struct_behind = *TestSetupConveyor(world_, {0, 1}, Orientation::up, transBelt_).structure;

        con_struct_behind.target = &con_struct;

        ConveyorDisconnectDown(world_, {0, 0});

        EXPECT_EQ(con_struct_behind.target, nullptr);
    }

    TEST_F(ConveyorUtilityTest, DisconnectLeftToNeighbor) {
        auto& con_struct        = *TestSetupConveyor(world_, {1, 0}, Orientation::right, transBelt_).structure;
        auto& con_struct_behind = *TestSetupConveyor(world_, {0, 0}, Orientation::right, transBelt_).structure;

        con_struct_behind.target = &con_struct;

        ConveyorDisconnectLeft(world_, {1, 0});

        EXPECT_EQ(con_struct_behind.target, nullptr);
    }

    //
    //
    //
    //
    //

    class ConveyorGroupingTest : public testing::Test
    {
    protected:
        /// Logic group chosen for the tests
        static constexpr LogicGroup kLogicGroup_ = LogicGroup::splitter;

        World world_;
        proto::TransportBelt transBelt_;

        /// Checks if conveyor at current coords with current_direction
        /// grouped with other conveyor at other_coord with other_direction
        /// \param compare Function which can be used to do additional comparisons
        /// \return true if grouped
        J_NODISCARD bool TestGrouping(
            const WorldCoord other_coord,
            const Orientation other_direction,
            const WorldCoord current_coord,
            const Orientation direction,
            const std::function<
                void(const World& world, proto::ConveyorData& current, const proto::ConveyorData& other)>& compare =
                [](auto&, auto&, auto&) {}) const {

            World world; // Cannot use test's world since this is building at the same tile multiple times
            world.EmplaceChunk({0, 0});

            // For testing grouping across chunk boundaries
            world.EmplaceChunk({0, -1});
            world.EmplaceChunk({0, 1});
            world.EmplaceChunk({-1, 0});
            world.EmplaceChunk({1, 0});

            auto& other_con_data = TestSetupConveyor(world, other_coord, other_direction, transBelt_);
            ConveyorCreate(world, other_coord, other_con_data, other_direction, kLogicGroup_);

            auto& con_data = TestSetupConveyor(world, current_coord, direction, transBelt_);
            ConveyorCreate(world, current_coord, con_data, direction, kLogicGroup_);

            compare(world, con_data, other_con_data);

            return con_data.structure == other_con_data.structure;
        }
    };

    /// Using ahead conveyor structure in same direction is prioritized over creating a new conveyor structure
    TEST_F(ConveyorGroupingTest, ConveyorCreateGroupAhead) {
        auto compare_func =
            [](const World& /*world*/, const proto::ConveyorData& current, const proto::ConveyorData& other) {
                EXPECT_EQ(current.structure->length, 2);

                EXPECT_EQ(other.structIndex, 0);
                EXPECT_EQ(current.structIndex, 1);
            };

        EXPECT_TRUE(TestGrouping({0, 0}, Orientation::up, {0, 1}, Orientation::up, compare_func));
        EXPECT_TRUE(TestGrouping({1, 0}, Orientation::right, {0, 0}, Orientation::right, compare_func));
        EXPECT_TRUE(TestGrouping({0, 1}, Orientation::down, {0, 0}, Orientation::down, compare_func));
        EXPECT_TRUE(TestGrouping({0, 0}, Orientation::left, {1, 0}, Orientation::left, compare_func));
    }

    /// Cannot use ahead conveyor if it is in different direction
    TEST_F(ConveyorGroupingTest, ConveyorCreateGroupAheadDifferentDirection) {
        EXPECT_FALSE(TestGrouping({0, 0}, Orientation::right, {0, 1}, Orientation::up));
        EXPECT_FALSE(TestGrouping({0, 0}, Orientation::up, {0, 1}, Orientation::down));
    }

    /// Grouping with behind conveyor prioritized over creating a new conveyor structure
    TEST_F(ConveyorGroupingTest, ConveyorCreateGroupBehind) {
        auto compare_func =
            [](const World& world, const proto::ConveyorData& current, const proto::ConveyorData& other) {
                EXPECT_EQ(world.LogicGetChunks().at(0)->GetLogicGroup(kLogicGroup_).size(), 1);

                EXPECT_EQ(current.structure->length, 2);

                EXPECT_EQ(current.structIndex, 0);
                EXPECT_EQ(other.structIndex, 1);
            };

        EXPECT_TRUE(TestGrouping({0, 1}, Orientation::up, {0, 0}, Orientation::up, compare_func));
        EXPECT_TRUE(TestGrouping({0, 0}, Orientation::right, {1, 0}, Orientation::right, compare_func));
        EXPECT_TRUE(TestGrouping({0, 0}, Orientation::down, {0, 1}, Orientation::down, compare_func));
        EXPECT_TRUE(TestGrouping({1, 0}, Orientation::left, {0, 0}, Orientation::left, compare_func));
    }

    /// Conveyors will not group across chunk boundaries
    TEST_F(ConveyorGroupingTest, ConveyorCreateNoGroupCrossChunk) {
        EXPECT_FALSE(TestGrouping({0, -1}, Orientation::up, {0, 0}, Orientation::up));
        EXPECT_FALSE(TestGrouping({31, 0}, Orientation::right, {32, 0}, Orientation::right));
        EXPECT_FALSE(TestGrouping({0, 31}, Orientation::down, {0, 32}, Orientation::down));
        EXPECT_FALSE(TestGrouping({-1, 0}, Orientation::left, {0, 0}, Orientation::left));
    }

    //
    //
    //
    //
    //

    /// Destroy head should unregister the head from logic updates
    TEST_F(ConveyorUtilityTest, DestroyHead) {
        //
        // >
        //

        {
            TestSetupConveyor(world_, {0, 0}, Orientation::right, transBelt_);

            world_.LogicRegister(kLogicGroup_, {0, 0}, TileLayer::entity);
        }

        ConveyorDestroy(world_, {0, 0}, kLogicGroup_);

        auto* con_data = world_.GetTile({0, 0}, TileLayer::entity)->GetUniqueData<proto::ConveyorData>();
        ASSERT_NE(con_data, nullptr);
        EXPECT_EQ(con_data->structure, nullptr);

        EXPECT_EQ(world_.GetChunkW({0, 0})->GetLogicGroup(kLogicGroup_).size(), 0);
    }

    /// Removing beginning of grouped conveyor segment with bending termination
    /// Creates new segment with what was the second tile now the head
    TEST_F(ConveyorUtilityTest, DestroyHeadBendingTermination) {
        //
        // > > v
        //

        {
            auto& head_con_data       = TestSetupConveyor(world_, {1, 0}, Orientation::right, transBelt_);
            head_con_data.structIndex = 1;
            head_con_data.structure->terminationType = ConveyorStruct::TerminationType::bend_right;
            head_con_data.structure->length          = 3;

            TestSetupConveyor(world_, {0, 0}, transBelt_, head_con_data.structure);

            TestSetupConveyor(world_, {2, 0}, Orientation::down, transBelt_);


            world_.LogicRegister(kLogicGroup_, {1, 0}, TileLayer::entity);
            world_.LogicRegister(kLogicGroup_, {2, 0}, TileLayer::entity);
        }

        ConveyorDestroy(world_, {1, 0}, kLogicGroup_);


        auto* behind_con_data = GetConData(world_, {0, 0});
        ASSERT_NE(behind_con_data, nullptr);

        EXPECT_EQ(behind_con_data->structure->length, 1);
        EXPECT_EQ(behind_con_data->structure->terminationType, ConveyorStruct::TerminationType::straight);

        EXPECT_EQ(world_.GetChunkW({0, 0})->GetLogicGroup(kLogicGroup_).size(), 2);
    }

    /// Removing middle of grouped conveyor segment
    /// Create new segment behind, shorten segment ahead
    TEST_F(ConveyorUtilityTest, DestroyMiddle) {
        //
        // > > /> > >
        //

        {
            auto& head_con_data                 = TestSetupConveyor(world_, {4, 0}, Orientation::right, transBelt_);
            head_con_data.structure->headOffset = 30;
            head_con_data.structure->length     = 5;

            auto& con_data_2       = TestSetupConveyor(world_, {3, 0}, transBelt_, head_con_data.structure);
            con_data_2.structIndex = 1;

            auto& con_data_3       = TestSetupConveyor(world_, {2, 0}, transBelt_, head_con_data.structure);
            con_data_3.structIndex = 2;

            auto& con_data_4       = TestSetupConveyor(world_, {1, 0}, transBelt_, head_con_data.structure);
            con_data_4.structIndex = 3;

            auto& con_data_5       = TestSetupConveyor(world_, {0, 0}, transBelt_, head_con_data.structure);
            con_data_5.structIndex = 4;
        }

        ConveyorDestroy(world_, {2, 0}, kLogicGroup_);

        auto* ahead_con_data = GetConData(world_, {4, 0});
        ASSERT_NE(ahead_con_data, nullptr);

        EXPECT_EQ(ahead_con_data->structure->length, 2);


        auto* behind_con_data = GetConData(world_, {0, 0});
        ASSERT_NE(behind_con_data, nullptr);

        EXPECT_EQ(behind_con_data->structIndex, 1);

        EXPECT_EQ(behind_con_data->structure->length, 2);
        EXPECT_EQ(behind_con_data->structure->headOffset, 27);


        // The newly created segment was registered for logic updates
        EXPECT_EQ(world_.GetChunkW({0, 0})->GetLogicGroup(kLogicGroup_).size(), 1);
    }

    //
    //
    //
    //
    //

    TEST_F(ConveyorUtilityTest, ConveyorRenumber) {
        auto& con_data_1             = TestSetupConveyor(world_, {0, 0}, Orientation::up, transBelt_);
        con_data_1.structure->length = 3;
        con_data_1.structIndex       = 5;

        auto& con_data_2       = TestSetupConveyor(world_, {0, 1}, Orientation::up, transBelt_);
        con_data_2.structIndex = 6;

        auto& con_data_3       = TestSetupConveyor(world_, {0, 2}, Orientation::up, transBelt_);
        con_data_3.structIndex = 7;

        ConveyorRenumber(world_, {0, 0}, 1);
        EXPECT_EQ(con_data_1.structIndex, 1);
        EXPECT_EQ(con_data_2.structIndex, 2);
        EXPECT_EQ(con_data_3.structIndex, 7);
    }

    TEST_F(ConveyorUtilityTest, ChangeStructure) {
        auto& new_con_struct   = TestSetupConveyor(world_, {5, 5}, Orientation::up, transBelt_).structure;
        new_con_struct->length = 3;


        auto& con_data_h             = TestSetupConveyor(world_, {0, 0}, Orientation::up, transBelt_);
        con_data_h.structure->length = 3;

        auto& con_data_2 = TestSetupConveyor(world_, {0, 1}, transBelt_, con_data_h.structure);
        auto& con_data_3 = TestSetupConveyor(world_, {0, 2}, transBelt_, con_data_h.structure);

        ConveyorChangeStructure(world_, {0, 0}, new_con_struct);
        EXPECT_EQ(con_data_h.structure, new_con_struct);
        EXPECT_EQ(con_data_2.structure, new_con_struct);
        EXPECT_EQ(con_data_3.structure, new_con_struct);
    }

    /// When changing structure, other structures which has the old structure as a target must be updated
    /// to use the new structure
    TEST_F(ConveyorUtilityTest, ChangeStructureUpdateTargets) {
        world_.EmplaceChunk({0, -1});

        auto& new_con_struct   = TestSetupConveyor(world_, {5, 5}, Orientation::left, transBelt_).structure;
        new_con_struct->length = 3;


        auto& con_data_h             = TestSetupConveyor(world_, {0, 0}, Orientation::left, transBelt_);
        con_data_h.structure->length = 200; // Should use length of new con struct, not old

        TestSetupConveyor(world_, {1, 0}, transBelt_, con_data_h.structure);
        TestSetupConveyor(world_, {2, 0}, transBelt_, con_data_h.structure);


        auto create_dependee_conveyor = [&](const WorldCoord& coord) -> proto::ConveyorData& {
            auto& dependee =
                TestSetupConveyor(world_,
                                  coord,
                                  // Orientation does not matter, it determines if is dependee based on target
                                  Orientation::up,
                                  transBelt_,
                                  ConveyorStruct::TerminationType::straight);

            dependee.structure->target = con_data_h.structure.get();

            return dependee;
        };

        // Dependee conveyors
        auto& d_con_data_1 = create_dependee_conveyor({0, -1});
        auto& d_con_data_2 = create_dependee_conveyor({1, 1});
        auto& d_con_data_3 = create_dependee_conveyor({2, -1});
        auto& d_con_data_4 = create_dependee_conveyor({3, 0});


        ConveyorChangeStructure(world_, {0, 0}, new_con_struct);

        EXPECT_EQ(d_con_data_1.structure->target, new_con_struct.get());
        EXPECT_EQ(d_con_data_2.structure->target, new_con_struct.get());
        EXPECT_EQ(d_con_data_3.structure->target, new_con_struct.get());
        EXPECT_EQ(d_con_data_4.structure->target, new_con_struct.get());
    }

    /// Ensures the function ConveyorLogicRemove also functions for splitters
    TEST_F(ConveyorUtilityTest, LogicRemoveSplitter) {
        const proto::Splitter splitter;

        auto& splitter_data = TestSetupSplitter(world_, {0, 0}, Orientation::up, splitter);
        world_.LogicRegister(kLogicGroup_, {0, 0}, TileLayer::entity);

        ConveyorLogicRemove(world_, {0, 0}, *splitter_data.left.structure, kLogicGroup_);

        EXPECT_EQ(world_.LogicGetChunks().size(), 0);
    }

    //
    //
    //
    //

    class ConveyorCalcLineOrienTest : public testing::Test
    {
    protected:
        World world_;

        void SetUp() override {
            world_.EmplaceChunk({0, 0});
        }

        /// Creates a conveyor with the provided orientation above/right/below/left of 1, 1
        auto& BuildTopConveyor(const Orientation orientation) {
            return BuildConveyor({1, 0}, orientation);
        }

        auto& BuildRightConveyor(const Orientation orientation) {
            return BuildConveyor({2, 1}, orientation);
        }

        auto& BuildBottomConveyor(const Orientation orientation) {
            return BuildConveyor({1, 2}, orientation);
        }

        auto& BuildLeftConveyor(const Orientation orientation) {
            return BuildConveyor({0, 1}, orientation);
        }

        /// Validates that a tile at coords 1,1 with the placement orientation produces the expected line
        /// orientation
        void ValidateResultOrientation(const Orientation place_orien,
                                       const proto::LineOrientation expected_l_orien) const {
            EXPECT_EQ(ConveyorCalcLineOrien(world_, {1, 1}, place_orien), expected_l_orien);
        }

    private:
        proto::TransportBelt lineProto_;

        proto::ConveyorData& BuildConveyor(const WorldCoord coord, const Orientation direction) {
            auto* tile = world_.GetTile(coord, TileLayer::entity);

            tile->SetPrototype(direction, &lineProto_);

            auto& con_data = tile->MakeUniqueData<proto::ConveyorData>();
            ConveyorCreate(world_, coord, con_data, direction, LogicGroup::conveyor);

            return con_data;
        }
    };

    /// A tile with no conveyor structure is treated as no conveyor at tile
    TEST_F(ConveyorCalcLineOrienTest, IgnoreNullptrStruct) {
        auto& l_con     = BuildLeftConveyor(Orientation::right);
        l_con.structure = nullptr;

        ValidateResultOrientation(Orientation::up, proto::LineOrientation::up);
    }

    TEST_F(ConveyorCalcLineOrienTest, RightBendUp) {
        //
        // > ^
        //
        BuildLeftConveyor(Orientation::right);
        ValidateResultOrientation(Orientation::up, proto::LineOrientation::right_up);
    }

    TEST_F(ConveyorCalcLineOrienTest, LeftBendUp) {
        //
        //  ^ <
        //
        BuildRightConveyor(Orientation::left);
        ValidateResultOrientation(Orientation::up, proto::LineOrientation::left_up);
    }

    TEST_F(ConveyorCalcLineOrienTest, LeftRightStraightUp) {
        //
        // > ^ <
        //
        // Top and bottom points to one line, line should be straight

        BuildLeftConveyor(Orientation::right);
        BuildRightConveyor(Orientation::left);
        ValidateResultOrientation(Orientation::up, proto::LineOrientation::up);
    }

    TEST_F(ConveyorCalcLineOrienTest, RightBendUpHasRightBehind) {
        //
        // > ^
        //   v
        //
        BuildLeftConveyor(Orientation::right);
        BuildBottomConveyor(Orientation::down);
        ValidateResultOrientation(Orientation::up, proto::LineOrientation::right_up);
    }

    TEST_F(ConveyorCalcLineOrienTest, RightStraightUpHasUpBehind) {
        //
        // > ^
        //   ^
        //
        BuildLeftConveyor(Orientation::right);
        BuildBottomConveyor(Orientation::up);
        ValidateResultOrientation(Orientation::up, proto::LineOrientation::up);
    }

    TEST_F(ConveyorCalcLineOrienTest, LeftBendUpHasLeftAtLeftSide) {
        //
        // < ^ <
        //

        BuildLeftConveyor(Orientation::left);
        BuildRightConveyor(Orientation::left);
        ValidateResultOrientation(Orientation::up, proto::LineOrientation::left_up);
    }

    // ======================================================================

    TEST_F(ConveyorCalcLineOrienTest, DownBendRight) {
        //
        //  v
        //  >
        //
        BuildTopConveyor(Orientation::down);
        ValidateResultOrientation(Orientation::right, proto::LineOrientation::down_right);
    }

    TEST_F(ConveyorCalcLineOrienTest, UpBendRight) {
        //
        // >
        // ^
        //
        BuildBottomConveyor(Orientation::up);
        ValidateResultOrientation(Orientation::right, proto::LineOrientation::up_right);
    }

    TEST_F(ConveyorCalcLineOrienTest, UpDownStraightRight) {
        //
        // v
        // >
        // ^
        //

        BuildTopConveyor(Orientation::down);
        BuildBottomConveyor(Orientation::up);
        ValidateResultOrientation(Orientation::right, proto::LineOrientation::right);
    }

    TEST_F(ConveyorCalcLineOrienTest, DownBendRightHasUpAtLeftSide) {
        //
        //   v
        // ^ >
        //
        BuildTopConveyor(Orientation::down);
        BuildLeftConveyor(Orientation::up);
        ValidateResultOrientation(Orientation::right, proto::LineOrientation::down_right);
    }

    TEST_F(ConveyorCalcLineOrienTest, DownStraightRightHasRightAtLeftSide) {
        //
        //   v
        // > >
        //
        BuildTopConveyor(Orientation::down);
        BuildLeftConveyor(Orientation::right); // Points at center, center now straight
        ValidateResultOrientation(Orientation::right, proto::LineOrientation::right);
    }

    TEST_F(ConveyorCalcLineOrienTest, UpBendRightHasUpAbove) {
        //
        // ^
        // >
        // ^
        //
        BuildTopConveyor(Orientation::up);
        BuildBottomConveyor(Orientation::up);
        ValidateResultOrientation(Orientation::right, proto::LineOrientation::up_right);
    }

    // ======================================================================

    TEST_F(ConveyorCalcLineOrienTest, RightBendDown) {
        //
        // > v
        //
        BuildLeftConveyor(Orientation::right);
        ValidateResultOrientation(Orientation::down, proto::LineOrientation::right_down);
    }

    TEST_F(ConveyorCalcLineOrienTest, LeftBendDown) {
        //
        // v <
        //
        BuildRightConveyor(Orientation::left);
        ValidateResultOrientation(Orientation::down, proto::LineOrientation::left_down);
    }

    TEST_F(ConveyorCalcLineOrienTest, LeftRightStraightDown) {
        //
        // > v <
        //
        BuildLeftConveyor(Orientation::right);
        BuildRightConveyor(Orientation::left);
        ValidateResultOrientation(Orientation::down, proto::LineOrientation::down);
    }

    TEST_F(ConveyorCalcLineOrienTest, RightBendDownHasLeftAbove) {
        //
        //   <
        // > v
        //
        BuildLeftConveyor(Orientation::right);
        BuildTopConveyor(Orientation::left);
        ValidateResultOrientation(Orientation::down, proto::LineOrientation::right_down);
    }

    TEST_F(ConveyorCalcLineOrienTest, RightStraightDownHasDownAbove) {
        //
        //   v
        // > v
        //
        BuildLeftConveyor(Orientation::right);
        BuildTopConveyor(Orientation::down);
        ValidateResultOrientation(Orientation::down, proto::LineOrientation::down);
    }

    TEST_F(ConveyorCalcLineOrienTest, LeftBendDownHasLeftAtLeftSide) {
        //
        // < v <
        //
        BuildLeftConveyor(Orientation::left);
        BuildRightConveyor(Orientation::left);
        ValidateResultOrientation(Orientation::down, proto::LineOrientation::left_down);
    }

    // ======================================================================

    TEST_F(ConveyorCalcLineOrienTest, DownBendLeft) {
        //
        // v
        // <
        //
        BuildTopConveyor(Orientation::down);
        ValidateResultOrientation(Orientation::left, proto::LineOrientation::down_left);
    }

    TEST_F(ConveyorCalcLineOrienTest, UpBendLeft) {
        //
        // <
        // ^
        //
        BuildBottomConveyor(Orientation::up);
        ValidateResultOrientation(Orientation::left, proto::LineOrientation::up_left);
    }

    TEST_F(ConveyorCalcLineOrienTest, UpDownStraightLeft) {
        //
        // v
        // <
        // ^
        //
        BuildTopConveyor(Orientation::down);
        BuildBottomConveyor(Orientation::up);
        ValidateResultOrientation(Orientation::left, proto::LineOrientation::left);
    }

    TEST_F(ConveyorCalcLineOrienTest, DownBendLeftHasUpRightSide) {
        //
        // v
        // < ^
        //
        BuildTopConveyor(Orientation::down);
        BuildRightConveyor(Orientation::up);
        ValidateResultOrientation(Orientation::left, proto::LineOrientation::down_left);
    }

    TEST_F(ConveyorCalcLineOrienTest, DownStraightLeftHasLeftRightSide) {
        //
        // v
        // < <
        //
        BuildTopConveyor(Orientation::down);
        BuildRightConveyor(Orientation::left);
        ValidateResultOrientation(Orientation::left, proto::LineOrientation::left);
    }

    TEST_F(ConveyorCalcLineOrienTest, UpBendLeftHasUpAbove) {
        //
        // ^
        // <
        // ^
        //
        BuildTopConveyor(Orientation::up);
        BuildBottomConveyor(Orientation::up);
        ValidateResultOrientation(Orientation::left, proto::LineOrientation::up_left);
    }


    /// Should change line orientation of right neighbor from {0, 0} to up_right (Aesthetics only)
    TEST_F(ConveyorUtilityTest, UpdateNeighborLineOrientation) {
        //
        // x > >
        //   ^
        //

        auto& con_data_r_head = TestSetupConveyor(world_, {2, 0}, Orientation::right, transBelt_);
        auto& con_data_r_end  = TestSetupConveyor(world_, {1, 0}, transBelt_, con_data_r_head.structure);

        TestSetupConveyor(world_, {1, 1}, Orientation::up, transBelt_);

        ConveyorUpdateNeighborLineOrien(world_, {0, 0});

        EXPECT_EQ(con_data_r_end.lOrien, proto::LineOrientation::up_right);
    }
} // namespace jactorio::game