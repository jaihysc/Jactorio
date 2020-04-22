// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 01/20/2020

#include <gtest/gtest.h>

#include "game/logic/placement_controller.h"

#include <memory>

#include "data/prototype/entity/container_entity.h"
#include "data/prototype/entity/entity.h"
#include "data/prototype/tile/tile.h"
#include "game/world/world_data.h"

namespace game
{
	// NOTE
	// !Checking if the selected tile is too far from the player is not done in these tests!
	//

	// Creates world_data and generates a test world within it
	class PlacementControllerTest : public testing::Test
	{
	public:
		static constexpr auto chunk_width = jactorio::game::World_data::chunk_width;

	private:
		static void generate_test_world(jactorio::game::World_data& world_data,
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
			land_tile->is_water  = false;


			using namespace jactorio::game;
			// Create chunk
			auto* chunk_tiles = new Chunk_tile[chunk_width * chunk_width];
			for (int y = 0; y < chunk_width; ++y) {
				bool y_water = false;

				// Water at Y index 1, 4, 8
				if (y == 1 || y == 4 || y == 8)
					y_water = true;

				for (int x = 0; x < chunk_width; ++x) {
					bool x_water = false;

					// Water at X index 1, 4, 8
					if (x == 1 || x == 4 || x == 8)
						x_water = true;

					auto* tile_ptr = land_tile;
					if (x_water || y_water) {
						tile_ptr = water_tile;
					}

					chunk_tiles[y * chunk_width + x].set_tile_prototype(Chunk_tile::chunkLayer::base, tile_ptr);
				}
			}

			world_data.add_chunk(new Chunk(0, 0, chunk_tiles));
		}

	protected:
		std::unique_ptr<jactorio::data::Tile> water_tile_ = std::make_unique<jactorio::data::Tile>();
		std::unique_ptr<jactorio::data::Tile> land_tile_  = std::make_unique<jactorio::data::Tile>();

		jactorio::game::World_data world_data_{};

		void SetUp() override {
			generate_test_world(world_data_, water_tile_.get(), land_tile_.get());

		}
	};


	TEST_F(PlacementControllerTest, PlaceEntity1x1Valid) {
		// Place an entity at various locations, checking that it does not place on invalid tiles
		const auto entity = std::make_unique<jactorio::data::Container_entity>();

		const auto chunk = world_data_.get_chunk(0, 0);

		EXPECT_EQ(jactorio::game::place_entity_at_coords(world_data_, entity.get(), 0, 0), true);

		// Set entity and sprite layer
		EXPECT_EQ(
			chunk->tiles_ptr()[0].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity),
			entity.get());

