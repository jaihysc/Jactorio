// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "game/world/world_data.h"

#include "jactorioTests.h"
#include "data/prototype/container_entity.h"
#include "data/prototype/tile.h"
#include "game/world/chunk.h"

namespace jactorio::game
{
	class WorldDataTest : public testing::Test
	{
	protected:
		WorldData worldData_{};
	};

	TEST_F(WorldDataTest, ToChunkCoords) {
		EXPECT_EQ(WorldData::ToChunkCoord(-33), -2);
		EXPECT_EQ(WorldData::ToChunkCoord(-32), -1);
		EXPECT_EQ(WorldData::ToChunkCoord(-1), -1);
		EXPECT_EQ(WorldData::ToChunkCoord(31), 0);
		EXPECT_EQ(WorldData::ToChunkCoord(32), 1);
	}

	TEST_F(WorldDataTest, ToOverlayCoords) {
		EXPECT_FLOAT_EQ(WorldData::ToOverlayCoord(0), 0);
		EXPECT_FLOAT_EQ(WorldData::ToOverlayCoord(31), 31);
		EXPECT_FLOAT_EQ(WorldData::ToOverlayCoord(32), 0);
		EXPECT_FLOAT_EQ(WorldData::ToOverlayCoord(63), 31);

		EXPECT_FLOAT_EQ(WorldData::ToOverlayCoord(-1), 31);
		EXPECT_FLOAT_EQ(WorldData::ToOverlayCoord(-32), 0);
		EXPECT_FLOAT_EQ(WorldData::ToOverlayCoord(-33), 31);
		EXPECT_FLOAT_EQ(WorldData::ToOverlayCoord(-65), 31);
	}

	TEST_F(WorldDataTest, WorldAddChunk) {
		// Chunks initialized with empty tiles
		const auto chunk = Chunk{5, 1};

		// Returns pointer to chunk which was added
		const auto* added_chunk = worldData_.AddChunk(chunk);

		// Chunk knows its own location
		EXPECT_EQ(added_chunk->GetPosition().x, 5);
		EXPECT_EQ(added_chunk->GetPosition().y, 1);

		// Should not initialize other chunks
		EXPECT_EQ(worldData_.GetChunkC(-1, -1), nullptr);
		EXPECT_EQ(worldData_.GetChunkC(1, 1), nullptr);
	}

	TEST_F(WorldDataTest, WorldAddChunkNegative) {
		// Chunks initialized with empty tiles
		// Returns pointer to chunk which was added
		auto* added_chunk = worldData_.AddChunk(Chunk{-5, -1});

		// Chunk knows its own location
		EXPECT_EQ(added_chunk->GetPosition().x, -5);
		EXPECT_EQ(added_chunk->GetPosition().y, -1);


		// Should not initialize other chunks
		EXPECT_EQ(worldData_.GetChunkC(-1, -1), nullptr);
		EXPECT_EQ(worldData_.GetChunkC(1, 1), nullptr);
	}


	TEST_F(WorldDataTest, WorldOverrideChunk) {
		// Adding a chunk to an existing location SHOULD NOT overwrite it
		const auto chunk = Chunk{5, 1};
		auto chunk2      = Chunk{5, 1};

		// Set a sprite at chunk2 so it can be tested
		data::Tile tile{};
		chunk2.Tiles()[0].SetTilePrototype(&tile);

		worldData_.AddChunk(chunk);
		worldData_.AddChunk(chunk2);

		EXPECT_NE(worldData_.GetChunkC(5, 1)->Tiles()[0]
		          .GetTilePrototype(), &tile);
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
		const auto chunk_tile = ChunkTile();

		// World coords 0, 0 - Chunk 0 0, position 0 0
		{
			auto* chunk = worldData_.EmplaceChunk(0, 0);
			auto& tiles = chunk->Tiles();
			tiles[0]    = chunk_tile;

			EXPECT_EQ(worldData_.GetTile(0, 0), &tiles[0]);
			EXPECT_NE(worldData_.GetTile(0, 1), &tiles[0]);

			EXPECT_EQ(worldData_.GetTile({0, 0}), &tiles[0]);
			EXPECT_NE(worldData_.GetTile({0, 1}), &tiles[0]);
		}
		worldData_.ClearChunkData();

		// World coords -31, -31 - Chunk -1 -1, position 1 1
		{
			auto* chunk = worldData_.EmplaceChunk(-1, -1);
			auto& tiles = chunk->Tiles();
			tiles[33]   = chunk_tile;

			EXPECT_EQ(worldData_.GetTile(-31, -31), &tiles[33]);
			EXPECT_NE(worldData_.GetTile(-31, -32), &tiles[33]);

			EXPECT_EQ(worldData_.GetTile({-31, -31}), &tiles[33]);
			EXPECT_NE(worldData_.GetTile({-31, -32}), &tiles[33]);
		}
		worldData_.ClearChunkData();

		// World coords -32, 0 - Chunk -1 0, position 0 0
		{
			auto* chunk = worldData_.EmplaceChunk(-1, 0);
			auto& tiles = chunk->Tiles();
			tiles[0]    = chunk_tile;

			EXPECT_EQ(worldData_.GetTile(-32, 0), &tiles[0]);
			EXPECT_NE(worldData_.GetTile(-31, 0), &tiles[0]);

			EXPECT_EQ(worldData_.GetTile({-32, 0}), &tiles[0]);
			EXPECT_NE(worldData_.GetTile({-31, 0}), &tiles[0]);
		}

	}

