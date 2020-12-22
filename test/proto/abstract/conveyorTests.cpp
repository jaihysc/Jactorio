// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "proto/transport_belt.h"

#include "jactorioTests.h"

#include "game/logic/conveyor_utility.h"

// ======================================================================
// Tests for the various bend orientations

namespace jactorio::proto
{
    // General tests
    // Neighbor updates
    // Bends
    // Side only

    class ConveyorTest : public testing::Test
    {
    protected:
        game::WorldData worldData_;
        game::LogicData logicData_;

        TransportBelt lineProto_;

        void SetUp() override {
            worldData_.EmplaceChunk({0, 0});
        }

        // ======================================================================

        static game::ConveyorStruct& GetSegment(game::ChunkTileLayer* tile_layer) {
            return *tile_layer->GetUniqueData<ConveyorData>()->structure;
        }


        ///
        /// Sets the prototype pointer for a conveyor at tile
        game::ChunkTileLayer& BuildConveyor(const WorldCoord world_coords, const Orientation orientation) {
            auto& layer = worldData_.GetTile(world_coords.x, world_coords.y)->GetLayer(game::TileLayer::entity);

            layer.prototypeData = &lineProto_;
            TlBuildEvents(world_coords, orientation);

            return layer;
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

        ///
        /// Dispatches the appropriate events for when a conveyor is built
        void TlBuildEvents(const WorldCoord& world_coords, const Orientation orientation) {
            auto& layer = worldData_.GetTile(world_coords)->GetLayer(game::TileLayer::entity);

            lineProto_.OnBuild(worldData_, logicData_, world_coords, layer, orientation);

            // Call on_neighbor_update for the 4 sides
            DispatchNeighborUpdate(world_coords, {world_coords.x, world_coords.y - 1}, Orientation::up);

            DispatchNeighborUpdate(world_coords, {world_coords.x + 1, world_coords.y}, Orientation::right);

            DispatchNeighborUpdate(world_coords, {world_coords.x, world_coords.y + 1}, Orientation::down);

            DispatchNeighborUpdate(world_coords, {world_coords.x - 1, world_coords.y}, Orientation::left);
        }

        ///
        /// Dispatches the appropriate events AFTER a conveyor is removed
        void TlRemoveEvents(const WorldCoord& world_coords) {

            auto& layer = worldData_.GetTile(world_coords)->GetLayer(game::TileLayer::entity);

            lineProto_.OnRemove(worldData_, logicData_, world_coords, layer);

            // Call on_neighbor_update for the 4 sides
            DispatchNeighborUpdate(world_coords, {world_coords.x, world_coords.y - 1}, Orientation::up);

            DispatchNeighborUpdate(world_coords, {world_coords.x + 1, world_coords.y}, Orientation::right);

            DispatchNeighborUpdate(world_coords, {world_coords.x, world_coords.y + 1}, Orientation::down);

            DispatchNeighborUpdate(world_coords, {world_coords.x - 1, world_coords.y}, Orientation::left);
        }

        // Bend

        ///
        /// \param l_index index for left only segment in logic group
        /// \param r_index index for right only segment in logic group
        void ValidateBendToSideOnly(const size_t l_index = 2, const size_t r_index = 1) {
            game::Chunk& chunk = *worldData_.GetChunkC(0, 0);
            auto& logic_group  = chunk.GetLogicGroup(game::Chunk::LogicGroup::conveyor);

            ASSERT_EQ(logic_group.size(), 3);

            {
                auto& line_segment = GetSegment(logic_group[r_index]);
                EXPECT_EQ(line_segment.terminationType, game::ConveyorStruct::TerminationType::right_only);
                EXPECT_EQ(line_segment.length, 2);
            }
            {
                auto& line_segment = GetSegment(logic_group[l_index]);
                EXPECT_EQ(line_segment.terminationType, game::ConveyorStruct::TerminationType::left_only);
                EXPECT_EQ(line_segment.length, 2);
            }
        }

        // Grouping

        std::vector<game::ChunkTileLayer*>& GetConveyors(const ChunkCoord& chunk_coords) {
            return worldData_.GetChunkC(chunk_coords.x, chunk_coords.y)
                ->GetLogicGroup(game::Chunk::LogicGroup::conveyor);
        }

        J_NODISCARD auto& GetConveyorData(const WorldCoord& world_coords) {
            return *GetConData(worldData_, world_coords);
        }

        auto GetStructIndex(const WorldCoord& world_coords) {
            return GetConveyorData(world_coords).structIndex;
        }

    private:
        void DispatchNeighborUpdate(const WorldCoord& emit_coords,
                                    const WorldCoord& receive_coords,
                                    const Orientation emit_orientation) {

            auto* tile = worldData_.GetTile(receive_coords);
            if (tile == nullptr)
                return;

            auto& layer = tile->GetLayer(game::TileLayer::entity);
            if (layer.prototypeData.Get() == nullptr)
                return;

            static_cast<const Entity*>(layer.prototypeData.Get())
                ->OnNeighborUpdate(worldData_, logicData_, emit_coords, receive_coords, emit_orientation);
        }
    };


