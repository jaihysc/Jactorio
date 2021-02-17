// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "game/logic/conveyor_controller.h"

#include <memory>

#include "jactorioTests.h"

#include "proto/transport_belt.h"

namespace jactorio::game
{
    // Tests:
    // - Line logic
    // - Line properties
    // - Item transition (to another segment)


    class ConveyorControllerTest : public testing::Test
    {
    protected:
        World world_;
        Logic logic_;

        Chunk* chunk_ = nullptr;

        proto::Item itemProto_{};
        const std::unique_ptr<proto::TransportBelt> transportBeltProto_ = std::make_unique<proto::TransportBelt>();

        ///
        /// Creates a world, chunk and logic chunk at 0, 0
        void SetUp() override {
            chunk_ = &world_.EmplaceChunk({0, 0});
            world_.LogicAddChunk(*chunk_);
        }

        void CreateSegment(const WorldCoord& coord, const std::shared_ptr<ConveyorStruct>& segment) {
            TestCreateConveyorSegment(world_, coord, segment, *transportBeltProto_);
        }
    };

    TEST_F(ConveyorControllerTest, LineLogic) {
        // Same as line logic, but belts are faster (0.06), which seems to break the current logic at the time of
        // writing
        const auto j_belt_speed = 0.06;

        transportBeltProto_->speed = j_belt_speed; // <---

        // Segments (Logic chunk must be created first)
        auto up_segment =
            std::make_shared<ConveyorStruct>(Orientation::up, ConveyorStruct::TerminationType::bend_right, 5);
        const auto right_segment =
            std::make_shared<ConveyorStruct>(Orientation::right, ConveyorStruct::TerminationType::bend_right, 5);
        const auto down_segment =
            std::make_shared<ConveyorStruct>(Orientation::down, ConveyorStruct::TerminationType::bend_right, 5);
        auto left_segment =
            std::make_shared<ConveyorStruct>(Orientation::left, ConveyorStruct::TerminationType::bend_right, 5);

        up_segment->target    = right_segment.get();
        right_segment->target = down_segment.get();
        down_segment->target  = left_segment.get();
        left_segment->target  = up_segment.get();

        CreateSegment({0, 0}, up_segment);
        CreateSegment({4, 0}, right_segment);
        CreateSegment({4, 5}, down_segment);
        CreateSegment({0, 5}, left_segment);

        // Logic
        left_segment->AppendItem(true, 0.f, itemProto_);
        left_segment->AppendItem(true, ConveyorProp::kItemSpacing, itemProto_);
        left_segment->AppendItem(true, ConveyorProp::kItemSpacing, itemProto_);

        // 1 update
        // first item moved to up segment
        ConveyorLogicUpdate(world_);
        ASSERT_EQ(up_segment->left.lane.size(), 1);
        ASSERT_EQ(left_segment->left.lane.size(), 2);

        EXPECT_DOUBLE_EQ(up_segment->left.lane[0].dist.getAsDouble(), 4.40 - j_belt_speed);
        EXPECT_DOUBLE_EQ(left_segment->left.lane[0].dist.getAsDouble(), 0.25 - j_belt_speed);
        EXPECT_DOUBLE_EQ(left_segment->left.lane[1].dist.getAsDouble(), 0.25);

        // 2 updates | 0.12
        for (int i = 0; i < 2; ++i) {
            ConveyorLogicUpdate(world_);
        }
        ASSERT_EQ(up_segment->left.lane.size(), 1);
        ASSERT_EQ(left_segment->left.lane.size(), 2);

        EXPECT_DOUBLE_EQ(up_segment->left.lane[0].dist.getAsDouble(), 4.40 - (3 * j_belt_speed));
        EXPECT_DOUBLE_EQ(left_segment->left.lane[0].dist.getAsDouble(), 0.25 - (3 * j_belt_speed));
        EXPECT_DOUBLE_EQ(left_segment->left.lane[1].dist.getAsDouble(), 0.25);


        // 2 updates | Total distance = 4(0.06) = 0.24
        // second item moved to up segment
        for (int i = 0; i < 2; ++i) {
            ConveyorLogicUpdate(world_);
        }
        ASSERT_EQ(up_segment->left.lane.size(), 2);
        ASSERT_EQ(left_segment->left.lane.size(), 1);

        EXPECT_DOUBLE_EQ(up_segment->left.lane[0].dist.getAsDouble(), 4.40 - (5 * j_belt_speed));
        EXPECT_DOUBLE_EQ(up_segment->left.lane[1].dist.getAsDouble(), 0.25); // Spacing maintained
        // Item 2 was 0.01 -> -0.05
        // | -0.05 - 0.20 | = 0.25 Maintains distance
        EXPECT_DOUBLE_EQ(left_segment->left.lane[0].dist.getAsDouble(), 0.20);
    }