		EXPECT_FALSE(chunk->tiles_ptr()[0].get_layer(jactorio::game::Chunk_tile::chunkLayer::entity).is_multi_tile());
	}

	TEST_F(PlacementControllerTest, PlaceEntity1x1Invalid) {
		// Place an entity at various locations, checking that it does not place on invalid tiles
		const auto entity = std::make_unique<jactorio::data::Container_entity>();

		// Invalid, placing on a base tile which is water
		const auto chunk = world_data_.get_chunk(0, 0);

		EXPECT_EQ(jactorio::game::place_entity_at_coords(world_data_, entity.get(), 1, 0), false);
		EXPECT_EQ(
			chunk->tiles_ptr()[0].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity),
			nullptr);

	}


	TEST_F(PlacementControllerTest, RemoveEntity1x1Valid) {
		// An existing tile location should have its entity and layer sprite pointer set to nullptr
		// To remove, pass a nullptr as entity
		const auto entity = std::make_unique<jactorio::data::Container_entity>();


		const auto chunk = world_data_.get_chunk(0, 0);

		// Place entity, taken from the test above (place_entity_1x1_valid)
		{
			// const auto chunk = world_data.get_chunk(0, 0);

			EXPECT_TRUE(jactorio::game::place_entity_at_coords(world_data_, entity.get(), 0, 0));

			// Set entity and sprite layer
			EXPECT_EQ(
				chunk->tiles_ptr()[0].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity),
				entity.get());
		}

		// Valid Removal
		EXPECT_TRUE(jactorio::game::place_entity_at_coords(world_data_, nullptr, 0, 0));

		// Should all be nullptr after being removed
		EXPECT_EQ(
			chunk->tiles_ptr()[0].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity),
			nullptr);
	}

	TEST_F(PlacementControllerTest, RemoveEntity1x1Invalid) {
		// Removing a location with nullptr entity and sprite does nothing, returns false to indicate nothing was removed
		const auto chunk = world_data_.get_chunk(0, 0);

		// Invalid Removal
		EXPECT_FALSE(jactorio::game::place_entity_at_coords(world_data_, nullptr, 0, 0));

		// Should all remain nullptr
		EXPECT_EQ(
			chunk->tiles_ptr()[0].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity),
			nullptr);
	}


	TEST_F(PlacementControllerTest, PlaceEntity3x3Valid) {
		// For entities spanning > 1 tiles, the given location is the top left of the entity

		// Place an entity at various locations, checking that it does not place on invalid tiles
		const auto entity   = std::make_unique<jactorio::data::Container_entity>();
		entity->tile_width  = 3;
		entity->tile_height = 3;

		const auto chunk = world_data_.get_chunk(0, 0);


		EXPECT_TRUE(jactorio::game::place_entity_at_coords(world_data_, entity.get(), 5, 5));

		// Expect entity and sprite layer to be set, as well as entity_index
		int entity_index = 0;
		for (int y = 5; y < 5 + 3; ++y) {
			for (int x = 5; x < 5 + 3; ++x) {
				const auto index = y * chunk_width + x;
				EXPECT_EQ(
					chunk->tiles_ptr()[index].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity),
					entity.get());

				// Should count up according to the rules specified in entity_index
				EXPECT_EQ(
					chunk->tiles_ptr()[index].get_layer(jactorio::game::Chunk_tile::chunkLayer::entity).
					multi_tile_index,
					entity_index++);
			}
		}

	}

	TEST_F(PlacementControllerTest, PlaceEntity3x3Invalid1) {
		// For entities spanning > 1 tiles, the given location is the top left of the entity

		// Place an entity at various locations, checking that it does not place on invalid tiles
		const auto entity   = std::make_unique<jactorio::data::Container_entity>();
		entity->tile_width  = 3;
		entity->tile_height = 3;

		const auto chunk = world_data_.get_chunk(0, 0);


		EXPECT_FALSE(jactorio::game::place_entity_at_coords(world_data_, entity.get(), 4, 5));

		// Expect entity and sprite layer to be set, as well as entity_index
		for (int y = 5; y < 5 + 3; ++y) {
			for (int x = 5; x < 5 + 3; ++x) {
				const auto index = y * chunk_width + x;
				EXPECT_EQ(
					chunk->tiles_ptr()[index].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity),
					nullptr);
			}
		}

	}

	TEST_F(PlacementControllerTest, PlaceEntity3x3Invalid2) {
		// For entities spanning > 1 tiles, the given location is the top left of the entity

		// Place an entity at various locations, checking that it does not place on invalid tiles
		const auto entity   = std::make_unique<jactorio::data::Container_entity>();
		entity->tile_width  = 3;
		entity->tile_height = 3;

		const auto chunk = world_data_.get_chunk(0, 0);


		EXPECT_FALSE(jactorio::game::place_entity_at_coords(world_data_, entity.get(), 9, 2));

		// Expect entity and sprite layer to be set, as well as entity_index
		for (int y = 5; y < 5 + 3; ++y) {
			for (int x = 5; x < 5 + 3; ++x) {
				const auto index = y * chunk_width + x;
				EXPECT_EQ(
					chunk->tiles_ptr()[index].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity),
					nullptr);
			}
		}

	}

	TEST_F(PlacementControllerTest, PlaceEntity3x3Invalid3) {
		// When the placed entity overlaps another entity, the placement is also invalid
		const auto entity   = std::make_unique<jactorio::data::Container_entity>();
		entity->tile_width  = 3;
		entity->tile_height = 3;


		EXPECT_TRUE(jactorio::game::place_entity_at_coords(world_data_, entity.get(), 9, 10));
		EXPECT_FALSE(jactorio::game::place_entity_at_coords(world_data_, entity.get(), 9, 9));

	}

	TEST_F(PlacementControllerTest, RemoveEntity3x3Valid1) {
		// When removing an entity, specifying anywhere will remove the entire entity
		const auto entity   = std::make_unique<jactorio::data::Container_entity>();
		entity->tile_width  = 3;
		entity->tile_height = 3;

		const auto chunk = world_data_.get_chunk(0, 0);


		EXPECT_TRUE(jactorio::game::place_entity_at_coords(world_data_, entity.get(), 5, 5));
		EXPECT_TRUE(jactorio::game::place_entity_at_coords(world_data_, nullptr, 5, 5));

		// Check that it has been deleted
		for (int y = 5; y < 5 + 3; ++y) {
			for (int x = 5; x < 5 + 3; ++x) {
				const auto index = y * chunk_width + x;
				EXPECT_EQ(chunk->tiles_ptr()[index].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity),
				          nullptr);

				// Entity index is undefined since no entity exists now
			}
		}
	}

	TEST_F(PlacementControllerTest, RemoveEntity3x3Valid2) {
		// When removing an entity, specifying anywhere will remove the entire entity
		const auto entity   = std::make_unique<jactorio::data::Container_entity>();
		entity->tile_width  = 3;
		entity->tile_height = 3;

		const auto chunk = world_data_.get_chunk(0, 0);


		EXPECT_TRUE(jactorio::game::place_entity_at_coords(world_data_, entity.get(), 5, 5));
		EXPECT_TRUE(jactorio::game::place_entity_at_coords(world_data_, nullptr, 7, 5));
		// Check that it has been deleted
		for (int y = 5; y < 5 + 3; ++y) {
			for (int x = 5; x < 5 + 3; ++x) {
				const auto index = y * chunk_width + x;
				EXPECT_EQ(chunk->tiles_ptr()[index].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity),
				          nullptr);

				// Entity index is undefined since no entity exists now
			}
		}
	}


	// 3 x 4

	TEST_F(PlacementControllerTest, PlaceEntity3x4Valid) {
		// Ensure that irregular shaped multi-tiles fully remove

		// For entities spanning > 1 tiles, the given location is the top left of the entity
		const auto entity   = std::make_unique<jactorio::data::Container_entity>();
		entity->tile_width  = 3;
		entity->tile_height = 4;

		const auto chunk = world_data_.get_chunk(0, 0);


		EXPECT_TRUE(jactorio::game::place_entity_at_coords(world_data_, entity.get(), 9, 10));

		// Expect entity and sprite layer to be set, as well as entity_index
		int entity_index = 0;
		for (int y = 10; y < 10 + 4; ++y) {
			for (int x = 9; x < 9 + 3; ++x) {
				const auto index = y * chunk_width + x;
				EXPECT_EQ(
					chunk->tiles_ptr()[index].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity),
					entity.get());

				// Should count up according to the rules specified in entity_index
				EXPECT_EQ(
					chunk->tiles_ptr()[index].get_layer(jactorio::game::Chunk_tile::chunkLayer::entity).
					multi_tile_index,
					entity_index++);


				// Ensure tile width and height are properly set
				EXPECT_EQ(
					chunk->tiles_ptr()[index].get_layer(jactorio::game::Chunk_tile::chunkLayer::entity).get_multi_tile_data()
					.multi_tile_span,
					3
				);
				EXPECT_EQ(
					chunk->tiles_ptr()[index].get_layer(jactorio::game::Chunk_tile::chunkLayer::entity).get_multi_tile_data().
					multi_tile_height,
					4
				);
			}
		}
	}

	TEST_F(PlacementControllerTest, RemoveEntity3x4Valid) {
		// Ensure that irregular shaped multi-tiles fully remove
		const auto entity   = std::make_unique<jactorio::data::Container_entity>();
		entity->tile_width  = 3;
		entity->tile_height = 4;

		const auto chunk = world_data_.get_chunk(0, 0);


		EXPECT_TRUE(jactorio::game::place_entity_at_coords(world_data_, entity.get(), 9, 10));
		EXPECT_TRUE(jactorio::game::place_entity_at_coords(world_data_, nullptr, 9, 13));

		// Expect entity and sprite layer to be set, as well as entity_index
		for (int y = 10; y < 10 + 4; ++y) {
			for (int x = 9; x < 9 + 3; ++x) {
				const auto index = y * chunk_width + x;
				EXPECT_EQ(chunk->tiles_ptr()[index].get_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity),
				          nullptr);
			}
		}
	}


	TEST_F(PlacementControllerTest, PlaceSprite3x3) {
		const auto sprite = std::make_unique<jactorio::data::Sprite>();
		const auto chunk  = world_data_.get_chunk(0, 0);


		place_sprite_at_coords(
			world_data_,
			jactorio::game::Chunk_tile::chunkLayer::overlay,
			sprite.get(),
			3, 3,
			9, 10);

		// Expect entity and sprite layer to be set, as well as entity_index
		for (int y = 10; y < 10 + 3; ++y) {
			for (int x = 9; x < 9 + 3; ++x) {
				const auto index = y * chunk_width + x;
				EXPECT_EQ(chunk->tiles_ptr()[index].get_sprite_prototype(jactorio::game::Chunk_tile::chunkLayer::overlay),
				          sprite.get());
			}
		}

	}

	TEST_F(PlacementControllerTest, RemoveSprite3x3) {
		const auto sprite = std::make_unique<jactorio::data::Sprite>();
		const auto chunk  = world_data_.get_chunk(0, 0);


		place_sprite_at_coords(
			world_data_,
			jactorio::game::Chunk_tile::chunkLayer::overlay,
			sprite.get(),
			3, 3,
			9, 10);

		place_sprite_at_coords(
			world_data_,
			jactorio::game::Chunk_tile::chunkLayer::overlay,
			nullptr,
			3, 3,
			9, 10);

		// Expect entity and sprite layer to be set, as well as entity_index
		for (int y = 10; y < 10 + 3; ++y) {
			for (int x = 9; x < 9 + 3; ++x) {
				const auto index = y * chunk_width + x;

				jactorio::game::Chunk_tile_layer& tile =
					chunk->tiles_ptr()[index].get_layer(jactorio::game::Chunk_tile::chunkLayer::overlay);

				EXPECT_EQ(tile.prototype_data, nullptr);
				EXPECT_EQ(tile.multi_tile_index, 0);
			}
		}

	}
}
