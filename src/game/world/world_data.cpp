// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#include "game/world/world_data.h"

#include <cmath>
#include <future>
#include <mutex>

void jactorio::game::WorldData::OnTickAdvance() {
	gameTick_++;

	// Dispatch deferred callbacks
	deferralTimer.DeferralUpdate(gameTick_);
}

jactorio::game::Chunk::ChunkCoord jactorio::game::WorldData::ToChunkCoord(WorldCoord world_coord) {
	Chunk::ChunkCoord chunk_coord = 0;

	if (world_coord < 0) {
		chunk_coord -= 1;
		world_coord += 1;
	}

	chunk_coord += static_cast<float>(world_coord) / Chunk::kChunkWidth;
	return chunk_coord;
}

jactorio::game::ChunkStructLayer::StructCoord jactorio::game::WorldData::ToStructCoord(const WorldCoord world_coord) {
	return fabs(ToChunkCoord(world_coord) * Chunk::kChunkWidth - world_coord);
}

jactorio::game::Chunk* jactorio::game::WorldData::AddChunk(Chunk* chunk) {
	const auto position = chunk->GetPosition();
	const auto coords   = std::tuple<int, int>{position.first, position.second};

	std::lock_guard<std::mutex> guard(worldChunksMutex_);

	// A chunk already exist at this position?
	if (worldChunks_.find(coords) != worldChunks_.end()) {
		delete worldChunks_[coords];
	}

	worldChunks_[coords] = chunk;

	// Return pointer to allocated chunk
	return chunk;
}

void jactorio::game::WorldData::ClearChunkData() {
	std::lock_guard<std::mutex> guard(worldChunksMutex_);

	// The chunk data itself needs to be deleted
	for (auto& world_chunk : worldChunks_) {
		delete world_chunk.second;
	}
	worldChunks_.clear();
	logicChunks_.clear();
}

// ======================================================================

jactorio::game::Chunk* jactorio::game::WorldData::GetChunkC(const Chunk::ChunkCoord chunk_x,
                                                            const Chunk::ChunkCoord chunk_y) const {
	std::lock_guard<std::mutex> guard(worldChunksMutex_);

	const auto key = std::tuple<int, int>{chunk_x, chunk_y};

	if (worldChunks_.find(key) == worldChunks_.end())
		return nullptr;

	return worldChunks_.at(key);
}

jactorio::game::Chunk* jactorio::game::WorldData::GetChunkC(const Chunk::ChunkPair& chunk_pair) const {
	return GetChunkC(chunk_pair.first, chunk_pair.second);
}

jactorio::game::Chunk* jactorio::game::WorldData::GetChunk(const WorldCoord world_x, const WorldCoord world_y) const {
	return GetChunkC(ToChunkCoord(world_x), ToChunkCoord(world_y));
}

jactorio::game::Chunk* jactorio::game::WorldData::GetChunk(const WorldPair& world_pair) const {
	return GetChunk(world_pair.first, world_pair.second);
}

// ======================================================================

jactorio::game::ChunkTile* jactorio::game::WorldData::GetTile(WorldCoord world_x, WorldCoord world_y) const {
	// The negative chunks start at -1, unlike positive chunks at 0
	// Thus add 1 to become 0 so the calculations can be performed
	bool negative_x = false;
	bool negative_y = false;

	float chunk_index_x = 0;
	float chunk_index_y = 0;

	if (world_x < 0) {
		negative_x = true;
		chunk_index_x -= 1;
		world_x += 1;
	}
	if (world_y < 0) {
		negative_y = true;
		chunk_index_y -= 1;
		world_y += 1;
	}

	chunk_index_x += static_cast<float>(world_x) / Chunk::kChunkWidth;
	chunk_index_y += static_cast<float>(world_y) / Chunk::kChunkWidth;


	auto* chunk = GetChunkC(static_cast<int>(chunk_index_x), static_cast<int>(chunk_index_y));

	if (chunk != nullptr) {
		int tile_index_x = static_cast<int>(world_x) % Chunk::kChunkWidth;
		int tile_index_y = static_cast<int>(world_y) % Chunk::kChunkWidth;

		if (negative_x) {
			tile_index_x = Chunk::kChunkWidth - 1 - tile_index_x * -1;
		}
		if (negative_y) {
			tile_index_y = Chunk::kChunkWidth - 1 - tile_index_y * -1;
		}

		return &chunk->Tiles()[Chunk::kChunkWidth * tile_index_y + tile_index_x];
	}

	return nullptr;
}

jactorio::game::ChunkTile* jactorio::game::WorldData::GetTile(const WorldPair& world_pair) const {
	return GetTile(world_pair.first, world_pair.second);
}

// ======================================================================
// Logic chunks
jactorio::game::LogicChunk& jactorio::game::WorldData::LogicAddChunk(Chunk* chunk) {
	assert(chunk != nullptr);
	const auto& iterator = logicChunks_.emplace(chunk, chunk);
	return iterator.first->second;
}

void jactorio::game::WorldData::LogicRemoveChunk(LogicChunk* chunk) {
	logicChunks_.erase(chunk->chunk);
}

std::map<const jactorio::game::Chunk*, jactorio::game::LogicChunk>& jactorio::game::WorldData::LogicGetAllChunks() {
	return logicChunks_;
}

jactorio::game::LogicChunk* jactorio::game::WorldData::LogicGetChunk(const Chunk* chunk) {
	return const_cast<LogicChunk*>(static_cast<const WorldData&>(*this).LogicGetChunk(chunk));
}

jactorio::game::LogicChunk* jactorio::game::WorldData::LogicGetChunk(const WorldCoord world_x, const WorldCoord world_y) {
	return LogicGetChunk(GetChunk(world_x, world_y));
}

jactorio::game::LogicChunk* jactorio::game::WorldData::LogicGetChunk(const WorldPair& world_pair) {
	return LogicGetChunk(world_pair.first, world_pair.second);
}

const jactorio::game::LogicChunk* jactorio::game::WorldData::LogicGetChunk(const Chunk* chunk) const {
	if (logicChunks_.find(chunk) == logicChunks_.end())
		return nullptr;

	return &logicChunks_.at(chunk);
}

const jactorio::game::LogicChunk* jactorio::game::WorldData::LogicGetChunk(const WorldCoord world_x,
                                                                           const WorldCoord world_y) const {
	return LogicGetChunk(GetChunk(world_x, world_y));
}

const jactorio::game::LogicChunk* jactorio::game::WorldData::LogicGetChunk(const WorldPair& world_pair) const {
	return LogicGetChunk(world_pair.first, world_pair.second);
}
