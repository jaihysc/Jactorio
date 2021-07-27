// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "jactorioTests.h"

#include "proto/transport_belt.h"

namespace jactorio::game
{
    class ConveyorStructTest : public testing::Test
    {
    protected:
        proto::Item item_;
        proto::TransportBelt transportBelt_;

        World world_;

        ConveyorStruct segment_{Orientation::left, ConveyorStruct::TerminationType::straight, 2};

        void SetUp() override {
            transportBelt_.speed = 0.01;
        }
    };

    TEST_F(ConveyorStructTest, CanInsertFilledConveyor) {
        // THe entire conveyor is compressed with items, cannot insert

        // At spacing of 0.25, 4 items per segment
        segment_.AppendItem(true, 0., item_);
        segment_.AppendItem(true, ConveyorProp::kItemSpacing, item_);
        segment_.AppendItem(true, ConveyorProp::kItemSpacing, item_);
        segment_.AppendItem(true, ConveyorProp::kItemSpacing, item_);

        segment_.AppendItem(true, ConveyorProp::kItemSpacing, item_);
        segment_.AppendItem(true, ConveyorProp::kItemSpacing, item_);
        segment_.AppendItem(true, ConveyorProp::kItemSpacing, item_);
        segment_.AppendItem(true, ConveyorProp::kItemSpacing, item_);

        // Location 1.75 tiles from beginning of conveyor is filled
        EXPECT_FALSE(segment_.CanInsert(true, proto::LineDistT(1.75)));
    }

    TEST_F(ConveyorStructTest, CanInsertEmptyConveyor) {
        // THe entire conveyor is empty and can thus insert
        EXPECT_TRUE(segment_.CanInsert(false, proto::LineDistT(1.75)));
    }

    TEST_F(ConveyorStructTest, CanInsertGap) {
        // Insert into a gap between 1 and 1.5
        // Is wide enough for the item (item_width - ConveyorProp::kItemSpacing) to fit there

        segment_.AppendItem(false, 0., item_);
        segment_.AppendItem(false, 1., item_);
        // Items can be inserted in this 0.5 gap
        segment_.AppendItem(false, 0.5, item_);


        // Overlaps with the item at 1 by 0.01
        EXPECT_FALSE(segment_.CanInsert(false, proto::LineDistT(ConveyorProp::kItemSpacing - 0.01)));
        // Will overlap with item at 1.5
        EXPECT_FALSE(segment_.CanInsert(false, proto::LineDistT(1.45)));


        // Sufficient space ahead and behind
        EXPECT_TRUE(segment_.CanInsert(false, proto::LineDistT(1.25)));
    }

    TEST_F(ConveyorStructTest, CanInsertFirstItem) {
        // The first item which is appended ignores an additional offset of ConveyorProp::kItemSpacing when calculating
        // whether or not it can be inserted

        const proto::LineDistT offset{0.f};

        bool result = segment_.CanInsert(true, offset); // Ok, Offset can be 0, is first item
        EXPECT_TRUE(result);

        segment_.AppendItem(true, 0, item_);
        result = segment_.CanInsert(true, offset); // Not ok, offset changed to ConveyorProp::kItemSpacing
        EXPECT_FALSE(result);

        segment_.AppendItem(true, 0, item_);
        result = segment_.CanInsert(true, offset); // Not ok, offset changed to ConveyorProp::kItemSpacing
        EXPECT_FALSE(result);
    }

    TEST_F(ConveyorStructTest, CanInsertAbs) {
        // the given offset should be adjusted as ConveyorSegment::itemOffset is adjusted where 1 = 1 tile
        segment_.headOffset = 1;
        const proto::LineDistT offset{0.f};

        segment_.AppendItem(true, 0, item_);
        const bool result = segment_.CanInsertAbs(true, offset); // Ok, offset (0) + itemOffset (1) = 1
        EXPECT_TRUE(result);
    }


    TEST_F(ConveyorStructTest, IsActive) {
        // Insert into a gap between 1 and 1.5
        // Is wide enough for the item (item_width - ConveyorProp::kItemSpacing) to fit there

        EXPECT_FALSE(segment_.left.IsActive());
        EXPECT_FALSE(segment_.right.IsActive());

        segment_.AppendItem(false, 0., item_);
        segment_.AppendItem(true, 0., item_);

        // Has items, now active
        EXPECT_TRUE(segment_.left.IsActive());
        EXPECT_TRUE(segment_.right.IsActive());

        // Invalid indices, inactive
        segment_.left.index  = 100;
        segment_.right.index = 100;
        EXPECT_FALSE(segment_.left.IsActive());
        EXPECT_FALSE(segment_.right.IsActive());
    }

