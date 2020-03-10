// 
// transport_line_controllerTests.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 02/13/2020
// Last modified: 03/10/2020
// 

#include <gtest/gtest.h>

#include "game/logic/transport_line_controller.h"
#include "game/world/world_manager.h"
#include "data/prototype/entity/transport/transport_belt.h"
#include "game/logic/transport_line_structure.h"

#include <memory>

namespace game::logic
{
	// For line_logic and line_logic_precision
	void test_item_positions(const jactorio::game::Transport_line_segment* up_segment,
	                         const jactorio::game::Transport_line_segment* right_segment,
	                         const jactorio::game::Transport_line_segment* down_segment,
	                         const jactorio::game::Transport_line_segment* left_segment) {
		using namespace jactorio::game;

		// Moving left at a speed of 0.01f per update, in 250 updates
		// Should reach end of transport line - Next update will change its direction to up

		// End of U | 5 - 2(0.7) / 0.01 = 360
		for (int i = 0; i < 360; ++i) {
			transport_line_c::transport_line_logic_update();
		}
		ASSERT_FLOAT_EQ(up_segment->right.front().first.getAsDouble(), 0.f);
		ASSERT_EQ(up_segment->right.size(), 1);

		// End of R | 4 - 2(0.7) / 0.01 = 260 updates
		for (int i = 0; i < 260; ++i) {
			transport_line_c::transport_line_logic_update();
		}
		ASSERT_FLOAT_EQ(right_segment->right.front().first.getAsDouble(), 0.f);
		ASSERT_EQ(right_segment->right.size(), 1);

		// End of D
		for (int i = 0; i < 360; ++i) {
			transport_line_c::transport_line_logic_update();
		}
		ASSERT_FLOAT_EQ(down_segment->right.front().first.getAsDouble(), 0.f);
		ASSERT_EQ(down_segment->right.size(), 1);

		// End of L 4 - 2(0.7)
		for (int i = 0; i < 260; ++i) {
			transport_line_c::transport_line_logic_update();
		}
		ASSERT_FLOAT_EQ(left_segment->right.front().first.getAsDouble(), 0.f);
		ASSERT_EQ(left_segment->right.size(), 1);
	}

	TEST(transport_line, line_logic) {
		// Tests that items move as expected (within a chunk)
		using namespace jactorio::game;

		const auto item_proto = std::make_unique<jactorio::data::Item>();
		const auto transport_belt_proto = std::make_unique<jactorio::data::Transport_belt>();
		transport_belt_proto->speed = 0.01f;

		jactorio::core::Resource_guard guard(&world_manager::clear_chunk_data);

		auto chunk = Chunk(0, 0, nullptr);
		auto* logic_chunk = &world_manager::logic_add_chunk(&chunk);

		// Segments (Logic chunk must be created first)
		auto* up_segment = new Transport_line_segment(
			Transport_line_segment::moveDir::up,
			Transport_line_segment::terminationType::bend_right,
			5);
		auto* right_segment = new Transport_line_segment(
			Transport_line_segment::moveDir::right,
			Transport_line_segment::terminationType::bend_right,
			4);
		auto* down_segment = new Transport_line_segment(
			Transport_line_segment::moveDir::down,
			Transport_line_segment::terminationType::bend_right,
			5);
		auto* left_segment = new Transport_line_segment(
			Transport_line_segment::moveDir::left,
			Transport_line_segment::terminationType::bend_right,
			4);

		// What each transport segment empties into
		up_segment->target_segment = right_segment;
		right_segment->target_segment = down_segment;
		down_segment->target_segment = left_segment;
		left_segment->target_segment = up_segment;
		{
			auto& up = logic_chunk->get_struct(Logic_chunk::structLayer::transport_line)
			                      .emplace_back(transport_belt_proto.get(), 0, 0);
			up.unique_data = up_segment;

			auto& right = logic_chunk->get_struct(Logic_chunk::structLayer::transport_line)
			                         .emplace_back(transport_belt_proto.get(), 4, 0);
			right.unique_data = right_segment;

			auto& down = logic_chunk->get_struct(Logic_chunk::structLayer::transport_line)
			                        .emplace_back(transport_belt_proto.get(), 4, 5);
			down.unique_data = down_segment;

			auto& left = logic_chunk->get_struct(Logic_chunk::structLayer::transport_line)
			                        .emplace_back(transport_belt_proto.get(), 0, 5);
			left.unique_data = left_segment;
		}

		// The actual lengths of the transport segments != the indicated length as it turns earlier

		// Insert item
		left_segment->append_item(false, 0.f, item_proto.get());

		test_item_positions(up_segment, right_segment, down_segment, left_segment);
	}

