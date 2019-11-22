#include "game/world/world_manager.h"

#include <unordered_map>
#include <future>
#include <chrono>

#include "core/logger.h"
#include "core/data_type/unordered_map.h"

using world_chunks_key = unsigned long long;

namespace
{
	// world_chunks_key correlate to a chunk
	std::unordered_map<std::tuple<int, int>, jactorio::game::Chunk*,
	                   jactorio::core::hash<std::tuple<int, int>>> world_chunks;
}

jactorio::game::Chunk* jactorio::game::world_manager::add_chunk(Chunk* chunk) {
	const auto position = chunk->get_position();
	const auto coords = std::tuple<int, int>{ position.first, position.second};

	world_chunks[coords] = chunk;

	// Return pointer to allocated chunk
	return world_chunks[coords];
}

jactorio::game::Chunk* jactorio::game::world_manager::get_chunk(int x, int y) {
	return world_chunks[std::tuple<int, int>{x, y}];
}

void jactorio::game::world_manager::clear_chunk_data() {
	for (auto& chunk_data : world_chunks) {
		// Free chunk pointer
		delete chunk_data.second;
	}

	world_chunks.clear();
}
