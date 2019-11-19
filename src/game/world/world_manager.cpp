#include "game/world/world_manager.h"

#include <unordered_map>
#include <future>
#include <chrono>

#include "core/logger.h"
#include "data/data_manager.h"

using world_chunks_key = unsigned long long;

namespace
{
	// world_chunks_key correlate to a chunk
	std::unordered_map<world_chunks_key, jactorio::game::Chunk*> world_chunks;

	// Threads for generating new chunks, threading world gen prevents the renderer from being blocked
	// while generating
	std::vector<std::future<void>> world_gen_threads;
}


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
                                                const int window_start_x,
                                                const int window_start_y,
                                                const int chunk_start_x,
                                                const int chunk_start_y,
                                                const unsigned int chunk_amount_x,
                                                const unsigned int chunk_amount_y) {
	// Check if world gen threads are done
	for (unsigned int i = 0; i < world_gen_threads.size(); ++i) {
		const auto& future = world_gen_threads[i];

		using namespace std::chrono_literals;

		// Is thread done? If so delete it
		if (future.wait_for(0ms) == std::future_status::ready) {
			world_gen_threads.erase(world_gen_threads.begin() + i);
		}
	}

	
	for (int chunk_y = 0; chunk_y < chunk_amount_y; ++chunk_y) {
		const unsigned int chunk_y_offset = chunk_y * 32;

		for (int chunk_x = 0; chunk_x < chunk_amount_x; ++chunk_x) {
			const unsigned int chunk_x_offset = chunk_x * 32;

			auto chunk = get_chunk(chunk_start_x + chunk_x,
			                       chunk_start_y + chunk_y);

			if (chunk == nullptr) {
				// Chunk is asynchronously generated
				world_gen_threads.push_back(
					std::async(std::launch::async,
					           generate_chunk, 
					           chunk_start_x + chunk_x,
					           chunk_start_y + chunk_y));

				// Put a blank chunk as a placeholder while it is being generated
				add_chunk(new Chunk{ chunk_start_x + chunk_x, chunk_start_y + chunk_y, nullptr});
				continue;
			}
			
			Tile* const* tiles = chunk->tiles_ptr();
			// Chunk is not yet ready if the tiles' prototypes point to nothing
			if (tiles[0]->tile_prototype == nullptr)
				continue;
			
			// Iterate through and draw tiles of a chunk
			for (int tile_y = 0; tile_y < 32; ++tile_y) {
				for (int tile_x = 0; tile_x < 32; ++tile_x) {
					// Todo: possibly batch this call??
					renderer.set_sprite(
						// Window offset, chunk offset, tile offset
						window_start_x + chunk_x_offset + tile_x,
						window_start_y + chunk_y_offset + tile_y,
						tiles[tile_x + tile_y]->tile_prototype->name);
				}
			}
		}
	}
}

void jactorio::game::world_manager::generate_chunk(const int x, const int y) {
	LOG_MESSAGE_f(debug, "Generating new chunk at %d, %d...", x, y);
	
	// World generator test
	// TODO, the data which the chunk tiles point to needs to be deleted

	// TODO perlin noise tile generation
	auto* tiles = new Tile[1024];
	for (int i = 0; i < 32 * 32; ++i) {
		const auto proto_tile = data::data_manager::data_raw_get(data::data_category::tile, "test_tile");

		tiles[i].tile_prototype = static_cast<data::Tile*>(proto_tile);
	}

	add_chunk(new Chunk{x, y, tiles});

	// This is just a test to mark the start of each chunk
	auto* g_tile = new Tile{};
	g_tile->tile_prototype = static_cast<data::Tile*>(data::data_manager::data_raw_get(
		data::data_category::tile, "grass-1"));
	get_chunk(x, y)->tiles_ptr()[0] = g_tile;
}


