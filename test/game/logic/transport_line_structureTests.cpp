// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#include <gtest/gtest.h>

#include "data/prototype/entity/transport/transport_belt.h"
#include "game/logic/transport_line_controller.h"
#include "game/logic/transport_line_structure.h"
#include "game/world/world_data.h"

namespace game
{
	class TransportLineStructureTest : public testing::Test
	{
	protected:
		std::unique_ptr<jactorio::data::Item> item_proto_ = std::make_unique<jactorio::data::Item>();
		std::unique_ptr<jactorio::data::Transport_belt> transport_belt_proto_ = std::make_unique<jactorio::data::Transport_belt>();
		
		jactorio::game::World_data world_data_{};

		jactorio::game::Transport_line_segment* segment_ = 
			new jactorio::game::Transport_line_segment(jactorio::data::Orientation::left,
													   jactorio::game::Transport_line_segment::TerminationType::straight,
													   2);

		void SetUp() override {
			transport_belt_proto_->speed = 0.01f;
			
			auto chunk = jactorio::game::Chunk(0, 0);
			auto* logic_chunk = &world_data_.logic_add_chunk(&chunk);

			// Add the transport segment
			auto& structs = logic_chunk->get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);
			auto& seg_1 = structs.emplace_back(transport_belt_proto_.get(), 0, 0);
			seg_1.unique_data = segment_;
		}
	};

	TEST_F(TransportLineStructureTest, CanInsertFilledTransportLine) {
		// THe entire transport line is compressed with items, cannot insert

		// At spacing of 0.25, 4 items per segment
		segment_->append_item(true, 0.f, item_proto_.get());
		segment_->append_item(true, jactorio::game::item_spacing, item_proto_.get());
		segment_->append_item(true, jactorio::game::item_spacing, item_proto_.get());
		segment_->append_item(true, jactorio::game::item_spacing, item_proto_.get());

		segment_->append_item(true, jactorio::game::item_spacing, item_proto_.get());
		segment_->append_item(true, jactorio::game::item_spacing, item_proto_.get());
		segment_->append_item(true, jactorio::game::item_spacing, item_proto_.get());
		segment_->append_item(true, jactorio::game::item_spacing, item_proto_.get());

		//	segment->append_item(true, item_spacing, item_proto.get());
		//	segment->append_item(true, item_spacing, item_proto.get());
		//	segment->append_item(true, item_spacing, item_proto.get());
		//	segment->append_item(true, item_spacing, item_proto.get());

		// Location 1.75 tiles from beginning of transport line is filled
		EXPECT_FALSE(segment_->can_insert(true,
			dec::decimal_cast<jactorio::transport_line_decimal_place>(1.75)));
	}

	TEST_F(TransportLineStructureTest, CanInsertEmptyTransportLine) {
		// THe entire transport line is empty and can thus insert
		EXPECT_TRUE(
			segment_->can_insert(false, dec::decimal_cast<jactorio::transport_line_decimal_place>(1.75)));
	}

	TEST_F(TransportLineStructureTest, CanInsertGap) {
		// Insert into a gap between 1 and 1.5
		// Is wide enough for the item (item_width - item_spacing) to fit there

		segment_->append_item(false, 0.f, item_proto_.get());
		segment_->append_item(false, 1.f, item_proto_.get());
		// Items can be inserted in this 0.5 gap
		segment_->append_item(false, 0.5f, item_proto_.get());


		// Overlaps with the item at 1 by 0.01
		EXPECT_FALSE(segment_->can_insert(false,
			dec::decimal_cast<jactorio::transport_line_decimal_place>(jactorio::game::item_spacing - 0.01)));
		// Will overlap with item at 1.5
		EXPECT_FALSE(segment_->can_insert(false,
			dec::decimal_cast<jactorio::transport_line_decimal_place>(1.45)));


		// Sufficient space ahead and behind
		EXPECT_TRUE(segment_->can_insert(false,
			dec::decimal_cast<jactorio::transport_line_decimal_place>(1.25)));
	}

	TEST_F(TransportLineStructureTest, CanInsertFirstItem) {
		// The first item which is appended ignores an additional offset of item_spacing when calculating
		// whether or not it can be inserted 

		const jactorio::transport_line_offset offset{0.f};

		bool result = segment_->can_insert(true, offset);  // Ok, Offset can be 0, is first item
		EXPECT_TRUE(result);

		segment_->append_item(true, 0, item_proto_.get());
		result = segment_->can_insert(true, offset);  // Not ok, offset changed to item_spacing
		EXPECT_FALSE(result);

		segment_->append_item(true, 0, item_proto_.get());
		result = segment_->can_insert(true, offset);  // Not ok, offset changed to item_spacing
		EXPECT_FALSE(result);
	}


	TEST_F(TransportLineStructureTest, IsActive) {
		// Insert into a gap between 1 and 1.5
		// Is wide enough for the item (item_width - item_spacing) to fit there

		EXPECT_FALSE(segment_->is_active_left());
		EXPECT_FALSE(segment_->is_active_right());

		segment_->append_item(false, 0.f, item_proto_.get());
		segment_->append_item(true, 0.f, item_proto_.get());

		// Has items, now active
		EXPECT_TRUE(segment_->is_active_left());
		EXPECT_TRUE(segment_->is_active_right());

		// Invalid indices, inactive
		segment_->l_index = 100;
		segment_->r_index = 100;
		EXPECT_FALSE(segment_->is_active_left());
		EXPECT_FALSE(segment_->is_active_right());
	}

	TEST_F(TransportLineStructureTest, AppendItem) {
		auto line_segment = std::make_unique<jactorio::game::Transport_line_segment>(
			jactorio::data::Orientation::up,
			jactorio::game::Transport_line_segment::TerminationType::bend_right,
			5
		);

		// Offset is from the beginning of the transport line OR the previous item if it exists
		line_segment->append_item(true, 1.3, item_proto_.get());
		EXPECT_FLOAT_EQ(line_segment.get()->left[0].first.getAsDouble(), 1.3);

		line_segment->append_item(true, 1.2, item_proto_.get());
		EXPECT_FLOAT_EQ(line_segment.get()->left[1].first.getAsDouble(), 1.2);

		line_segment->append_item(true, 1.5, item_proto_.get());
		EXPECT_FLOAT_EQ(line_segment.get()->left[2].first.getAsDouble(), 1.5);

		line_segment->append_item(true, 0.5, item_proto_.get());
		EXPECT_FLOAT_EQ(line_segment.get()->left[3].first.getAsDouble(), 0.5);
	}

	TEST_F(TransportLineStructureTest, AppendItemFirstItem) {
		// The first item which is appended ignores an additional offset of item_spacing when calculating
		// whether or not it can be appended

		auto line_segment = std::make_unique<jactorio::game::Transport_line_segment>(
			jactorio::data::Orientation::up,
			jactorio::game::Transport_line_segment::TerminationType::bend_right,
			5
		);

		line_segment->append_item(true, 0, item_proto_.get());  // Ok, Offset can be 0, is first item
		EXPECT_FLOAT_EQ(line_segment.get()->left[0].first.getAsDouble(), 0);

		line_segment->append_item(true, 0, item_proto_.get());  // Not ok, offset changed to item_spacing
		EXPECT_FLOAT_EQ(line_segment.get()->left[1].first.getAsDouble(), jactorio::game::item_spacing);

		line_segment->append_item(true, 0, item_proto_.get());  // Not ok, offset changed to item_spacing
		EXPECT_FLOAT_EQ(line_segment.get()->left[2].first.getAsDouble(), jactorio::game::item_spacing);
	}

	TEST_F(TransportLineStructureTest, InsertItem) {
		// Insert INSERTS an item at an arbitrary position offset from the beginning of the transport line

		auto line_segment = std::make_unique<jactorio::game::Transport_line_segment>(
			jactorio::data::Orientation::up,
			jactorio::game::Transport_line_segment::TerminationType::bend_right,
			5
		);

		// Offset is ALWAYS from the beginning of the transport line

		line_segment->insert_item(true, 1.2, item_proto_.get());
		EXPECT_FLOAT_EQ(line_segment.get()->left[0].first.getAsDouble(), 1.2);

		// Should be sorted by items closest to the end of the segment
		line_segment->insert_item(true, 1.5, item_proto_.get());
		EXPECT_FLOAT_EQ(line_segment.get()->left[0].first.getAsDouble(), 1.2);
		EXPECT_FLOAT_EQ(line_segment.get()->left[1].first.getAsDouble(), 0.3);  // 1.5

		line_segment->insert_item(true, 0.5, item_proto_.get());
		EXPECT_FLOAT_EQ(line_segment.get()->left[0].first.getAsDouble(), 0.5);
		EXPECT_FLOAT_EQ(line_segment.get()->left[1].first.getAsDouble(), 0.7);  // 1.2
		EXPECT_FLOAT_EQ(line_segment.get()->left[2].first.getAsDouble(), 0.3);  // 1.5

		line_segment->insert_item(true, 0.1, item_proto_.get());
		EXPECT_FLOAT_EQ(line_segment.get()->left[0].first.getAsDouble(), 0.1);
		EXPECT_FLOAT_EQ(line_segment.get()->left[1].first.getAsDouble(), 0.4);  // 0.5
		EXPECT_FLOAT_EQ(line_segment.get()->left[2].first.getAsDouble(), 0.7);  // 1.2
		EXPECT_FLOAT_EQ(line_segment.get()->left[3].first.getAsDouble(), 0.3);  // 1.5
	}

	TEST_F(TransportLineStructureTest, TryInsertItem) {

		auto line_segment = std::make_unique<jactorio::game::Transport_line_segment>(
			jactorio::data::Orientation::up,
			jactorio::game::Transport_line_segment::TerminationType::bend_right,
			5
		);

		// Offset is ALWAYS from the beginning of the transport line
		{
			const bool result = line_segment->try_insert_item(true, 1.2, item_proto_.get());
			ASSERT_TRUE(result);
			EXPECT_FLOAT_EQ(line_segment.get()->left[0].first.getAsDouble(), 1.2);
		}
		{
			// Too close
			const bool result = line_segment->try_insert_item(true, 1.3, item_proto_.get());
			ASSERT_FALSE(result);
		}


		// Should also reenable transport line upon insertion if it is disabled
		line_segment->l_index = 999;

		{
			const bool result = line_segment->try_insert_item(true, 0.5, item_proto_.get());
			ASSERT_TRUE(result);
			EXPECT_FLOAT_EQ(line_segment.get()->left[0].first.getAsDouble(), 0.5);
			EXPECT_FLOAT_EQ(line_segment.get()->left[1].first.getAsDouble(), 0.7);  // 1.2
		}

		EXPECT_EQ(line_segment->l_index, 0);
	}

	TEST_F(TransportLineStructureTest, BackItemDistanceLeft) {

		auto line_segment = std::make_unique<jactorio::game::Transport_line_segment>(
			jactorio::data::Orientation::up,
			jactorio::game::Transport_line_segment::TerminationType::bend_right,
			5
		);

		EXPECT_FLOAT_EQ(line_segment->l_back_item_distance.getAsDouble(), 0.f);


		// Appending
		line_segment->append_item(true, 1.2, item_proto_.get());
		EXPECT_FLOAT_EQ(line_segment->l_back_item_distance.getAsDouble(), 1.2f);

		line_segment->append_item(true, 3, item_proto_.get());
		EXPECT_FLOAT_EQ(line_segment->l_back_item_distance.getAsDouble(), 4.2f);

		line_segment->append_item(true, 1.8, item_proto_.get());
		EXPECT_FLOAT_EQ(line_segment->l_back_item_distance.getAsDouble(), 6.f);

		// Inserting (Starting at 6.f)
		line_segment->insert_item(true, 7, item_proto_.get());
		EXPECT_FLOAT_EQ(line_segment->l_back_item_distance.getAsDouble(), 7.f);

		line_segment->insert_item(true, 2, item_proto_.get());
		EXPECT_FLOAT_EQ(line_segment->l_back_item_distance.getAsDouble(), 7.f);  // Unchanged

		EXPECT_FLOAT_EQ(line_segment->r_back_item_distance.getAsDouble(), 0.f);
	}

	TEST_F(TransportLineStructureTest, BackItemDistanceRight) {

		auto line_segment = std::make_unique<jactorio::game::Transport_line_segment>(
			jactorio::data::Orientation::up,
			jactorio::game::Transport_line_segment::TerminationType::bend_right,
			5
		);

		EXPECT_FLOAT_EQ(line_segment->r_back_item_distance.getAsDouble(), 0.f);


		// Appending
		line_segment->append_item(false, 1.2, item_proto_.get());
		EXPECT_FLOAT_EQ(line_segment->r_back_item_distance.getAsDouble(), 1.2f);

		line_segment->append_item(false, 3, item_proto_.get());
		EXPECT_FLOAT_EQ(line_segment->r_back_item_distance.getAsDouble(), 4.2f);

		line_segment->append_item(false, 1.8, item_proto_.get());
		EXPECT_FLOAT_EQ(line_segment->r_back_item_distance.getAsDouble(), 6.f);

		// Inserting (Starting at 6.f)
		line_segment->insert_item(false, 7, item_proto_.get());
		EXPECT_FLOAT_EQ(line_segment->r_back_item_distance.getAsDouble(), 7.f);

		line_segment->insert_item(false, 2, item_proto_.get());
		EXPECT_FLOAT_EQ(line_segment->r_back_item_distance.getAsDouble(), 7.f);  // Unchanged

		EXPECT_FLOAT_EQ(line_segment->l_back_item_distance.getAsDouble(), 0.f);
	}
}
