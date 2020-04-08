// 
// item_logisticsTests.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 04/07/2020
// Last modified: 04/07/2020
// 

#include <gtest/gtest.h>

#include "data/prototype/entity/container_entity.h"
#include "data/prototype/entity/mining_drill.h"
#include "data/prototype/entity/transport/transport_belt.h"
#include "game/logic/item_logistics.h"

namespace game::logic
{
	TEST(item_logistics, can_accept_item) {
		jactorio::game::World_data world_data{};
		world_data.add_chunk(new jactorio::game::Chunk{0, 0});

		// Empty tile cannot be inserted into
		EXPECT_FALSE(jactorio::game::item_logistics::can_accept_item(world_data, 2, 4));

		// Transport belt can be inserted onto
		jactorio::data::Transport_belt belt{};
		world_data.get_tile_world_coords(2, 4)
		          ->set_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity, &belt);
		EXPECT_TRUE(jactorio::game::item_logistics::can_accept_item(world_data, 2, 4));

		// Mining drill cannot be inserted into 
		jactorio::data::Mining_drill drill{};
		world_data.get_tile_world_coords(2, 4)
		          ->set_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity, &drill);
		EXPECT_FALSE(jactorio::game::item_logistics::can_accept_item(world_data, 2, 4));

		// Container can be inserted into
		jactorio::data::Container_entity container{};
		world_data.get_tile_world_coords(2, 4)
		          ->set_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity, &container);
		EXPECT_TRUE(jactorio::game::item_logistics::can_accept_item(world_data, 2, 4));

	}
}