	TEST_F(WorldDataTest, GetChunkWorldCoords) {
		{
			const auto* chunk = worldData_.EmplaceChunk(0, 0);
			EXPECT_EQ(worldData_.GetChunkW(31, 31), chunk);

			EXPECT_EQ(worldData_.GetChunkW({31, 31}), chunk);
		}

		{
			const auto* chunk = worldData_.EmplaceChunk(-1, 0);
			EXPECT_EQ(worldData_.GetChunkW(-1, 0), chunk);

			EXPECT_EQ(worldData_.GetChunkW({-1, 0}), chunk);
		}
	}

	TEST_F(WorldDataTest, GetTileTopLeft) {
		worldData_.EmplaceChunk(0, 0);
		const WorldCoord bottom_coord = {6, 6};

		auto* bottom_tile  = worldData_.GetTile(bottom_coord);
		auto& bottom_layer = bottom_tile->GetLayer(ChunkTile::ChunkLayer::entity);

		data::ContainerEntity proto;
		TestSetupMultiTileProp(bottom_layer, {2, 1}, proto);


		// multiTileIndex is 0
		EXPECT_EQ(worldData_.GetTileTopLeft(bottom_coord, bottom_layer), bottom_tile);  // Returns self if not multi tile
		EXPECT_EQ(worldData_.GetTileTopLeft(bottom_coord, ChunkTile::ChunkLayer::entity), bottom_tile);

		//
		auto* top_tile = worldData_.GetTile(5, 6);

		bottom_layer.SetMultiTileIndex(1);
		EXPECT_EQ(worldData_.GetTileTopLeft(bottom_coord, bottom_layer), top_tile);
		EXPECT_EQ(worldData_.GetTileTopLeft(bottom_coord, ChunkTile::ChunkLayer::entity), top_tile);
	}

	TEST_F(WorldDataTest, GetLayerTopLeft) {
		worldData_.EmplaceChunk(0, 0);

		auto* top_tile    = worldData_.GetTile(0, 0);
		auto* unique_data = top_tile->GetLayer(ChunkTile::ChunkLayer::resource).MakeUniqueData<data::ContainerEntityData>(10);

		auto* bottom_tile  = worldData_.GetTile({1, 2});
		auto& bottom_layer = bottom_tile->GetLayer(ChunkTile::ChunkLayer::resource);

		data::ContainerEntity proto;
		TestSetupMultiTileProp(bottom_layer, {7, 10}, proto);
		bottom_layer.SetMultiTileIndex(15);

		EXPECT_EQ(worldData_.GetLayerTopLeft({1, 2}, ChunkTile::ChunkLayer::resource)->GetUniqueData(), unique_data);
	}

	TEST_F(WorldDataTest, GetLayerTopLeftUninitialized) {
		EXPECT_EQ(worldData_.GetLayerTopLeft({1, 2}, ChunkTile::ChunkLayer::entity), nullptr);
	}


	TEST_F(WorldDataTest, ClearChunkData) {
		const auto chunk        = Chunk{6, 6};
		const auto* added_chunk = worldData_.AddChunk(chunk);

		EXPECT_EQ(worldData_.GetChunkC(6, 6), added_chunk);

		worldData_.ClearChunkData();

		// Chunk no longer exists after it was cleared
		EXPECT_EQ(worldData_.GetChunkC(6, 6), nullptr);
	}


