// 
// placement_controllerTests.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 01/20/2020
// Last modified: 04/02/2020
// 

#include <gtest/gtest.h>

#include "data/prototype/entity/container_entity.h"
#include "data/prototype/tile/tile.h"
#include "game/logic/placement_controller.h"
#include "game/world/world_data.h"

#include <memory>

namespace game::logic
{
	void generate_test_world(jactorio::game::World_data& world_data,
	                         jactorio::data::Tile* water_tile,
	                         jactorio::data::Tile* land_tile) {
		// Generates a quarter chunk on which to test entity placement (16 x 16)
		// Following indices begin at 0:
		// Row of water: [1, 4, 8]
		// Column of water: [1, 4, 8]

		/* # is land
		 * - is water
		   #-##-###-#######
		   ----------------
		   #-##-###-#######
		   #-##-###-#######
		   ----------------
		   #-##-###-#######
		   #-##-###-#######
		   #-##-###-#######
		   ----------------
		   #-##-###-#######
		   #-##-###-#######
		   #-##-###-#######
		   #-##-###-#######
		   #-##-###-#######
		   #-##-###-#######
		   #-##-###-#######
		 */

		water_tile->is_water = true;
		land_tile->is_water = false;


		using namespace jactorio::game;
		// Create chunk
		auto* chunk_tiles = new Chunk_tile[32 * 32];
		for (int y = 0; y < 32; ++y) {
			bool y_water = false;

			// Water at Y index 1, 4, 8
			if (y == 1 || y == 4 || y == 8)
				y_water = true;

			for (int x = 0; x < 32; ++x) {
				bool x_water = false;

				// Water at X index 1, 4, 8
				if (x == 1 || x == 4 || x == 8)
					x_water = true;

				auto* tile_ptr = land_tile;
				if (x_water || y_water) {
					// printf("-");
					tile_ptr = water_tile;
				}
				// else
				// printf("#");

				chunk_tiles[y * 32 + x].set_tile_prototype(Chunk_tile::chunkLayer::base, tile_ptr);
			}
			// printf("\n");
		}

		world_data.add_chunk(new Chunk(0, 0, chunk_tiles));
	}

	// No longer necessary as worlds are stored as class instances with a destructor
	// 
	// void clear_test_world_data(jactorio::game::World_data& world_data) {
	// 	using namespace jactorio::game;
	// 	// Delete the land and water tile prototype
	// 	const auto chunk = world_data.get_chunk(0, 0);
	//
	// 	auto p1 = 
	// 		chunk_tile_getter::get_tile_prototype(chunk->tiles_ptr()[0], 
	// 											  Chunk_tile::chunkLayer::base);
	// 	auto p2 = 
	// 		chunk_tile_getter::get_tile_prototype(chunk->tiles_ptr()[1], 
	// 											  Chunk_tile::chunkLayer::base);
	//
	// 	// Delete chunk tiles
	// 	world_data.clear_chunk_data();
	//
	// 	// Prototype data must be deleted after world data
	// 	delete p1;
	// 	delete p2;
	// }

	//
	// !Checking if the selected tile is too far from the player is not done in these tests!
	//

	// Creates world_data and generates a test world within it
#define GENERATE_TEST_WORLD\
	auto water_tile_ = std::make_unique<jactorio::data::Tile>();\
	auto land_tile_ = std::make_unique<jactorio::data::Tile>();\
	\
	jactorio::game::World_data world_data{};\
	generate_test_world(world_data, water_tile_.get(), land_tile_.get());


