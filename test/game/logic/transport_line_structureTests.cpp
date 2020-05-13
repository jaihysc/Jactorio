// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#include <gtest/gtest.h>

#include "data/prototype/entity/transport/transport_belt.h"
#include "game/logic/transport_line_controller.h"
#include "game/logic/transport_line_structure.h"
#include "game/world/world_data.h"

namespace game
{
	class TransportStructureTest : public testing::Test
	{
	protected:
		std::unique_ptr<jactorio::data::Item> itemProto_                   = std::make_unique<jactorio::data::Item>();
		std::unique_ptr<jactorio::data::TransportBelt> transportBeltProto_ = std::make_unique<jactorio::data::TransportBelt>();

		jactorio::game::WorldData worldData_{};

		jactorio::game::TransportSegment* segment_ =
			new jactorio::game::TransportSegment(jactorio::data::Orientation::left,
			                                     jactorio::game::TransportSegment::TerminationType::straight,
			                                     2);

		void SetUp() override {
			transportBeltProto_->speed = 0.01f;

			auto chunk        = jactorio::game::Chunk(0, 0);
			auto* logic_chunk = &worldData_.LogicAddChunk(&chunk);

			// Add the transport segment
			auto& structs    = logic_chunk->GetStruct(jactorio::game::LogicChunk::StructLayer::transport_line);
			auto& seg_1      = structs.emplace_back(transportBeltProto_.get(), 0, 0);
			seg_1.uniqueData = segment_;
		}
	};

	TEST_F(TransportStructureTest, CanInsertFilledTransportLine) {
		// THe entire transport line is compressed with items, cannot insert

		// At spacing of 0.25, 4 items per segment
		segment_->AppendItem(true, 0.f, itemProto_.get());
		segment_->AppendItem(true, jactorio::game::kItemSpacing, itemProto_.get());
		segment_->AppendItem(true, jactorio::game::kItemSpacing, itemProto_.get());
		segment_->AppendItem(true, jactorio::game::kItemSpacing, itemProto_.get());

		segment_->AppendItem(true, jactorio::game::kItemSpacing, itemProto_.get());
		segment_->AppendItem(true, jactorio::game::kItemSpacing, itemProto_.get());
		segment_->AppendItem(true, jactorio::game::kItemSpacing, itemProto_.get());
		segment_->AppendItem(true, jactorio::game::kItemSpacing, itemProto_.get());

		// Location 1.75 tiles from beginning of transport line is filled
		EXPECT_FALSE(segment_->CanInsert(true,
			dec::decimal_cast<jactorio::game::kTransportLineDecimalPlace>(1.75)));
	}

	TEST_F(TransportStructureTest, CanInsertEmptyTransportLine) {
		// THe entire transport line is empty and can thus insert
		EXPECT_TRUE(
			segment_->CanInsert(false, dec::decimal_cast<jactorio::game::kTransportLineDecimalPlace>(1.75)));
	}

	TEST_F(TransportStructureTest, CanInsertGap) {
		// Insert into a gap between 1 and 1.5
		// Is wide enough for the item (item_width - kItemSpacing) to fit there

		segment_->AppendItem(false, 0.f, itemProto_.get());
		segment_->AppendItem(false, 1.f, itemProto_.get());
		// Items can be inserted in this 0.5 gap
		segment_->AppendItem(false, 0.5f, itemProto_.get());


		// Overlaps with the item at 1 by 0.01
		EXPECT_FALSE(segment_->CanInsert(false,
			dec::decimal_cast<jactorio::game::kTransportLineDecimalPlace>(jactorio::game::kItemSpacing - 0.01)));
		// Will overlap with item at 1.5
		EXPECT_FALSE(segment_->CanInsert(false,
			dec::decimal_cast<jactorio::game::kTransportLineDecimalPlace>(1.45)));


		// Sufficient space ahead and behind
		EXPECT_TRUE(segment_->CanInsert(false,
			dec::decimal_cast<jactorio::game::kTransportLineDecimalPlace>(1.25)));
	}