    // ======================================================================
    // General tests

    TEST_F(ConveyorTest, OnBuildCreateConveyorSegment) {
        // Should create a conveyor segment and add its chunk to logic chunks
        worldData_.EmplaceChunk({-1, 0});

        auto& layer         = worldData_.GetTile(-5, 0)->GetLayer(game::TileLayer::entity);
        layer.prototypeData = &lineProto_;

        TlBuildEvents({-5, 0}, Orientation::right);

        // ======================================================================

        // Added current chunk as a logic chunk
        ASSERT_EQ(worldData_.LogicGetChunks().size(), 1);

        auto& tile_layers = GetConveyors({-1, 0});

        // Should have created a conveyor structure
        ASSERT_EQ(tile_layers.size(), 1);
        ASSERT_TRUE(dynamic_cast<ConveyorData*>(tile_layers.front()->GetUniqueData()));

        auto& line_segment = GetSegment(tile_layers[0]);
        EXPECT_EQ(line_segment.direction, Orientation::right);
        EXPECT_EQ(line_segment.terminationType, game::ConveyorStruct::TerminationType::straight);
        EXPECT_EQ(line_segment.length, 1);
    }

    TEST_F(ConveyorTest, OnRemoveDeleteConveyorSegment) {
        // Removing a conveyor needs to delete the conveyor segment associated with it
        BuildConveyor({0, 0}, Orientation::left);

        TlRemoveEvents({0, 0});

        // Conveyor structure count should be 0 as it was removed
        EXPECT_TRUE(worldData_.GetChunkC({0, 0})->GetLogicGroup(game::Chunk::LogicGroup::conveyor).empty());
    }

    TEST_F(ConveyorTest, OnDeserializeRelinkTarget) {
        // In this configuration, segment at {0, 1} will not group with the center one
        BuildRightConveyor(Orientation::right);
        const auto& line_left = BuildLeftConveyor(Orientation::right);

        BuildTopConveyor(Orientation::up);
        BuildBottomConveyor(Orientation::down);

        const auto& center_line    = BuildConveyor({1, 1}, Orientation::right);
        const auto& center_segment = center_line.GetUniqueData<ConveyorData>()->structure;

        const auto& left_segment = line_left.GetUniqueData<ConveyorData>()->structure;
        left_segment->target     = nullptr;

        // Re links target segment
        worldData_.DeserializePostProcess();

        EXPECT_EQ(left_segment->target, center_segment.get());
    }

    // ======================================================================
    // Neighbor updates

    TEST_F(ConveyorTest, OnBuildNeighborBend) {
        // Removing a conveyor must update neighboring conveyor segments
        // The bottom segment goes from right_only to bend_right

        /*
         *  > > >
         *    ^
         */
        /* Order:
         * 4 3 2
         *   1
         */
        BuildConveyor({1, 1}, Orientation::up);

        BuildConveyor({2, 0}, Orientation::right);
        BuildConveyor({1, 0}, Orientation::right);
        BuildConveyor({0, 0}, Orientation::right);

        auto& tile_layers = GetConveyors({0, 0});

        ASSERT_EQ(tile_layers.size(), 2);

        auto& line_segment = GetSegment(tile_layers[0]);
        EXPECT_EQ(line_segment.terminationType, game::ConveyorStruct::TerminationType::right_only);
        EXPECT_EQ(line_segment.length, 2);
    }