    TEST_F(ConveyorControllerTest, LineLogicRightBend) {
        // Validates the correct handling of multiple items across conveyors
        // The spacing between items should be maintained

        transportBeltProto_->speed = 0.01f;

        /*
         *    --------- RIGHT -------- >
         *    ^
         *    |
         *    | UP
         *    |
         *    |
         */

        auto up_segment =
            std::make_shared<ConveyorStruct>(Orientation::up, ConveyorStruct::TerminationType::bend_right, 4);
        auto right_segment =
            std::make_shared<ConveyorStruct>(Orientation::right, ConveyorStruct::TerminationType::straight, 4);

        up_segment->target = right_segment.get();

        CreateSegment({0, 0}, up_segment);
        CreateSegment({3, 0}, right_segment);

        // Offset is distance from beginning, or previous item
        up_segment->AppendItem(true, 0.f, itemProto_);
        up_segment->AppendItem(true, 1, itemProto_);
        up_segment->AppendItem(true, 1, itemProto_);
        static_assert(ConveyorProp::kItemSpacing < 1); // Tested positions would otherwise be invalid

        // Logic
        // Should transfer the first item
        ConveyorLogicUpdate(world_);


        EXPECT_EQ(up_segment->left.lane.size(), 2);
        EXPECT_DOUBLE_EQ(up_segment->left.lane[0].dist.getAsDouble(), 0.99);
        EXPECT_DOUBLE_EQ(up_segment->left.lane[1].dist.getAsDouble(), 1.);

        EXPECT_EQ(right_segment->left.lane.size(), 1);
        // Moved forward once 4 - 0.3 - 0.01
        EXPECT_DOUBLE_EQ(right_segment->left.lane[0].dist.getAsDouble(), 3.69);

        // Transfer second item after (1 / 0.01) + 1 update - 1 update (Already moved once above)
        for (int i = 0; i < 100; ++i) {
            ConveyorLogicUpdate(world_);
        }

        EXPECT_EQ(up_segment->left.lane.size(), 1);
        EXPECT_EQ(right_segment->left.lane.size(), 2);
        // Spacing of 1 tile between the items is maintained across belts
        EXPECT_DOUBLE_EQ(right_segment->left.lane[0].dist.getAsDouble(), 2.69);
        EXPECT_DOUBLE_EQ(right_segment->left.lane[1].dist.getAsDouble(), 1);


        // Third item
        for (int i = 0; i < 100; ++i) {
            ConveyorLogicUpdate(world_);
        }
        EXPECT_EQ(up_segment->left.lane.size(), 0);
        EXPECT_EQ(right_segment->left.lane.size(), 3);

        EXPECT_DOUBLE_EQ(right_segment->left.lane[0].dist.getAsDouble(), 1.69);
        EXPECT_DOUBLE_EQ(right_segment->left.lane[1].dist.getAsDouble(), 1);
        EXPECT_DOUBLE_EQ(right_segment->left.lane[2].dist.getAsDouble(), 1);
    }

