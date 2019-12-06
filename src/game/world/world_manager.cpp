#include "game/world/world_manager.h"

#include <unordered_map>
#include <future>
#include <mutex>

#include "core/data_type/unordered_map.h"

using world_chunks_key = unsigned long long;

namespace
{
	// world_chunks_key correlate to a chunk
	std::unordered_map<std::tuple<int, int>, jactorio::game::Chunk*,
	                   jactorio::core::hash<std::tuple<int, int>>> world_chunks;
	std::mutex m_world_chunks;
}

jactorio::game::Chunk* jactorio::game::world_manager::add_chunk(Chunk* chunk) {
	const auto position = chunk->get_position();
	const auto coords = std::tuple<int, int>{ position.first, position.second};

	std::lock_guard<std::mutex> guard(m_world_chunks);

	// A chunk already exist at this position?
	if (world_chunks.find(coords) != world_chunks.end()) {
		delete world_chunks[coords];
	}
	
	world_chunks[coords] = chunk;

	// Return pointer to allocated chunk
	return world_chunks[coords];
}

jactorio::game::Chunk* jactorio::game::world_manager::get_chunk(int x, int y) {
	const auto key = std::tuple<int, int>{ x, y };
	if (world_chunks.find(key) == world_chunks.end())
		return nullptr;
	
	return world_chunks[key];
}

void jactorio::game::world_manager::clear_chunk_data() {
	// The chunk data itself needs to be deleted
	for (auto& world_chunk : world_chunks) {
		delete world_chunk.second;
	}
	world_chunks.clear();
}
