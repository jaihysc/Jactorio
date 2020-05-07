// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#include "game/world/world_data.h"

#include <cmath>
#include <future>
#include <mutex>

void jactorio::game::World_data::on_tick_advance() {
	game_tick_++;

	// Dispatch deferred callbacks
	deferral_timer.deferral_update(game_tick_);
}

jactorio::game::Chunk::chunk_coord jactorio::game::World_data::to_chunk_coord(world_coord world_coord) {
	Chunk::chunk_coord chunk_coord = 0;

	if (world_coord < 0) {
		chunk_coord -= 1;
		world_coord += 1;
	}

	chunk_coord += static_cast<float>(world_coord) / Chunk::chunk_width;
	return chunk_coord;
}

jactorio::game::Chunk_struct_layer::struct_coord jactorio::game::World_data::to_struct_coord(const world_coord world_coord) {
	return fabs(to_chunk_coord(world_coord) * Chunk::chunk_width - world_coord);
}

jactorio::game::Chunk* jactorio::game::World_data::add_chunk(Chunk* chunk) {
	const auto position = chunk->get_position();
	const auto coords   = std::tuple<int, int>{position.first, position.second};

	std::lock_guard<std::mutex> guard(world_chunks_mutex_);

	// A chunk already exist at this position?
	if (world_chunks_.find(coords) != world_chunks_.end()) {
		delete world_chunks_[coords];
	}

	world_chunks_[coords] = chunk;

	// Return pointer to allocated chunk
	return chunk;
}

void jactorio::game::World_data::clear_chunk_data() {
	std::lock_guard<std::mutex> guard(world_chunks_mutex_);

	// The chunk data itself needs to be deleted
	for (auto& world_chunk : world_chunks_) {
		delete world_chunk.second;
	}
	world_chunks_.clear();
	logic_chunks_.clear();
}

// ======================================================================

jactorio::game::Chunk* jactorio::game::World_data::get_chunk_c(const Chunk::chunk_coord chunk_x,
                                                               const Chunk::chunk_coord chunk_y) const {
	std::lock_guard<std::mutex> guard(world_chunks_mutex_);

	const auto key = std::tuple<int, int>{chunk_x, chunk_y};

	if (world_chunks_.find(key) == world_chunks_.end())
		return nullptr;

	return world_chunks_.at(key);
}

jactorio::game::Chunk* jactorio::game::World_data::get_chunk_c(const Chunk::chunk_pair& chunk_pair) const {
	return get_chunk_c(chunk_pair.first, chunk_pair.second);
}

jactorio::game::Chunk* jactorio::game::World_data::get_chunk(const world_coord world_x, const world_coord world_y) const {
	return get_chunk_c(to_chunk_coord(world_x), to_chunk_coord(world_y));
}

jactorio::game::Chunk* jactorio::game::World_data::get_chunk(const world_pair& world_pair) const {
	return get_chunk(world_pair.first, world_pair.second);
}

// ======================================================================

jactorio::game::Chunk_tile* jactorio::game::World_data::get_tile(world_coord world_x, world_coord world_y) const {
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

	chunk_index_x += static_cast<float>(world_x) / Chunk::chunk_width;
	chunk_index_y += static_cast<float>(world_y) / Chunk::chunk_width;


	auto* chunk = get_chunk_c(static_cast<int>(chunk_index_x), static_cast<int>(chunk_index_y));

	if (chunk != nullptr) {
		int tile_index_x = static_cast<int>(world_x) % Chunk::chunk_width;
		int tile_index_y = static_cast<int>(world_y) % Chunk::chunk_width;

		if (negative_x) {
			tile_index_x = Chunk::chunk_width - 1 - tile_index_x * -1;
		}
		if (negative_y) {
			tile_index_y = Chunk::chunk_width - 1 - tile_index_y * -1;
		}

		return &chunk->tiles_ptr()[Chunk::chunk_width * tile_index_y + tile_index_x];
	}

	return nullptr;
}

jactorio::game::Chunk_tile* jactorio::game::World_data::get_tile(const world_pair& world_pair) const {
	return get_tile(world_pair.first, world_pair.second);
}

// ======================================================================
// Logic chunks
jactorio::game::Logic_chunk& jactorio::game::World_data::logic_add_chunk(Chunk* chunk) {
	assert(chunk != nullptr);
	const auto& iterator = logic_chunks_.emplace(chunk, chunk);
	return iterator.first->second;
}

void jactorio::game::World_data::logic_remove_chunk(Logic_chunk* chunk) {
	logic_chunks_.erase(chunk->chunk);
}

std::map<const jactorio::game::Chunk*, jactorio::game::Logic_chunk>& jactorio::game::World_data::logic_get_all_chunks() {
	return logic_chunks_;
}

jactorio::game::Logic_chunk* jactorio::game::World_data::logic_get_chunk(const Chunk* chunk) {
	return const_cast<Logic_chunk*>(static_cast<const World_data&>(*this).logic_get_chunk(chunk));
}

jactorio::game::Logic_chunk* jactorio::game::World_data::logic_get_chunk(const world_coord world_x, const world_coord world_y) {
	return logic_get_chunk(get_chunk(world_x, world_y));
}

jactorio::game::Logic_chunk* jactorio::game::World_data::logic_get_chunk(const world_pair& world_pair) {
	return logic_get_chunk(world_pair.first, world_pair.second);
}

const jactorio::game::Logic_chunk* jactorio::game::World_data::logic_get_chunk(const Chunk* chunk) const {
	if (logic_chunks_.find(chunk) == logic_chunks_.end())
		return nullptr;

	return &logic_chunks_.at(chunk);
}

const jactorio::game::Logic_chunk* jactorio::game::World_data::logic_get_chunk(const world_coord world_x,
                                                                               const world_coord world_y) const {
	return logic_get_chunk(get_chunk(world_x, world_y));
}

const jactorio::game::Logic_chunk* jactorio::game::World_data::logic_get_chunk(const world_pair& world_pair) const {
	return logic_get_chunk(world_pair.first, world_pair.second);
}
