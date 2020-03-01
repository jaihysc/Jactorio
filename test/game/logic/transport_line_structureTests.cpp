//
// Created on 2/29/2020.
// This file is subject to the terms and conditions defined in 'LICENSE' included in the source code package
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


TEST(transport_line_structure, can_insert_filled_transport_line) {
	// THe entire transport line is compressed with items, cannot insert
	TRANSPORT_LINE_SEGMENT

	// At spacing of 0.25, 4 items per segment
	transport_line_c::belt_insert_item(true, segment, 0.f, item_proto.get());
	transport_line_c::belt_insert_item(true, segment, transport_line_c::item_spacing, item_proto.get());
	transport_line_c::belt_insert_item(true, segment, transport_line_c::item_spacing, item_proto.get());
	transport_line_c::belt_insert_item(true, segment, transport_line_c::item_spacing, item_proto.get());

	transport_line_c::belt_insert_item(true, segment, transport_line_c::item_spacing, item_proto.get());
	transport_line_c::belt_insert_item(true, segment, transport_line_c::item_spacing, item_proto.get());
	transport_line_c::belt_insert_item(true, segment, transport_line_c::item_spacing, item_proto.get());
	transport_line_c::belt_insert_item(true, segment, transport_line_c::item_spacing, item_proto.get());

//	transport_line_c::belt_insert_item(true, segment, transport_line_c::item_spacing, item_proto.get());
//	transport_line_c::belt_insert_item(true, segment, transport_line_c::item_spacing, item_proto.get());
//	transport_line_c::belt_insert_item(true, segment, transport_line_c::item_spacing, item_proto.get());
//	transport_line_c::belt_insert_item(true, segment, transport_line_c::item_spacing, item_proto.get());

	// Location 1.75 tiles from beginning of transport line is filled
	EXPECT_FALSE(segment->can_insert(true, 1.75));
}

TEST(transport_line_structure, can_insert_empty_transport_line) {
	// THe entire transport line is empty and can thus insert
	TRANSPORT_LINE_SEGMENT
	EXPECT_TRUE(segment->can_insert(false, 1.75));
}

TEST(transport_line_structure, can_insert_gap) {
	// Insert into a gap between 1 and 1.5
	// Is wide enough for the item (item_width - item_spacing) to fit there
	TRANSPORT_LINE_SEGMENT

	transport_line_c::belt_insert_item(false, segment, 0.f, item_proto.get());
	transport_line_c::belt_insert_item(false, segment, 1.f, item_proto.get());
	// Items can be inserted in this 0.5 gap
	transport_line_c::belt_insert_item(false, segment, 0.5f, item_proto.get());


	// Overlaps with the item at 1 by 0.01
	EXPECT_FALSE(segment->can_insert(false, transport_line_c::item_spacing - 0.01));
	// Will overlap with item at 1.5
	EXPECT_FALSE(segment->can_insert(false, 1.45));


	// Sufficient space ahead and behind
	EXPECT_TRUE(segment->can_insert(false, 1.25));
}
