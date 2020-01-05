#include <gtest/gtest.h>

#include "core/resource_guard.h"
#include "game/logic/entity_place_controller.h"
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
			auto* tile_ptr = land_tile;

			// Water at Y index 1, 4, 8
			if (y == 1 || y == 4 || y == 8)
				tile_ptr = water_tile;
			
			for (int x = 0; x < 32; ++x) {
				// Water at X index 1, 4, 8
				if (x == 1 || x == 4 || x == 8)
					tile_ptr = water_tile;
				
				chunk_tiles[y * 32 + x]
					.set_tile_layer_tile_prototype(Chunk_tile::chunk_layer::base, tile_ptr);
			}
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
	
	TEST(entity_place_controller, place_entity_1x1_valid) {
		// Place an entity at various locations, checking that it does not place on invalid tiles
		
		auto guard = jactorio::core::Resource_guard(&clear_test_world_data);
		generate_test_world();
		
		const auto entity = std::make_unique<jactorio::data::Container_entity>();

		const auto chunk = jactorio::game::world_manager::get_chunk(0, 0);

		EXPECT_EQ(jactorio::game::logic::place_entity_at_coords_unranged(entity.get(), 0, 0), true);

		// Set entity and sprite layer
		EXPECT_EQ(chunk->tiles_ptr()[0].entity, entity.get());
		EXPECT_EQ(
			chunk->tiles_ptr()[0].get_tile_layer_sprite_prototype(jactorio::game::Chunk_tile::chunk_layer::entity),
			entity.get()->sprite);
	}
	
	TEST(entity_place_controller, place_entity_1x1_invalid) {
		// Place an entity at various locations, checking that it does not place on invalid tiles

		auto guard = jactorio::core::Resource_guard(&clear_test_world_data);
		generate_test_world();

		const auto entity = std::make_unique<jactorio::data::Container_entity>();

		// Invalid, placing on a base tile which is water
		const auto chunk = jactorio::game::world_manager::get_chunk(0, 0);

		EXPECT_EQ(jactorio::game::logic::place_entity_at_coords_unranged(entity.get(), 1, 0), false);
		EXPECT_EQ(chunk->tiles_ptr()[1].entity, nullptr);
		EXPECT_EQ(
			chunk->tiles_ptr()[0].get_tile_layer_sprite_prototype(jactorio::game::Chunk_tile::chunk_layer::entity),
			nullptr);

	}

	
	TEST(entity_place_controller, remove_entity_1x1_valid) {
		// An existing tile location should have its entity and layer sprite pointer set to nullptr
		// To remove, pass a nullptr as entity

		auto guard = jactorio::core::Resource_guard(&clear_test_world_data);
		generate_test_world();

		const auto entity = std::make_unique<jactorio::data::Container_entity>();


		const auto chunk = jactorio::game::world_manager::get_chunk(0, 0);

		// Place entity, taken from the test above (place_entity_1x1_valid)
		{
			const auto chunk = jactorio::game::world_manager::get_chunk(0, 0);

			EXPECT_EQ(jactorio::game::logic::place_entity_at_coords_unranged(entity.get(), 0, 0), true);

			// Set entity and sprite layer
			EXPECT_EQ(chunk->tiles_ptr()[0].entity, entity.get());
			EXPECT_EQ(
				chunk->tiles_ptr()[0].get_tile_layer_sprite_prototype(jactorio::game::Chunk_tile::chunk_layer::entity),
				entity.get()->sprite);
		}

		// Valid Removal
		EXPECT_EQ(jactorio::game::logic::place_entity_at_coords_unranged(nullptr, 0, 0), true);

		// Should all be nullptr after being removed
		EXPECT_EQ(chunk->tiles_ptr()[0].entity, nullptr);
		EXPECT_EQ(
			chunk->tiles_ptr()[0].get_tile_layer_sprite_prototype(jactorio::game::Chunk_tile::chunk_layer::entity),
			nullptr);
	}

	TEST(entity_place_controller, remove_entity_1x1_invalid) {
		// Removing a location with nullptr entity and sprite does nothing, returns false to indicate nothing was removed

		auto guard = jactorio::core::Resource_guard(&clear_test_world_data);
		generate_test_world();

		const auto chunk = jactorio::game::world_manager::get_chunk(0, 0);

		// Invalid Removal
		EXPECT_EQ(jactorio::game::logic::place_entity_at_coords_unranged(nullptr, 0, 0), false);

		// Should all remain nullptr
		EXPECT_EQ(chunk->tiles_ptr()[0].entity, nullptr);
		EXPECT_EQ(
			chunk->tiles_ptr()[0].get_tile_layer_sprite_prototype(jactorio::game::Chunk_tile::chunk_layer::entity),
			nullptr);
	}

	
	TEST(entity_place_controller, place_entity_3x3) {
		// For entities spanning > 1 tiles, the given location is the top left of the entity
		// TODO
		// FAIL();
	}
}