    TEST_F(ConveyorControllerTest, LineLogicCompressedRightBend) {
        // Same as line_logic_right_bend, but items are compressed

        transportBeltProto_->speed = 0.01f;

        /*
         * COMPRESSED
         *    --------- RIGHT -------- >
         *    ^
         *    |
         *    | UP
         *    |
         *    |
         */

        auto up_segment =
            std::make_shared<ConveyorStruct>(Orientation::up, ConveyorStruct::TerminationType::bend_right, 4);
        auto right_segment =
            std::make_shared<ConveyorStruct>(Orientation::right, ConveyorStruct::TerminationType::straight, 4);

        up_segment->target = right_segment.get();

        CreateSegment({0, 0}, up_segment);
        CreateSegment({3, 0}, right_segment);

        // Offset is distance from beginning, or previous item
        up_segment->AppendItem(true, 0.f, itemProto_);
        up_segment->AppendItem(true, ConveyorProp::kItemSpacing, itemProto_);

        // First item
        ConveyorLogicUpdate(world_);


        EXPECT_EQ(up_segment->left.lane.size(), 1);
        EXPECT_DOUBLE_EQ(up_segment->left.lane[0].dist.getAsDouble(), 0.24);

        EXPECT_EQ(right_segment->left.lane.size(), 1);
        // Moved forward once 4 - 0.3 - 0.01
        EXPECT_DOUBLE_EQ(right_segment->left.lane[0].dist.getAsDouble(), 3.69);


        // Transfer second item after (0.25 / 0.01) + 1 update - 1 update (Already moved once above)
        for (int i = 0; i < 25; ++i) {
            ConveyorLogicUpdate(world_);
        }

        EXPECT_EQ(up_segment->left.lane.size(), 0);
        EXPECT_EQ(right_segment->left.lane.size(), 2);
        // Spacing is maintained across belts
        EXPECT_DOUBLE_EQ(right_segment->left.lane[0].dist.getAsDouble(), 3.44);
        EXPECT_DOUBLE_EQ(right_segment->left.lane[1].dist.getAsDouble(), 0.25);
    }

    TEST_F(ConveyorControllerTest, LineLogicStopAtEndOfLine) {
        // When no target_segment is provided:
        // First Item will stop at the end of line (Distance is 0)
        // Trailing items will stop at item_width from the previous item

        transportBeltProto_->speed = 0.01f;

        auto segment =
            std::make_shared<ConveyorStruct>(Orientation::left, ConveyorStruct::TerminationType::straight, 10);

        CreateSegment({0, 0}, segment);

        segment->AppendItem(true, 0.5f, itemProto_);
        segment->AppendItem(true, ConveyorProp::kItemSpacing, itemProto_);
        segment->AppendItem(true, ConveyorProp::kItemSpacing + 1.f, itemProto_);

        // Will reach distance 0 after 0.5 / 0.01 updates
        for (int i = 0; i < 50; ++i) {
            ConveyorLogicUpdate(world_);
        }

        EXPECT_EQ(segment->left.index, 0);
        EXPECT_DOUBLE_EQ(segment->left.lane[0].dist.getAsDouble(), 0);

        // On the next update, with no target segment, first item is kept at 0, second item untouched
        // move index to 2 (was 0) as it has a distance greater than item_width
        ConveyorLogicUpdate(world_);


        EXPECT_EQ(segment->left.index, 2);
        EXPECT_DOUBLE_EQ(segment->left.lane[0].dist.getAsDouble(), 0);
        EXPECT_DOUBLE_EQ(segment->left.lane[1].dist.getAsDouble(), ConveyorProp::kItemSpacing);
        EXPECT_DOUBLE_EQ(segment->left.lane[2].dist.getAsDouble(), ConveyorProp::kItemSpacing + 0.99);

        // After 0.2 + 0.99 / 0.01 updates, the Third item will not move in following updates
        for (int j = 0; j < 99; ++j) {
            ConveyorLogicUpdate(world_);
        }
        EXPECT_DOUBLE_EQ(segment->left.lane[2].dist.getAsDouble(), ConveyorProp::kItemSpacing);

        ConveyorLogicUpdate(world_);

        // Index set to 0, checking if a valid target exists to move items forward
        EXPECT_EQ(segment->left.index, 0);

        EXPECT_DOUBLE_EQ(segment->left.lane[2].dist.getAsDouble(), ConveyorProp::kItemSpacing);


        // Updates do nothing since all items are compressed
        for (int k = 0; k < 50; ++k) {
            ConveyorLogicUpdate(world_);
        }
    }