    TEST_F(ConveyorTest, OnRemoveNeighborBend) {
        // Removing a conveyor must update neighboring conveyor segments
        // The bottom segment goes from right_only to bend_right

        /*
         * /> > >
         *    ^
         */
        BuildConveyor({0, 0}, Orientation::right);
        BuildConveyor({1, 0}, Orientation::right);
        BuildConveyor({2, 0}, Orientation::right);

        BuildConveyor({1, 1}, Orientation::up);


        TlRemoveEvents({0, 0});

        auto& tile_layers = GetConveyors({0, 0});

        ASSERT_EQ(tile_layers.size(), 2);

        auto& line_segment = GetSegment(tile_layers[1]);
        EXPECT_EQ(line_segment.terminationType, game::ConveyorStruct::TerminationType::bend_right);
        EXPECT_EQ(line_segment.length, 2);
    }

    TEST_F(ConveyorTest, OnBuildUpdateNeighboringLines) {

        /*
         * >
         * ^
         */
        BuildTopConveyor(Orientation::right);

        auto& layer = worldData_.GetTile(1, 1)->GetLayer(game::TileLayer::entity);


        TransportBelt proto;
        layer.prototypeData = &proto;


        // Should update line above, turn right to a up-right
        TlBuildEvents({1, 1}, Orientation::up);

        {
            auto& result_layer = worldData_.GetTile(1, 0)->GetLayer(game::TileLayer::entity);

            EXPECT_EQ(static_cast<ConveyorData*>(result_layer.GetUniqueData())->lOrien, LineOrientation::up_right);
        }
    }

    TEST_F(ConveyorTest, OnRemoveUpdateNeighboringLines) {
        // The on_remove event should update the orientations of the neighboring belts to if the current conveyor
        // line is not there

        /*
         *  v
         *  >
         *  ^
         */
        BuildTopConveyor(Orientation::down);
        BuildBottomConveyor(Orientation::up);
        BuildConveyor({1, 1}, Orientation::right); // Between the 2 above and below

        auto& layer         = worldData_.GetTile(1, 2)->GetLayer(game::TileLayer::entity);
        layer.prototypeData = &lineProto_;


        // Removing the bottom line makes the center one bend down-right
        TlRemoveEvents({1, 2});

        {
            auto& result_layer = worldData_.GetTile(1, 1)->GetLayer(game::TileLayer::entity);

            EXPECT_EQ(static_cast<ConveyorData*>(result_layer.GetUniqueData())->lOrien, LineOrientation::down_right);
        }
    }

    // ======================================================================
    // Bends

    // Various custom arrangements of conveyors
    TEST_F(ConveyorTest, OnBuildBendingConveyorSegmentTrailing) {
        // Change the conveyor_struct termination type in accordance with orientation when placed behind existing
        // line


        auto& down_layer         = worldData_.GetTile(0, 0)->GetLayer(game::TileLayer::entity);
        down_layer.prototypeData = &lineProto_;
        TlBuildEvents({0, 0}, Orientation::down);

        auto& left_layer         = worldData_.GetTile(1, 0)->GetLayer(game::TileLayer::entity);
        left_layer.prototypeData = &lineProto_;
        TlBuildEvents({1, 0}, Orientation::left);

        auto& tile_layers = GetConveyors({0, 0});

        ASSERT_EQ(tile_layers.size(), 2);

        auto& segment = GetSegment(tile_layers[1]);
        EXPECT_EQ(segment.terminationType, game::ConveyorStruct::TerminationType::bend_left);

        // Should have lengthened segment and moved x 1 left
        EXPECT_EQ(segment.length, 2);
        EXPECT_EQ(static_cast<ConveyorData*>(left_layer.GetUniqueData())->structIndex, 1);
    }

