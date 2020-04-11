// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 04/06/2020

#include <gtest/gtest.h>

#include "data/prototype/entity/mining_drill.h"
#include "data/prototype/entity/resource_entity.h"

namespace data::prototype
{
	TEST(mining_drill, on_can_build) {
		/*
		 * [ ] [ ] [ ] [ ] [ ] [ ] [ ] [ ]
		 * [ ] [ ] [ ] [ ] [ ] [ ] [ ] [ ]
		 * [ ] [ ] [X] [x] [x] [x] [ ] [ ]
		 * [ ] [ ] [x] [x] [x] [x] [ ] [ ]
		 * [ ] [ ] [x] [x] [x] [x] [ ] [ ]
		 * [ ] [ ] [ ] [ ] [ ] [ ] [ ] [ ]
		 * [ ] [ ] [ ] [ ] [ ] [ ] [ ] [ ]
		 */

		jactorio::game::World_data world_data{};
		world_data.add_chunk(new jactorio::game::Chunk{0, 0});

		jactorio::data::Mining_drill drill{};
		drill.tile_width = 4;
		drill.tile_height = 3;
		drill.mining_radius = 2;

		// Has no resource tiles
		EXPECT_FALSE(drill.on_can_build(world_data, {2, 2}));

		// Has resource tiles
		jactorio::data::Resource_entity resource{};
		world_data.get_tile_world_coords(0, 0)
		          ->get_layer(jactorio::game::Chunk_tile::chunkLayer::resource).prototype_data = &resource;

		EXPECT_TRUE(drill.on_can_build(world_data, {2, 2}));
	}

	TEST(mining_drill, on_can_build_2) {
		/*
		 * [ ] [ ] [ ] [ ] [ ] [ ] [ ] [ ]
		 * [ ] [ ] [ ] [ ] [ ] [ ] [ ] [ ]
		 * [ ] [ ] [X] [x] [x] [x] [ ] [ ]
		 * [ ] [ ] [x] [x] [x] [x] [ ] [ ]
		 * [ ] [ ] [x] [x] [x] [x] [ ] [ ]
		 * [ ] [ ] [ ] [ ] [ ] [ ] [ ] [ ]
		 * [ ] [ ] [ ] [ ] [ ] [ ] [ ] [ ]
		 */

		jactorio::game::World_data world_data{};
		world_data.add_chunk(new jactorio::game::Chunk{0, 0});

		jactorio::data::Mining_drill drill{};
		drill.tile_width = 4;
		drill.tile_height = 3;
		drill.mining_radius = 2;

		jactorio::data::Resource_entity resource{};
		world_data.get_tile_world_coords(7, 6)
		          ->get_layer(jactorio::game::Chunk_tile::chunkLayer::resource).prototype_data = &resource;

		EXPECT_TRUE(drill.on_can_build(world_data, {2, 2}));
	}
}