	TEST(transport_line, line_logic_precision) {
		// Tests for data type precision representing the distance between items
		using namespace jactorio::game;

		const auto item_proto = std::make_unique<jactorio::data::Item>();
		const auto transport_belt_proto = std::make_unique<jactorio::data::Transport_belt>();
		transport_belt_proto->speed = 0.01f;

		jactorio::core::Resource_guard guard(&world_manager::clear_chunk_data);

		auto chunk = Chunk(0, 0, nullptr);
		auto* logic_chunk = &world_manager::logic_add_chunk(&chunk);

		// Segments (Logic chunk must be created first)
		auto* up_segment = new Transport_line_segment(
			Transport_line_segment::moveDir::up,
			Transport_line_segment::terminationType::bend_right,
			5);
		auto* right_segment = new Transport_line_segment(
			Transport_line_segment::moveDir::right,
			Transport_line_segment::terminationType::bend_right,
			4);
		auto* down_segment = new Transport_line_segment(
			Transport_line_segment::moveDir::down,
			Transport_line_segment::terminationType::bend_right,
			5);
		auto* left_segment = new Transport_line_segment(
			Transport_line_segment::moveDir::left,
			Transport_line_segment::terminationType::bend_right,
			4);

		// What each transport segment empties into
		up_segment->target_segment = right_segment;
		right_segment->target_segment = down_segment;
		down_segment->target_segment = left_segment;
		left_segment->target_segment = up_segment;
		{
			auto& up = logic_chunk->get_struct(Logic_chunk::structLayer::transport_line)
			                      .emplace_back(transport_belt_proto.get(), 0, 0);
			up.unique_data = up_segment;

			auto& right = logic_chunk->get_struct(Logic_chunk::structLayer::transport_line)
			                         .emplace_back(transport_belt_proto.get(), 4, 0);
			right.unique_data = right_segment;

			auto& down = logic_chunk->get_struct(Logic_chunk::structLayer::transport_line)
			                        .emplace_back(transport_belt_proto.get(), 4, 5);
			down.unique_data = down_segment;

			auto& left = logic_chunk->get_struct(Logic_chunk::structLayer::transport_line)
			                        .emplace_back(transport_belt_proto.get(), 0, 5);
			left.unique_data = left_segment;
		}


		// Insert item
		left_segment->append_item(false, 0.f, item_proto.get());

		// Should manage to make 100 000 laps
		for (int i = 0; i < 100000; ++i) {
			test_item_positions(up_segment, right_segment, down_segment, left_segment);

			if (HasFatalFailure()) {
				printf("Precision failed on lap %d/100000\n", i + 1);
				FAIL();
			}
		}
	}