	// Logic chunks

	TEST_F(WorldDataTest, LogicRegister) {
		worldData_.EmplaceChunk(1, 0);  // 32, 0 is chunk coords 1, 0
		worldData_.LogicRegister(Chunk::LogicGroup::inserter,
		                         {32, 0},
		                         ChunkTile::ChunkLayer::entity);

		// Added chunk
		ASSERT_EQ(worldData_.LogicGetChunks().size(), 1);

		EXPECT_NE(worldData_.LogicGetChunks().find(worldData_.GetChunkC({1, 0})),
		          worldData_.LogicGetChunks().end());


		// Registering again will not duplicate logic chunk
		worldData_.LogicRegister(Chunk::LogicGroup::inserter,
		                         {42, 0},
		                         ChunkTile::ChunkLayer::entity);
		EXPECT_EQ(worldData_.LogicGetChunks().size(), 1);


		// Registering same position does nothing
		worldData_.LogicRegister(Chunk::LogicGroup::inserter,
		                         {42, 0},
		                         ChunkTile::ChunkLayer::entity);
		EXPECT_EQ((*worldData_.LogicGetChunks().begin())
		          ->GetLogicGroup(Chunk::LogicGroup::inserter).size(), 2);
	}

	TEST_F(WorldDataTest, LogicRemove) {
		worldData_.EmplaceChunk(1, 0);
		worldData_.LogicRegister(Chunk::LogicGroup::inserter,
		                         {32, 0},
		                         ChunkTile::ChunkLayer::entity);

		worldData_.LogicRegister(Chunk::LogicGroup::transport_line,
		                         {33, 0},
		                         ChunkTile::ChunkLayer::entity);

		// Registering again will not duplicate logic chunk
		worldData_.LogicRegister(Chunk::LogicGroup::inserter,
		                         {42, 0},
		                         ChunkTile::ChunkLayer::entity);


		// Removed 1, another one remains
		worldData_.LogicRemove(Chunk::LogicGroup::inserter,
		                       {32, 0},
		                       ChunkTile::ChunkLayer::entity);
		EXPECT_EQ(worldData_.LogicGetChunks().size(), 1);


		// Inserter group empty, but transport line group is not, DO NOT remove from logic chunks
		worldData_.LogicRemove(Chunk::LogicGroup::inserter,
		                       {42, 0},
		                       ChunkTile::ChunkLayer::entity);
		EXPECT_EQ(worldData_.LogicGetChunks().size(), 1);


		// All groups empty, remove logic chunk
		worldData_.LogicRemove(Chunk::LogicGroup::transport_line,
		                       {33, 0},
		                       ChunkTile::ChunkLayer::entity);
		EXPECT_EQ(worldData_.LogicGetChunks().size(), 0);
	}

	TEST_F(WorldDataTest, LogicRemoveNonExistent) {
		worldData_.EmplaceChunk(1, 0);

		// Removed 1, another one remains
		worldData_.LogicRemove(Chunk::LogicGroup::inserter,
		                       {32, 0},
		                       ChunkTile::ChunkLayer::entity);
		EXPECT_EQ(worldData_.LogicGetChunks().size(), 0);
	}

	TEST_F(WorldDataTest, LogicAddChunk) {
		Chunk chunk(0, 0);

		worldData_.LogicAddChunk(&chunk);
		// Should return reference to newly created and added chunk

		EXPECT_EQ(worldData_.LogicGetChunks().size(), 1);
	}

	TEST_F(WorldDataTest, LogicAddChunkNoDuplicate) {
		// If the chunk already exists, it should not add it
		Chunk chunk(0, 0);

		worldData_.LogicAddChunk(&chunk);
		worldData_.LogicAddChunk(&chunk);  // Attempting to add the same chunk again

		EXPECT_EQ(worldData_.LogicGetChunks().size(), 1);
	}

	TEST_F(WorldDataTest, LogicClearChunkData) {
		Chunk chunk(0, 0);

		worldData_.LogicAddChunk(&chunk);

		// Clear
		worldData_.ClearChunkData();

		// Vector reference should now be empty
		EXPECT_EQ(worldData_.LogicGetChunks().size(), 0);
	}
}
