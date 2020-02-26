#include <gtest/gtest.h>

#include "game/logic/transport_line_controller.h"
#include "game/world/world_manager.h"
#include "data/prototype/entity/transport/transport_belt.h"
#include "core/float_math.h"
#include "game/logic/transport_line_structure.h"

#include <memory>

namespace game::logic
{
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
		auto* up_segment = new jactorio::game::Transport_line_segment(
			jactorio::game::Transport_line_segment::move_dir::up,
			jactorio::game::Transport_line_segment::terminationType::bend_right,
			5);
		auto* right_segment = new jactorio::game::Transport_line_segment(
			jactorio::game::Transport_line_segment::move_dir::right,
			jactorio::game::Transport_line_segment::terminationType::bend_right,
			4);
		auto* down_segment = new jactorio::game::Transport_line_segment(
			jactorio::game::Transport_line_segment::move_dir::down,
			jactorio::game::Transport_line_segment::terminationType::bend_right,
			5);
		auto* left_segment = new jactorio::game::Transport_line_segment(
			jactorio::game::Transport_line_segment::move_dir::left,
			jactorio::game::Transport_line_segment::terminationType::bend_right,
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
		transport_line_c::belt_insert_item(false, left_segment, 2.5f, item_proto.get());

		std::queue<jactorio::game::transport_line_c::Segment_transition_item> queue;

		// Moving left at a speed of 0.01f per update, in 250 updates
		// Should reach end of transport line - Next update will change its direction to up
		for (int i = 0; i < 250; ++i) {
			transport_line_c::logic_update(queue, logic_chunk);
			transport_line_c::logic_process_queued_items(queue);
		}
		EXPECT_NEAR(left_segment->right.front().first, 0.f, jactorio::core::transport_line_epsilon);
		EXPECT_EQ(left_segment->right.size(), 1);

		// The actual lengths of the transport segments != the indicated length as it turns earlier

		// End of U | 5 - 2(0.7) / 0.01 = 360
		for (int i = 0; i < 360; ++i) {
			transport_line_c::logic_update(queue, logic_chunk);
			transport_line_c::logic_process_queued_items(queue);
		}
		EXPECT_NEAR(up_segment->right.front().first, 0.f, jactorio::core::transport_line_epsilon);
		EXPECT_EQ(up_segment->right.size(), 1);

		// End of R | 4 - 2(0.7) / 0.01 = 260 updates
		for (int i = 0; i < 260; ++i) {
			transport_line_c::logic_update(queue, logic_chunk);
			transport_line_c::logic_process_queued_items(queue);
		}
		EXPECT_NEAR(right_segment->right.front().first, 0.f, jactorio::core::transport_line_epsilon);
		EXPECT_EQ(right_segment->right.size(), 1);

		// End of D
		for (int i = 0; i < 360; ++i) {
			transport_line_c::logic_update(queue, logic_chunk);
			transport_line_c::logic_process_queued_items(queue);
		}
		EXPECT_NEAR(down_segment->right.front().first, 0.f, jactorio::core::transport_line_epsilon);
		EXPECT_EQ(down_segment->right.size(), 1);

		// End of L 4 - 2(0.7)
		for (int i = 0; i < 260; ++i) {
			transport_line_c::logic_update(queue, logic_chunk);
			transport_line_c::logic_process_queued_items(queue);
		}
		EXPECT_NEAR(left_segment->right.front().first, 0.f, jactorio::core::transport_line_epsilon);
		EXPECT_EQ(left_segment->right.size(), 1);

		// Queue for items awaiting transitions to another segment should be empty
		EXPECT_TRUE(queue.empty());
	}