    TEST_F(ConveyorControllerTest, LineLogicStopAtFilledTargetSegment) {
        // For the right lane:

        transportBeltProto_->speed = 0.01f;

        /*
         *    --------- RIGHT -------- >
         *    ^
         *    |
         *    | UP
         *    |
         *    |
         */

        auto up_segment =
            std::make_shared<ConveyorStruct>(Orientation::up, ConveyorStruct::TerminationType::bend_right, 4);
        auto right_segment =
            std::make_shared<ConveyorStruct>(Orientation::right, ConveyorStruct::TerminationType::straight, 4);

        up_segment->target = right_segment.get();

        CreateSegment({0, 0}, up_segment);
        CreateSegment({3, 0}, right_segment);

        // RIGHT LINE: 14 items can be fit on the right lane: (4 - 0.7) / ConveyorProp::kItemSpacing{0.25} = 13.2
        for (int i = 0; i < 14; ++i) {
            right_segment->AppendItem(false, 0.f, itemProto_);
        }

        // Items on up line should stop
        up_segment->AppendItem(false, 0.f, itemProto_);

        // WIll not move after an arbitrary number of updates
        for (int i = 0; i < 34; ++i) {
            ConveyorLogicUpdate(world_);
        }

        EXPECT_DOUBLE_EQ(up_segment->right.lane.front().dist.getAsDouble(), 0);
    }

    TEST_F(ConveyorControllerTest, LineLogicNewSegmentAddedAhead) {
        //     2      1
        // < ----- < -----

        transportBeltProto_->speed = 0.04f;

        // Segments (Logic chunk must be created first)
        auto left_segment =
            std::make_shared<ConveyorStruct>(Orientation::left, ConveyorStruct::TerminationType::straight, 2);

        CreateSegment({2, 1}, left_segment);

        // One item stopped, one still moving
        left_segment->AppendItem(true, 0, itemProto_);
        ConveyorLogicUpdate(world_);
        EXPECT_EQ(left_segment.get()->left.index, 0);

        left_segment->AppendItem(true, 2, itemProto_);
        ConveyorLogicUpdate(world_);
        EXPECT_EQ(left_segment.get()->left.index, 1);


        // ======================================================================
        const auto left_segment_2 =
            std::make_shared<ConveyorStruct>(Orientation::left, ConveyorStruct::TerminationType::straight, 1);

        left_segment->target = left_segment_2.get();

        CreateSegment({1, 1}, left_segment_2);

        // Update neighboring segments as a new segment was placed
        transportBeltProto_->OnNeighborUpdate(world_, logic_, {1, 1}, {2, 1}, Orientation::right);

        EXPECT_EQ(left_segment.get()->left.index, 0);
    }

    TEST_F(ConveyorControllerTest, LineLogicTargetTemporarilyBlocked) {
        // If the target segment is temporarily blocked, it will move into it at the next opportunity

        //     1      2
        // < ----- < -----

        transportBeltProto_->speed = 0.04f;

        const auto left_segment =
            std::make_shared<ConveyorStruct>(Orientation::left, ConveyorStruct::TerminationType::straight, 1);
        CreateSegment({1, 1}, left_segment);


        auto left_segment_2 =
            std::make_shared<ConveyorStruct>(Orientation::left, ConveyorStruct::TerminationType::straight, 1);
        left_segment_2->target = left_segment.get();
        CreateSegment({2, 1}, left_segment_2);


        // ======================================================================


        left_segment->AppendItem(true, 1 - ConveyorProp::kItemSpacing + 0.01, itemProto_);

        left_segment_2->AppendItem(true, 0, itemProto_);
        left_segment_2->AppendItem(true, 0.5, itemProto_);
        left_segment_2->AppendItem(true, 2, itemProto_);

        ConveyorLogicUpdate(world_);

        ASSERT_EQ(left_segment->left.lane.size(), 1);

        ConveyorLogicUpdate(world_);

        ASSERT_EQ(left_segment->left.lane.size(), 2);
    }


    // ======================================================================
    // Line properties

    TEST_F(ConveyorControllerTest, ItemSpacing) {
        // A minimum distance of ConveyorProp::kItemSpacing is maintained between items

        auto right_segment =
            std::make_shared<ConveyorStruct>(Orientation::right, ConveyorStruct::TerminationType::bend_right, 4);

        CreateSegment({0, 0}, right_segment);

        right_segment->AppendItem(true, 0.f, itemProto_);
        right_segment->AppendItem(true, 0.f, itemProto_); // Insert behind previous item

        // Check that second item has a minimum distance of ConveyorProp::kItemSpacing
        EXPECT_DOUBLE_EQ(right_segment->left.lane[0].dist.getAsDouble(), 0.);
        EXPECT_DOUBLE_EQ(right_segment->left.lane[1].dist.getAsDouble(), ConveyorProp::kItemSpacing);
    }