    TEST_F(ConveyorTest, OnBuildBendingConveyorSegmentLeading) {
        // Change the conveyor_struct termination type in accordance with orientation when placed ahead of
        // existing line

        auto& left_layer         = worldData_.GetTile(1, 0)->GetLayer(game::TileLayer::entity);
        left_layer.prototypeData = &lineProto_;
        TlBuildEvents({1, 0}, Orientation::left);

        auto& down_layer         = worldData_.GetTile(0, 0)->GetLayer(game::TileLayer::entity);
        down_layer.prototypeData = &lineProto_;
        TlBuildEvents({0, 0}, Orientation::down);

        auto& tile_layers = GetConveyors({0, 0});

        ASSERT_EQ(tile_layers.size(), 2);

        auto& segment = GetSegment(tile_layers[0]);
        EXPECT_EQ(segment.terminationType, game::ConveyorStruct::TerminationType::bend_left);

        // Should have lengthened segment and moved x 1 left
        EXPECT_EQ(segment.length, 2);
        EXPECT_EQ(static_cast<ConveyorData*>(left_layer.GetUniqueData())->structIndex, 1);
    }


    TEST_F(ConveyorTest, OnRemoveBend) {
        // After removing a conveyor the conveyor that connects to it with a bend must become straight,
        // decrement segment_length by 1, and shift its position

        /*
         * v
         * >
         */
        BuildConveyor({0, 0}, Orientation::down);
        BuildConveyor({0, 1}, Orientation::right);


        TlRemoveEvents({0, 1});

        auto& tile_layers = GetConveyors({0, 0});

        ASSERT_EQ(tile_layers.size(), 1);

        auto& segment = GetSegment(tile_layers[0]);
        EXPECT_EQ(segment.terminationType, game::ConveyorStruct::TerminationType::straight);
        EXPECT_EQ(segment.length, 1);
    }

    TEST_F(ConveyorTest, SetOrientation) {
        // When the orientation is set, the member "set" should also be updated

        // Arbitrary segment is fine since no logic updates are performed
        const auto segment = std::make_shared<game::ConveyorStruct>(
            Orientation::left, game::ConveyorStruct::TerminationType::straight, 1);

        ConveyorData line_data{segment};
        line_data.structIndex = 1; // Prevents it from attempting to delete line segment

        line_data.SetOrientation(LineOrientation::down);
        EXPECT_EQ(line_data.set, static_cast<uint16_t>(LineOrientation::down));

        line_data.SetOrientation(LineOrientation::left_down);
        EXPECT_EQ(line_data.set, static_cast<uint16_t>(LineOrientation::left_down));
    }


    // ======================================================================
    // Side only

    // Change bend to side only as initially, it forms a bend without the line on top / bottom
    TEST_F(ConveyorTest, OnBuildUpChangeBendToSideOnly) {

        /*
         *   ^
         * > ^ <
         */
        BuildConveyor({1, 0}, Orientation::up);
        BuildConveyor({1, 1}, Orientation::up);

        BuildConveyor({2, 1}, Orientation::left);
        BuildConveyor({0, 1}, Orientation::right);

        ValidateBendToSideOnly();
        EXPECT_EQ(GetStructIndex({0, 1}), 1);
        EXPECT_EQ(GetStructIndex({2, 1}), 1);

        EXPECT_EQ(GetConveyorData({0, 1}).structure->targetInsertOffset, 1);
        EXPECT_EQ(GetConveyorData({2, 1}).structure->targetInsertOffset, 1);

        // Incremented 1 forwards
        EXPECT_EQ(GetConveyorData({0, 1}).structure->itemOffset, 1);
        EXPECT_EQ(GetConveyorData({2, 1}).structure->itemOffset, 1);
    }

    TEST_F(ConveyorTest, OnBuildRightChangeBendToSideOnly) {

        /*
         *   v
         *   > >
         *   ^
         */
        BuildConveyor({1, 2}, Orientation::up);
        BuildConveyor({1, 0}, Orientation::down);

        BuildConveyor({1, 1}, Orientation::right);
        BuildConveyor({2, 1}, Orientation::right);


        ValidateBendToSideOnly(1, 0);
        EXPECT_EQ(GetConveyorData({1, 0}).structure->targetInsertOffset, 0);
        EXPECT_EQ(GetConveyorData({1, 2}).structure->targetInsertOffset, 0);

        EXPECT_EQ(GetConveyorData({1, 0}).structure->itemOffset, 1);
        EXPECT_EQ(GetConveyorData({1, 2}).structure->itemOffset, 1);
    }