	TEST_F(TransportStructureTest, CanInsertFirstItem) {
		// The first item which is appended ignores an additional offset of kItemSpacing when calculating
		// whether or not it can be inserted 

		const jactorio::game::TransportLineOffset offset{0.f};

		bool result = segment_->CanInsert(true, offset);  // Ok, Offset can be 0, is first item
		EXPECT_TRUE(result);

		segment_->AppendItem(true, 0, itemProto_.get());
		result = segment_->CanInsert(true, offset);  // Not ok, offset changed to kItemSpacing
		EXPECT_FALSE(result);

		segment_->AppendItem(true, 0, itemProto_.get());
		result = segment_->CanInsert(true, offset);  // Not ok, offset changed to kItemSpacing
		EXPECT_FALSE(result);
	}

	/*
	TEST_F(TransportStructureTest, CanInsertOffset) {
		// the given offset should be adjusted as TransportSegment::itemOffset is adjusted where 1 = 1 tile
		segment_->itemOffset = 1;
		const jactorio::game::TransportLineOffset offset{0.f};

		segment_->AppendItem(true, 0, itemProto_.get());
		bool result = segment_->CanInsert(true, offset);  // Ok, offset (0) + itemOffset (1) = 1 
		EXPECT_TRUE(result);
	}
	*/


	TEST_F(TransportStructureTest, IsActive) {
		// Insert into a gap between 1 and 1.5
		// Is wide enough for the item (item_width - kItemSpacing) to fit there

		EXPECT_FALSE(segment_->left.IsActive());
		EXPECT_FALSE(segment_->right.IsActive());

		segment_->AppendItem(false, 0.f, itemProto_.get());
		segment_->AppendItem(true, 0.f, itemProto_.get());

		// Has items, now active
		EXPECT_TRUE(segment_->left.IsActive());
		EXPECT_TRUE(segment_->right.IsActive());

		// Invalid indices, inactive
		segment_->left.index  = 100;
		segment_->right.index = 100;
		EXPECT_FALSE(segment_->left.IsActive());
		EXPECT_FALSE(segment_->right.IsActive());
	}

	TEST_F(TransportStructureTest, AppendItem) {
		auto line_segment = std::make_unique<jactorio::game::TransportSegment>(
			jactorio::data::Orientation::up,
			jactorio::game::TransportSegment::TerminationType::bend_right,
			5
		);

		// Offset is from the beginning of the transport line OR the previous item if it exists
		line_segment->AppendItem(true, 1.3, itemProto_.get());
		EXPECT_FLOAT_EQ(line_segment.get()->left.lane[0].first.getAsDouble(), 1.3);

		line_segment->AppendItem(true, 1.2, itemProto_.get());
		EXPECT_FLOAT_EQ(line_segment.get()->left.lane[1].first.getAsDouble(), 1.2);

		line_segment->AppendItem(true, 1.5, itemProto_.get());
		EXPECT_FLOAT_EQ(line_segment.get()->left.lane[2].first.getAsDouble(), 1.5);

		line_segment->AppendItem(true, 0.5, itemProto_.get());
		EXPECT_FLOAT_EQ(line_segment.get()->left.lane[3].first.getAsDouble(), 0.5);
	}

	TEST_F(TransportStructureTest, AppendItemFirstItem) {
		// The first item which is appended ignores an additional offset of kItemSpacing when calculating
		// whether or not it can be appended

		auto line_segment = std::make_unique<jactorio::game::TransportSegment>(
			jactorio::data::Orientation::up,
			jactorio::game::TransportSegment::TerminationType::bend_right,
			5
		);

		line_segment->AppendItem(true, 0, itemProto_.get());  // Ok, Offset can be 0, is first item
		EXPECT_FLOAT_EQ(line_segment.get()->left.lane[0].first.getAsDouble(), 0);

		line_segment->AppendItem(true, 0, itemProto_.get());  // Not ok, offset changed to kItemSpacing
		EXPECT_FLOAT_EQ(line_segment.get()->left.lane[1].first.getAsDouble(), jactorio::game::kItemSpacing);

		line_segment->AppendItem(true, 0, itemProto_.get());  // Not ok, offset changed to kItemSpacing
		EXPECT_FLOAT_EQ(line_segment.get()->left.lane[2].first.getAsDouble(), jactorio::game::kItemSpacing);
	}