    TEST_F(ConveyorControllerTest, BackItemDistance) {
        /*
         * ^
         * |
         * |
         *
         * ^
         * |
         * |
         */

        transportBeltProto_->speed = 0.05;


        // Segments (Logic chunk must be created first)
        auto up_segment_1 =
            std::make_shared<ConveyorStruct>(Orientation::up, ConveyorStruct::TerminationType::straight, 1);
        auto up_segment_2 =
            std::make_shared<ConveyorStruct>(Orientation::up, ConveyorStruct::TerminationType::straight, 1);

        up_segment_2->target = up_segment_1.get();

        CreateSegment({0, 0}, up_segment_1);
        CreateSegment({0, 1}, up_segment_2);

        up_segment_2->AppendItem(true, 0.05, itemProto_);
        EXPECT_DOUBLE_EQ(up_segment_2->left.backItemDistance.getAsDouble(), 0.05);

        ConveyorLogicUpdate(world_);
        EXPECT_DOUBLE_EQ(up_segment_2->left.backItemDistance.getAsDouble(), 0);

        // Segment 1
        ConveyorLogicUpdate(world_);
        EXPECT_DOUBLE_EQ(up_segment_2->left.backItemDistance.getAsDouble(), 0);

        EXPECT_DOUBLE_EQ(up_segment_1->left.backItemDistance.getAsDouble(), 0.95); // First segment now

        for (int i = 0; i < 19; ++i) {
            ConveyorLogicUpdate(world_);
        }
        EXPECT_DOUBLE_EQ(up_segment_1->left.backItemDistance.getAsDouble(), 0);

        // Remains at 0
        ConveyorLogicUpdate(world_);
        EXPECT_DOUBLE_EQ(up_segment_1->left.backItemDistance.getAsDouble(), 0);


        // ======================================================================
        // Fill the first segment up to 4 items
        up_segment_1->AppendItem(true, 0, itemProto_);
        up_segment_1->AppendItem(true, 0, itemProto_);
        up_segment_1->AppendItem(true, 0, itemProto_);
        EXPECT_DOUBLE_EQ(up_segment_1->left.backItemDistance.getAsDouble(), 0.75);


        // Will not enter since segment 1 is full
        up_segment_2->AppendItem(true, 0.05, itemProto_);
        ConveyorLogicUpdate(world_);
        ConveyorLogicUpdate(world_);
        ConveyorLogicUpdate(world_);
        EXPECT_DOUBLE_EQ(up_segment_1->left.backItemDistance.getAsDouble(), 0.75);
        EXPECT_DOUBLE_EQ(up_segment_2->left.backItemDistance.getAsDouble(), 0);
    }


    // ======================================================================
    // Item transition


    TEST_F(ConveyorControllerTest, TransitionStraight) {
        // Transferring from a straight segment traveling left to another one traveling left
        /*
         * < ------ LEFT (1) ------		< ------ LEFT (2) -------
         */

        transportBeltProto_->speed = 0.01f;

        auto segment_1 =
            std::make_shared<ConveyorStruct>(Orientation::left, ConveyorStruct::TerminationType::straight, 4);
        auto segment_2 =
            std::make_shared<ConveyorStruct>(Orientation::left, ConveyorStruct::TerminationType::straight, 4);

        segment_2->target = segment_1.get();

        CreateSegment({0, 0}, segment_1);
        CreateSegment({3, 0}, segment_2);

        // Insert item on left + right side
        segment_2->AppendItem(true, 0.02f, itemProto_);
        segment_2->AppendItem(false, 0.02f, itemProto_);

        // Travel to the next belt in 0.02 / 0.01 + 1 updates
        for (int i = 0; i < 3; ++i) {
            ConveyorLogicUpdate(world_);
        }

        EXPECT_EQ(segment_2->left.lane.size(), 0);
        EXPECT_EQ(segment_2->right.lane.size(), 0);
        // 3.99 tiles from the end of this conveyor
        EXPECT_DOUBLE_EQ(segment_1->left.lane[0].dist.getAsDouble(), 3.99);
        EXPECT_DOUBLE_EQ(segment_1->right.lane[0].dist.getAsDouble(), 3.99);
    }