	TEST(transport_line, line_logic_fast) {
		// Same as line logic, but belts are faster (0.06), which seems to break the current logic at the time of writing
		using namespace jactorio::game;

		const auto j_belt_speed = 0.06f;

		const auto item_proto = std::make_unique<jactorio::data::Item>();
		const auto transport_belt_proto = std::make_unique<jactorio::data::Transport_belt>();
		transport_belt_proto->speed = j_belt_speed;  // <---

		jactorio::core::Resource_guard guard(&world_manager::clear_chunk_data);

		auto chunk = Chunk(0, 0, nullptr);
		auto* logic_chunk = &world_manager::logic_add_chunk(&chunk);

		// Segments (Logic chunk must be created first)
		auto* up_segment = new Transport_line_segment(
			Transport_line_segment::moveDir::up,
			Transport_line_segment::terminationType::bend_right,
			5);
		auto* right_segment = new Transport_line_segment(
			Transport_line_segment::moveDir::right,
			Transport_line_segment::terminationType::bend_right,
			5);
		auto* down_segment = new Transport_line_segment(
			Transport_line_segment::moveDir::down,
			Transport_line_segment::terminationType::bend_right,
			5);
		auto* left_segment = new Transport_line_segment(
			Transport_line_segment::moveDir::left,
			Transport_line_segment::terminationType::bend_right,
			5);

		// What each transport segment empties into
		up_segment->target_segment = right_segment;
		right_segment->target_segment = down_segment;
		down_segment->target_segment = left_segment;
		left_segment->target_segment = up_segment;
		{
			auto& up = logic_chunk->get_struct(Logic_chunk::structLayer::transport_line)
			                      .emplace_back(transport_belt_proto.get(), 0, 0);
			up.unique_data = up_segment;

			auto& right = logic_chunk->get_struct(Logic_chunk::structLayer::transport_line)
			                         .emplace_back(transport_belt_proto.get(), 4, 0);
			right.unique_data = right_segment;

			auto& down = logic_chunk->get_struct(Logic_chunk::structLayer::transport_line)
			                        .emplace_back(transport_belt_proto.get(), 4, 5);
			down.unique_data = down_segment;

			auto& left = logic_chunk->get_struct(Logic_chunk::structLayer::transport_line)
			                        .emplace_back(transport_belt_proto.get(), 0, 5);
			left.unique_data = left_segment;
		}

		// Logic
		left_segment->append_item(true, 0.f, item_proto.get());
		left_segment->append_item(true, transport_line_c::item_spacing, item_proto.get());
		left_segment->append_item(true, transport_line_c::item_spacing, item_proto.get());

		// 1 update
		// first item moved to up segment
		transport_line_c::transport_line_logic_update();
		ASSERT_EQ(up_segment->left.size(), 1);
		ASSERT_EQ(left_segment->left.size(), 2);

		EXPECT_FLOAT_EQ(up_segment->left[0].first.getAsDouble(), 4.40 - j_belt_speed);
		EXPECT_FLOAT_EQ(left_segment->left[0].first.getAsDouble(), 0.25 - j_belt_speed);
		EXPECT_FLOAT_EQ(left_segment->left[1].first.getAsDouble(), 0.25);

		// 2 updates | 0.12
		for (int i = 0; i < 2; ++i) {
			transport_line_c::transport_line_logic_update();
		}
		ASSERT_EQ(up_segment->left.size(), 1);
		ASSERT_EQ(left_segment->left.size(), 2);

		EXPECT_FLOAT_EQ(up_segment->left[0].first.getAsDouble(), 4.40 - (3 * j_belt_speed));
		EXPECT_FLOAT_EQ(left_segment->left[0].first.getAsDouble(), 0.25 - (3 * j_belt_speed));
		EXPECT_FLOAT_EQ(left_segment->left[1].first.getAsDouble(), 0.25);


		// 2 updates | Total distance = 4(0.06) = 0.24
		// second item moved to up segment
		for (int i = 0; i < 2; ++i) {
			transport_line_c::transport_line_logic_update();
		}
		ASSERT_EQ(up_segment->left.size(), 2);
		ASSERT_EQ(left_segment->left.size(), 1);

		EXPECT_FLOAT_EQ(up_segment->left[0].first.getAsDouble(), 4.40 - (5 * j_belt_speed));
		EXPECT_FLOAT_EQ(up_segment->left[1].first.getAsDouble(), 0.25);  // Spacing maintained
		// Item 2 was 0.01 -> -0.05
		// | -0.05 - 0.20 | = 0.25 Maintains distance
		EXPECT_FLOAT_EQ(left_segment->left[0].first.getAsDouble(), 0.20);
	}


