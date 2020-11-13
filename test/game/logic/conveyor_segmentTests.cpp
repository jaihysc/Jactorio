// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "jactorioTests.h"

#include "proto/transport_belt.h"

namespace jactorio::game
{
    class ConveyorSegmentTest : public testing::Test
    {
    protected:
        std::unique_ptr<proto::Item> itemProto_                   = std::make_unique<proto::Item>();
        std::unique_ptr<proto::TransportBelt> transportBeltProto_ = std::make_unique<proto::TransportBelt>();

        WorldData worldData_{};

        std::unique_ptr<ConveyorSegment> segment_ =
            std::make_unique<ConveyorSegment>(proto::Orientation::left, ConveyorSegment::TerminationType::straight, 2);

        void SetUp() override {
            transportBeltProto_->speed = 0.01;
        }
    };

    TEST_F(ConveyorSegmentTest, CanInsertFilledConveyor) {
        // THe entire conveyor is compressed with items, cannot insert

        // At spacing of 0.25, 4 items per segment
        segment_->AppendItem(true, 0., *itemProto_);
        segment_->AppendItem(true, ConveyorProp::kItemSpacing, *itemProto_);
        segment_->AppendItem(true, ConveyorProp::kItemSpacing, *itemProto_);
        segment_->AppendItem(true, ConveyorProp::kItemSpacing, *itemProto_);

        segment_->AppendItem(true, ConveyorProp::kItemSpacing, *itemProto_);
        segment_->AppendItem(true, ConveyorProp::kItemSpacing, *itemProto_);
        segment_->AppendItem(true, ConveyorProp::kItemSpacing, *itemProto_);
        segment_->AppendItem(true, ConveyorProp::kItemSpacing, *itemProto_);

        // Location 1.75 tiles from beginning of conveyor is filled
        EXPECT_FALSE(segment_->CanInsert(true, proto::LineDistT(1.75)));
    }

    TEST_F(ConveyorSegmentTest, CanInsertEmptyConveyor) {
        // THe entire conveyor is empty and can thus insert
        EXPECT_TRUE(segment_->CanInsert(false, proto::LineDistT(1.75)));
    }

    TEST_F(ConveyorSegmentTest, CanInsertGap) {
        // Insert into a gap between 1 and 1.5
        // Is wide enough for the item (item_width - ConveyorProp::kItemSpacing) to fit there

        segment_->AppendItem(false, 0., *itemProto_);
        segment_->AppendItem(false, 1., *itemProto_);
        // Items can be inserted in this 0.5 gap
        segment_->AppendItem(false, 0.5, *itemProto_);


        // Overlaps with the item at 1 by 0.01
        EXPECT_FALSE(segment_->CanInsert(false, proto::LineDistT(ConveyorProp::kItemSpacing - 0.01)));
        // Will overlap with item at 1.5
        EXPECT_FALSE(segment_->CanInsert(false, proto::LineDistT(1.45)));


        // Sufficient space ahead and behind
        EXPECT_TRUE(segment_->CanInsert(false, proto::LineDistT(1.25)));
    }

    TEST_F(ConveyorSegmentTest, CanInsertFirstItem) {
        // The first item which is appended ignores an additional offset of ConveyorProp::kItemSpacing when calculating
        // whether or not it can be inserted

        const proto::LineDistT offset{0.f};

        bool result = segment_->CanInsert(true, offset); // Ok, Offset can be 0, is first item
        EXPECT_TRUE(result);

        segment_->AppendItem(true, 0, *itemProto_);
        result = segment_->CanInsert(true, offset); // Not ok, offset changed to ConveyorProp::kItemSpacing
        EXPECT_FALSE(result);

        segment_->AppendItem(true, 0, *itemProto_);
        result = segment_->CanInsert(true, offset); // Not ok, offset changed to ConveyorProp::kItemSpacing
        EXPECT_FALSE(result);
    }