    TEST_F(ConveyorControllerTest, TransitionSideLeft) {
        // Belt feeding into only one side of another belt
        /*
         *                           Right     Left
         *                             |    -    |
         *                             |    -    |
         *        --------- A ----->   |    -    |
         *        --------- B ----->   | v  -    | Downwards belt
         *                             |    -    |
         *                             |    -    |
         *                             |  v -    |
         */
        // A first, fill entire lane, if A is not compressed, B moves

        transportBeltProto_->speed = 0.05;


        // Segments (Logic chunk must be created first)
        auto right_segment =
            std::make_shared<ConveyorStruct>(Orientation::right, ConveyorStruct::TerminationType::right_only, 5);
        auto down_segment =
            std::make_shared<ConveyorStruct>(Orientation::down, ConveyorStruct::TerminationType::straight, 10);

        right_segment->target          = down_segment.get();
        right_segment->sideInsertIndex = 8; // 8 + 1 = 9

        down_segment->headOffset = 1;

        CreateSegment({4, 0}, right_segment);
        CreateSegment({4, 9}, down_segment);

        // Insert items
        for (int i = 0; i < 3; ++i) {
            right_segment->AppendItem(true, 0.f, itemProto_);
            right_segment->AppendItem(false, 0.f, itemProto_);
        }

        // Logic tests
        ConveyorLogicUpdate(world_);

        // Since the target belt is empty, both A + B inserts into right lane
        EXPECT_EQ(right_segment->left.lane.size(), 2);
        EXPECT_EQ(right_segment->left.lane[0].dist.getAsDouble(), 0.2); // 0.25 - 0.05

        EXPECT_EQ(right_segment->right.lane.size(), 2);
        EXPECT_EQ(right_segment->right.lane[0].dist.getAsDouble(), 0.2);


        ASSERT_EQ(down_segment->right.lane.size(), 2);
        // 10 - 0.7 - 0.05
        EXPECT_DOUBLE_EQ(down_segment->right.lane[0].dist.getAsDouble(), 9.25);
        // (10 - 0.3 - 0.05) - (10 - 0.7 - 0.05)
        EXPECT_DOUBLE_EQ(down_segment->right.lane[1].dist.getAsDouble(), 0.4);


        // ======================================================================
        // End on One update prior to transitioning
        for (int j = 0; j < 4; ++j) {
            ConveyorLogicUpdate(world_);
        }
        EXPECT_EQ(right_segment->left.lane[0].dist.getAsDouble(), 0.0);
        EXPECT_EQ(right_segment->right.lane[0].dist.getAsDouble(), 0.0);

        EXPECT_DOUBLE_EQ(down_segment->right.lane[0].dist.getAsDouble(), 9.05);


        // ======================================================================
        // Transition items
        ConveyorLogicUpdate(world_);
        EXPECT_EQ(right_segment->left.lane.size(), 1);
        EXPECT_EQ(right_segment->left.lane[0].dist.getAsDouble(), 0.2); // 0.25 - 0.05


        // ======================================================================
        // Right lane (B) stops, left (A) takes priority
        EXPECT_EQ(right_segment->right.lane.size(), 2); // Unmoved
        EXPECT_EQ(right_segment->right.lane[0].dist.getAsDouble(), 0.f);

        ASSERT_EQ(down_segment->right.lane.size(), 3);
        EXPECT_DOUBLE_EQ(down_segment->right.lane[0].dist.getAsDouble(), 9.00);
        EXPECT_DOUBLE_EQ(down_segment->right.lane[1].dist.getAsDouble(), 0.4);
        EXPECT_DOUBLE_EQ(down_segment->right.lane[2].dist.getAsDouble(), 0.25);


        // ======================================================================
        // Transition third item for Lane A, should wake up lane B after passing
        for (int j = 0; j < 4 + 13 + 1; ++j) { // 0.20 / 0.05 + (0.40 + 0.25) / 0.05 + 1 for transition
            ConveyorLogicUpdate(world_);
        }
        EXPECT_EQ(right_segment->left.lane.size(), 0);
        EXPECT_EQ(right_segment->right.lane.size(), 1); // Woke and moved

        ASSERT_EQ(down_segment->right.lane.size(), 5);
        EXPECT_DOUBLE_EQ(down_segment->right.lane[0].dist.getAsDouble(), 8.10);
        EXPECT_DOUBLE_EQ(down_segment->right.lane[3].dist.getAsDouble(), 0.25);
    }