	TEST(placement_controller, place_entity_1x1_valid) {
		// Place an entity at various locations, checking that it does not place on invalid tiles
		GENERATE_TEST_WORLD

		const auto entity = std::make_unique<jactorio::data::Container_entity>();

		const auto chunk = world_data.get_chunk(0, 0);

		EXPECT_EQ(jactorio::game::placement_c::place_entity_at_coords(world_data, entity.get(), 0, 0), true);

		// Set entity and sprite layer
		EXPECT_EQ(
			chunk->tiles_ptr()[0].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity)
			,
			entity.get());
	}

	TEST(placement_controller, place_entity_1x1_invalid) {
		// Place an entity at various locations, checking that it does not place on invalid tiles
		GENERATE_TEST_WORLD

		const auto entity = std::make_unique<jactorio::data::Container_entity>();

		// Invalid, placing on a base tile which is water
		const auto chunk = world_data.get_chunk(0, 0);

		EXPECT_EQ(jactorio::game::placement_c::place_entity_at_coords(world_data, entity.get(), 1, 0), false);
		EXPECT_EQ(
			chunk->tiles_ptr()[0].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity)
			,
			nullptr);

	}


	TEST(placement_controller, remove_entity_1x1_valid) {
		// An existing tile location should have its entity and layer sprite pointer set to nullptr
		// To remove, pass a nullptr as entity
		GENERATE_TEST_WORLD

		const auto entity = std::make_unique<jactorio::data::Container_entity>();


		const auto chunk = world_data.get_chunk(0, 0);

		// Place entity, taken from the test above (place_entity_1x1_valid)
		{
			// const auto chunk = world_data.get_chunk(0, 0);

			EXPECT_TRUE(jactorio::game::placement_c::place_entity_at_coords(world_data, entity.get(), 0, 0));

			// Set entity and sprite layer
			EXPECT_EQ(
				chunk->tiles_ptr()[0].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity)
				,
				entity.get());
		}

		// Valid Removal
		EXPECT_TRUE(jactorio::game::placement_c::place_entity_at_coords(world_data, nullptr, 0, 0));

		// Should all be nullptr after being removed
		EXPECT_EQ(
			chunk->tiles_ptr()[0].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity)
			,
			nullptr);
	}

	TEST(placement_controller, remove_entity_1x1_invalid) {
		// Removing a location with nullptr entity and sprite does nothing, returns false to indicate nothing was removed
		GENERATE_TEST_WORLD

		const auto chunk = world_data.get_chunk(0, 0);

		// Invalid Removal
		EXPECT_FALSE(jactorio::game::placement_c::place_entity_at_coords(world_data, nullptr, 0, 0));

		// Should all remain nullptr
		EXPECT_EQ(
			chunk->tiles_ptr()[0].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity)
			,
			nullptr);
	}


	TEST(placement_controller, place_entity_3x3_valid) {
		// For entities spanning > 1 tiles, the given location is the top left of the entity

		// Place an entity at various locations, checking that it does not place on invalid tiles
		GENERATE_TEST_WORLD

		const auto entity = std::make_unique<jactorio::data::Container_entity>();
		entity->tile_width = 3;
		entity->tile_height = 3;
		const auto chunk = world_data.get_chunk(0, 0);


		EXPECT_TRUE(jactorio::game::placement_c::place_entity_at_coords(world_data, entity.get(), 5, 5));

		// Expect entity and sprite layer to be set, as well as entity_index
		int entity_index = 0;
		for (int y = 5; y < 5 + 3; ++y) {
			for (int x = 5; x < 5 + 3; ++x) {
				const auto index = y * 32 + x;
				EXPECT_EQ(
					chunk->tiles_ptr()[index].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity)
					,
					entity.get());

				// Should count up according to the rules specified in entity_index
				EXPECT_EQ(
					chunk->tiles_ptr()[index].get_layer(jactorio::game::Chunk_tile::chunkLayer::entity).
					multi_tile_index, entity_index++);
			}
		}

	}

	TEST(placement_controller, place_entity_3x3_invalid_1) {
		// For entities spanning > 1 tiles, the given location is the top left of the entity

		// Place an entity at various locations, checking that it does not place on invalid tiles
		GENERATE_TEST_WORLD

		const auto entity = std::make_unique<jactorio::data::Container_entity>();
		entity->tile_width = 3;
		entity->tile_height = 3;
		const auto chunk = world_data.get_chunk(0, 0);


		EXPECT_FALSE(jactorio::game::placement_c::place_entity_at_coords(world_data, entity.get(), 4, 5));

		// Expect entity and sprite layer to be set, as well as entity_index
		for (int y = 5; y < 5 + 3; ++y) {
			for (int x = 5; x < 5 + 3; ++x) {
				const auto index = y * 32 + x;
				EXPECT_EQ(
					chunk->tiles_ptr()[index].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity)
					,
					nullptr);
			}
		}

	}

	TEST(placement_controller, place_entity_3x3_invalid_2) {
		// For entities spanning > 1 tiles, the given location is the top left of the entity

		// Place an entity at various locations, checking that it does not place on invalid tiles
		GENERATE_TEST_WORLD

		const auto entity = std::make_unique<jactorio::data::Container_entity>();
		entity->tile_width = 3;
		entity->tile_height = 3;
		const auto chunk = world_data.get_chunk(0, 0);


		EXPECT_FALSE(jactorio::game::placement_c::place_entity_at_coords(world_data, entity.get(), 9, 2));

		// Expect entity and sprite layer to be set, as well as entity_index
		for (int y = 5; y < 5 + 3; ++y) {
			for (int x = 5; x < 5 + 3; ++x) {
				const auto index = y * 32 + x;
				EXPECT_EQ(
					chunk->tiles_ptr()[index].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity)
					,
					nullptr);
			}
		}

	}

	TEST(placement_controller, place_entity_3x3_invalid_3) {
		// When the placed entity overlaps another entity, the placement is also invalid
		GENERATE_TEST_WORLD

		const auto entity = std::make_unique<jactorio::data::Container_entity>();
		entity->tile_width = 3;
		entity->tile_height = 3;


		EXPECT_TRUE(jactorio::game::placement_c::place_entity_at_coords(world_data, entity.get(), 9, 10));
		EXPECT_FALSE(jactorio::game::placement_c::place_entity_at_coords(world_data, entity.get(), 9, 9));

	}

	TEST(placement_controller, remove_entity_3x3_valid_1) {
		// When removing an entity, specifying anywhere will remove the entire entity
		GENERATE_TEST_WORLD

		const auto entity = std::make_unique<jactorio::data::Container_entity>();
		entity->tile_width = 3;
		entity->tile_height = 3;
		const auto chunk = world_data.get_chunk(0, 0);


		EXPECT_TRUE(jactorio::game::placement_c::place_entity_at_coords(world_data, entity.get(), 5, 5));
		EXPECT_TRUE(jactorio::game::placement_c::place_entity_at_coords(world_data, nullptr, 5, 5));

		// Check that it has been deleted
		for (int y = 5; y < 5 + 3; ++y) {
			for (int x = 5; x < 5 + 3; ++x) {
				const auto index = y * 32 + x;
				EXPECT_EQ(chunk->tiles_ptr()[index].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity)
				          ,
				          nullptr);

				// Entity index is undefined since no entity exists now
			}
		}
	}

	TEST(placement_controller, remove_entity_3x3_valid_2) {
		// When removing an entity, specifying anywhere will remove the entire entity
		GENERATE_TEST_WORLD

		const auto entity = std::make_unique<jactorio::data::Container_entity>();
		entity->tile_width = 3;
		entity->tile_height = 3;
		const auto chunk = world_data.get_chunk(0, 0);


		EXPECT_TRUE(jactorio::game::placement_c::place_entity_at_coords(world_data, entity.get(), 5, 5));
		EXPECT_TRUE(jactorio::game::placement_c::place_entity_at_coords(world_data, nullptr, 7, 5));
		// Check that it has been deleted
		for (int y = 5; y < 5 + 3; ++y) {
			for (int x = 5; x < 5 + 3; ++x) {
				const auto index = y * 32 + x;
				EXPECT_EQ(chunk->tiles_ptr()[index].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity)
				          ,
				          nullptr);

				// Entity index is undefined since no entity exists now
			}
		}
	}


	// 3 x 4

	TEST(placement_controller, place_entity_3x4_valid) {
		// Ensure that irregular shaped multi-tiles fully remove

		// For entities spanning > 1 tiles, the given location is the top left of the entity
		GENERATE_TEST_WORLD

		const auto entity = std::make_unique<jactorio::data::Container_entity>();
		entity->tile_width = 3;
		entity->tile_height = 4;
		const auto chunk = world_data.get_chunk(0, 0);


		EXPECT_TRUE(jactorio::game::placement_c::place_entity_at_coords(world_data, entity.get(), 9, 10));

		// Expect entity and sprite layer to be set, as well as entity_index
		int entity_index = 0;
		for (int y = 10; y < 10 + 4; ++y) {
			for (int x = 9; x < 9 + 3; ++x) {
				const auto index = y * 32 + x;
				EXPECT_EQ(
					chunk->tiles_ptr()[index].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity)
					,
					entity.get());

				// Should count up according to the rules specified in entity_index
				EXPECT_EQ(
					chunk->tiles_ptr()[index].get_layer(jactorio::game::Chunk_tile::chunkLayer::entity).
					multi_tile_index, entity_index++);


				// Ensure tile width and height are properly set
				EXPECT_EQ(
					chunk->tiles_ptr()[index].get_layer(jactorio::game::Chunk_tile::chunkLayer::entity).multi_tile_span,
					3
				);
				EXPECT_EQ(
					chunk->tiles_ptr()[index].get_layer(jactorio::game::Chunk_tile::chunkLayer::entity).
					multi_tile_height,
					4
				);
			}
		}
	}

	TEST(placement_controller, remove_entity_3x4_valid) {
		// Ensure that irregular shaped multi-tiles fully remove
		GENERATE_TEST_WORLD

		const auto entity = std::make_unique<jactorio::data::Container_entity>();
		entity->tile_width = 3;
		entity->tile_height = 4;
		const auto chunk = world_data.get_chunk(0, 0);


		EXPECT_TRUE(jactorio::game::placement_c::place_entity_at_coords(world_data, entity.get(), 9, 10));
		EXPECT_TRUE(jactorio::game::placement_c::place_entity_at_coords(world_data, nullptr, 9, 13));

		// Expect entity and sprite layer to be set, as well as entity_index
		for (int y = 10; y < 10 + 4; ++y) {
			for (int x = 9; x < 9 + 3; ++x) {
				const auto index = y * 32 + x;
				EXPECT_EQ(chunk->tiles_ptr()[index].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity)
				          ,
				          nullptr);
			}
		}
	}
}