    TEST_F(ConveyorTest, OnBuildDownChangeBendToSideOnly) {

        /*
         * > v <
         *   v
         */
        BuildConveyor({1, 1}, Orientation::down);
        BuildConveyor({1, 2}, Orientation::down);

        BuildConveyor({0, 1}, Orientation::right);
        BuildConveyor({2, 1}, Orientation::left);

        ValidateBendToSideOnly();
        EXPECT_EQ(GetConveyorData({0, 1}).structure->targetInsertOffset, 0);
        EXPECT_EQ(GetConveyorData({2, 1}).structure->targetInsertOffset, 0);
    }

    TEST_F(ConveyorTest, OnBuildLeftChangeBendToSideOnly) {

        /*
         *   v
         * < <
         *   ^
         */
        BuildConveyor({0, 1}, Orientation::left);
        BuildConveyor({1, 1}, Orientation::left);

        BuildConveyor({1, 0}, Orientation::down);
        BuildConveyor({1, 2}, Orientation::up);

        ValidateBendToSideOnly();
        EXPECT_EQ(GetConveyorData({1, 0}).structure->targetInsertOffset, 1);
        EXPECT_EQ(GetConveyorData({1, 2}).structure->targetInsertOffset, 1);
    }

    TEST_F(ConveyorTest, OnBuildUpUpdateNeighboringSegmentToSideOnly) {
        /*
         *   ^
         *   ^
         * > ^ <
         */
        /* Order:
         *
         *   3
         *   4
         * 1 5 2
         */
        BuildConveyor({0, 2}, Orientation::right);
        BuildConveyor({2, 2}, Orientation::left);

        BuildConveyor({1, 0}, Orientation::up);
        BuildConveyor({1, 1}, Orientation::up);
        BuildConveyor({1, 2}, Orientation::up);

        auto& tile_layers = GetConveyors({0, 0});

        ASSERT_EQ(tile_layers.size(), 3);

        {
            auto& line_segment = GetSegment(tile_layers[0]);
            EXPECT_EQ(line_segment.terminationType, game::ConveyorStruct::TerminationType::left_only);
        }
        {
            auto& line_segment = GetSegment(tile_layers[1]);
            EXPECT_EQ(line_segment.terminationType, game::ConveyorStruct::TerminationType::right_only);
        }

        EXPECT_EQ(GetStructIndex({0, 2}), 1);
        EXPECT_EQ(GetStructIndex({2, 2}), 1);

        EXPECT_EQ(GetConveyorData({0, 2}).structure->targetInsertOffset, 2);
        EXPECT_EQ(GetConveyorData({2, 2}).structure->targetInsertOffset, 2);
    }

    TEST_F(ConveyorTest, OnBuildRightUpdateNeighboringSegmentToSideOnly) {
        // Line 2 should change to right_only

        /*  v
         *  >
         *  ^
         */
        /* Order:
         *  2
         *  3
         *  1
         */
        BuildConveyor({1, 2}, Orientation::up);
        BuildConveyor({1, 0}, Orientation::down);

        BuildConveyor({1, 1}, Orientation::right);

        auto& tile_layers = GetConveyors({0, 0});

        ASSERT_EQ(tile_layers.size(), 3);

        {
            auto& line_segment = GetSegment(tile_layers[0]);
            EXPECT_EQ(line_segment.terminationType, game::ConveyorStruct::TerminationType::right_only);
        }
        {
            auto& line_segment = GetSegment(tile_layers[1]);
            EXPECT_EQ(line_segment.terminationType, game::ConveyorStruct::TerminationType::left_only);
        }

        EXPECT_EQ(GetStructIndex({1, 2}), 1);
        EXPECT_EQ(GetStructIndex({1, 0}), 1);
    }

