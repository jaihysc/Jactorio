#include "game/world/world_generator.h"

#include <noise/noise.h>
#include <noise/noiseutils.h>
#include <algorithm>
#include <map>

#include "core/logger.h"
#include "game/world/tile.h"
#include "game/world/world_manager.h"
#include "data/data_manager.h"
#include "data/prototype/noise_layer.h"
#include "renderer/rendering/renderer_manager.h"

// Prevents constant erasing of buffers
int world_gen_seed = 1001;

bool world_gen_buffer_cleared = true;
unsigned int chunks_awaiting_generation = 0;
// Stores whether or not a chunk is being generated, this gets cleared once all world generation is done
std::map<std::tuple<int, int>, bool> world_gen_chunks;

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

	// This will be deleted by the world_manager
	auto* tiles = new game::Chunk_tile[1024];

	for (auto& noise_layer : noise_layers) {

		module::Perlin base_terrain_noise_module;
		base_terrain_noise_module.SetSeed(world_gen_seed);

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

				auto& chunk_tile = tiles[y * 32 + x];
				std::vector<data::Tile*>& prototype_vector = chunk_tile.tile_prototypes;

				
				// Noise layer is generating resources, check if at this tile there are any
				if (noise_layer->tile_data_category == data::data_category::resource_tile) {
					// tiles marked as water or not accepting resource, do not place resources
					if (chunk_tile.is_water || !chunk_tile.accepts_resource)
						continue;

					// A resource already exists at this prototype
					if (chunk_tile.resource_prototype != nullptr) {
						// Remove all resources since it cannot be guaranteed which resource is
						// generated first
						chunk_tile.accepts_resource = false;
						chunk_tile.resource_prototype = nullptr;
						continue;
					}

					chunk_tile.resource_prototype = data::data_manager::data_raw_get<data::Resource_tile>(
						data::data_category::resource_tile, new_tile->name
					);
				}
				else {
					// Add the tile prototype to the Chunk_tile
					prototype_vector.push_back(
						data::data_manager::data_raw_get<data::Tile>(
							data::data_category::tile, new_tile->name)
					);
				}

				// ############################################################
				// Post tile addition processing

				// Set the is_water status of the current Chunk_tile
				if (new_tile->is_water)
					chunk_tile.is_water = true;
				
				// Increment layer count based on the largest tile_prototypes vector size
				if (prototype_vector.size() > renderer::renderer_manager::prototype_layer_count) {
					renderer::renderer_manager::prototype_layer_count = prototype_vector.size();
				}
				
			}
		}

	}
	
	game::world_manager::add_chunk(new game::Chunk{chunk_x, chunk_y, tiles});
}


void jactorio::game::world_generator::set_world_generator_seed(int seed) {
	world_gen_seed = seed;
}

int jactorio::game::world_generator::get_world_generator_seed() {
	return world_gen_seed;
}

void jactorio::game::world_generator::queue_chunk_generation(const int chunk_x, const int chunk_y) {
	const auto chunk_key = std::tuple<int, int>{ + chunk_x, chunk_y };

	// Is the chunk already under generation? If so return
	if (world_gen_chunks.find(chunk_key) != world_gen_chunks.end())
		return;

	world_gen_chunks[chunk_key] = true;
	chunks_awaiting_generation++;
	
	// The map is now populated
	world_gen_buffer_cleared = false;
}

void jactorio::game::world_generator::gen_chunk() {
	if (world_gen_buffer_cleared)
		return;
	
	// Clear world gen threads once the vector is empty, this is faster than deleting it one by one
	if (!world_gen_buffer_cleared && chunks_awaiting_generation == 0) {
		LOG_MESSAGE(debug, "Chunk generation queue cleared");
		world_gen_chunks.clear();
		world_gen_buffer_cleared = true;
		return;
	}


	// Generate a chunk
	// Find the first chunk which has yet been generated, ->second is true indicates it NEEDS generation
	for (auto& c : world_gen_chunks) {
		if (c.second) {			
			auto& coords = c.first;
			generate(std::get<0>(coords), std::get<1>(coords));

			// Mark the chunk as done generating
			c.second = false;
			chunks_awaiting_generation--;
			break;
		}
	}

}