    TEST_F(ConveyorSegmentTest, CanInsertAbs) {
        // the given offset should be adjusted as ConveyorSegment::itemOffset is adjusted where 1 = 1 tile
        segment_->itemOffset = 1;
        const proto::LineDistT offset{0.f};

        segment_->AppendItem(true, 0, *itemProto_);
        const bool result = segment_->CanInsertAbs(true, offset); // Ok, offset (0) + itemOffset (1) = 1
        EXPECT_TRUE(result);
    }


    TEST_F(ConveyorSegmentTest, IsActive) {
        // Insert into a gap between 1 and 1.5
        // Is wide enough for the item (item_width - ConveyorProp::kItemSpacing) to fit there

        EXPECT_FALSE(segment_->left.IsActive());
        EXPECT_FALSE(segment_->right.IsActive());

        segment_->AppendItem(false, 0., *itemProto_);
        segment_->AppendItem(true, 0., *itemProto_);

        // Has items, now active
        EXPECT_TRUE(segment_->left.IsActive());
        EXPECT_TRUE(segment_->right.IsActive());

        // Invalid indices, inactive
        segment_->left.index  = 100;
        segment_->right.index = 100;
        EXPECT_FALSE(segment_->left.IsActive());
        EXPECT_FALSE(segment_->right.IsActive());
    }

    TEST_F(ConveyorSegmentTest, AppendItem) {
        auto line_segment =
            std::make_unique<ConveyorSegment>(proto::Orientation::up, ConveyorSegment::TerminationType::bend_right, 5);

        // Offset is from the beginning of the conveyor OR the previous item if it exists
        line_segment->AppendItem(true, 1.3, *itemProto_);
        EXPECT_DOUBLE_EQ(line_segment.get()->left.lane[0].dist.getAsDouble(), 1.3);

        line_segment->AppendItem(true, 1.2, *itemProto_);
        EXPECT_DOUBLE_EQ(line_segment.get()->left.lane[1].dist.getAsDouble(), 1.2);

        line_segment->AppendItem(true, 1.5, *itemProto_);
        EXPECT_DOUBLE_EQ(line_segment.get()->left.lane[2].dist.getAsDouble(), 1.5);

        line_segment->AppendItem(true, 0.5, *itemProto_);
        EXPECT_DOUBLE_EQ(line_segment.get()->left.lane[3].dist.getAsDouble(), 0.5);
    }

    TEST_F(ConveyorSegmentTest, AppendItemFirstItem) {
        // The first item which is appended ignores an additional offset of ConveyorProp::kItemSpacing when calculating
        // whether or not it can be appended

        auto line_segment =
            std::make_unique<ConveyorSegment>(proto::Orientation::up, ConveyorSegment::TerminationType::bend_right, 5);

        line_segment->AppendItem(true, 0, *itemProto_); // Ok, Offset can be 0, is first item
        EXPECT_DOUBLE_EQ(line_segment.get()->left.lane[0].dist.getAsDouble(), 0);

        line_segment->AppendItem(true, 0, *itemProto_); // Not ok, offset changed to ConveyorProp::kItemSpacing
        EXPECT_DOUBLE_EQ(line_segment.get()->left.lane[1].dist.getAsDouble(), ConveyorProp::kItemSpacing);

        line_segment->AppendItem(true, 0, *itemProto_); // Not ok, offset changed to ConveyorProp::kItemSpacing
        EXPECT_DOUBLE_EQ(line_segment.get()->left.lane[2].dist.getAsDouble(), ConveyorProp::kItemSpacing);
    }