	TEST(transport_line, line_logic_right_bend) {
		// Validates the correct handling of multiple items across transport lines
		// The spacing between items should be maintained
		using namespace jactorio::game;

		const auto item_proto = std::make_unique<jactorio::data::Item>();
		const auto transport_belt_proto = std::make_unique<jactorio::data::Transport_belt>();
		transport_belt_proto->speed = 0.01f;

		jactorio::core::Resource_guard guard(&world_manager::clear_chunk_data);

		auto chunk = Chunk(0, 0, nullptr);
		auto* logic_chunk = &world_manager::logic_add_chunk(&chunk);

		/*
		 *    --------- RIGHT -------- >
		 *    ^
		 *    |
		 *    | UP
		 *    |
		 *    |
		 */

		auto* up_segment = new Transport_line_segment(
			Transport_line_segment::moveDir::up,
			Transport_line_segment::terminationType::bend_right,
			4);
		auto* right_segment = new Transport_line_segment(
			Transport_line_segment::moveDir::right,
			Transport_line_segment::terminationType::straight,
			4);

		up_segment->target_segment = right_segment;
		{
			auto& up = logic_chunk->get_struct(Logic_chunk::structLayer::transport_line)
			                      .emplace_back(transport_belt_proto.get(), 0, 0);
			up.unique_data = up_segment;

			auto& right = logic_chunk->get_struct(Logic_chunk::structLayer::transport_line)
			                         .emplace_back(transport_belt_proto.get(), 3, 0);
			right.unique_data = right_segment;
		}

		// Offset is distance from beginning, or previous item
		up_segment->append_item(true, 0.f, item_proto.get());
		up_segment->append_item(true, 1, item_proto.get());
		up_segment->append_item(true, 1, item_proto.get());
		static_assert(transport_line_c::item_spacing < 1);  // Tested positions would otherwise be invalid

		// Logic
		// Should transfer the first item
		transport_line_c::transport_line_logic_update();


		EXPECT_EQ(up_segment->left.size(), 2);
		EXPECT_FLOAT_EQ(up_segment->left[0].first.getAsDouble(), 0.99f);
		EXPECT_FLOAT_EQ(up_segment->left[1].first.getAsDouble(), 1.f);

		EXPECT_EQ(right_segment->left.size(), 1);
		// Moved forward once 4 - 0.3 - 0.01
		EXPECT_FLOAT_EQ(right_segment->left[0].first.getAsDouble(), 3.69f);

		// Transfer second item after (1 / 0.01) + 1 update - 1 update (Already moved once above)
		for (int i = 0; i < 100; ++i) {
			transport_line_c::transport_line_logic_update();
		}

		EXPECT_EQ(up_segment->left.size(), 1);
		EXPECT_EQ(right_segment->left.size(), 2);
		// Spacing of 1 tile between the items is maintained across belts
		EXPECT_FLOAT_EQ(right_segment->left[0].first.getAsDouble(), 2.69f);
		EXPECT_FLOAT_EQ(right_segment->left[1].first.getAsDouble(), 1.f);


		// Third item
		for (int i = 0; i < 100; ++i) {
			transport_line_c::transport_line_logic_update();
		}
		EXPECT_EQ(up_segment->left.size(), 0);
		EXPECT_EQ(right_segment->left.size(), 3);

		EXPECT_FLOAT_EQ(right_segment->left[0].first.getAsDouble(), 1.69f);
		EXPECT_FLOAT_EQ(right_segment->left[1].first.getAsDouble(), 1.f);
		EXPECT_FLOAT_EQ(right_segment->left[2].first.getAsDouble(), 1.f);
	}

	TEST(transport_line, line_logic_compressed_right_bend) {
		// Same as line_logic_right_bend, but items are compressed
		using namespace jactorio::game;

		const auto item_proto = std::make_unique<jactorio::data::Item>();
		const auto transport_belt_proto = std::make_unique<jactorio::data::Transport_belt>();
		transport_belt_proto->speed = 0.01f;

		jactorio::core::Resource_guard guard(&world_manager::clear_chunk_data);

		auto chunk = Chunk(0, 0, nullptr);
		auto* logic_chunk = &world_manager::logic_add_chunk(&chunk);

		/*
		 * COMPRESSED
		 *    --------- RIGHT -------- >
		 *    ^
		 *    |
		 *    | UP
		 *    |
		 *    |
		 */

		auto* up_segment = new Transport_line_segment(
			Transport_line_segment::moveDir::up,
			Transport_line_segment::terminationType::bend_right,
			4);
		auto* right_segment = new Transport_line_segment(
			Transport_line_segment::moveDir::right,
			Transport_line_segment::terminationType::straight,
			4);

		up_segment->target_segment = right_segment;
		{
			auto& up = logic_chunk->get_struct(Logic_chunk::structLayer::transport_line)
			                      .emplace_back(transport_belt_proto.get(), 0, 0);
			up.unique_data = up_segment;

			auto& right = logic_chunk->get_struct(Logic_chunk::structLayer::transport_line)
			                         .emplace_back(transport_belt_proto.get(), 3, 0);
			right.unique_data = right_segment;
		}

		// Offset is distance from beginning, or previous item
		up_segment->append_item(true, 0.f, item_proto.get());
		up_segment->append_item(true, transport_line_c::item_spacing, item_proto.get());

		// First item
		transport_line_c::transport_line_logic_update();


		EXPECT_EQ(up_segment->left.size(), 1);
		EXPECT_FLOAT_EQ(up_segment->left[0].first.getAsDouble(), 0.24f);

		EXPECT_EQ(right_segment->left.size(), 1);
		// Moved forward once 4 - 0.3 - 0.01
		EXPECT_FLOAT_EQ(right_segment->left[0].first.getAsDouble(), 3.69f);


		// Transfer second item after (0.25 / 0.01) + 1 update - 1 update (Already moved once above)
		for (int i = 0; i < 25; ++i) {
			transport_line_c::transport_line_logic_update();
		}

		EXPECT_EQ(up_segment->left.size(), 0);
		EXPECT_EQ(right_segment->left.size(), 2);
		// Spacing is maintained across belts
		EXPECT_FLOAT_EQ(right_segment->left[0].first.getAsDouble(), 3.44f);
		EXPECT_FLOAT_EQ(right_segment->left[1].first.getAsDouble(), 0.25f);
	}