    TEST_F(ConveyorControllerTest, TransitionSideRight) {
        // Belt feeding into only one side of another belt moving updards
        /*
         * Left     Right
         *  |    -    |
         *  |    -    |
         *  |    -    |	<------------ A -------------
         *  |  ^ -    | <------------ B -------------
         *  |    -    |
         *  |    -    |
         *  |    -    |
         */
        // B first, fill entire lane, if B is not compressed, A moves

        transportBeltProto_->speed = 0.05;


        // Segments (Logic chunk must be created first)
        auto left_segment =
            std::make_shared<ConveyorStruct>(Orientation::left, ConveyorStruct::TerminationType::right_only, 5);
        auto down_segment =
            std::make_shared<ConveyorStruct>(Orientation::down, ConveyorStruct::TerminationType::straight, 20);

        left_segment->target = down_segment.get();

        left_segment->sideInsertIndex = -1; // Will insert into up_segment with offset of 9 absolute
        down_segment->headOffset      = 10;

        CreateSegment({4, 0}, left_segment);
        CreateSegment({4, 9}, down_segment);


        // Insert items
        for (int i = 0; i < 3; ++i) {
            left_segment->AppendItem(true, 0.f, itemProto_);
            left_segment->AppendItem(false, 0.f, itemProto_);
        }

        // Logic tests
        ConveyorLogicUpdate(world_);

        // Since the target belt is empty, both A + B inserts into right lane
        EXPECT_EQ(left_segment->left.lane.size(), 2);
        EXPECT_EQ(left_segment->left.lane[0].dist.getAsDouble(), 0.2); // 0.25 - 0.05

        EXPECT_EQ(left_segment->right.lane.size(), 2);
        EXPECT_EQ(left_segment->right.lane[0].dist.getAsDouble(), 0.2);


        ASSERT_EQ(down_segment->right.lane.size(), 2);
        // 10 - 0.7 - 0.05
        EXPECT_DOUBLE_EQ(down_segment->right.lane[0].dist.getAsDouble(), 9.25);
        // (10 - 0.3 - 0.05) - (10 - 0.7 - 0.05)
        EXPECT_DOUBLE_EQ(down_segment->right.lane[1].dist.getAsDouble(), 0.4);


        // ======================================================================
        // End on One update prior to transitioning
        for (int j = 0; j < 4; ++j) {
            ConveyorLogicUpdate(world_);
        }
        EXPECT_EQ(left_segment->left.lane[0].dist.getAsDouble(), 0.0);
        EXPECT_EQ(left_segment->right.lane[0].dist.getAsDouble(), 0.0);

        EXPECT_DOUBLE_EQ(down_segment->right.lane[0].dist.getAsDouble(), 9.05);


        // ======================================================================
        // Transition items
        ConveyorLogicUpdate(world_);
        EXPECT_EQ(left_segment->left.lane.size(), 1);
        EXPECT_EQ(left_segment->left.lane[0].dist.getAsDouble(), 0.2); // 0.25 - 0.05


        // ======================================================================
        // Right lane (B) stops, left (A) takes priority
        EXPECT_EQ(left_segment->right.lane.size(), 2); // Unmoved
        EXPECT_EQ(left_segment->right.lane[0].dist.getAsDouble(), 0.f);

        ASSERT_EQ(down_segment->right.lane.size(), 3);
        EXPECT_DOUBLE_EQ(down_segment->right.lane[0].dist.getAsDouble(), 9.00);
        EXPECT_DOUBLE_EQ(down_segment->right.lane[1].dist.getAsDouble(), 0.4);
        EXPECT_DOUBLE_EQ(down_segment->right.lane[2].dist.getAsDouble(), 0.25);


        // ======================================================================
        // Transition third item for Lane A, should wake up lane B after passing
        for (int j = 0; j < 4 + 13 + 1; ++j) { // 0.20 / 0.05 + (0.40 + 0.25) / 0.05 + 1 for transition
            ConveyorLogicUpdate(world_);
        }
        EXPECT_EQ(left_segment->left.lane.size(), 0);
        EXPECT_EQ(left_segment->right.lane.size(), 1); // Woke and moved

        ASSERT_EQ(down_segment->right.lane.size(), 5);
        EXPECT_DOUBLE_EQ(down_segment->right.lane[0].dist.getAsDouble(), 8.10);
        EXPECT_DOUBLE_EQ(down_segment->right.lane[3].dist.getAsDouble(), 0.25);
    }