    TEST_F(ConveyorSegmentTest, InsertItem) {
        // Insert INSERTS an item at an arbitrary position offset from the beginning of the conveyor

        auto line_segment =
            std::make_unique<ConveyorSegment>(proto::Orientation::up, ConveyorSegment::TerminationType::bend_right, 5);

        // Offset is ALWAYS from the beginning of the conveyor

        line_segment->InsertItem(true, 1.2, *itemProto_);
        EXPECT_DOUBLE_EQ(line_segment.get()->left.lane[0].dist.getAsDouble(), 1.2); // < 1.2

        // Should be sorted by items closest to the end of the segment
        line_segment->InsertItem(true, 2.5, *itemProto_);
        EXPECT_DOUBLE_EQ(line_segment.get()->left.lane[0].dist.getAsDouble(), 1.2); // 1.2
        EXPECT_DOUBLE_EQ(line_segment.get()->left.lane[1].dist.getAsDouble(), 1.3); // < 2.5

        line_segment->InsertItem(true, 0.5, *itemProto_);
        EXPECT_DOUBLE_EQ(line_segment.get()->left.lane[0].dist.getAsDouble(), 0.5); // < 0.5
        EXPECT_DOUBLE_EQ(line_segment.get()->left.lane[1].dist.getAsDouble(), 0.7); // 1.2
        EXPECT_DOUBLE_EQ(line_segment.get()->left.lane[2].dist.getAsDouble(), 1.3); // 2.5

        line_segment->InsertItem(true, 0.1, *itemProto_);
        EXPECT_DOUBLE_EQ(line_segment.get()->left.lane[0].dist.getAsDouble(), 0.1); // < 0.1
        EXPECT_DOUBLE_EQ(line_segment.get()->left.lane[1].dist.getAsDouble(), 0.4); // 0.5
        EXPECT_DOUBLE_EQ(line_segment.get()->left.lane[2].dist.getAsDouble(), 0.7); // 1.2
        EXPECT_DOUBLE_EQ(line_segment.get()->left.lane[3].dist.getAsDouble(), 1.3); // 2.5

        line_segment->InsertItem(true, 1.8, *itemProto_);
        EXPECT_DOUBLE_EQ(line_segment.get()->left.lane[0].dist.getAsDouble(), 0.1); // 0.1
        EXPECT_DOUBLE_EQ(line_segment.get()->left.lane[1].dist.getAsDouble(), 0.4); // 0.5
        EXPECT_DOUBLE_EQ(line_segment.get()->left.lane[2].dist.getAsDouble(), 0.7); // 1.2
        EXPECT_DOUBLE_EQ(line_segment.get()->left.lane[3].dist.getAsDouble(), 0.6); // < 1.8
        EXPECT_DOUBLE_EQ(line_segment.get()->left.lane[4].dist.getAsDouble(), 0.7); // 2.5
    }

    TEST_F(ConveyorSegmentTest, InsertItemAbs) {
        auto line_segment =
            std::make_unique<ConveyorSegment>(proto::Orientation::up, ConveyorSegment::TerminationType::bend_right, 5);
        line_segment->itemOffset = 2;

        // Offset is ALWAYS from the beginning of the conveyor + itemOffset

        line_segment->InsertItemAbs(true, 1.2, *itemProto_);
        EXPECT_DOUBLE_EQ(line_segment.get()->left.lane[0].dist.getAsDouble(), 3.2);

        line_segment->InsertItemAbs(true, 1.5, *itemProto_);
        EXPECT_DOUBLE_EQ(line_segment.get()->left.lane[0].dist.getAsDouble(), 3.2);
        EXPECT_DOUBLE_EQ(line_segment.get()->left.lane[1].dist.getAsDouble(), 0.3); // 1.5
    }

    TEST_F(ConveyorSegmentTest, TryInsertItem) {

        auto line_segment =
            std::make_unique<ConveyorSegment>(proto::Orientation::up, ConveyorSegment::TerminationType::bend_right, 5);

        // Offset is ALWAYS from the beginning of the conveyor
        {
            const bool result = line_segment->TryInsertItem(true, 1.2, *itemProto_);
            ASSERT_TRUE(result);
            EXPECT_DOUBLE_EQ(line_segment.get()->left.lane[0].dist.getAsDouble(), 1.2);
        }
        {
            // Too close
            const bool result = line_segment->TryInsertItem(true, 1.3, *itemProto_);
            ASSERT_FALSE(result);
        }


        // Should also reenable conveyor upon insertion if it is disabled
        line_segment->left.index = 999;

        {
            const bool result = line_segment->TryInsertItem(true, 0.5, *itemProto_);
            ASSERT_TRUE(result);
            EXPECT_DOUBLE_EQ(line_segment.get()->left.lane[0].dist.getAsDouble(), 0.5);
            EXPECT_DOUBLE_EQ(line_segment.get()->left.lane[1].dist.getAsDouble(), 0.7); // 1.2
        }

        EXPECT_EQ(line_segment->left.index, 0);
    }