	TEST(transport_line, line_logic_transition_straight) {
		// Transferring from a straight segment traveling left to another one traveling left
		/*
		 * < ------ LEFT (1) ------		< ------ LEFT (2) -------
		 */

		using namespace jactorio::game;

		const auto item_proto = std::make_unique<jactorio::data::Item>();
		const auto transport_belt_proto = std::make_unique<jactorio::data::Transport_belt>();
		transport_belt_proto->speed = 0.01f;

		jactorio::core::Resource_guard guard(&world_manager::clear_chunk_data);

		auto chunk = Chunk(0, 0, nullptr);
		auto* logic_chunk = &world_manager::logic_add_chunk(&chunk);

		auto* segment_1 = new Transport_line_segment(
			Transport_line_segment::moveDir::left,
			Transport_line_segment::terminationType::straight,
			4);
		auto* segment_2 = new Transport_line_segment(
			Transport_line_segment::moveDir::left,
			Transport_line_segment::terminationType::straight,
			4);

		segment_2->target_segment = segment_1;
		{
			auto& structs = logic_chunk->get_struct(Logic_chunk::structLayer::transport_line);

			auto& seg_1 = structs.emplace_back(transport_belt_proto.get(), 0, 0);
			seg_1.unique_data = segment_1;

			auto& seg_2 = logic_chunk->get_struct(Logic_chunk::structLayer::transport_line)
			                         .emplace_back(transport_belt_proto.get(), 3, 0);
			seg_2.unique_data = segment_2;
		}

		// Insert item on left + right side
		segment_2->append_item(true, 0.02f, item_proto.get());
		segment_2->append_item(false, 0.02f, item_proto.get());

		// Travel to the next belt in 0.02 / 0.01 + 1 updates
		for (int i = 0; i < 3; ++i) {
			transport_line_c::transport_line_logic_update();
		}

		EXPECT_EQ(segment_2->left.size(), 0);
		EXPECT_EQ(segment_2->right.size(), 0);
		// 3.99 tiles from the end of this transport line
		EXPECT_FLOAT_EQ(segment_1->left[0].first.getAsDouble(), 3.99);
		EXPECT_FLOAT_EQ(segment_1->right[0].first.getAsDouble(), 3.99);
	}

	TEST(transport_line, line_logic_stop_at_end_of_line) {
		// When no target_segment is provided:
		// First Item will stop at the end of line (Distance is 0)
		// Trailing items will stop at item_width from the previous item

		using namespace jactorio::game;

		const auto item_proto = std::make_unique<jactorio::data::Item>();
		const auto transport_belt_proto = std::make_unique<jactorio::data::Transport_belt>();
		transport_belt_proto->speed = 0.01f;

		jactorio::core::Resource_guard guard(&world_manager::clear_chunk_data);

		auto chunk = Chunk(0, 0, nullptr);
		auto* logic_chunk = &world_manager::logic_add_chunk(&chunk);

		auto* segment = new Transport_line_segment(
			Transport_line_segment::moveDir::left,
			Transport_line_segment::terminationType::straight, 10);

		{
			auto& structs = logic_chunk->get_struct(Logic_chunk::structLayer::transport_line);
			auto& seg_1 = structs.emplace_back(transport_belt_proto.get(), 0, 0);
			seg_1.unique_data = segment;
		}

		segment->append_item(true, 0.5f, item_proto.get());
		segment->append_item(true, transport_line_c::item_spacing, item_proto.get());
		segment->append_item(true, transport_line_c::item_spacing + 1.f, item_proto.get());

		// Will reach distance 0 after 0.5 / 0.01 updates
		for (int i = 0; i < 50; ++i) {
			transport_line_c::transport_line_logic_update();
		}

		EXPECT_EQ(segment->l_index, 0);
		EXPECT_FLOAT_EQ(segment->left[0].first.getAsDouble(), 0);

		// On the next update, with no target segment, first item is kept at 0, second item untouched
		// move index to 2 (was 0) as it has a distance greater than item_width
		transport_line_c::transport_line_logic_update();


		EXPECT_EQ(segment->l_index, 2);
		EXPECT_FLOAT_EQ(segment->left[0].first.getAsDouble(), 0);
		EXPECT_FLOAT_EQ(segment->left[1].first.getAsDouble(), transport_line_c::item_spacing);
		EXPECT_FLOAT_EQ(segment->left[2].first.getAsDouble(), transport_line_c::item_spacing + 0.99f);

		// After 0.2 + 0.99 / 0.01 updates, the Third item will not move in following updates
		for (int j = 0; j < 99; ++j) {
			transport_line_c::transport_line_logic_update();
		}
		EXPECT_FLOAT_EQ(segment->left[2].first.getAsDouble(), transport_line_c::item_spacing);

		// Index set to 3 (indicating the current items should not be moved)
		// Should not move after further updates
		transport_line_c::transport_line_logic_update();

		EXPECT_EQ(segment->l_index, 3);
		EXPECT_FLOAT_EQ(segment->left[2].first.getAsDouble(), transport_line_c::item_spacing);


		// Updates not do nothing as index is at 3, where no item exists
		for (int k = 0; k < 50; ++k) {
			transport_line_c::transport_line_logic_update();
		}
	}

