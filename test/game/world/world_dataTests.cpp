// 
// world_dataTests.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 11/09/2019
// Last modified: 03/14/2020
// 

#include <gtest/gtest.h>

#include "game/world/chunk.h"
#include "game/world/world_data.h"

namespace game
{
	TEST(world_manager, world_add_chunk) {
		jactorio::game::World_data world_data{};

		// Chunks initialized with empty tiles
		const auto chunk = new jactorio::game::Chunk{5, 1, nullptr};

		// Returns pointer to chunk which was added
		const auto added_chunk = world_data.add_chunk(chunk);

		// Chunk knows its own location
		EXPECT_EQ(added_chunk->get_position().first, 5);
		EXPECT_EQ(added_chunk->get_position().second, 1);

		// Should not initialize other chunks
		EXPECT_EQ(world_data.get_chunk(-1, -1), nullptr);
		EXPECT_EQ(world_data.get_chunk(1, 1), nullptr);
	}

	TEST(world_manager, world_add_chunk_negative) {
		jactorio::game::World_data world_data{};

		// Chunks initialized with empty tiles
		const auto chunk = new jactorio::game::Chunk{-5, -1, nullptr};

		// Returns pointer to chunk which was added
		const auto added_chunk = world_data.add_chunk(chunk);

		// Chunk knows its own location
		EXPECT_EQ(added_chunk->get_position().first, -5);
		EXPECT_EQ(added_chunk->get_position().second, -1);


		// Should not initialize other chunks
		EXPECT_EQ(world_data.get_chunk(-1, -1), nullptr);
		EXPECT_EQ(world_data.get_chunk(1, 1), nullptr);


		world_data.clear_chunk_data();
	}


	TEST(world_manager, world_add_chunk_override) {
		jactorio::game::World_data world_data{};

		// Chunks initialized with empty tiles
		const auto chunk = new jactorio::game::Chunk{5, 1, nullptr};
		const auto chunk2 = new jactorio::game::Chunk{5, 1, nullptr};

		// Adding a chunk to an existing location overwrites it
		const auto added_chunk = world_data.add_chunk(chunk);
		const auto added_chunk2 = world_data.add_chunk(chunk2);

		// chunk should have been deleted as it was overwritten
		// No test exists to test if chunk is valid since one cannot determine if pointer is valid

		// Retrieved chunk should be chunk2
		EXPECT_NE(added_chunk, added_chunk2);

		// Returned pointers are equal
		EXPECT_EQ(added_chunk, chunk);
		EXPECT_EQ(added_chunk2, chunk2);

		world_data.clear_chunk_data();
	}

	TEST(world_manager, world_get_chunk) {
		jactorio::game::World_data world_data{};

		const auto chunk = new jactorio::game::Chunk{5, 1, nullptr};
		const auto* added_chunk = world_data.add_chunk(chunk);

		EXPECT_EQ(world_data.get_chunk(0, 0), nullptr);
		EXPECT_EQ(world_data.get_chunk(5, 1), added_chunk);

		world_data.clear_chunk_data();
	}

