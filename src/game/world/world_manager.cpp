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


// Rendering

void jactorio::game::world_manager::draw_chunks(const renderer::Renderer& renderer,
                                                const unsigned int window_start_x,
                                                const unsigned int window_start_y,
                                                const int chunk_start_x,
                                                const int chunk_start_y,
                                                const unsigned int chunk_amount_x,
                                                const unsigned int chunk_amount_y) {
	for (unsigned int chunk_y = 0; chunk_y < chunk_amount_y; ++chunk_y) {
		const unsigned int chunk_y_offset = chunk_y * 32;

		for (unsigned int chunk_x = 0; chunk_x < chunk_amount_x; ++chunk_x) {
			const unsigned int chunk_x_offset = chunk_x * 32;
			
			Tile* const* chunk = get_chunk(chunk_start_x + chunk_x,
			                               chunk_start_y + chunk_y)->tiles_ptr();

			// Iterate through and draw tiles of a chunk
			for (int tile_y = 0; tile_y < 32; ++tile_y) {
				for (int tile_x = 0; tile_x < 32; ++tile_x) {
					renderer.set_sprite(
						// Window offset, chunk offset, tile offset
						window_start_x + chunk_x_offset + tile_x,
						window_start_y + chunk_y_offset + tile_y,
						chunk[tile_x + tile_y]->tile_prototype->name);
				}
			}
		}
	}
}