    TEST_F(ConveyorStructTest, AppendItem) {
        ConveyorStruct line_segment(Orientation::up, ConveyorStruct::TerminationType::bend_right, 5);

        // Offset is from the beginning of the conveyor OR the previous item if it exists
        line_segment.AppendItem(true, 1.3, item_);
        EXPECT_DOUBLE_EQ(line_segment.left.lane[0].dist.getAsDouble(), 1.3);

        line_segment.AppendItem(true, 1.2, item_);
        EXPECT_DOUBLE_EQ(line_segment.left.lane[1].dist.getAsDouble(), 1.2);

        line_segment.AppendItem(true, 1.5, item_);
        EXPECT_DOUBLE_EQ(line_segment.left.lane[2].dist.getAsDouble(), 1.5);

        line_segment.AppendItem(true, 0.5, item_);
        EXPECT_DOUBLE_EQ(line_segment.left.lane[3].dist.getAsDouble(), 0.5);
    }

    TEST_F(ConveyorStructTest, AppendItemFirstItem) {
        // The first item which is appended ignores an additional offset of ConveyorProp::kItemSpacing when calculating
        // whether or not it can be appended

        ConveyorStruct line_segment(Orientation::up, ConveyorStruct::TerminationType::bend_right, 5);

        line_segment.AppendItem(true, 0, item_); // Ok, Offset can be 0, is first item
        EXPECT_DOUBLE_EQ(line_segment.left.lane[0].dist.getAsDouble(), 0);

        line_segment.AppendItem(true, 0, item_); // Not ok, offset changed to ConveyorProp::kItemSpacing
        EXPECT_DOUBLE_EQ(line_segment.left.lane[1].dist.getAsDouble(), ConveyorProp::kItemSpacing);

        line_segment.AppendItem(true, 0, item_); // Not ok, offset changed to ConveyorProp::kItemSpacing
        EXPECT_DOUBLE_EQ(line_segment.left.lane[2].dist.getAsDouble(), ConveyorProp::kItemSpacing);
    }

    TEST_F(ConveyorStructTest, InsertItem) {
        // Insert INSERTS an item at an arbitrary position offset from the beginning of the conveyor

        ConveyorStruct line_segment(Orientation::up, ConveyorStruct::TerminationType::bend_right, 5);

        // Offset is ALWAYS from the beginning of the conveyor

        line_segment.InsertItem(true, 1.2, item_);
        EXPECT_DOUBLE_EQ(line_segment.left.lane[0].dist.getAsDouble(), 1.2); // < 1.2

        // Should be sorted by items closest to the end of the segment
        line_segment.InsertItem(true, 2.5, item_);
        EXPECT_DOUBLE_EQ(line_segment.left.lane[0].dist.getAsDouble(), 1.2); // 1.2
        EXPECT_DOUBLE_EQ(line_segment.left.lane[1].dist.getAsDouble(), 1.3); // < 2.5

        line_segment.InsertItem(true, 0.5, item_);
        EXPECT_DOUBLE_EQ(line_segment.left.lane[0].dist.getAsDouble(), 0.5); // < 0.5
        EXPECT_DOUBLE_EQ(line_segment.left.lane[1].dist.getAsDouble(), 0.7); // 1.2
        EXPECT_DOUBLE_EQ(line_segment.left.lane[2].dist.getAsDouble(), 1.3); // 2.5

        line_segment.InsertItem(true, 0.1, item_);
        EXPECT_DOUBLE_EQ(line_segment.left.lane[0].dist.getAsDouble(), 0.1); // < 0.1
        EXPECT_DOUBLE_EQ(line_segment.left.lane[1].dist.getAsDouble(), 0.4); // 0.5
        EXPECT_DOUBLE_EQ(line_segment.left.lane[2].dist.getAsDouble(), 0.7); // 1.2
        EXPECT_DOUBLE_EQ(line_segment.left.lane[3].dist.getAsDouble(), 1.3); // 2.5

        line_segment.InsertItem(true, 1.8, item_);
        EXPECT_DOUBLE_EQ(line_segment.left.lane[0].dist.getAsDouble(), 0.1); // 0.1
        EXPECT_DOUBLE_EQ(line_segment.left.lane[1].dist.getAsDouble(), 0.4); // 0.5
        EXPECT_DOUBLE_EQ(line_segment.left.lane[2].dist.getAsDouble(), 0.7); // 1.2
        EXPECT_DOUBLE_EQ(line_segment.left.lane[3].dist.getAsDouble(), 0.6); // < 1.8
        EXPECT_DOUBLE_EQ(line_segment.left.lane[4].dist.getAsDouble(), 0.7); // 2.5
    }

