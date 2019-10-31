#include "game/world/world_manager.h"

#include <unordered_map>

using world_chunks_key = unsigned long long;

// world_chunks_key correlate to a chunk
std::unordered_map<world_chunks_key, jactorio::game::Chunk*> world_chunks;


// Packs 2 32 bit integers into a 64 bit integer to index world_chunks
world_chunks_key to_world_chunks_key(const int x, const int y) {
	const world_chunks_key i = static_cast<world_chunks_key>(x) << 32 | y;
	return i;	
}


jactorio::game::Chunk* jactorio::game::world_manager::add_chunk(Chunk* chunk) {
	const auto position = chunk->get_position();
	const world_chunks_key coords = to_world_chunks_key(position.first, position.second);
	world_chunks[coords] = chunk;

	// Return pointer to allocated chunk
	return world_chunks[coords];
}

jactorio::game::Chunk* jactorio::game::world_manager::get_chunk(int x, int y) {
	const world_chunks_key coords = to_world_chunks_key(x, y);
	return world_chunks[coords];
}

void jactorio::game::world_manager::clear_chunk_data() {
	for (auto& chunk_data : world_chunks) {
		// Free chunk pointer
		delete chunk_data.second;
	}

	world_chunks.clear();
}
