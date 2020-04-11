// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#include "game/world/world_data.h"

#include <future>
#include <mutex>

#include "game/input/mouse_selection.h"

void jactorio::game::World_data::on_tick_advance() {
	game_tick_++;

	// Dispatch deferred callbacks
	deferral_timer.deferral_update(game_tick_);
}

jactorio::game::Chunk* jactorio::game::World_data::add_chunk(Chunk* chunk) {
	const auto position = chunk->get_position();
	const auto coords = std::tuple<int, int>{position.first, position.second};

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

jactorio::game::Chunk* jactorio::game::World_data::get_chunk(const Chunk::chunk_coord chunk_x,
                                                             const Chunk::chunk_coord chunk_y) const {
	std::lock_guard<std::mutex> guard(world_chunks_mutex_);

	const auto key = std::tuple<int, int>{chunk_x, chunk_y};

	if (world_chunks_.find(key) == world_chunks_.end())
		return nullptr;

	return world_chunks_.at(key);
}

jactorio::game::Chunk* jactorio::game::World_data::get_chunk_world_coords(world_coord world_x, world_coord world_y) const {
	// See get_tile_world_coords() for documentation on the purpose of if statements

	float chunk_index_x = 0;
	float chunk_index_y = 0;

	if (world_x < 0) {
		chunk_index_x -= 1;
		world_x += 1;
	}
	if (world_y < 0) {
		chunk_index_y -= 1;
		world_y += 1;
	}

	chunk_index_x += static_cast<float>(world_x) / 32;
	chunk_index_y += static_cast<float>(world_y) / 32;

	return get_chunk(static_cast<int>(chunk_index_x), static_cast<int>(chunk_index_y));
}

// ======================================================================

jactorio::game::Chunk_tile* jactorio::game::World_data::get_tile_world_coords(world_coord world_x, world_coord world_y) const {
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

	chunk_index_x += static_cast<float>(world_x) / 32;
	chunk_index_y += static_cast<float>(world_y) / 32;


	auto* chunk = get_chunk(static_cast<int>(chunk_index_x), static_cast<int>(chunk_index_y));

	if (chunk != nullptr) {
		int tile_index_x = static_cast<int>(world_x) % 32;
		int tile_index_y = static_cast<int>(world_y) % 32;

		// Chunk is 32 tiles
		if (negative_x) {
			tile_index_x = 31 - tile_index_x * -1;
		}
		if (negative_y) {
			tile_index_y = 31 - tile_index_y * -1;
		}

		return &chunk->tiles_ptr()[32 * tile_index_y + tile_index_x];
	}

	return nullptr;
}

// ======================================================================
// Logic chunks
jactorio::game::Logic_chunk& jactorio::game::World_data::logic_add_chunk(Chunk* chunk) {
	assert(chunk != nullptr);
	const auto& iterator = logic_chunks_.emplace(chunk, chunk);
	return iterator.first->second;
}

std::map<const jactorio::game::Chunk*, jactorio::game::Logic_chunk>& jactorio::game::World_data::logic_get_all_chunks() {
	return logic_chunks_;
}

jactorio::game::Logic_chunk* jactorio::game::World_data::logic_get_chunk(const Chunk* chunk) {
	if (logic_chunks_.find(chunk) == logic_chunks_.end())
		return nullptr;

	return &logic_chunks_.at(chunk);
}

const jactorio::game::Logic_chunk* jactorio::game::World_data::logic_get_chunk_read_only(const Chunk* chunk) const {
	if (logic_chunks_.find(chunk) == logic_chunks_.end())
		return nullptr;

	return &logic_chunks_.at(chunk);
}