    TEST_F(ConveyorControllerTest, TransitionSideOnlyToBending) {
        //     v
        // < < <
        //     ^

        transportBeltProto_->speed = 0.06;

        auto left_segment =
            std::make_shared<ConveyorStruct>(Orientation::left, ConveyorStruct::TerminationType::bend_right, 4);
        left_segment->headOffset = 1;
        CreateSegment({2, 2}, left_segment);

        // ======================================================================

        auto down_segment =
            std::make_shared<ConveyorStruct>(Orientation::down, ConveyorStruct::TerminationType::right_only, 1);

        down_segment->target          = left_segment.get();
        down_segment->sideInsertIndex = 2;

        CreateSegment({3, 1}, down_segment);


        // Left lane


        down_segment->AppendItem(true, 0, itemProto_);

        ConveyorLogicUpdate(world_);

        ASSERT_EQ(left_segment->right.lane.size(), 1);

        // (line offset) - belt speed
        EXPECT_DOUBLE_EQ(left_segment->right.lane[0].dist.getAsDouble(), (0.3 + 0.7) + 2. - 0.06);


        // Right lane


        left_segment->right.lane.clear();

        down_segment->AppendItem(false, 0, itemProto_);

        ConveyorLogicUpdate(world_);

        ASSERT_EQ(left_segment->right.lane.size(), 1);

        EXPECT_DOUBLE_EQ(left_segment->right.lane[0].dist.getAsDouble(), (0.3 + 0.3) + 2. - 0.06);


        // ======================================================================

        auto up_segment =
            std::make_shared<ConveyorStruct>(Orientation::up, ConveyorStruct::TerminationType::left_only, 1);

        up_segment->target          = left_segment.get();
        up_segment->sideInsertIndex = 2;

        CreateSegment({3, 3}, up_segment);


        // Left lane


        up_segment->AppendItem(true, 0, itemProto_);

        ConveyorLogicUpdate(world_);

        ASSERT_EQ(left_segment->left.lane.size(), 1);

        EXPECT_DOUBLE_EQ(left_segment->left.lane[0].dist.getAsDouble(), (0.7 + 0.3) + 2. - 0.06);


        // Right lane


        left_segment->left.lane.clear();

        up_segment->AppendItem(false, 0, itemProto_);

        ConveyorLogicUpdate(world_);

        ASSERT_EQ(left_segment->left.lane.size(), 1);

        EXPECT_DOUBLE_EQ(left_segment->left.lane[0].dist.getAsDouble(), (0.7 + 0.7) + 2. - 0.06);
    }

    TEST_F(ConveyorControllerTest, TransitionBendingToSideOnly) {
        // > v
        //   v
        // < < <

        transportBeltProto_->speed = 0.06;

        auto down_segment =
            std::make_shared<ConveyorStruct>(Orientation::down, ConveyorStruct::TerminationType::right_only, 3);

        CreateSegment({3, 2}, down_segment);


        auto right_segment =
            std::make_shared<ConveyorStruct>(Orientation::right, ConveyorStruct::TerminationType::bend_right, 2);

        right_segment->target = down_segment.get();

        CreateSegment({2, 1}, right_segment);


        // ======================================================================
        // Left

        right_segment->AppendItem(true, 0, itemProto_);

        ConveyorLogicUpdate(world_);

        ASSERT_EQ(down_segment->left.lane.size(), 1);
        EXPECT_DOUBLE_EQ(down_segment->left.lane[0].dist.getAsDouble(), (0.3 + 1. + 0.7) - 0.06);


        // Right

        right_segment->AppendItem(false, 0, itemProto_);

        ConveyorLogicUpdate(world_);

        ASSERT_EQ(down_segment->right.lane.size(), 1);
        EXPECT_DOUBLE_EQ(down_segment->right.lane[0].dist.getAsDouble(), (0.3 + 1. + 0.3) - 0.06);
    }
} // namespace jactorio::game