    TEST_F(ConveyorSegmentTest, BackItemDistanceLeft) {

        auto line_segment =
            std::make_unique<ConveyorSegment>(proto::Orientation::up, ConveyorSegment::TerminationType::bend_right, 5);

        EXPECT_DOUBLE_EQ(line_segment->left.backItemDistance.getAsDouble(), 0);


        // Appending
        line_segment->AppendItem(true, 1.2, *itemProto_);
        EXPECT_DOUBLE_EQ(line_segment->left.backItemDistance.getAsDouble(), 1.2);

        line_segment->AppendItem(true, 3, *itemProto_);
        EXPECT_DOUBLE_EQ(line_segment->left.backItemDistance.getAsDouble(), 4.2);

        line_segment->AppendItem(true, 1.8, *itemProto_);
        EXPECT_DOUBLE_EQ(line_segment->left.backItemDistance.getAsDouble(), 6);

        // Inserting (Starting at 6.f)
        line_segment->InsertItem(true, 7, *itemProto_);
        EXPECT_DOUBLE_EQ(line_segment->left.backItemDistance.getAsDouble(), 7);

        line_segment->InsertItem(true, 2, *itemProto_);
        EXPECT_DOUBLE_EQ(line_segment->left.backItemDistance.getAsDouble(), 7); // Unchanged

        EXPECT_DOUBLE_EQ(line_segment->right.backItemDistance.getAsDouble(), 0);
    }

    TEST_F(ConveyorSegmentTest, BackItemDistanceRight) {

        auto line_segment =
            std::make_unique<ConveyorSegment>(proto::Orientation::up, ConveyorSegment::TerminationType::bend_right, 5);

        EXPECT_DOUBLE_EQ(line_segment->right.backItemDistance.getAsDouble(), 0.);


        // Appending
        line_segment->AppendItem(false, 1.2, *itemProto_);
        EXPECT_DOUBLE_EQ(line_segment->right.backItemDistance.getAsDouble(), 1.2);

        line_segment->AppendItem(false, 3, *itemProto_);
        EXPECT_DOUBLE_EQ(line_segment->right.backItemDistance.getAsDouble(), 4.2);

        line_segment->AppendItem(false, 1.8, *itemProto_);
        EXPECT_DOUBLE_EQ(line_segment->right.backItemDistance.getAsDouble(), 6);

        // Inserting (Starting at 6.f)
        line_segment->InsertItem(false, 7, *itemProto_);
        EXPECT_DOUBLE_EQ(line_segment->right.backItemDistance.getAsDouble(), 7);

        line_segment->InsertItem(false, 2, *itemProto_);
        EXPECT_DOUBLE_EQ(line_segment->right.backItemDistance.getAsDouble(), 7); // Unchanged

        EXPECT_DOUBLE_EQ(line_segment->left.backItemDistance.getAsDouble(), 0);
    }

    TEST_F(ConveyorSegmentTest, GetOffsetAbs) {
        {
            segment_->itemOffset = 0;

            ConveyorSegment::FloatOffsetT o = 3;
            segment_->GetOffsetAbs(o);
            EXPECT_DOUBLE_EQ(o, 3.);
        }
        {
            segment_->itemOffset = 3;

            ConveyorSegment::FloatOffsetT o = 3;
            segment_->GetOffsetAbs(o);
            EXPECT_DOUBLE_EQ(o, 0.);
        }

        {
            segment_->itemOffset = -2;

            ConveyorSegment::IntOffsetT o = 3;
            segment_->GetOffsetAbs(o);
            EXPECT_DOUBLE_EQ(o, 5.);
        }
    }

