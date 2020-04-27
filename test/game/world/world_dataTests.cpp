// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#include <gtest/gtest.h>

#include "game/world/chunk.h"
#include "game/world/world_data.h"

namespace game
{
	class WorldDataTest : public testing::Test
	{
	protected:
		jactorio::game::World_data world_data_{};
	};

	TEST_F(WorldDataTest, OnTickAdvance) {
		// Should move the game_tick forward
		EXPECT_EQ(world_data_.game_tick(), 0);

		world_data_.on_tick_advance();
		EXPECT_EQ(world_data_.game_tick(), 1);

		world_data_.on_tick_advance();
		EXPECT_EQ(world_data_.game_tick(), 2);

		world_data_.on_tick_advance();
		EXPECT_EQ(world_data_.game_tick(), 3);
	}

	TEST_F(WorldDataTest, WorldAddChunk) {
		// Chunks initialized with empty tiles
		auto* chunk = new jactorio::game::Chunk{5, 1};

		// Returns pointer to chunk which was added
		const auto added_chunk = world_data_.add_chunk(chunk);

		// Chunk knows its own location
		EXPECT_EQ(added_chunk->get_position().first, 5);
		EXPECT_EQ(added_chunk->get_position().second, 1);

		// Should not initialize other chunks
		EXPECT_EQ(world_data_.get_chunk_c(-1, -1), nullptr);
		EXPECT_EQ(world_data_.get_chunk_c(1, 1), nullptr);
	}

	TEST_F(WorldDataTest, WorldAddChunkNegative) {
		// Chunks initialized with empty tiles
		auto* chunk = new jactorio::game::Chunk{-5, -1};

		// Returns pointer to chunk which was added
		auto* added_chunk = world_data_.add_chunk(chunk);

		// Chunk knows its own location
		EXPECT_EQ(added_chunk->get_position().first, -5);
		EXPECT_EQ(added_chunk->get_position().second, -1);


		// Should not initialize other chunks
		EXPECT_EQ(world_data_.get_chunk_c(-1, -1), nullptr);
		EXPECT_EQ(world_data_.get_chunk_c(1, 1), nullptr);
	}


	TEST_F(WorldDataTest, WorldAddChunkOverride) {
		// Chunks initialized with empty tiles
		auto* chunk = new jactorio::game::Chunk{5, 1};
		auto* chunk2 = new jactorio::game::Chunk{5, 1};

		// Adding a chunk to an existing location overwrites it
		auto* added_chunk = world_data_.add_chunk(chunk);
		auto* added_chunk2 = world_data_.add_chunk(chunk2);

		// chunk should have been deleted as it was overwritten
		// No test exists to test if chunk is valid since one cannot determine if pointer is valid

		// Retrieved chunk should be chunk2
		EXPECT_NE(added_chunk, added_chunk2);

		// Returned pointers are equal
		EXPECT_EQ(added_chunk, chunk);
		EXPECT_EQ(added_chunk2, chunk2);
	}

	TEST_F(WorldDataTest, WorldGetChunkChunkCoords) {
		auto* chunk = new jactorio::game::Chunk{5, 1};
		const auto* added_chunk = world_data_.add_chunk(chunk);

		EXPECT_EQ(world_data_.get_chunk_c(0, 0), nullptr);
		EXPECT_EQ(world_data_.get_chunk_c(5, 1), added_chunk);
		EXPECT_EQ(world_data_.get_chunk_c(5, 1), chunk);
	}

	TEST_F(WorldDataTest, GetTileWorldCoords) {
		// Tests both overloads int, int and std::pair<int, int>
		constexpr auto chunk_width = jactorio::game::World_data::chunk_width;
		const auto chunk_tile = jactorio::game::Chunk_tile();

		// World coords 0, 0 - Chunk 0 0, position 0 0
		{
			auto* tiles = new jactorio::game::Chunk_tile[32 * 32];
			tiles[0] = chunk_tile;
			world_data_.add_chunk(new jactorio::game::Chunk(0, 0, tiles));

			EXPECT_EQ(world_data_.get_tile(0, 0), &tiles[0]);
			EXPECT_NE(world_data_.get_tile(0, 1), &tiles[0]);

			EXPECT_EQ(world_data_.get_tile({0, 0}), &tiles[0]);
			EXPECT_NE(world_data_.get_tile({0, 1}), &tiles[0]);
		}
		world_data_.clear_chunk_data();

		// World coords -31, -31 - Chunk -1 -1, position 1 1
		{
			auto* tiles = new jactorio::game::Chunk_tile[chunk_width * chunk_width];
			tiles[33] = chunk_tile;
			world_data_.add_chunk(new jactorio::game::Chunk(-1, -1, tiles));

			EXPECT_EQ(world_data_.get_tile(-31, -31), &tiles[33]);
			EXPECT_NE(world_data_.get_tile(-31, -32), &tiles[33]);

			EXPECT_EQ(world_data_.get_tile({-31, -31}), &tiles[33]);
			EXPECT_NE(world_data_.get_tile({-31, -32}), &tiles[33]);
		}
		world_data_.clear_chunk_data();

		// World coords -32, 0 - Chunk -1 0, position 0 0
		{
			auto* tiles = new jactorio::game::Chunk_tile[chunk_width * chunk_width];
			tiles[0] = chunk_tile;
			world_data_.add_chunk(new jactorio::game::Chunk(-1, 0, tiles));

			EXPECT_EQ(world_data_.get_tile(-32, 0), &tiles[0]);
			EXPECT_NE(world_data_.get_tile(-31, 0), &tiles[0]);

			EXPECT_EQ(world_data_.get_tile({-32, 0}), &tiles[0]);
			EXPECT_NE(world_data_.get_tile({-31, 0}), &tiles[0]);
		}

	}