	TEST(transport_line, line_logic_stop_at_filled_target_segment) {
		// For the right lane:
		using namespace jactorio::game;

		const auto item_proto = std::make_unique<jactorio::data::Item>();
		const auto transport_belt_proto = std::make_unique<jactorio::data::Transport_belt>();
		transport_belt_proto->speed = 0.01f;

		jactorio::core::Resource_guard guard(&world_manager::clear_chunk_data);

		auto chunk = Chunk(0, 0, nullptr);
		auto* logic_chunk = &world_manager::logic_add_chunk(&chunk);

		/*
		 *    --------- RIGHT -------- >
		 *    ^
		 *    |
		 *    | UP
		 *    |
		 *    |
		 */

		auto* up_segment = new Transport_line_segment(
			Transport_line_segment::moveDir::up,
			Transport_line_segment::terminationType::bend_right,
			4);
		auto* right_segment = new Transport_line_segment(
			Transport_line_segment::moveDir::right,
			Transport_line_segment::terminationType::straight,
			4);

		up_segment->target_segment = right_segment;
		{
			auto& up = logic_chunk->get_struct(Logic_chunk::structLayer::transport_line)
			                      .emplace_back(transport_belt_proto.get(), 0, 0);
			up.unique_data = up_segment;

			auto& right = logic_chunk->get_struct(Logic_chunk::structLayer::transport_line)
			                         .emplace_back(transport_belt_proto.get(), 3, 0);
			right.unique_data = right_segment;
		}

		// RIGHT LINE: 14 items can be fit on the right lane: (4 - 0.7) / item_spacing{0.25} = 13.2
		for (int i = 0; i < 14; ++i) {
			right_segment->append_item(false, 0.f, item_proto.get());
		}

		// Items on up line should stop
		up_segment->append_item(false, 0.f, item_proto.get());

		// WIll not move after an arbitrary number of updates
		for (int i = 0; i < 34; ++i) {
			transport_line_c::transport_line_logic_update();
		}

		EXPECT_FLOAT_EQ(up_segment->right.front().first.getAsDouble(), 0);
	}

	TEST(transport_line, line_logic_item_spacing) {
		// A minimum distance of transport_line_c::item_spacing is maintained between items
		using namespace jactorio::game;

		const auto item_proto = std::make_unique<jactorio::data::Item>();
		const auto transport_belt_proto = std::make_unique<jactorio::data::Transport_belt>();

		jactorio::core::Resource_guard guard(&world_manager::clear_chunk_data);

		auto chunk = Chunk(0, 0, nullptr);
		auto* logic_chunk = &world_manager::logic_add_chunk(&chunk);

		auto* right_segment = new Transport_line_segment(
			Transport_line_segment::moveDir::right,
			Transport_line_segment::terminationType::bend_right,
			4);

		auto& right = logic_chunk->get_struct(Logic_chunk::structLayer::transport_line)
		                         .emplace_back(transport_belt_proto.get(), 3, 0);
		right.unique_data = right_segment;

		right_segment->append_item(true, 0.f, item_proto.get());
		right_segment->append_item(true, 0.f, item_proto.get());  // Insert behind previous item

		// Check that second item has a minimum distance of transport_line_c::item_spacing
		EXPECT_FLOAT_EQ(right_segment->left[0].first.getAsDouble(), 0.f);
		EXPECT_FLOAT_EQ(right_segment->left[1].first.getAsDouble(), transport_line_c::item_spacing);
	}