	TEST_F(TransportStructureTest, InsertItem) {
		// Insert INSERTS an item at an arbitrary position offset from the beginning of the transport line

		auto line_segment = std::make_unique<jactorio::game::TransportSegment>(
			jactorio::data::Orientation::up,
			jactorio::game::TransportSegment::TerminationType::bend_right,
			5
		);

		// Offset is ALWAYS from the beginning of the transport line

		line_segment->InsertItem(true, 1.2, itemProto_.get());
		EXPECT_FLOAT_EQ(line_segment.get()->left.lane[0].first.getAsDouble(), 1.2);

		// Should be sorted by items closest to the end of the segment
		line_segment->InsertItem(true, 1.5, itemProto_.get());
		EXPECT_FLOAT_EQ(line_segment.get()->left.lane[0].first.getAsDouble(), 1.2);
		EXPECT_FLOAT_EQ(line_segment.get()->left.lane[1].first.getAsDouble(), 0.3);  // 1.5

		line_segment->InsertItem(true, 0.5, itemProto_.get());
		EXPECT_FLOAT_EQ(line_segment.get()->left.lane[0].first.getAsDouble(), 0.5);
		EXPECT_FLOAT_EQ(line_segment.get()->left.lane[1].first.getAsDouble(), 0.7);  // 1.2
		EXPECT_FLOAT_EQ(line_segment.get()->left.lane[2].first.getAsDouble(), 0.3);  // 1.5

		line_segment->InsertItem(true, 0.1, itemProto_.get());
		EXPECT_FLOAT_EQ(line_segment.get()->left.lane[0].first.getAsDouble(), 0.1);
		EXPECT_FLOAT_EQ(line_segment.get()->left.lane[1].first.getAsDouble(), 0.4);  // 0.5
		EXPECT_FLOAT_EQ(line_segment.get()->left.lane[2].first.getAsDouble(), 0.7);  // 1.2
		EXPECT_FLOAT_EQ(line_segment.get()->left.lane[3].first.getAsDouble(), 0.3);  // 1.5
	}

	/*
	TEST_F(TransportStructureTest, InsertItemOffset) {
		auto line_segment = std::make_unique<jactorio::game::TransportSegment>(
			jactorio::data::Orientation::up,
			jactorio::game::TransportSegment::TerminationType::bend_right,
			5
		);
		line_segment->itemOffset = 2;

		// Offset is ALWAYS from the beginning of the transport line + itemOffset

		line_segment->InsertItem(true, 1.2, itemProto_.get());
		EXPECT_FLOAT_EQ(line_segment.get()->left.lane[0].first.getAsDouble(), 3.2);

		line_segment->InsertItem(true, 1.5, itemProto_.get());
		EXPECT_FLOAT_EQ(line_segment.get()->left.lane[0].first.getAsDouble(), 3.2);
		EXPECT_FLOAT_EQ(line_segment.get()->left.lane[1].first.getAsDouble(), 0.3);  // 1.5
	}
	*/

	TEST_F(TransportStructureTest, TryInsertItem) {

		auto line_segment = std::make_unique<jactorio::game::TransportSegment>(
			jactorio::data::Orientation::up,
			jactorio::game::TransportSegment::TerminationType::bend_right,
			5
		);

		// Offset is ALWAYS from the beginning of the transport line
		{
			const bool result = line_segment->TryInsertItem(true, 1.2, itemProto_.get());
			ASSERT_TRUE(result);
			EXPECT_FLOAT_EQ(line_segment.get()->left.lane[0].first.getAsDouble(), 1.2);
		}
		{
			// Too close
			const bool result = line_segment->TryInsertItem(true, 1.3, itemProto_.get());
			ASSERT_FALSE(result);
		}


		// Should also reenable transport line upon insertion if it is disabled
		line_segment->left.index = 999;

		{
			const bool result = line_segment->TryInsertItem(true, 0.5, itemProto_.get());
			ASSERT_TRUE(result);
			EXPECT_FLOAT_EQ(line_segment.get()->left.lane[0].first.getAsDouble(), 0.5);
			EXPECT_FLOAT_EQ(line_segment.get()->left.lane[1].first.getAsDouble(), 0.7);  // 1.2
		}

		EXPECT_EQ(line_segment->left.index, 0);
	}

