// 
// transport_line_structureTests.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 02/29/2020
// Last modified: 03/08/2020
// 

#include "core/float_math.h"
#include "data/prototype/entity/transport/transport_belt.h"
#include "game/logic/transport_line_structure.h"
#include "game/logic/transport_line_controller.h"
#include "game/world/world_manager.h"

#include <gtest/gtest.h>


// Common code for setting up a line segment
#define TRANSPORT_LINE_SEGMENT\
    using namespace jactorio::game;\
\
const auto item_proto = std::make_unique<jactorio::data::Item>();\
const auto transport_belt_proto = std::make_unique<jactorio::data::Transport_belt>();\
transport_belt_proto->speed = 0.01f;\
\
jactorio::core::Resource_guard guard(&world_manager::clear_chunk_data);\
\
auto chunk = Chunk(0, 0, nullptr);\
auto* logic_chunk = &world_manager::logic_add_chunk(&chunk);\
\
auto* segment = new jactorio::game::Transport_line_segment(\
    jactorio::game::Transport_line_segment::moveDir::left,\
    jactorio::game::Transport_line_segment::terminationType::straight, 2);\
{\
auto& structs = logic_chunk->get_struct(Logic_chunk::structLayer::transport_line);\
auto& seg_1 = structs.emplace_back(transport_belt_proto.get(), 0, 0);\
seg_1.unique_data = segment;\
}


namespace game
{
	TEST(transport_line_structure, can_insert_filled_transport_line) {
		// THe entire transport line is compressed with items, cannot insert
		TRANSPORT_LINE_SEGMENT

		// At spacing of 0.25, 4 items per segment
		segment->append_item(true, 0.f, item_proto.get());
		segment->append_item(true, transport_line_c::item_spacing, item_proto.get());
		segment->append_item(true, transport_line_c::item_spacing, item_proto.get());
		segment->append_item(true, transport_line_c::item_spacing, item_proto.get());

		segment->append_item(true, transport_line_c::item_spacing, item_proto.get());
		segment->append_item(true, transport_line_c::item_spacing, item_proto.get());
		segment->append_item(true, transport_line_c::item_spacing, item_proto.get());
		segment->append_item(true, transport_line_c::item_spacing, item_proto.get());

		//	segment->append_item(true, transport_line_c::item_spacing, item_proto.get());
		//	segment->append_item(true, transport_line_c::item_spacing, item_proto.get());
		//	segment->append_item(true, transport_line_c::item_spacing, item_proto.get());
		//	segment->append_item(true, transport_line_c::item_spacing, item_proto.get());

		// Location 1.75 tiles from beginning of transport line is filled
		EXPECT_FALSE(segment->can_insert(true,
			dec::decimal_cast<jactorio::transport_line_decimal_place>(1.75)));
	}

	TEST(transport_line_structure, can_insert_empty_transport_line) {
		// THe entire transport line is empty and can thus insert
		TRANSPORT_LINE_SEGMENT
		EXPECT_TRUE(segment->can_insert(false,
			dec::decimal_cast<jactorio::transport_line_decimal_place>(1.75)));
	}

	TEST(transport_line_structure, can_insert_gap) {
		// Insert into a gap between 1 and 1.5
		// Is wide enough for the item (item_width - item_spacing) to fit there
		TRANSPORT_LINE_SEGMENT

		segment->append_item(false, 0.f, item_proto.get());
		segment->append_item(false, 1.f, item_proto.get());
		// Items can be inserted in this 0.5 gap
		segment->append_item(false, 0.5f, item_proto.get());


		// Overlaps with the item at 1 by 0.01
		EXPECT_FALSE(segment->can_insert(false,
			dec::decimal_cast<jactorio::transport_line_decimal_place>(transport_line_c::item_spacing - 0.01)));
		// Will overlap with item at 1.5
		EXPECT_FALSE(segment->can_insert(false,
			dec::decimal_cast<jactorio::transport_line_decimal_place>(1.45)));


		// Sufficient space ahead and behind
		EXPECT_TRUE(segment->can_insert(false,
			dec::decimal_cast<jactorio::transport_line_decimal_place>(1.25)));
	}

