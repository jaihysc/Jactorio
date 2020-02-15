#include <gtest/gtest.h>

#include "game/logic/transport_line_controller.h"
#include "game/world/world_manager.h"

namespace game::logic
{
	TEST(transport_line, insert_item_left) {
		// Setup world
		jactorio::game::Chunk chunk{0, 0, nullptr};  // Tiles not needed as items do not follow tile grid
		jactorio::game::world_manager::add_chunk(&chunk);
		
		// 
		jactorio::data::Sprite sprite;
		
		jactorio::game::transport_line_c::belt_insert_item_l(1, 0, &sprite);

		
		// Should have placed the item according to placement specifications in transport_line_controller.h
		ASSERT_FLOAT_EQ(
			chunk.get_object(jactorio::game::Chunk::object_layer::item_entity)[0].position_x,
			1.3f);  // Center is 0.5, item is 0.4 wide. Thus 0.3 to center

		ASSERT_FLOAT_EQ(
			chunk.get_object(jactorio::game::Chunk::object_layer::item_entity)[0].position_y,
			0.1f);


		// Item width
		ASSERT_FLOAT_EQ(
			chunk.get_object(jactorio::game::Chunk::object_layer::item_entity)[0].size_x,
			jactorio::game::transport_line_c::item_width);

		ASSERT_FLOAT_EQ(
			chunk.get_object(jactorio::game::Chunk::object_layer::item_entity)[0].size_y,
			jactorio::game::transport_line_c::item_width);
	}

	TEST(transport_line, insert_item_right) {

	}
}