	TEST(world_manager, get_tile_world_coords) {
		jactorio::game::World_data world_data{};

		const auto chunk_tile = jactorio::game::Chunk_tile();

		// World coords 0, 0 - Chunk 0 0, position 0 0
		{
			const auto tiles = new jactorio::game::Chunk_tile[32 * 32];
			tiles[0] = chunk_tile;
			world_data.add_chunk(new jactorio::game::Chunk(0, 0, tiles));

			EXPECT_EQ(world_data.get_tile_world_coords(0, 0), &tiles[0]);
			EXPECT_NE(world_data.get_tile_world_coords(0, 1), &tiles[0]);
		}
		world_data.clear_chunk_data();

		// World coords -31, -31 - Chunk -1 -1, position 1 1
		{
			const auto tiles = new jactorio::game::Chunk_tile[32 * 32];
			tiles[33] = chunk_tile;
			world_data.add_chunk(new jactorio::game::Chunk(-1, -1, tiles));

			EXPECT_EQ(world_data.get_tile_world_coords(-31, -31), &tiles[33]);
			EXPECT_NE(world_data.get_tile_world_coords(-31, -32), &tiles[33]);
		}
		world_data.clear_chunk_data();

		// World coords -32, 0 - Chunk -1 0, position 0 0
		{
			const auto tiles = new jactorio::game::Chunk_tile[32 * 32];
			tiles[0] = chunk_tile;
			world_data.add_chunk(new jactorio::game::Chunk(-1, 0, tiles));

			EXPECT_EQ(world_data.get_tile_world_coords(-32, 0), &tiles[0]);
			EXPECT_NE(world_data.get_tile_world_coords(-31, 0), &tiles[0]);
		}

	}

	TEST(world_manager, get_chunk_world_coords) {
		jactorio::game::World_data world_data{};

		{
			const auto chunk = world_data.add_chunk(new jactorio::game::Chunk(0, 0, nullptr));
			EXPECT_EQ(world_data.get_chunk_world_coords(31, 31), chunk);
		}

		{
			const auto chunk = world_data.add_chunk(new jactorio::game::Chunk(-1, 0, nullptr));
			EXPECT_EQ(world_data.get_chunk_world_coords(-1, 0), chunk);
		}
	}


	TEST(world_manager, clear_chunk_data) {
		jactorio::game::World_data world_data{};

		const auto chunk = new jactorio::game::Chunk{6, 6, nullptr};
		const auto* added_chunk = world_data.add_chunk(chunk);

		EXPECT_EQ(world_data.get_chunk(6, 6), added_chunk);

		world_data.clear_chunk_data();

		// Chunk no longer exists after it was cleared
		EXPECT_EQ(world_data.get_chunk(6, 6), nullptr);
	}


	// Logic chunks


	TEST(world_manager, logic_add_chunk) {
		jactorio::game::World_data world_data{};

		using namespace jactorio::game;
		Chunk chunk(0, 0, nullptr);

		auto& logic_chunk = world_data.logic_add_chunk(&chunk);
		// Should return reference to newly created and added chunk

		EXPECT_EQ(world_data.logic_get_all_chunks().size(), 1);

		// Should be referencing the same logic chunk
		EXPECT_EQ(&world_data.logic_get_all_chunks().at(&chunk), &logic_chunk);
	}

	TEST(world_manager, logic_add_chunk_no_duplicate) {
		// If the chunk already exists, it should not add it
		jactorio::game::World_data world_data{};

		using namespace jactorio::game;
		Chunk chunk(0, 0, nullptr);

		world_data.logic_add_chunk(&chunk);
		world_data.logic_add_chunk(&chunk);  // Attempting to add the same chunk again

		EXPECT_EQ(world_data.logic_get_all_chunks().size(), 1);
	}

	// TEST(world_manager, logic_remove_chunk) {
	// 	jactorio::core::Resource_guard guard(&world_data.clear_chunk_data);
	//
	// 	using namespace jactorio::game;
	// 	Chunk chunk(0, 0, nullptr);
	//
	// 	auto& logic_chunk = world_manager::logic_add_chunk(&chunk);  // Add
	// 	world_manager::logic_remove_chunk(&logic_chunk);  // Remove
	//
	// 	EXPECT_EQ(world_manager::logic_get_all_chunks().size(), 0);
	// }

	TEST(world_manager, logic_clear_chunk_data) {
		jactorio::game::World_data world_data{};

		using namespace jactorio::game;
		Chunk chunk(0, 0, nullptr);

		world_data.logic_add_chunk(&chunk);

		// Clear
		world_data.clear_chunk_data();

		// Vector reference should now be empty
		EXPECT_EQ(world_data.logic_get_all_chunks().size(), 0);
	}
}