    TEST_F(ConveyorStructTest, InsertItemAbs) {
        ConveyorStruct line_segment(Orientation::up, ConveyorStruct::TerminationType::bend_right, 5);

        line_segment.headOffset = 2;

        // Offset is ALWAYS from the beginning of the conveyor + itemOffset

        line_segment.InsertItemAbs(true, 1.2, item_);
        EXPECT_DOUBLE_EQ(line_segment.left.lane[0].dist.getAsDouble(), 3.2);

        line_segment.InsertItemAbs(true, 1.5, item_);
        EXPECT_DOUBLE_EQ(line_segment.left.lane[0].dist.getAsDouble(), 3.2);
        EXPECT_DOUBLE_EQ(line_segment.left.lane[1].dist.getAsDouble(), 0.3); // 1.5
    }

    TEST_F(ConveyorStructTest, TryInsertItem) {
        ConveyorStruct line_segment(Orientation::up, ConveyorStruct::TerminationType::bend_right, 5);

        // Offset is ALWAYS from the beginning of the conveyor
        {
            const bool result = line_segment.TryInsertItem(true, 1.2, item_);
            ASSERT_TRUE(result);
            EXPECT_DOUBLE_EQ(line_segment.left.lane[0].dist.getAsDouble(), 1.2);
        }
        {
            // Too close
            const bool result = line_segment.TryInsertItem(true, 1.3, item_);
            ASSERT_FALSE(result);
        }


        // Should also reenable conveyor upon insertion if it is disabled
        line_segment.left.index = 999;

        {
            const bool result = line_segment.TryInsertItem(true, 0.5, item_);
            ASSERT_TRUE(result);
            EXPECT_DOUBLE_EQ(line_segment.left.lane[0].dist.getAsDouble(), 0.5);
            EXPECT_DOUBLE_EQ(line_segment.left.lane[1].dist.getAsDouble(), 0.7); // 1.2
        }

        EXPECT_EQ(line_segment.left.index, 0);
    }

    TEST_F(ConveyorStructTest, BackItemDistanceLeft) {
        ConveyorStruct line_segment(Orientation::up, ConveyorStruct::TerminationType::bend_right, 5);

        EXPECT_DOUBLE_EQ(line_segment.left.backItemDistance.getAsDouble(), 0);


        // Appending
        line_segment.AppendItem(true, 1.2, item_);
        EXPECT_DOUBLE_EQ(line_segment.left.backItemDistance.getAsDouble(), 1.2);

        line_segment.AppendItem(true, 3, item_);
        EXPECT_DOUBLE_EQ(line_segment.left.backItemDistance.getAsDouble(), 4.2);

        line_segment.AppendItem(true, 1.8, item_);
        EXPECT_DOUBLE_EQ(line_segment.left.backItemDistance.getAsDouble(), 6);

        // Inserting (Starting at 6.f)
        line_segment.InsertItem(true, 7, item_);
        EXPECT_DOUBLE_EQ(line_segment.left.backItemDistance.getAsDouble(), 7);

        line_segment.InsertItem(true, 2, item_);
        EXPECT_DOUBLE_EQ(line_segment.left.backItemDistance.getAsDouble(), 7); // Unchanged

        EXPECT_DOUBLE_EQ(line_segment.right.backItemDistance.getAsDouble(), 0);
    }

    TEST_F(ConveyorStructTest, BackItemDistanceRight) {
        ConveyorStruct line_segment(Orientation::up, ConveyorStruct::TerminationType::bend_right, 5);

        EXPECT_DOUBLE_EQ(line_segment.right.backItemDistance.getAsDouble(), 0.);


        // Appending
        line_segment.AppendItem(false, 1.2, item_);
        EXPECT_DOUBLE_EQ(line_segment.right.backItemDistance.getAsDouble(), 1.2);

        line_segment.AppendItem(false, 3, item_);
        EXPECT_DOUBLE_EQ(line_segment.right.backItemDistance.getAsDouble(), 4.2);

        line_segment.AppendItem(false, 1.8, item_);
        EXPECT_DOUBLE_EQ(line_segment.right.backItemDistance.getAsDouble(), 6);

        // Inserting (Starting at 6.f)
        line_segment.InsertItem(false, 7, item_);
        EXPECT_DOUBLE_EQ(line_segment.right.backItemDistance.getAsDouble(), 7);

        line_segment.InsertItem(false, 2, item_);
        EXPECT_DOUBLE_EQ(line_segment.right.backItemDistance.getAsDouble(), 7); // Unchanged

        EXPECT_DOUBLE_EQ(line_segment.left.backItemDistance.getAsDouble(), 0);
    }

