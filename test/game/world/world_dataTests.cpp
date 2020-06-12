// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#include <gtest/gtest.h>

#include "data/prototype/sprite.h"
#include "data/prototype/interface/deferred.h"
#include "data/prototype/interface/update_listener.h"
#include "game/world/chunk.h"
#include "game/world/world_data.h"

namespace jactorio::game
{
	class WorldDataTest : public testing::Test
	{
	protected:
		WorldData worldData_{};
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

	TEST_F(WorldDataTest, WorldAddChunk) {
		// Chunks initialized with empty tiles
		const auto chunk = Chunk{5, 1};

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
		auto* added_chunk = worldData_.AddChunk(Chunk{-5, -1});

		// Chunk knows its own location
		EXPECT_EQ(added_chunk->GetPosition().first, -5);
		EXPECT_EQ(added_chunk->GetPosition().second, -1);


		// Should not initialize other chunks
		EXPECT_EQ(worldData_.GetChunkC(-1, -1), nullptr);
		EXPECT_EQ(worldData_.GetChunkC(1, 1), nullptr);
	}


	TEST_F(WorldDataTest, WorldOverrideChunk) {
		// Adding a chunk to an existing location SHOULD NOT overwrite it
		const auto chunk  = Chunk{5, 1};
		const auto chunk2 = Chunk{5, 1};

		// Set a sprite at chunk2 so it can be tested
		data::Sprite sprite{};
		chunk2.Tiles()[0].SetSpritePrototype(ChunkTile::ChunkLayer::overlay, &sprite);

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
		constexpr auto chunk_width = WorldData::kChunkWidth;
		const auto chunk_tile      = ChunkTile();

		// World coords 0, 0 - Chunk 0 0, position 0 0
		{
			auto* tiles = new ChunkTile[32 * 32];
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
			auto* tiles = new ChunkTile[chunk_width * chunk_width];
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
			auto* tiles = new ChunkTile[chunk_width * chunk_width];
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


	// ======================================================================


	class DeferralTimerTest : public testing::Test
	{
		WorldData worldData_;

	protected:
		WorldData::DeferralTimer& timer_ = worldData_.deferralTimer;

		class MockDeferred final : public data::IDeferred
		{
		public:
			mutable bool callbackCalled              = false;
			mutable data::UniqueDataBase* dataPtr    = nullptr;
			mutable WorldData::DeferralTimer* dTimer = nullptr;

			void OnDeferTimeElapsed(WorldData& world_data,
			                        data::UniqueDataBase* unique_data) const override {
				callbackCalled = true;
				dataPtr        = unique_data;
				dTimer         = &world_data.deferralTimer;
			};
		};

		const MockDeferred deferred_{};

		class MockUniqueData final : public data::UniqueDataBase
		{
		};
	};

	TEST_F(DeferralTimerTest, RregisterAtTick) {
		const auto unique_data = std::make_unique<MockUniqueData>();

		const auto index = timer_.RegisterAtTick(deferred_, unique_data.get(), 2);
		EXPECT_EQ(index.first, 2);
		EXPECT_EQ(index.second, 1);

		timer_.DeferralUpdate(0);
		EXPECT_FALSE(deferred_.callbackCalled);

		timer_.DeferralUpdate(1);
		EXPECT_FALSE(deferred_.callbackCalled);

		timer_.DeferralUpdate(2);
		EXPECT_TRUE(deferred_.callbackCalled);
		EXPECT_EQ(deferred_.dataPtr, unique_data.get());
		EXPECT_EQ(deferred_.dTimer, &timer_);
	}

	TEST_F(DeferralTimerTest, RegisterFromTick) {
		const auto unique_data = std::make_unique<MockUniqueData>();

		// Elapse 2 ticks from now
		const auto index = timer_.RegisterFromTick(deferred_, unique_data.get(), 2);
		EXPECT_EQ(index.first, 2);
		EXPECT_EQ(index.second, 1);

		timer_.DeferralUpdate(0);
		EXPECT_FALSE(deferred_.callbackCalled);

		timer_.DeferralUpdate(1);
		EXPECT_FALSE(deferred_.callbackCalled);

		timer_.DeferralUpdate(2);
		EXPECT_TRUE(deferred_.callbackCalled);
		EXPECT_EQ(deferred_.dataPtr, unique_data.get());
		EXPECT_EQ(deferred_.dTimer, &timer_);
	}

	TEST_F(DeferralTimerTest, RegisterDeferralRemoveOldCallbacks) {
		timer_.RegisterAtTick(deferred_, nullptr, 2);

		timer_.DeferralUpdate(2);
		ASSERT_TRUE(deferred_.callbackCalled);

		// Callback at 2 has been removed since it update was called for game tick 2
		deferred_.callbackCalled = false;  // Reset
		timer_.DeferralUpdate(2);
		EXPECT_FALSE(deferred_.callbackCalled);
	}

	TEST_F(DeferralTimerTest, RemoveDeferral) {
		const auto entry = timer_.RegisterAtTick(deferred_, nullptr, 2);

		timer_.RemoveDeferral(entry);

		// Callback removed
		timer_.DeferralUpdate(2);
		EXPECT_FALSE(deferred_.callbackCalled);
	}

	TEST_F(DeferralTimerTest, RemoveDeferralMultiple) {
		const auto deferral_entry   = timer_.RegisterAtTick(deferred_, nullptr, 2);
		const auto deferral_entry_2 = timer_.RegisterAtTick(deferred_, nullptr, 2);

		timer_.RemoveDeferral(deferral_entry);
		timer_.RemoveDeferral(deferral_entry_2);

		// Both deferrals removed
		timer_.DeferralUpdate(2);
		EXPECT_FALSE(deferred_.callbackCalled);
	}

	TEST_F(DeferralTimerTest, RemoveDeferralNonExistent) {
		timer_.RemoveDeferral({32, 999});
	}

	TEST_F(DeferralTimerTest, RemoveDeferralEntry) {
		{
			const MockDeferred deferred{};
			auto entry = timer_.RegisterAtTick(deferred, nullptr, 1);

			timer_.RemoveDeferralEntry(entry);

			EXPECT_EQ(entry.second, 0);

			timer_.DeferralUpdate(1);
			EXPECT_FALSE(deferred.callbackCalled);
		}
		{
			WorldData::DeferralTimer::DeferralEntry entry{12, 0};  // Invalid entry since .second is 0
			timer_.RemoveDeferralEntry(entry);
		}
	}


	// ======================================================================

	class UpdateDispatcherTest : public testing::Test
	{
		WorldData worldData_;

		class MockUpdateListener final : public data::IUpdateListener
		{
		public:
			mutable WorldData::WorldPair emit;
			mutable WorldData::WorldPair receive;
			mutable int calls = 0;

			mutable data::UpdateType type = data::UpdateType::remove;

			void OnTileUpdate(WorldData&,
			                  const WorldData::WorldPair& emit_coords,
			                  const WorldData::WorldPair& receive_coords,
			                  const data::UpdateType type) const override {
				emit    = emit_coords;
				receive = receive_coords;

				calls++;
				this->type = type;
			}
		};

	protected:
		WorldData::UpdateDispatcher& dispatcher_ = worldData_.updateDispatcher;

		MockUpdateListener mock_{};
	};

	TEST_F(UpdateDispatcherTest, Register) {
		dispatcher_.Register(2, 3,
		                     5, 6, mock_);

		dispatcher_.Dispatch(3, 7, data::UpdateType::place);
		EXPECT_EQ(mock_.emit.first, 0);
		EXPECT_EQ(mock_.emit.second, 0);

		EXPECT_EQ(mock_.receive.first, 0);
		EXPECT_EQ(mock_.receive.second, 0);


		dispatcher_.Dispatch(5, 6, data::UpdateType::remove);

		EXPECT_EQ(mock_.emit.first, 5);
		EXPECT_EQ(mock_.emit.second, 6);

		EXPECT_EQ(mock_.receive.first, 2);
		EXPECT_EQ(mock_.receive.second, 3);

		EXPECT_EQ(mock_.type, data::UpdateType::remove);
	}

	TEST_F(UpdateDispatcherTest, Unregister) {
		const auto entry = dispatcher_.Register(2, 3,
		                                        5, 6, mock_);

		dispatcher_.Register(4, 7,
		                    5, 6, mock_);

		dispatcher_.Unregister(entry);

		// 1 registered, 1 unregistered
		dispatcher_.Dispatch(5, 6, data::UpdateType::place);
		EXPECT_EQ(mock_.emit.first, 5);
		EXPECT_EQ(mock_.emit.second, 6);

		EXPECT_EQ(mock_.receive.first, 4);
		EXPECT_EQ(mock_.receive.second, 7);
		EXPECT_EQ(mock_.calls, 1);
	}
}
