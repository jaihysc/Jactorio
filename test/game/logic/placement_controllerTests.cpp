#include <gtest/gtest.h>

#include "core/resource_guard.h"
#include "game/logic/placement_controller.h"
#include "game/world/world_manager.h"
#include "data/prototype/entity/container_entity.h"

#include <memory>

namespace game::logic
{
	void generate_test_world() {
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

		// Water tile, land tile prototype
		// Must be manually deleted!!
		auto* water_tile = new jactorio::data::Tile();
		water_tile->is_water = true;

		auto* land_tile = new jactorio::data::Tile();
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
				
				chunk_tiles[y * 32 + x]
					.set_tile_layer_tile_prototype(Chunk_tile::chunk_layer::base, tile_ptr);
			}
			// printf("\n");
		}

		world_manager::add_chunk(new Chunk(0, 0, chunk_tiles));
	}

	void clear_test_world_data() {
		using namespace jactorio::game;
		// Delete the land and water tile prototype
		const auto chunk = world_manager::get_chunk(0, 0);
		
		delete chunk->tiles_ptr()[0].get_tile_layer_tile_prototype(Chunk_tile::chunk_layer::entity);
		delete chunk->tiles_ptr()[1].get_tile_layer_tile_prototype(Chunk_tile::chunk_layer::entity);

		// Delete chunk tiles
		world_manager::clear_chunk_data();
	}

	//
	// !Checking if the selected tile is too far from the player is not done in these tests!
	//
	
	TEST(placement_controller, place_entity_1x1_valid) {
		// Place an entity at various locations, checking that it does not place on invalid tiles
		
		auto guard = jactorio::core::Resource_guard(&clear_test_world_data);
		generate_test_world();
		
		const auto entity = std::make_unique<jactorio::data::Container_entity>();

		const auto chunk = jactorio::game::world_manager::get_chunk(0, 0);

		EXPECT_EQ(jactorio::game::placement_c::place_entity_at_coords(entity.get(), 0, 0), true);

		// Set entity and sprite layer
		EXPECT_EQ(chunk->tiles_ptr()[0].entity, entity.get());
		EXPECT_EQ(
			chunk->tiles_ptr()[0].get_tile_layer_sprite_prototype(jactorio::game::Chunk_tile::chunk_layer::entity),
			entity.get()->sprite);
	}
	
	TEST(placement_controller, place_entity_1x1_invalid) {
		// Place an entity at various locations, checking that it does not place on invalid tiles

		auto guard = jactorio::core::Resource_guard(&clear_test_world_data);
		generate_test_world();

		const auto entity = std::make_unique<jactorio::data::Container_entity>();

		// Invalid, placing on a base tile which is water
		const auto chunk = jactorio::game::world_manager::get_chunk(0, 0);

		EXPECT_EQ(jactorio::game::placement_c::place_entity_at_coords(entity.get(), 1, 0), false);
		EXPECT_EQ(chunk->tiles_ptr()[1].entity, nullptr);
		EXPECT_EQ(
			chunk->tiles_ptr()[0].get_tile_layer_sprite_prototype(jactorio::game::Chunk_tile::chunk_layer::entity),
			nullptr);

	}

	
	TEST(placement_controller, remove_entity_1x1_valid) {
		// An existing tile location should have its entity and layer sprite pointer set to nullptr
		// To remove, pass a nullptr as entity

		auto guard = jactorio::core::Resource_guard(&clear_test_world_data);
		generate_test_world();

		const auto entity = std::make_unique<jactorio::data::Container_entity>();


		const auto chunk = jactorio::game::world_manager::get_chunk(0, 0);

		// Place entity, taken from the test above (place_entity_1x1_valid)
		{
			const auto chunk = jactorio::game::world_manager::get_chunk(0, 0);

			EXPECT_EQ(jactorio::game::placement_c::place_entity_at_coords(entity.get(), 0, 0), true);

			// Set entity and sprite layer
			EXPECT_EQ(chunk->tiles_ptr()[0].entity, entity.get());
			EXPECT_EQ(
				chunk->tiles_ptr()[0].get_tile_layer_sprite_prototype(jactorio::game::Chunk_tile::chunk_layer::entity),
				entity.get()->sprite);
		}

		// Valid Removal
		EXPECT_EQ(jactorio::game::placement_c::place_entity_at_coords(nullptr, 0, 0), true);

		// Should all be nullptr after being removed
		EXPECT_EQ(chunk->tiles_ptr()[0].entity, nullptr);
		EXPECT_EQ(
			chunk->tiles_ptr()[0].get_tile_layer_sprite_prototype(jactorio::game::Chunk_tile::chunk_layer::entity),
			nullptr);
	}

	TEST(placement_controller, remove_entity_1x1_invalid) {
		// Removing a location with nullptr entity and sprite does nothing, returns false to indicate nothing was removed

		auto guard = jactorio::core::Resource_guard(&clear_test_world_data);
		generate_test_world();

		const auto chunk = jactorio::game::world_manager::get_chunk(0, 0);

		// Invalid Removal
		EXPECT_EQ(jactorio::game::placement_c::place_entity_at_coords(nullptr, 0, 0), false);

		// Should all remain nullptr
		EXPECT_EQ(chunk->tiles_ptr()[0].entity, nullptr);
		EXPECT_EQ(
			chunk->tiles_ptr()[0].get_tile_layer_sprite_prototype(jactorio::game::Chunk_tile::chunk_layer::entity),
			nullptr);
	}

	
	TEST(placement_controller, place_entity_3x3_valid) {
		// For entities spanning > 1 tiles, the given location is the top left of the entity

		// Place an entity at various locations, checking that it does not place on invalid tiles

		auto guard = jactorio::core::Resource_guard(&clear_test_world_data);
		generate_test_world();

		const auto entity = std::make_unique<jactorio::data::Container_entity>();
		entity->tile_width = 3;
		entity->tile_height = 3;
		const auto chunk = jactorio::game::world_manager::get_chunk(0, 0);

		
		EXPECT_EQ(jactorio::game::placement_c::place_entity_at_coords(entity.get(), 5, 5), true);

		// Expect entity and sprite layer to be set, as well as entity_index
		int entity_index = 0;
		for (int y = 5; y < 5 + 3; ++y) {
			for (int x = 5; x < 5 + 3; ++x) {
				const auto index = y * 32 + x;
				EXPECT_EQ(chunk->tiles_ptr()[index].entity, entity.get());
				EXPECT_EQ(
					chunk->tiles_ptr()[index].get_tile_layer_sprite_prototype(
						jactorio::game::Chunk_tile::chunk_layer::entity),
					entity.get()->sprite);

				// Should count up according to the rules specified in entity_index
				EXPECT_EQ(
					chunk->tiles_ptr()[index].get_layer(jactorio::game::Chunk_tile::chunk_layer::entity).
					multi_tile_index, entity_index++);
			}
		}
		
	}

	TEST(placement_controller, place_entity_3x3_invalid_1) {
		// For entities spanning > 1 tiles, the given location is the top left of the entity

		// Place an entity at various locations, checking that it does not place on invalid tiles

		auto guard = jactorio::core::Resource_guard(&clear_test_world_data);
		generate_test_world();

		const auto entity = std::make_unique<jactorio::data::Container_entity>();
		entity->tile_width = 3;
		entity->tile_height = 3;
		const auto chunk = jactorio::game::world_manager::get_chunk(0, 0);


		EXPECT_EQ(jactorio::game::placement_c::place_entity_at_coords(entity.get(), 4, 5), false);

		// Expect entity and sprite layer to be set, as well as entity_index
		for (int y = 5; y < 5 + 3; ++y) {
			for (int x = 5; x < 5 + 3; ++x) {
				const auto index = y * 32 + x;
				EXPECT_EQ(chunk->tiles_ptr()[index].entity, nullptr);
				EXPECT_EQ(
					chunk->tiles_ptr()[index].get_tile_layer_sprite_prototype(
						jactorio::game::Chunk_tile::chunk_layer::entity),
					nullptr);
			}
		}

	}

	TEST(placement_controller, place_entity_3x3_invalid_2) {
		// For entities spanning > 1 tiles, the given location is the top left of the entity

		// Place an entity at various locations, checking that it does not place on invalid tiles

		auto guard = jactorio::core::Resource_guard(&clear_test_world_data);
		generate_test_world();

		const auto entity = std::make_unique<jactorio::data::Container_entity>();
		entity->tile_width = 3;
		entity->tile_height = 3;
		const auto chunk = jactorio::game::world_manager::get_chunk(0, 0);


		EXPECT_EQ(jactorio::game::placement_c::place_entity_at_coords(entity.get(), 9, 2), false);

		// Expect entity and sprite layer to be set, as well as entity_index
		for (int y = 5; y < 5 + 3; ++y) {
			for (int x = 5; x < 5 + 3; ++x) {
				const auto index = y * 32 + x;
				EXPECT_EQ(chunk->tiles_ptr()[index].entity, nullptr);
				EXPECT_EQ(
					chunk->tiles_ptr()[index].get_tile_layer_sprite_prototype(
						jactorio::game::Chunk_tile::chunk_layer::entity),
					nullptr);
			}
		}

	}

	TEST(placement_controller, place_entity_3x3_invalid_3) {
		// When the placed entity overlaps another entity, the placement is also invalid

		auto guard = jactorio::core::Resource_guard(&clear_test_world_data);
		generate_test_world();

		const auto entity = std::make_unique<jactorio::data::Container_entity>();
		entity->tile_width = 3;
		entity->tile_height = 3;
		const auto chunk = jactorio::game::world_manager::get_chunk(0, 0);


		EXPECT_EQ(jactorio::game::placement_c::place_entity_at_coords(entity.get(), 9, 10), true);
		EXPECT_EQ(jactorio::game::placement_c::place_entity_at_coords(entity.get(), 9, 9), false);

	}
	
	TEST(placement_controller, remove_entity_3x3_valid_1) {
		// When removing an entity, specifying anywhere will remove the entire entity

		auto guard = jactorio::core::Resource_guard(&clear_test_world_data);
		generate_test_world();

		const auto entity = std::make_unique<jactorio::data::Container_entity>();
		entity->tile_width = 3;
		entity->tile_height = 3;
		const auto chunk = jactorio::game::world_manager::get_chunk(0, 0);


		EXPECT_EQ(jactorio::game::placement_c::place_entity_at_coords(entity.get(), 5, 5), true);
		
		EXPECT_EQ(jactorio::game::placement_c::place_entity_at_coords(nullptr, 5, 5), true);
		// Check that it has been deleted
		for (int y = 5; y < 5 + 3; ++y) {
			for (int x = 5; x < 5 + 3; ++x) {
				const auto index = y * 32 + x;
				EXPECT_EQ(chunk->tiles_ptr()[index].entity, nullptr);
				EXPECT_EQ(chunk->tiles_ptr()[index].get_tile_layer_sprite_prototype(
					          jactorio::game::Chunk_tile::chunk_layer::entity),
				          nullptr);

				// Entity index is undefined since no entity exists now
			}
		}
	}

	TEST(placement_controller, remove_entity_3x3_valid_2) {
		// When removing an entity, specifying anywhere will remove the entire entity

		auto guard = jactorio::core::Resource_guard(&clear_test_world_data);
		generate_test_world();

		const auto entity = std::make_unique<jactorio::data::Container_entity>();
		entity->tile_width = 3;
		entity->tile_height = 3;
		const auto chunk = jactorio::game::world_manager::get_chunk(0, 0);


		EXPECT_EQ(jactorio::game::placement_c::place_entity_at_coords(entity.get(), 5, 5), true);

		EXPECT_EQ(jactorio::game::placement_c::place_entity_at_coords(nullptr, 7, 5), true);
		// Check that it has been deleted
		for (int y = 5; y < 5 + 3; ++y) {
			for (int x = 5; x < 5 + 3; ++x) {
				const auto index = y * 32 + x;
				EXPECT_EQ(chunk->tiles_ptr()[index].entity, nullptr);
				EXPECT_EQ(chunk->tiles_ptr()[index].get_tile_layer_sprite_prototype(
					          jactorio::game::Chunk_tile::chunk_layer::entity),
				          nullptr);

				// Entity index is undefined since no entity exists now
			}
		}
	}

	
	// 3 x 4

	TEST(placement_controller, place_entity_3x4_valid) {
		// Ensure that irregular shaped multi-tiles fully remove

		// For entities spanning > 1 tiles, the given location is the top left of the entity

		auto guard = jactorio::core::Resource_guard(&clear_test_world_data);
		generate_test_world();

		const auto entity = std::make_unique<jactorio::data::Container_entity>();
		entity->tile_width = 3;
		entity->tile_height = 4;
		const auto chunk = jactorio::game::world_manager::get_chunk(0, 0);


		EXPECT_EQ(jactorio::game::placement_c::place_entity_at_coords(entity.get(), 9, 10), true);

		// Expect entity and sprite layer to be set, as well as entity_index
		int entity_index = 0;
		for (int y = 10; y < 10 + 4; ++y) {
			for (int x = 9; x < 9 + 3; ++x) {
				const auto index = y * 32 + x;
				EXPECT_EQ(chunk->tiles_ptr()[index].entity, entity.get());
				EXPECT_EQ(
					chunk->tiles_ptr()[index].get_tile_layer_sprite_prototype(
						jactorio::game::Chunk_tile::chunk_layer::entity),
					entity.get()->sprite);

				// Should count up according to the rules specified in entity_index
				EXPECT_EQ(
					chunk->tiles_ptr()[index].get_layer(jactorio::game::Chunk_tile::chunk_layer::entity).
					multi_tile_index, entity_index++);

				
				// Ensure tile width and height are properly set
				EXPECT_EQ(
					chunk->tiles_ptr()[index].get_layer(jactorio::game::Chunk_tile::chunk_layer::entity).multi_tile_span,
					3
				);
				EXPECT_EQ(
					chunk->tiles_ptr()[index].get_layer(jactorio::game::Chunk_tile::chunk_layer::entity).multi_tile_height,
					4
				);
			}
		}
	}

	TEST(placement_controller, remove_entity_3x4_valid) {
		// Ensure that irregular shaped multi-tiles fully remove
		
		auto guard = jactorio::core::Resource_guard(&clear_test_world_data);
		generate_test_world();

		const auto entity = std::make_unique<jactorio::data::Container_entity>();
		entity->tile_width = 3;
		entity->tile_height = 4;
		const auto chunk = jactorio::game::world_manager::get_chunk(0, 0);


		EXPECT_EQ(jactorio::game::placement_c::place_entity_at_coords(entity.get(), 9, 10), true);
		EXPECT_EQ(jactorio::game::placement_c::place_entity_at_coords(nullptr, 9, 13), true);

		// Expect entity and sprite layer to be set, as well as entity_index
		for (int y = 10; y < 10 + 4; ++y) {
			for (int x = 9; x < 9 + 3; ++x) {
				const auto index = y * 32 + x;
				EXPECT_EQ(chunk->tiles_ptr()[index].entity, nullptr);
				EXPECT_EQ(chunk->tiles_ptr()[index].get_tile_layer_sprite_prototype(
					          jactorio::game::Chunk_tile::chunk_layer::entity),
				          nullptr);
			}
		}
	}
}
