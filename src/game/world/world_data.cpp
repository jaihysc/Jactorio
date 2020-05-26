// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#include "game/world/world_data.h"

#include <cmath>
#include <future>

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

jactorio::game::Chunk* jactorio::game::WorldData::AddChunk(const Chunk& chunk) {
	const auto position = chunk.GetPosition();

	auto conditional = worldChunks_.emplace(std::make_tuple(position.first, position.second), chunk);
	return &conditional.first->second;
}

void jactorio::game::WorldData::DeleteChunk(Chunk::ChunkCoord chunk_x, Chunk::ChunkCoord chunk_y) {
	worldChunks_.erase(std::make_tuple(chunk_x, chunk_y));
}

void jactorio::game::WorldData::ClearChunkData() {
	worldChunks_.clear();
	logicChunks_.clear();
}

// ======================================================================

jactorio::game::Chunk* jactorio::game::WorldData::GetChunkC(const Chunk::ChunkCoord chunk_x, const Chunk::ChunkCoord chunk_y) {
	return const_cast<Chunk*>(static_cast<const WorldData&>(*this).GetChunkC(chunk_x, chunk_y));
}

const jactorio::game::Chunk* jactorio::game::WorldData::GetChunkC(const Chunk::ChunkCoord chunk_x,
                                                                  const Chunk::ChunkCoord chunk_y) const {
	const auto key = std::tuple<int, int>{chunk_x, chunk_y};

	if (worldChunks_.find(key) == worldChunks_.end())
		return nullptr;

	return &worldChunks_.at(key);
}


jactorio::game::Chunk* jactorio::game::WorldData::GetChunkC(const Chunk::ChunkPair& chunk_pair) {
	return GetChunkC(chunk_pair.first, chunk_pair.second);
}

const jactorio::game::Chunk* jactorio::game::WorldData::GetChunkC(const Chunk::ChunkPair& chunk_pair) const {
	return GetChunkC(chunk_pair.first, chunk_pair.second);
}


jactorio::game::Chunk* jactorio::game::WorldData::GetChunk(const WorldCoord world_x, const WorldCoord world_y) {
	return GetChunkC(ToChunkCoord(world_x), ToChunkCoord(world_y));
}

const jactorio::game::Chunk* jactorio::game::WorldData::GetChunk(const WorldCoord world_x, const WorldCoord world_y) const {
	return GetChunkC(ToChunkCoord(world_x), ToChunkCoord(world_y));
}


jactorio::game::Chunk* jactorio::game::WorldData::GetChunk(const WorldPair& world_pair) {
	return GetChunk(world_pair.first, world_pair.second);
}

const jactorio::game::Chunk* jactorio::game::WorldData::GetChunk(const WorldPair& world_pair) const {
	return GetChunk(world_pair.first, world_pair.second);
}

// ======================================================================

jactorio::game::ChunkTile* jactorio::game::WorldData::GetTile(const WorldCoord world_x, const WorldCoord world_y) {
	return const_cast<ChunkTile*>(static_cast<const WorldData&>(*this).GetTile(world_x, world_y));
}

const jactorio::game::ChunkTile* jactorio::game::WorldData::GetTile(WorldCoord world_x, WorldCoord world_y) const {
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


	const auto* chunk = GetChunkC(static_cast<int>(chunk_index_x), static_cast<int>(chunk_index_y));

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


jactorio::game::ChunkTile* jactorio::game::WorldData::GetTile(const WorldPair& world_pair) {
	return GetTile(world_pair.first, world_pair.second);
}

const jactorio::game::ChunkTile* jactorio::game::WorldData::GetTile(const WorldPair& world_pair) const {
	return GetTile(world_pair.first, world_pair.second);
}


// ======================================================================
// Logic chunks

void jactorio::game::WorldData::LogicRegister(const Chunk::LogicGroup group, const WorldPair& world_pair,
                                              const ChunkTile::ChunkLayer layer) {
	assert(group != Chunk::LogicGroup::count_);
	assert(layer != ChunkTile::ChunkLayer::count_);

	auto* chunk = GetChunk(world_pair);
	assert(chunk);

	logicChunks_.emplace(chunk);

	auto* tile_layer = &GetTile(world_pair)->GetLayer(layer);
	chunk->GetLogicGroup(group)
	     .push_back(tile_layer);
}

void jactorio::game::WorldData::LogicRemove(const Chunk::LogicGroup group, const WorldPair& world_pair,
                                            const std::function<bool(ChunkTileLayer*)>& pred) {
	auto* chunk = GetChunk(world_pair);
	assert(chunk);

	auto& logic_group = chunk->GetLogicGroup(group);

	logic_group.erase(
		std::remove_if(logic_group.begin(), logic_group.end(), pred),
		logic_group.end());

	// Remove from logic chunks if now empty
	if (logic_group.empty())
		logicChunks_.erase(chunk);
}

void jactorio::game::WorldData::LogicRemove(const Chunk::LogicGroup group, const WorldPair& world_pair,
                                            const ChunkTile::ChunkLayer layer) {
	auto* chunk = GetChunk(world_pair);
	assert(chunk);

	auto* tile_layer = &GetTile(world_pair)->GetLayer(layer);

	LogicRemove(group, world_pair, [&](ChunkTileLayer* t_layer) {
		return t_layer == tile_layer;
	});
}

void jactorio::game::WorldData::LogicAddChunk(Chunk* chunk) {
	assert(chunk != nullptr);
	logicChunks_.emplace(chunk);
}

std::set<jactorio::game::Chunk*>& jactorio::game::WorldData::LogicGetChunks() {
	return logicChunks_;
}