	TEST_F(WorldDataTest, GetChunkWorldCoords) {
		{
			const auto* chunk = world_data_.add_chunk(new jactorio::game::Chunk(0, 0));
			EXPECT_EQ(world_data_.get_chunk(31, 31), chunk);

			EXPECT_EQ(world_data_.get_chunk({31, 31}), chunk);
		}

		{
			const auto* chunk = world_data_.add_chunk(new jactorio::game::Chunk(-1, 0));
			EXPECT_EQ(world_data_.get_chunk(-1, 0), chunk);

			EXPECT_EQ(world_data_.get_chunk({-1, 0}), chunk);
		}
	}


	TEST_F(WorldDataTest, ClearChunkData) {
		auto* chunk = new jactorio::game::Chunk{6, 6};
		const auto* added_chunk = world_data_.add_chunk(chunk);

		EXPECT_EQ(world_data_.get_chunk_c(6, 6), added_chunk);

		world_data_.clear_chunk_data();

		// Chunk no longer exists after it was cleared
		EXPECT_EQ(world_data_.get_chunk_c(6, 6), nullptr);
	}


	// Logic chunks


	TEST_F(WorldDataTest, LogicAddChunk) {
		jactorio::game::Chunk chunk(0, 0);

		auto& logic_chunk = world_data_.logic_add_chunk(&chunk);
		// Should return reference to newly created and added chunk

		EXPECT_EQ(world_data_.logic_get_all_chunks().size(), 1);

		// Should be referencing the same logic chunk
		EXPECT_EQ(&world_data_.logic_get_all_chunks().at(&chunk), &logic_chunk);
	}

	TEST_F(WorldDataTest, LogicAddChunkNoDuplicate) {
		// If the chunk already exists, it should not add it
		jactorio::game::Chunk chunk(0, 0);

		world_data_.logic_add_chunk(&chunk);
		world_data_.logic_add_chunk(&chunk);  // Attempting to add the same chunk again

		EXPECT_EQ(world_data_.logic_get_all_chunks().size(), 1);
	}

	// TEST_F(WorldDataTest, logic_remove_chunk) {
	// 	jactorio::core::Resource_guard guard(&world_data.clear_chunk_data);
	//
	// 	using namespace jactorio::game;
	// 	Chunk chunk(0, 0, nullptr);
	//
	// 	auto& logic_chunk = world_data::logic_add_chunk(&chunk);  // Add
	// 	world_data::logic_remove_chunk(&logic_chunk);  // Remove
	//
	// 	EXPECT_EQ(world_data::logic_get_all_chunks().size(), 0);
	// }

	TEST_F(WorldDataTest, LogicGetChunk) {
		jactorio::game::Chunk chunk(0, 0);

		auto& logic_chunk = world_data_.logic_add_chunk(&chunk);

		EXPECT_EQ(world_data_.logic_get_chunk(&chunk), &logic_chunk);
		EXPECT_EQ(world_data_.logic_get_chunk(nullptr), nullptr);
	}

	TEST_F(WorldDataTest, LogicGetChunkReadOnly) {
		jactorio::game::Chunk chunk(0, 0);

		auto& logic_chunk = world_data_.logic_add_chunk(&chunk);

		EXPECT_EQ(world_data_.logic_get_chunk_read_only(&chunk), &logic_chunk);
		EXPECT_EQ(world_data_.logic_get_chunk_read_only(nullptr), nullptr);
	}

	TEST_F(WorldDataTest, LogicClearChunkData) {
		jactorio::game::Chunk chunk(0, 0);

		world_data_.logic_add_chunk(&chunk);

		// Clear
		world_data_.clear_chunk_data();

		// Vector reference should now be empty
		EXPECT_EQ(world_data_.logic_get_all_chunks().size(), 0);
	}
}