	TEST(transport_line, line_logic_transition_side_left) {
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
		using namespace jactorio::game;

		const auto item_proto = std::make_unique<jactorio::data::Item>();
		const auto transport_belt_proto = std::make_unique<jactorio::data::Transport_belt>();
		transport_belt_proto->speed = 0.05;

		jactorio::core::Resource_guard guard(&world_manager::clear_chunk_data);

		auto chunk = Chunk(0, 0, nullptr);

		auto& logic_chunk = world_manager::logic_add_chunk(&chunk);

		// Segments (Logic chunk must be created first)
		auto* right_segment = new Transport_line_segment(
			Transport_line_segment::moveDir::right,
			Transport_line_segment::terminationType::right_only,
			5);
		auto* down_segment = new Transport_line_segment(
			Transport_line_segment::moveDir::down,
			Transport_line_segment::terminationType::straight,
			10);

		right_segment->target_segment = down_segment;

		// Right dir belt empties only onto down belt Right side
		auto& right = logic_chunk.get_struct(Logic_chunk::structLayer::transport_line)
		                         .emplace_back(transport_belt_proto.get(), 4, 0);
		right.unique_data = right_segment;


		auto& down = logic_chunk.get_struct(Logic_chunk::structLayer::transport_line)
		                        .emplace_back(transport_belt_proto.get(), 4, 9);
		down.unique_data = down_segment;

		// Insert items
		for (int i = 0; i < 3; ++i) {
			right_segment->append_item(true, 0.f, item_proto.get());
			right_segment->append_item(false, 0.f, item_proto.get());
		}

		// Logic tests
		transport_line_c::transport_line_logic_update();

		// Since the target belt is empty, both A + B inserts into right lane
		EXPECT_EQ(right_segment->left.size(), 2);
		EXPECT_EQ(right_segment->left[0].first.getAsDouble(), 0.2);  // 0.25 - 0.05

		EXPECT_EQ(right_segment->right.size(), 2);
		EXPECT_EQ(right_segment->right[0].first.getAsDouble(), 0.2);


		ASSERT_EQ(down_segment->right.size(), 2);
		// 10 - 0.7 - 0.05
		EXPECT_FLOAT_EQ(down_segment->right[0].first.getAsDouble(), 9.25f);
		// (10 - 0.3 - 0.05) - (10 - 0.7 - 0.05)
		EXPECT_FLOAT_EQ(down_segment->right[1].first.getAsDouble(), 0.4f);


		// ======================================================================
		// End on One update prior to transitioning
		for (int j = 0; j < 4; ++j) {
			transport_line_c::transport_line_logic_update();
		}
		EXPECT_EQ(right_segment->left[0].first.getAsDouble(), 0.0);
		EXPECT_EQ(right_segment->right[0].first.getAsDouble(), 0.0);

		EXPECT_FLOAT_EQ(down_segment->right[0].first.getAsDouble(), 9.05f);


		// ======================================================================
		// Transition items
		transport_line_c::transport_line_logic_update();
		EXPECT_EQ(right_segment->left.size(), 1);
		EXPECT_EQ(right_segment->left[0].first.getAsDouble(), 0.2);  // 0.25 - 0.05


		// ======================================================================
		// Right lane (B) stops, left (A) takes priority
		EXPECT_EQ(right_segment->right.size(), 2);  // Unmoved
		EXPECT_EQ(right_segment->right[0].first.getAsDouble(), 0.f);

		ASSERT_EQ(down_segment->right.size(), 3);
		EXPECT_FLOAT_EQ(down_segment->right[0].first.getAsDouble(), 9.00f);
		EXPECT_FLOAT_EQ(down_segment->right[1].first.getAsDouble(), 0.4f);
		EXPECT_FLOAT_EQ(down_segment->right[2].first.getAsDouble(), 0.25f);


		// ======================================================================
		// Transition third item for Lane A, should wake up lane B after passing
		for (int j = 0; j < 4 + 13 + 1; ++j) {  // 0.20 / 0.05 + (0.40 + 0.25) / 0.05 + 1 for transition
			transport_line_c::transport_line_logic_update();
		}
		EXPECT_EQ(right_segment->left.size(), 0);
		EXPECT_EQ(right_segment->right.size(), 1);  // Woke and moved

		ASSERT_EQ(down_segment->right.size(), 5);
		EXPECT_FLOAT_EQ(down_segment->right[0].first.getAsDouble(), 8.10f);
		EXPECT_FLOAT_EQ(down_segment->right[3].first.getAsDouble(), 0.25f);

	}