    TEST_F(ConveyorSegmentTest, GetItem) {
        auto get_item = [&]() { // true is an item is found
            // Valid range is 1.3 and 1.7 inclusive
            return segment_->GetItem(true, 1.5).second.item != nullptr;
        };

        segment_->AppendItem(true, 0.5, *itemProto_);
        segment_->AppendItem(true, 0.5, *itemProto_);  // 1.00
        segment_->AppendItem(true, 0.29, *itemProto_); // 1.29
        EXPECT_FALSE(get_item());

        segment_->AppendItem(true, 0.42, *itemProto_); // 1.71
        EXPECT_FALSE(get_item());
    }

    TEST_F(ConveyorSegmentTest, TryPopItem) {
        EXPECT_EQ(segment_->TryPopItem(true, 0.25), nullptr);
        EXPECT_EQ(segment_->TryPopItem(false, 0.25), nullptr);

        // Pop off appended item
        segment_->AppendItem(true, 0.3, *itemProto_);

        EXPECT_EQ(segment_->TryPopItem(true, 0.4, 0.1), itemProto_.get());
        ASSERT_TRUE(segment_->left.lane.empty());

        //

        proto::Item item2{};
        segment_->AppendItem(true, 0.1, *itemProto_);
        segment_->AppendItem(true, 0.8, item2);       // 0.9
        segment_->AppendItem(true, 0.9, *itemProto_); // 1.8

        EXPECT_EQ(segment_->TryPopItem(true, 0.9, 0.7), &item2);
        ASSERT_EQ(segment_->left.lane.size(), 2);

        // Should preserve spacing
        EXPECT_DOUBLE_EQ(segment_->left.lane[1].dist.getAsDouble(), 1.7);
    }

    TEST_F(ConveyorSegmentTest, Serialize) {
        data::PrototypeManager proto_manager;
        auto& item = proto_manager.AddProto<proto::Item>();

        auto segment =
            std::make_unique<ConveyorSegment>(proto::Orientation::down, ConveyorSegment::TerminationType::bend_left, 4);

        segment->AppendItem(true, 0.43, item);
        segment->AppendItem(true, 0.43, item);
        segment->left.backItemDistance = 65.456;
        segment->left.index            = 40;
        segment->left.visible          = true;

        segment->AppendItem(false, 0.223, item);
        segment->right.index            = 50;
        segment->right.backItemDistance = 23.456;
        segment->right.visible          = false;


        data::active_prototype_manager = &proto_manager;
        proto_manager.GenerateRelocationTable();

        // ======================================================================
        auto result = TestSerializeDeserialize(segment);

        EXPECT_EQ(result->direction, proto::Orientation::down);
        EXPECT_EQ(result->terminationType, ConveyorSegment::TerminationType::bend_left);
        EXPECT_EQ(result->length, 4);

        auto& l_lane = result->left;
        EXPECT_DOUBLE_EQ(l_lane.backItemDistance.getAsDouble(), 65.456);
        EXPECT_EQ(l_lane.index, 40);
        EXPECT_TRUE(l_lane.visible);

        ASSERT_EQ(l_lane.lane.size(), 2);
        EXPECT_DOUBLE_EQ(l_lane.lane[0].dist.getAsDouble(), 0.43);
        EXPECT_EQ(l_lane.lane[0].item, &item);


        auto& r_lane = result->right;
        EXPECT_DOUBLE_EQ(r_lane.backItemDistance.getAsDouble(), 23.456);
        EXPECT_EQ(r_lane.index, 50);
        EXPECT_FALSE(r_lane.visible);

        ASSERT_EQ(r_lane.lane.size(), 1);
        EXPECT_DOUBLE_EQ(r_lane.lane[0].dist.getAsDouble(), 0.223);
        EXPECT_EQ(r_lane.lane[0].item, &item);
    }
} // namespace jactorio::game
