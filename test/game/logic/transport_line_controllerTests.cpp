#include <gtest/gtest.h>

#include "game/logic/transport_line_controller.h"
#include "game/world/world_manager.h"

#include <memory>

#include "core/float_math.h"

namespace game::logic
{
	TEST(transport_line, item_logic) {
		// Tests that items move as expected
		using namespace jactorio::game;

		const auto item_proto = std::make_unique<jactorio::data::Item>();
		jactorio::core::Resource_guard guard(&world_manager::clear_chunk_data);

		auto chunk = Chunk(0, 0, nullptr);
		auto* logic_chunk = &world_manager::logic_add_chunk(&chunk);
		
		// Item update points (Logic chunk must be created first
		world_manager::logic_get_all_chunks()[&chunk].transport_line_updates[{0.5f, 5.1f}] =
			jactorio::data::Transport_line_item_data::move_dir::up;

		world_manager::logic_get_all_chunks()[&chunk].transport_line_updates[{0.5f, 0.5f}] =
			jactorio::data::Transport_line_item_data::move_dir::right;

		world_manager::logic_get_all_chunks()[&chunk].transport_line_updates[{4.1f, 0.5f}] =
			jactorio::data::Transport_line_item_data::move_dir::down;

		world_manager::logic_get_all_chunks()[&chunk].transport_line_updates[{4.1f, 5.1f}] =
			jactorio::data::Transport_line_item_data::move_dir::left;

		// Insert item
		transport_line_c::chunk_insert_item(&chunk, 3.f, 5.1f, item_proto.get());

		const auto& item = chunk.get_object(Chunk::object_layer::item_entity)[0];
		// Moving left at a speed of 0.01f per update, in 250 updates
		// Should be at 0.5f, 5.1f moving right - Next update will change its direction to up
		for (int i = 0; i < 250; ++i) {
			transport_line_c::logic_update(logic_chunk);
		}
		
		EXPECT_NEAR(item.position_x, 0.5f, jactorio::core::epsilon);
		EXPECT_NEAR(item.position_y, 5.1f, jactorio::core::epsilon);
		EXPECT_EQ(static_cast<jactorio::data::Transport_line_item_data*>(item.unique_data)->direction,
		          jactorio::data::Transport_line_item_data::move_dir::left);

		
		// Now moving up, passing through the update point to move right at 0.5f, 5.1f
		// Should be at the update point to move left (0.5f, 0.5f) after 460 updates
		for (int i = 0; i < 460; ++i) {
			transport_line_c::logic_update(logic_chunk);
		}
		EXPECT_NEAR(item.position_x, 0.5f, jactorio::core::epsilon);
		EXPECT_NEAR(item.position_y, 0.5f, jactorio::core::epsilon);
		EXPECT_EQ(static_cast<jactorio::data::Transport_line_item_data*>(item.unique_data)->direction,
		          jactorio::data::Transport_line_item_data::move_dir::up);

		
		// At update point to move down (4.1f, 0.5f) after 360 updates
		for (int i = 0; i < 360; ++i) {
			transport_line_c::logic_update(logic_chunk);
		}
		EXPECT_NEAR(item.position_x, 4.1f, jactorio::core::epsilon);
		EXPECT_NEAR(item.position_y, 0.5f, jactorio::core::epsilon);
		EXPECT_EQ(static_cast<jactorio::data::Transport_line_item_data*>(item.unique_data)->direction,
		          jactorio::data::Transport_line_item_data::move_dir::right);


		// At update point to move left (4.1f, 5.1f) after 460 updates
		for (int i = 0; i < 460; ++i) {
			transport_line_c::logic_update(logic_chunk);
		}
		EXPECT_NEAR(item.position_x, 4.1f, jactorio::core::epsilon);
		EXPECT_NEAR(item.position_y, 5.1f, jactorio::core::epsilon);
		EXPECT_EQ(static_cast<jactorio::data::Transport_line_item_data*>(item.unique_data)->direction,
		          jactorio::data::Transport_line_item_data::move_dir::down);


		// Back to the update point to move up (0.5f, 5.1f) after 360 updates
		for (int i = 0; i < 360; ++i) {
			transport_line_c::logic_update(logic_chunk);
		}
		EXPECT_NEAR(item.position_x, 0.5f, jactorio::core::epsilon);
		EXPECT_NEAR(item.position_y, 5.1f, jactorio::core::epsilon);
		EXPECT_EQ(static_cast<jactorio::data::Transport_line_item_data*>(item.unique_data)->direction,
		          jactorio::data::Transport_line_item_data::move_dir::left);
	}
	
	TEST(transport_line, insert_item) {
		jactorio::core::Resource_guard guard(&jactorio::game::world_manager::clear_chunk_data);
		
		// Insert item on belt left side
		
		jactorio::game::Chunk chunk{0, 0, nullptr};  // Tiles not needed as items do not follow tile grid
		
		jactorio::data::Item item;
		jactorio::game::transport_line_c::chunk_insert_item(&chunk, 1.3, 0.5, &item);

		
		// Should have placed the item according to placement specifications in transport_line_controller.h
		ASSERT_FLOAT_EQ(
			chunk.get_object(jactorio::game::Chunk::object_layer::item_entity)[0].position_x,
			1.3f);  // Center is 0.5, item is 0.4 wide. Thus 0.3 to center

		ASSERT_FLOAT_EQ(
			chunk.get_object(jactorio::game::Chunk::object_layer::item_entity)[0].position_y,
			0.5f);


		// Item width
		ASSERT_FLOAT_EQ(
			chunk.get_object(jactorio::game::Chunk::object_layer::item_entity)[0].size_x,
			jactorio::game::transport_line_c::item_width);

		ASSERT_FLOAT_EQ(
			chunk.get_object(jactorio::game::Chunk::object_layer::item_entity)[0].size_y,
			jactorio::game::transport_line_c::item_width);

		// Set unique data to moving left
		EXPECT_EQ(
			static_cast<jactorio::data::Transport_line_item_data*>(
				chunk.get_object(jactorio::game::Chunk::object_layer::item_entity)[0].unique_data)->direction,
			jactorio::data::Transport_line_item_data::move_dir::left);

		
		// Will NOT add chunk to vector of logic chunks automatically
		EXPECT_EQ(jactorio::game::world_manager::logic_get_all_chunks().size(), 0);
	}
}
