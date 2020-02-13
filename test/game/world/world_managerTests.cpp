#include <gtest/gtest.h>

#include "game/world/chunk.h"
#include "game/world/world_manager.h"

namespace game
{
	TEST(world_manager, world_add_chunk) {
		// Chunks initialized with empty tiles
		const auto chunk = new jactorio::game::Chunk{ 5, 1, nullptr };

		// Returns pointer to chunk which was added
		const auto added_chunk = jactorio::game::world_manager::add_chunk(chunk);

		// Chunk knows its own location
		EXPECT_EQ(added_chunk->get_position().first, 5);
		EXPECT_EQ(added_chunk->get_position().second, 1);

		// Should not initialize other chunks
		EXPECT_EQ(jactorio::game::world_manager::get_chunk(-1, -1), nullptr);
		EXPECT_EQ(jactorio::game::world_manager::get_chunk(1, 1), nullptr);

		jactorio::game::world_manager::clear_chunk_data();
	}

	TEST(world_manager, world_add_chunk_negative) {
		// Chunks initialized with empty tiles
		const auto chunk = new jactorio::game::Chunk{ -5, -1, nullptr };

		// Returns pointer to chunk which was added
		const auto added_chunk = jactorio::game::world_manager::add_chunk(chunk);

		// Chunk knows its own location
		EXPECT_EQ(added_chunk->get_position().first, -5);
		EXPECT_EQ(added_chunk->get_position().second, -1);


		// Should not initialize other chunks
		EXPECT_EQ(jactorio::game::world_manager::get_chunk(-1, -1), nullptr);
		EXPECT_EQ(jactorio::game::world_manager::get_chunk(1, 1), nullptr);


		jactorio::game::world_manager::clear_chunk_data();
	}


	TEST(world_manager, world_add_chunk_override) {
		// Chunks initialized with empty tiles
		const auto chunk = new jactorio::game::Chunk{ 5, 1, nullptr };
		const auto chunk2 = new jactorio::game::Chunk{ 5, 1, nullptr };

		// Adding a chunk to an existing location overwrites it
		const auto added_chunk = jactorio::game::world_manager::add_chunk(chunk);
		const auto added_chunk2 = jactorio::game::world_manager::add_chunk(chunk2);

		// chunk should have been deleted as it was overwritten
		// No test exists to test if chunk is valid since one cannot determine if pointer is valid

		// Retrieved chunk should be chunk2
		EXPECT_NE(added_chunk, added_chunk2);

		// Returned pointers are equal
		EXPECT_EQ(added_chunk, chunk);
		EXPECT_EQ(added_chunk2, chunk2);

		jactorio::game::world_manager::clear_chunk_data();
	}

	TEST(world_manager, world_get_chunk) {
		const auto chunk = new jactorio::game::Chunk{ 5, 1, nullptr };
		const auto* added_chunk = jactorio::game::world_manager::add_chunk(chunk);

		EXPECT_EQ(jactorio::game::world_manager::get_chunk(0, 0), nullptr);
		EXPECT_EQ(jactorio::game::world_manager::get_chunk(5, 1), added_chunk);

		jactorio::game::world_manager::clear_chunk_data();
	}

	TEST(world_manager, get_tile_world_coords) {
		using namespace jactorio::game::world_manager;

		const auto chunk_tile = jactorio::game::Chunk_tile();

		// World coords 0, 0 - Chunk 0 0, position 0 0
		{
			const auto tiles = new jactorio::game::Chunk_tile[32 * 32];
			tiles[0] = chunk_tile;
			add_chunk(new jactorio::game::Chunk(0, 0, tiles));
			
			EXPECT_EQ(get_tile_world_coords(0, 0), &tiles[0]);
			EXPECT_NE(get_tile_world_coords(0, 1), &tiles[0]);
		}
		clear_chunk_data();

		// World coords -31, -31 - Chunk -1 -1, position 1 1
		{
			const auto tiles = new jactorio::game::Chunk_tile[32 * 32];
			tiles[33] = chunk_tile;
			add_chunk(new jactorio::game::Chunk(-1, -1, tiles));

			EXPECT_EQ(get_tile_world_coords(-31, -31), &tiles[33]);
			EXPECT_NE(get_tile_world_coords(-31, -32), &tiles[33]);
		}
		clear_chunk_data();

		// World coords -32, 0 - Chunk -1 0, position 0 0
		{
			const auto tiles = new jactorio::game::Chunk_tile[32 * 32];
			tiles[0] = chunk_tile;
			add_chunk(new jactorio::game::Chunk(-1, 0, tiles));

			EXPECT_EQ(get_tile_world_coords(-32, 0), &tiles[0]);
			EXPECT_NE(get_tile_world_coords(-31, 0), &tiles[0]);
		}
		clear_chunk_data();

	}

	TEST(world_manager, clear_chunk_data) {
		const auto chunk = new jactorio::game::Chunk{ 6, 6, nullptr };
		const auto* added_chunk = jactorio::game::world_manager::add_chunk(chunk);

		EXPECT_EQ(jactorio::game::world_manager::get_chunk(6, 6), added_chunk);

		jactorio::game::world_manager::clear_chunk_data();

		// Chunk no longer exists after it was cleared
		EXPECT_EQ(jactorio::game::world_manager::get_chunk(6, 6), nullptr);
	}


	// Logic chunks


	TEST(world_manager, logic_add_chunk) {
		jactorio::core::Resource_guard guard(&jactorio::game::world_manager::clear_chunk_data);
		
		using namespace jactorio::game;
		Chunk chunk(0, 0, nullptr);
		
		auto& logic_chunk = world_manager::logic_add_chunk(&chunk);
		// Should return reference to newly created and added chunk
		
		EXPECT_EQ(world_manager::logic_get_all_chunks().size(), 1);

		// Should be referencing the same logic chunk
		EXPECT_EQ(&world_manager::logic_get_all_chunks()[0], &logic_chunk);
	}

	TEST(world_manager, logic_remove_chunk) {
		jactorio::core::Resource_guard guard(&jactorio::game::world_manager::clear_chunk_data);

		using namespace jactorio::game;
		Chunk chunk(0, 0, nullptr);

		auto& logic_chunk = world_manager::logic_add_chunk(&chunk);  // Add
		world_manager::logic_remove_chunk(logic_chunk);  // Remove

		EXPECT_EQ(world_manager::logic_get_all_chunks().size(), 0);
	}
	
	TEST(world_manager, logic_clear_chunk_data) {
		jactorio::core::Resource_guard guard(&jactorio::game::world_manager::clear_chunk_data);

		using namespace jactorio::game;
		Chunk chunk(0, 0, nullptr);

		world_manager::logic_add_chunk(&chunk);

		// Clear
		world_manager::clear_chunk_data();

		// Vector reference should now be empty
		EXPECT_EQ(world_manager::logic_get_all_chunks().size(), 0);
	}
}