	TEST(transport_line_structure, is_active) {
		// Insert into a gap between 1 and 1.5
		// Is wide enough for the item (item_width - item_spacing) to fit there
		TRANSPORT_LINE_SEGMENT

		EXPECT_FALSE(segment->is_active_left());
		EXPECT_FALSE(segment->is_active_right());

		segment->append_item(false, 0.f, item_proto.get());
		segment->append_item(true, 0.f, item_proto.get());

		// Has items, now active
		EXPECT_TRUE(segment->is_active_left());
		EXPECT_TRUE(segment->is_active_right());

		// Invalid indices, inactive
		segment->l_index = 100;
		segment->r_index = 100;
		EXPECT_FALSE(segment->is_active_left());
		EXPECT_FALSE(segment->is_active_right());
	}

	TEST(transport_line_structure, append_item) {
		const auto item_proto = std::make_unique<jactorio::data::Item>();

		auto line_segment = std::make_unique<jactorio::game::Transport_line_segment>(
			jactorio::game::Transport_line_segment::moveDir::up,
			jactorio::game::Transport_line_segment::terminationType::bend_right,
			5
		);

		// Offset is from the beginning of the transport line OR the previous item if it exists
		line_segment->append_item(true, 1.3, item_proto.get());
		EXPECT_FLOAT_EQ(line_segment.get()->left[0].first.getAsDouble(), 1.3);

		line_segment->append_item(true, 1.2, item_proto.get());
		EXPECT_FLOAT_EQ(line_segment.get()->left[1].first.getAsDouble(), 1.2);

		line_segment->append_item(true, 1.5, item_proto.get());
		EXPECT_FLOAT_EQ(line_segment.get()->left[2].first.getAsDouble(), 1.5);

		line_segment->append_item(true, 0.5, item_proto.get());
		EXPECT_FLOAT_EQ(line_segment.get()->left[3].first.getAsDouble(), 0.5);
	}

	TEST(transport_line_structure, insert_item) {
		const auto item_proto = std::make_unique<jactorio::data::Item>();

		auto line_segment = std::make_unique<jactorio::game::Transport_line_segment>(
			jactorio::game::Transport_line_segment::moveDir::up,
			jactorio::game::Transport_line_segment::terminationType::bend_right,
			5
		);

		// Offset is ALWAYS from the beginning of the transport line

		line_segment->insert_item(true, 1.2, item_proto.get());
		EXPECT_FLOAT_EQ(line_segment.get()->left[0].first.getAsDouble(), 1.2);

		// Should be sorted by items closest to the end of the segment
		line_segment->insert_item(true, 1.5, item_proto.get());
		EXPECT_FLOAT_EQ(line_segment.get()->left[0].first.getAsDouble(), 1.2);
		EXPECT_FLOAT_EQ(line_segment.get()->left[1].first.getAsDouble(), 0.3);  // 1.5

		line_segment->insert_item(true, 0.5, item_proto.get());
		EXPECT_FLOAT_EQ(line_segment.get()->left[0].first.getAsDouble(), 0.5);
		EXPECT_FLOAT_EQ(line_segment.get()->left[1].first.getAsDouble(), 0.7);  // 1.2
		EXPECT_FLOAT_EQ(line_segment.get()->left[2].first.getAsDouble(), 0.3);  // 1.5

		line_segment->insert_item(true, 0.1, item_proto.get());
		EXPECT_FLOAT_EQ(line_segment.get()->left[0].first.getAsDouble(), 0.1);
		EXPECT_FLOAT_EQ(line_segment.get()->left[1].first.getAsDouble(), 0.4);  // 0.5
		EXPECT_FLOAT_EQ(line_segment.get()->left[2].first.getAsDouble(), 0.7);  // 1.2
		EXPECT_FLOAT_EQ(line_segment.get()->left[3].first.getAsDouble(), 0.3);  // 1.5
	}
}