	TEST(transport_line, line_logic_multiple_item) {
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

		auto* up_segment = new jactorio::game::Transport_line_segment(
			jactorio::game::Transport_line_segment::move_dir::up,
			jactorio::game::Transport_line_segment::terminationType::bend_right,
			4);
		auto* right_segment = new jactorio::game::Transport_line_segment(
			jactorio::game::Transport_line_segment::move_dir::right,
			jactorio::game::Transport_line_segment::terminationType::straight,
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
		transport_line_c::belt_insert_item(true, up_segment, 0.f, item_proto.get());
		transport_line_c::belt_insert_item(true, up_segment, 1, item_proto.get());
		transport_line_c::belt_insert_item(true, up_segment, 1, item_proto.get());
		static_assert(transport_line_c::item_spacing < 1);  // Tested positions would otherwise be invalid

		// Logic
		std::queue<jactorio::game::transport_line_c::Segment_transition_item> queue;

		// Should transfer the first item
		transport_line_c::logic_update(queue, logic_chunk);
		transport_line_c::logic_process_queued_items(queue);

		EXPECT_EQ(up_segment->left.size(), 2);
		EXPECT_NEAR(up_segment->left[0].first, 0.99f, jactorio::core::transport_line_epsilon);
		EXPECT_NEAR(up_segment->left[1].first, 1.f, jactorio::core::transport_line_epsilon);

		EXPECT_EQ(right_segment->left.size(), 1);
		// Moved forward once 4 - 0.3 - 0.01
		EXPECT_NEAR(right_segment->left[0].first, 3.69f, jactorio::core::transport_line_epsilon);

		// Transfer second item after (1 / 0.01) + 1 update - 1 update (Already moved once above)
		for (int i = 0; i < 100; ++i) {
			transport_line_c::logic_update(queue, logic_chunk);
			transport_line_c::logic_process_queued_items(queue);
		}

		EXPECT_EQ(up_segment->left.size(), 1);
		EXPECT_EQ(right_segment->left.size(), 2);
		// Spacing of 1 tile between the items is maintained across belts
		EXPECT_NEAR(right_segment->left[0].first, 2.69f, jactorio::core::transport_line_epsilon);
		EXPECT_NEAR(right_segment->left[1].first, 1.f, jactorio::core::transport_line_epsilon);


		// Third item
		for (int i = 0; i < 100; ++i) {
			transport_line_c::logic_update(queue, logic_chunk);
			transport_line_c::logic_process_queued_items(queue);
		}
		EXPECT_EQ(up_segment->left.size(), 0);
		EXPECT_EQ(right_segment->left.size(), 3);

		EXPECT_NEAR(right_segment->left[0].first, 1.69f, jactorio::core::transport_line_epsilon);
		EXPECT_NEAR(right_segment->left[1].first, 1.f, jactorio::core::transport_line_epsilon);
		EXPECT_NEAR(right_segment->left[2].first, 1.f, jactorio::core::transport_line_epsilon);
	}

	TEST(transport_line, line_logic_item_spacing) {
		// A minimum distance of transport_line_c::item_spacing is maintained between items
		using namespace jactorio::game;

		const auto item_proto = std::make_unique<jactorio::data::Item>();
		const auto transport_belt_proto = std::make_unique<jactorio::data::Transport_belt>();

		jactorio::core::Resource_guard guard(&world_manager::clear_chunk_data);

		auto chunk = Chunk(0, 0, nullptr);
		auto* logic_chunk = &world_manager::logic_add_chunk(&chunk);

		auto* right_segment = new jactorio::game::Transport_line_segment(
			jactorio::game::Transport_line_segment::move_dir::right,
			jactorio::game::Transport_line_segment::terminationType::bend_right,
			4);

		auto& right = logic_chunk->get_struct(Logic_chunk::structLayer::transport_line)
			.emplace_back(transport_belt_proto.get(), 3, 0);
		right.unique_data = right_segment;

		transport_line_c::belt_insert_item(true, right_segment, 0.f, item_proto.get());
		transport_line_c::belt_insert_item(true, right_segment, 0.f, item_proto.get());  // Insert behind previous item

		// Check that second item has a minimum distance of transport_line_c::item_spacing
		EXPECT_NEAR(right_segment->left[0].first, 0.f, jactorio::core::transport_line_epsilon);
		EXPECT_NEAR(right_segment->left[1].first, transport_line_c::item_spacing, jactorio::core::transport_line_epsilon);
	}

	TEST(transport_line, insert_item) {
		jactorio::core::Resource_guard guard(&jactorio::game::world_manager::clear_chunk_data);

		// Insert item on belt left side

		const auto item_proto = std::make_unique<jactorio::data::Item>();
		jactorio::game::Chunk chunk{0, 0, nullptr};  // Tiles not needed as items do not follow tile grid
		jactorio::game::world_manager::logic_add_chunk(&chunk);

		// Segments (Logic chunk must be created first)
		auto* line_segment = new jactorio::game::Transport_line_segment(
			jactorio::game::Transport_line_segment::move_dir::up,
			jactorio::game::Transport_line_segment::terminationType::bend_right,
			5);

		// Offset is from the beginning of the transport line OR the previous item if it exists
		jactorio::game::transport_line_c::belt_insert_item(true, line_segment, 1.3, item_proto.get());
		EXPECT_NEAR(line_segment->left[0].first, 1.3, jactorio::core::transport_line_epsilon);

		// Should be sorted by items closest to the end of the segment
		jactorio::game::transport_line_c::belt_insert_item(true, line_segment, 1.2, item_proto.get());
		EXPECT_NEAR(line_segment->left[1].first, 1.2, jactorio::core::transport_line_epsilon);

		jactorio::game::transport_line_c::belt_insert_item(true, line_segment, 1.5, item_proto.get());
		EXPECT_NEAR(line_segment->left[2].first, 1.5, jactorio::core::transport_line_epsilon);

		jactorio::game::transport_line_c::belt_insert_item(true, line_segment, 0.5, item_proto.get());
		EXPECT_NEAR(line_segment->left[3].first, 0.5, jactorio::core::transport_line_epsilon);
	}
}