    TEST_F(ConveyorStructTest, GetOffsetAbs) {
        {
            segment_.headOffset = 0;

            ConveyorStruct::FloatOffsetT o = 3;
            segment_.GetOffsetAbs(o);
            EXPECT_DOUBLE_EQ(o, 3.);
        }
        {
            segment_.headOffset = 3;

            ConveyorStruct::FloatOffsetT o = 3;
            segment_.GetOffsetAbs(o);
            EXPECT_DOUBLE_EQ(o, 0.);
        }

        {
            segment_.headOffset = -2;

            ConveyorStruct::IntOffsetT o = 3;
            segment_.GetOffsetAbs(o);
            EXPECT_DOUBLE_EQ(o, 5.);
        }
    }

    TEST_F(ConveyorStructTest, GetItem) {
        auto get_item = [&]() { // true is an item is found
            // Valid range is 1.3 and 1.7 inclusive
            return segment_.GetItem(true, 1.5).second.item != nullptr;
        };

        segment_.AppendItem(true, 0.5, item_);
        segment_.AppendItem(true, 0.5, item_);  // 1.00
        segment_.AppendItem(true, 0.29, item_); // 1.29
        EXPECT_FALSE(get_item());

        segment_.AppendItem(true, 0.42, item_); // 1.71
        EXPECT_FALSE(get_item());
    }

    TEST_F(ConveyorStructTest, RemoveItem) {
        segment_.AppendItem(false, 1, item_);
        segment_.AppendItem(false, 0.25, item_);
        segment_.AppendItem(false, 0.5, item_);

        segment_.RemoveItem(false, 1);
        ASSERT_EQ(segment_.right.lane.size(), 2);
        EXPECT_DOUBLE_EQ(segment_.right.lane[1].dist.getAsDouble(), 0.75);
        EXPECT_TRUE(segment_.left.lane.empty());

        segment_.RemoveItem(false, 1);
        ASSERT_EQ(segment_.right.lane.size(), 1);
        EXPECT_DOUBLE_EQ(segment_.right.lane[0].dist.getAsDouble(), 1);
        EXPECT_TRUE(segment_.left.lane.empty());
    }

    TEST_F(ConveyorStructTest, TryPopItem) {
        EXPECT_EQ(segment_.TryPopItem(true, 0.25), nullptr);
        EXPECT_EQ(segment_.TryPopItem(false, 0.25), nullptr);

        // Pop off appended item
        segment_.AppendItem(true, 0.3, item_);

        EXPECT_EQ(segment_.TryPopItem(true, 0.4, 0.1), &item_);
        ASSERT_TRUE(segment_.left.lane.empty());

        //

        proto::Item item2;
        segment_.AppendItem(true, 0.1, item_);
        segment_.AppendItem(true, 0.8, item2); // 0.9
        segment_.AppendItem(true, 0.9, item_); // 1.8

        EXPECT_EQ(segment_.TryPopItem(true, 0.9, 0.7), &item2);
        ASSERT_EQ(segment_.left.lane.size(), 2);

        // Should preserve spacing
        EXPECT_DOUBLE_EQ(segment_.left.lane[1].dist.getAsDouble(), 1.7);
    }

    TEST_F(ConveyorStructTest, Serialize) {
        data::PrototypeManager proto;
        auto& item = proto.Make<proto::Item>();

        // Unique ptr for cereal load construct
        auto segment =
            std::make_unique<ConveyorStruct>(Orientation::down, ConveyorStruct::TerminationType::bend_left, 4);

        segment->AppendItem(true, 0.43, item);
        segment->AppendItem(true, 0.43, item);
        segment->left.backItemDistance = 65.456;
        segment->left.index            = 40;
        segment->left.visible          = true;

        segment->AppendItem(false, 0.223, item);
        segment->right.index            = 50;
        segment->right.backItemDistance = 23.456;
        segment->right.visible          = false;


        data::active_prototype_manager = &proto;
        proto.GenerateRelocationTable();

        // ======================================================================
        auto result = TestSerializeDeserialize(segment);

        EXPECT_EQ(result->direction, Orientation::down);
        EXPECT_EQ(result->terminationType, ConveyorStruct::TerminationType::bend_left);
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