	TEST_F(TransportStructureTest, BackItemDistanceLeft) {

		auto line_segment = std::make_unique<jactorio::game::TransportSegment>(
			jactorio::data::Orientation::up,
			jactorio::game::TransportSegment::TerminationType::bend_right,
			5
		);

		EXPECT_FLOAT_EQ(line_segment->left.backItemDistance.getAsDouble(), 0.f);


		// Appending
		line_segment->AppendItem(true, 1.2, itemProto_.get());
		EXPECT_FLOAT_EQ(line_segment->left.backItemDistance.getAsDouble(), 1.2f);

		line_segment->AppendItem(true, 3, itemProto_.get());
		EXPECT_FLOAT_EQ(line_segment->left.backItemDistance.getAsDouble(), 4.2f);

		line_segment->AppendItem(true, 1.8, itemProto_.get());
		EXPECT_FLOAT_EQ(line_segment->left.backItemDistance.getAsDouble(), 6.f);

		// Inserting (Starting at 6.f)
		line_segment->InsertItem(true, 7, itemProto_.get());
		EXPECT_FLOAT_EQ(line_segment->left.backItemDistance.getAsDouble(), 7.f);

		line_segment->InsertItem(true, 2, itemProto_.get());
		EXPECT_FLOAT_EQ(line_segment->left.backItemDistance.getAsDouble(), 7.f);  // Unchanged

		EXPECT_FLOAT_EQ(line_segment->right.backItemDistance.getAsDouble(), 0.f);
	}

	TEST_F(TransportStructureTest, BackItemDistanceRight) {

		auto line_segment = std::make_unique<jactorio::game::TransportSegment>(
			jactorio::data::Orientation::up,
			jactorio::game::TransportSegment::TerminationType::bend_right,
			5
		);

		EXPECT_FLOAT_EQ(line_segment->right.backItemDistance.getAsDouble(), 0.f);


		// Appending
		line_segment->AppendItem(false, 1.2, itemProto_.get());
		EXPECT_FLOAT_EQ(line_segment->right.backItemDistance.getAsDouble(), 1.2f);

		line_segment->AppendItem(false, 3, itemProto_.get());
		EXPECT_FLOAT_EQ(line_segment->right.backItemDistance.getAsDouble(), 4.2f);

		line_segment->AppendItem(false, 1.8, itemProto_.get());
		EXPECT_FLOAT_EQ(line_segment->right.backItemDistance.getAsDouble(), 6.f);

		// Inserting (Starting at 6.f)
		line_segment->InsertItem(false, 7, itemProto_.get());
		EXPECT_FLOAT_EQ(line_segment->right.backItemDistance.getAsDouble(), 7.f);

		line_segment->InsertItem(false, 2, itemProto_.get());
		EXPECT_FLOAT_EQ(line_segment->right.backItemDistance.getAsDouble(), 7.f);  // Unchanged

		EXPECT_FLOAT_EQ(line_segment->left.backItemDistance.getAsDouble(), 0.f);
	}

	TEST_F(TransportStructureTest, AdjustInsertionOffset) {
		{
			segment_->itemOffset = 0;

			jactorio::game::TransportSegment::InsertOffsetT o = 3;
			segment_->AdjustInsertionOffset(o);
			EXPECT_FLOAT_EQ(o, 3.f);
		}
		{
			segment_->itemOffset = 3;

			jactorio::game::TransportSegment::InsertOffsetT o = 3;
			segment_->AdjustInsertionOffset(o);
			EXPECT_FLOAT_EQ(o, 0.f);
		}

		{
			segment_->itemOffset = -2;

			jactorio::game::TransportSegment::InsertOffsetT o = 3;
			segment_->AdjustInsertionOffset(o);
			EXPECT_FLOAT_EQ(o, 5.f);
		}
	}
}