    TEST_F(ConveyorTest, OnBuildDownUpdateNeighboringSegmentToSideOnly) {

        /*
         * > v <
         */
        /* Order:
         *
         * 1 3 2
         */
        BuildConveyor({0, 0}, Orientation::right);
        BuildConveyor({2, 0}, Orientation::left);

        BuildConveyor({1, 0}, Orientation::down);

        auto& tile_layers = GetConveyors({0, 0});

        ASSERT_EQ(tile_layers.size(), 3);

        {
            auto& line_segment = GetSegment(tile_layers[0]);
            EXPECT_EQ(line_segment.terminationType, game::ConveyorStruct::TerminationType::right_only);
            EXPECT_EQ(line_segment.itemOffset, 1); // Incremented 1 when turned side only
        }
        {
            auto& line_segment = GetSegment(tile_layers[1]);
            EXPECT_EQ(line_segment.terminationType, game::ConveyorStruct::TerminationType::left_only);
            EXPECT_EQ(line_segment.itemOffset, 1);
        }

        EXPECT_EQ(GetStructIndex({0, 0}), 1);
        EXPECT_EQ(GetStructIndex({2, 0}), 1);
    }

    TEST_F(ConveyorTest, OnBuildLeftUpdateNeighboringSegmentToSideOnly) {
        // Line 2 should change to right_only

        /*   v
         *   <
         *   ^
         */
        /* Order:
         *   1
         *   3
         *   2
         */
        BuildConveyor({0, 0}, Orientation::down);
        BuildConveyor({0, 2}, Orientation::up);

        BuildConveyor({0, 1}, Orientation::left);

        auto& tile_layers = GetConveyors({0, 0});

        ASSERT_EQ(tile_layers.size(), 3);

        {
            auto& line_segment = GetSegment(tile_layers[0]);
            EXPECT_EQ(line_segment.terminationType, game::ConveyorStruct::TerminationType::right_only);
        }
        {
            auto& line_segment = GetSegment(tile_layers[1]);
            EXPECT_EQ(line_segment.terminationType, game::ConveyorStruct::TerminationType::left_only);
        }

        EXPECT_EQ(GetStructIndex({0, 0}), 1);
        EXPECT_EQ(GetStructIndex({0, 2}), 1);
    }

    // ======================================================================
    // Various custom arrangements of conveyors

    TEST_F(ConveyorTest, ConveyorCircle) {
        // Creates a circle of conveyors

        /*
         * > v
         * ^ <
         */

        {
            auto& layer         = worldData_.GetTile(0, 0)->GetLayer(game::TileLayer::entity);
            layer.prototypeData = &lineProto_;
            TlBuildEvents({0, 0}, Orientation::right);
        }
        {
            auto& layer         = worldData_.GetTile(1, 0)->GetLayer(game::TileLayer::entity);
            layer.prototypeData = &lineProto_;
            TlBuildEvents({1, 0}, Orientation::down);
        }
        {
            auto& layer         = worldData_.GetTile(1, 1)->GetLayer(game::TileLayer::entity);
            layer.prototypeData = &lineProto_;
            TlBuildEvents({1, 1}, Orientation::left);
        }
        {
            auto& layer         = worldData_.GetTile(0, 1)->GetLayer(game::TileLayer::entity);
            layer.prototypeData = &lineProto_;
            TlBuildEvents({0, 1}, Orientation::up);
        }


        auto& tile_layers = GetConveyors({0, 0});
        ASSERT_EQ(tile_layers.size(), 4);

        // Right
        {
            auto& line_segment = GetSegment(tile_layers[0]);
            EXPECT_EQ(line_segment.terminationType, game::ConveyorStruct::TerminationType::bend_right);
            EXPECT_EQ(line_segment.length, 2);
        }
        // Down
        {
            auto& line_segment = GetSegment(tile_layers[1]);
            EXPECT_EQ(line_segment.terminationType, game::ConveyorStruct::TerminationType::bend_right);
            EXPECT_EQ(line_segment.length, 2);
        }

        // Left
        {
            auto& line_segment = GetSegment(tile_layers[2]);
            EXPECT_EQ(line_segment.terminationType, game::ConveyorStruct::TerminationType::bend_right);
            EXPECT_EQ(line_segment.length, 2);
        }

        // Up
        {
            auto& line_segment = GetSegment(tile_layers[3]);
            EXPECT_EQ(line_segment.terminationType, game::ConveyorStruct::TerminationType::bend_right);
            EXPECT_EQ(line_segment.length, 2);
        }
    }
} // namespace jactorio::proto
