#include "game/world/world_generator.h"

#include <noise/noise.h>
#include <noise/noiseutils.h>
#include <algorithm>
#include <set>
#include <mutex>

#include "core/logger.h"
#include "game/world/chunk_tile.h"
#include "game/world/world_manager.h"
#include "data/data_manager.h"
#include "data/prototype/noise_layer.h"
#include "data/prototype/tile/resource_tile.h"

// Prevents constant erasing of buffers
int world_gen_seed = 1001;

// Stores whether or not a chunk is being generated, this gets cleared once all world generation is done
std::set<std::tuple<int, int>> world_gen_chunks;

/**
 * Generates a chunk and adds it to the world when done <br>
 * Call this with a std::thread to to this in async
 * @param chunk_x X position of chunk to generate
 * @param chunk_y Y position of chunk to generate
 */
void generate(const int chunk_x, const int chunk_y) {
	using namespace jactorio;
	
	LOG_MESSAGE_f(debug, "Generating new chunk at %d, %d...", chunk_x, chunk_y);

	// The Y axis for libnoise is inverted. It causes no issues as of right now. I am leaving this here
	// In case something happens in the future

	
	// Get all noise layers for building terrain
	std::vector<data::Noise_layer*> noise_layers = data::data_manager::data_raw_get_all<data::
		Noise_layer>(
		data::data_category::noise_layer);


	// Sort Noise layers, the one with the highest order takes priority if tiles overlap
	std::sort(noise_layers.begin(), noise_layers.end(),
	          [](data::Noise_layer* left, data::Noise_layer* right) {
		          return left->order < right->order;
	          }
	);

	
	// This will be deleted by the world_manager
	auto* tiles = new game::Chunk_tile[1024];

	int seed_offset = 0;
	for (auto& noise_layer : noise_layers) {

		module::Perlin base_terrain_noise_module;
		base_terrain_noise_module.SetSeed(world_gen_seed + seed_offset++);

		// Load properties of each noise layer
		base_terrain_noise_module.SetOctaveCount(noise_layer->octave_count);
		base_terrain_noise_module.SetFrequency(noise_layer->frequency);
		base_terrain_noise_module.SetPersistence(noise_layer->persistence);
		
		utils::NoiseMap base_terrain_height_map;
		utils::NoiseMapBuilderPlane height_map_builder;
		height_map_builder.SetSourceModule(base_terrain_noise_module);
		height_map_builder.SetDestNoiseMap(base_terrain_height_map);
		height_map_builder.SetDestSize(32, 32);

		// Since x, y represents the center of the chunk, +- 0.5 to get the edges 
		height_map_builder.SetBounds(chunk_x - 0.5, chunk_x + 0.5,
		                             chunk_y - 0.5, chunk_y + 0.5);
		height_map_builder.Build();


		// Transfer noise values from height map to chunk tiles
		for (int y = 0; y < 32; ++y) {
			for (int x = 0; x < 32; ++x) {
				const auto* new_tile = noise_layer->get_tile(base_terrain_height_map.GetValue(x, y));

				if (new_tile == nullptr)
					continue;

				game::Chunk_tile& chunk_tile = tiles[y * 32 + x];
				
				// Noise layer is generating resources, check if at this tile there are any
				if (noise_layer->tile_data_category == data::data_category::resource_tile) {
					// RESOURCE
					chunk_tile.set_tile_layer_tile_prototype(game::Chunk_tile::chunk_layer::resource, 
					                                         data::data_manager::data_raw_get<data::Resource_tile>(
						                                         data::data_category::resource_tile, new_tile->name));
				}
				else {
					// TILE
					// Add the tile prototype to the Chunk_tile
					chunk_tile.set_tile_layer_tile_prototype(game::Chunk_tile::chunk_layer::base, 
					                                         data::data_manager::data_raw_get<data::Tile>(
						                                         data::data_category::tile, new_tile->name)
					);
				}
				
			}
		}

	}
	
	// Post generation processing
	for (int y = 0; y < 32; ++y) {
		for (int x = 0; x < 32; ++x) {
			auto& tile = tiles[y * 32 + x];

			// Remove any resources on water
			if (tile.get_tile_layer_tile_prototype(game::Chunk_tile::chunk_layer::base)->is_water) {
				tile.set_tile_layer_sprite_prototype(game::Chunk_tile::chunk_layer::resource, nullptr);
				tile.set_tile_layer_tile_prototype(game::Chunk_tile::chunk_layer::resource, nullptr);
			}
		}
	}
	
	game::world_manager::add_chunk(new game::Chunk{chunk_x, chunk_y, tiles});
}


void jactorio::game::world_generator::set_world_generator_seed(const int seed) {
	world_gen_seed = seed;
}

int jactorio::game::world_generator::get_world_generator_seed() {
	return world_gen_seed;
}

std::mutex m;
void jactorio::game::world_generator::queue_chunk_generation(const int chunk_x, const int chunk_y) {
	const auto chunk_key = std::tuple<int, int>{ + chunk_x, chunk_y };
	
	// Is the chunk already under generation? If so return
	if (world_gen_chunks.find(chunk_key) != world_gen_chunks.end())
		return;

	// Writing
	std::lock_guard<std::mutex> lk{m};
	world_gen_chunks.insert(std::pair{chunk_x, chunk_y});
}

void jactorio::game::world_generator::gen_chunk(uint8_t amount) {
	assert(amount > 0);
	
	// Generate a chunk
	// Find the first chunk which has yet been generated, ->second is true indicates it NEEDS generation
	for (auto& coords : world_gen_chunks) {
		generate(std::get<0>(coords), std::get<1>(coords));

		// Mark the chunk as done generating
		world_gen_chunks.erase(coords);

		if (--amount == 0)
			break;
	}

}
