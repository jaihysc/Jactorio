// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#include <gtest/gtest.h>

#include "data/prototype/sprite.h"
#include "game/world/chunk.h"
#include "game/world/world_data.h"

namespace game
{
	class WorldDataTest : public testing::Test
	{
	protected:
		jactorio::game::WorldData worldData_{};
	};

	TEST_F(WorldDataTest, OnTickAdvance) {
		// Should move the game_tick forward
		EXPECT_EQ(worldData_.GameTick(), 0);

		worldData_.OnTickAdvance();
		EXPECT_EQ(worldData_.GameTick(), 1);

		worldData_.OnTickAdvance();
		EXPECT_EQ(worldData_.GameTick(), 2);

		worldData_.OnTickAdvance();
		EXPECT_EQ(worldData_.GameTick(), 3);
	}

	TEST_F(WorldDataTest, ToChunkCoords) {
		EXPECT_EQ(jactorio::game::WorldData::ToChunkCoord(-33), -2);
		EXPECT_EQ(jactorio::game::WorldData::ToChunkCoord(-32), -1);
		EXPECT_EQ(jactorio::game::WorldData::ToChunkCoord(-1), -1);
		EXPECT_EQ(jactorio::game::WorldData::ToChunkCoord(31), 0);
		EXPECT_EQ(jactorio::game::WorldData::ToChunkCoord(32), 1);
	}

	TEST(ChunkStructLayer, ToStructCoords) {
		EXPECT_FLOAT_EQ(jactorio::game::WorldData::ToStructCoord(10), 10.f);
		EXPECT_FLOAT_EQ(jactorio::game::WorldData::ToStructCoord(64), 0.f);

		EXPECT_FLOAT_EQ(jactorio::game::WorldData::ToStructCoord(-32), 0.f);
		EXPECT_FLOAT_EQ(jactorio::game::WorldData::ToStructCoord(-1), 31.f);
	}

	TEST_F(WorldDataTest, WorldAddChunk) {
		// Chunks initialized with empty tiles
		const auto chunk = jactorio::game::Chunk{5, 1};

		// Returns pointer to chunk which was added
		const auto* added_chunk = worldData_.AddChunk(chunk);

		// Chunk knows its own location
		EXPECT_EQ(added_chunk->GetPosition().first, 5);
		EXPECT_EQ(added_chunk->GetPosition().second, 1);

		// Should not initialize other chunks
		EXPECT_EQ(worldData_.GetChunkC(-1, -1), nullptr);
		EXPECT_EQ(worldData_.GetChunkC(1, 1), nullptr);
	}

	TEST_F(WorldDataTest, WorldAddChunkNegative) {
		// Chunks initialized with empty tiles
		// Returns pointer to chunk which was added
		auto* added_chunk = worldData_.AddChunk(jactorio::game::Chunk{-5, -1});

		// Chunk knows its own location
		EXPECT_EQ(added_chunk->GetPosition().first, -5);
		EXPECT_EQ(added_chunk->GetPosition().second, -1);


		// Should not initialize other chunks
		EXPECT_EQ(worldData_.GetChunkC(-1, -1), nullptr);
		EXPECT_EQ(worldData_.GetChunkC(1, 1), nullptr);
	}


	TEST_F(WorldDataTest, WorldOverrideChunk) {
		// Adding a chunk to an existing location SHOULD NOT overwrite it
		const auto chunk  = jactorio::game::Chunk{5, 1};
		const auto chunk2 = jactorio::game::Chunk{5, 1};

		// Set a sprite at chunk2 so it can be tested
		jactorio::data::Sprite sprite{};
		chunk2.Tiles()[0].SetSpritePrototype(jactorio::game::ChunkTile::ChunkLayer::overlay, &sprite);

		worldData_.AddChunk(chunk);
		worldData_.AddChunk(chunk2);

		EXPECT_NE(worldData_.GetChunkC(5, 1)->Tiles()[0]
				  .GetSpritePrototype(jactorio::game::ChunkTile::ChunkLayer::overlay),
				  &sprite);
	}

	TEST_F(WorldDataTest, WorldDeleteChunk) {
		worldData_.EmplaceChunk(3, 2);
		worldData_.DeleteChunk(3, 2);

		EXPECT_EQ(worldData_.GetChunkC(3, 2), nullptr);

		// No effect, no chunk
		worldData_.DeleteChunk(2, 2);
	}

	TEST_F(WorldDataTest, WorldGetChunkChunkCoords) {
		const auto* added_chunk = worldData_.EmplaceChunk(5, 1);

		EXPECT_EQ(worldData_.GetChunkC(0, 0), nullptr);
		EXPECT_EQ(worldData_.GetChunkC(5, 1), added_chunk);
		EXPECT_EQ(worldData_.GetChunkC(5, 1), added_chunk);
	}