	TEST(transport_line, line_logic_transition_side_right) {
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
		using namespace jactorio::game;

		const auto item_proto = std::make_unique<jactorio::data::Item>();
		const auto transport_belt_proto = std::make_unique<jactorio::data::Transport_belt>();
		transport_belt_proto->speed = 0.05;

		jactorio::core::Resource_guard guard(&world_manager::clear_chunk_data);

		auto chunk = Chunk(0, 0, nullptr);

		auto& logic_chunk = world_manager::logic_add_chunk(&chunk);

		// Segments (Logic chunk must be created first)
		auto* left_segment = new Transport_line_segment(
			Transport_line_segment::moveDir::left,
			Transport_line_segment::terminationType::right_only,
			5);
		auto* up_segment = new Transport_line_segment(
			Transport_line_segment::moveDir::down,
			Transport_line_segment::terminationType::straight,
			10);

		left_segment->target_segment = up_segment;

		// Right dir belt empties only onto down belt Right side
		{
			auto& left = logic_chunk.get_struct(Logic_chunk::structLayer::transport_line)
			                        .emplace_back(transport_belt_proto.get(), 4, 0);
			left.unique_data = left_segment;


			auto& down = logic_chunk.get_struct(Logic_chunk::structLayer::transport_line)
			                        .emplace_back(transport_belt_proto.get(), 4, 9);
			down.unique_data = up_segment;
		}


		// Insert items
		for (int i = 0; i < 3; ++i) {
			left_segment->append_item(true, 0.f, item_proto.get());
			left_segment->append_item(false, 0.f, item_proto.get());
		}

		// Logic tests
		transport_line_c::transport_line_logic_update();

		// Since the target belt is empty, both A + B inserts into right lane
		EXPECT_EQ(left_segment->left.size(), 2);
		EXPECT_EQ(left_segment->left[0].first.getAsDouble(), 0.2);  // 0.25 - 0.05

		EXPECT_EQ(left_segment->right.size(), 2);
		EXPECT_EQ(left_segment->right[0].first.getAsDouble(), 0.2);


		ASSERT_EQ(up_segment->right.size(), 2);
		// 10 - 0.7 - 0.05
		EXPECT_FLOAT_EQ(up_segment->right[0].first.getAsDouble(), 9.25f);
		// (10 - 0.3 - 0.05) - (10 - 0.7 - 0.05)
		EXPECT_FLOAT_EQ(up_segment->right[1].first.getAsDouble(), 0.4f);


		// ======================================================================
		// End on One update prior to transitioning
		for (int j = 0; j < 4; ++j) {
			transport_line_c::transport_line_logic_update();
		}
		EXPECT_EQ(left_segment->left[0].first.getAsDouble(), 0.0);
		EXPECT_EQ(left_segment->right[0].first.getAsDouble(), 0.0);

		EXPECT_FLOAT_EQ(up_segment->right[0].first.getAsDouble(), 9.05f);


		// ======================================================================
		// Transition items
		transport_line_c::transport_line_logic_update();
		EXPECT_EQ(left_segment->left.size(), 1);
		EXPECT_EQ(left_segment->left[0].first.getAsDouble(), 0.2);  // 0.25 - 0.05


		// ======================================================================
		// Right lane (B) stops, left (A) takes priority
		EXPECT_EQ(left_segment->right.size(), 2);  // Unmoved
		EXPECT_EQ(left_segment->right[0].first.getAsDouble(), 0.f);

		ASSERT_EQ(up_segment->right.size(), 3);
		EXPECT_FLOAT_EQ(up_segment->right[0].first.getAsDouble(), 9.00f);
		EXPECT_FLOAT_EQ(up_segment->right[1].first.getAsDouble(), 0.4f);
		EXPECT_FLOAT_EQ(up_segment->right[2].first.getAsDouble(), 0.25f);


		// ======================================================================
		// Transition third item for Lane A, should wake up lane B after passing
		for (int j = 0; j < 4 + 13 + 1; ++j) {  // 0.20 / 0.05 + (0.40 + 0.25) / 0.05 + 1 for transition
			transport_line_c::transport_line_logic_update();
		}
		EXPECT_EQ(left_segment->left.size(), 0);
		EXPECT_EQ(left_segment->right.size(), 1);  // Woke and moved

		ASSERT_EQ(up_segment->right.size(), 5);
		EXPECT_FLOAT_EQ(up_segment->right[0].first.getAsDouble(), 8.10f);
		EXPECT_FLOAT_EQ(up_segment->right[3].first.getAsDouble(), 0.25f);

	}

	// TODO if target segment is stopped, the segment pointing to it will also be stopped
}