	TEST_F(WorldDataTest, GetTileWorldCoords) {
		// Tests both overloads int, int and std::pair<int, int>
		constexpr auto chunk_width = jactorio::game::WorldData::kChunkWidth;
		const auto chunk_tile      = jactorio::game::ChunkTile();

		// World coords 0, 0 - Chunk 0 0, position 0 0
		{
			auto* tiles = new jactorio::game::ChunkTile[32 * 32];
			tiles[0]    = chunk_tile;
			worldData_.EmplaceChunk(0, 0, tiles);

			EXPECT_EQ(worldData_.GetTile(0, 0), &tiles[0]);
			EXPECT_NE(worldData_.GetTile(0, 1), &tiles[0]);

			EXPECT_EQ(worldData_.GetTile({0, 0}), &tiles[0]);
			EXPECT_NE(worldData_.GetTile({0, 1}), &tiles[0]);
		}
		worldData_.ClearChunkData();

		// World coords -31, -31 - Chunk -1 -1, position 1 1
		{
			auto* tiles = new jactorio::game::ChunkTile[chunk_width * chunk_width];
			tiles[33]   = chunk_tile;
			worldData_.EmplaceChunk(-1, -1, tiles);

			EXPECT_EQ(worldData_.GetTile(-31, -31), &tiles[33]);
			EXPECT_NE(worldData_.GetTile(-31, -32), &tiles[33]);

			EXPECT_EQ(worldData_.GetTile({-31, -31}), &tiles[33]);
			EXPECT_NE(worldData_.GetTile({-31, -32}), &tiles[33]);
		}
		worldData_.ClearChunkData();

		// World coords -32, 0 - Chunk -1 0, position 0 0
		{
			auto* tiles = new jactorio::game::ChunkTile[chunk_width * chunk_width];
			tiles[0]    = chunk_tile;
			worldData_.EmplaceChunk(-1, 0, tiles);

			EXPECT_EQ(worldData_.GetTile(-32, 0), &tiles[0]);
			EXPECT_NE(worldData_.GetTile(-31, 0), &tiles[0]);

			EXPECT_EQ(worldData_.GetTile({-32, 0}), &tiles[0]);
			EXPECT_NE(worldData_.GetTile({-31, 0}), &tiles[0]);
		}

	}

	TEST_F(WorldDataTest, GetChunkWorldCoords) {
		{
			const auto* chunk = worldData_.EmplaceChunk(0, 0);
			EXPECT_EQ(worldData_.GetChunk(31, 31), chunk);

			EXPECT_EQ(worldData_.GetChunk({31, 31}), chunk);
		}

		{
			const auto* chunk = worldData_.EmplaceChunk(-1, 0);
			EXPECT_EQ(worldData_.GetChunk(-1, 0), chunk);

			EXPECT_EQ(worldData_.GetChunk({-1, 0}), chunk);
		}
	}


	TEST_F(WorldDataTest, ClearChunkData) {
		const auto chunk        = jactorio::game::Chunk{6, 6};
		const auto* added_chunk = worldData_.AddChunk(chunk);

		EXPECT_EQ(worldData_.GetChunkC(6, 6), added_chunk);

		worldData_.ClearChunkData();

		// Chunk no longer exists after it was cleared
		EXPECT_EQ(worldData_.GetChunkC(6, 6), nullptr);
	}


	// Logic chunks


	TEST_F(WorldDataTest, LogicAddChunk) {
		jactorio::game::Chunk chunk(0, 0);

		auto& logic_chunk = worldData_.LogicAddChunk(&chunk);
		// Should return reference to newly created and added chunk

		EXPECT_EQ(worldData_.LogicGetAllChunks().size(), 1);

		// Should be referencing the same logic chunk
		EXPECT_EQ(&worldData_.LogicGetAllChunks().at(&chunk), &logic_chunk);
	}

	TEST_F(WorldDataTest, LogicAddChunkNoDuplicate) {
		// If the chunk already exists, it should not add it
		jactorio::game::Chunk chunk(0, 0);

		worldData_.LogicAddChunk(&chunk);
		worldData_.LogicAddChunk(&chunk);  // Attempting to add the same chunk again

		EXPECT_EQ(worldData_.LogicGetAllChunks().size(), 1);
	}

	TEST_F(WorldDataTest, logic_remove_chunk) {
		jactorio::game::Chunk chunk(0, 0);

		auto& logic_chunk = worldData_.LogicAddChunk(&chunk);
		worldData_.LogicRemoveChunk(&logic_chunk);  // Remove

		EXPECT_EQ(worldData_.LogicGetAllChunks().size(), 0);
	}

	TEST_F(WorldDataTest, LogicGetChunk) {
		jactorio::game::Chunk chunk(0, 0);

		auto& logic_chunk = worldData_.LogicAddChunk(&chunk);

		EXPECT_EQ(worldData_.LogicGetChunk(&chunk), &logic_chunk);
		EXPECT_EQ(worldData_.LogicGetChunk(nullptr), nullptr);
	}

	TEST_F(WorldDataTest, LogicGetChunkReadOnly) {
		jactorio::game::Chunk chunk(0, 0);

		auto& logic_chunk = worldData_.LogicAddChunk(&chunk);

		EXPECT_EQ(worldData_.LogicGetChunk(&chunk), &logic_chunk);
		EXPECT_EQ(worldData_.LogicGetChunk(nullptr), nullptr);
	}

	TEST_F(WorldDataTest, LogicClearChunkData) {
		jactorio::game::Chunk chunk(0, 0);

		worldData_.LogicAddChunk(&chunk);

		// Clear
		worldData_.ClearChunkData();

		// Vector reference should now be empty
		EXPECT_EQ(worldData_.LogicGetAllChunks().size(), 0);
	}
}
