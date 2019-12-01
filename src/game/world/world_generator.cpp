#include "game/world/world_generator.h"

#include <noise/noise.h>
#include <noise/noiseutils.h>
#include <algorithm>

#include "core/logger.h"
#include "game/world/tile.h"
#include "game/world/world_manager.h"
#include "data/data_manager.h"
#include "data/prototype/noise_layer.h"
#include "renderer/rendering/renderer_manager.h"

void jactorio::game::world_generator::generate_chunk(const int chunk_x, const int chunk_y,
                                                     std::atomic<int>* thread_counter) {
	// TODO configurable base seed
	constexpr int base_seed = 1001;
	
	LOG_MESSAGE_f(debug, "Generating new chunk at %d, %d...", chunk_x, chunk_y);

	// The Y axis for libnoise is inverted. It causes no issues as of right now. I am leaving this here
	// In case something happens in the future

	// Get all noise layers for building terrain
	std::vector<data::Noise_layer*> noise_layers = data::data_manager::data_raw_get_all<data::
		Noise_layer>(
		data::data_category::noise_layer);

	// Non resource noise layers gets generated first, ensuring that all tiles marked is_water
	// is generated first, then the resources
	std::sort(noise_layers.begin(), noise_layers.end(), 
	          [](data::Noise_layer* left, data::Noise_layer* right) {
		          if (left->tile_data_category == data::data_category::tile && 
			          right->tile_data_category != data::data_category::tile)
			          return true;
		
		          return false;
	          }
	);

	auto* tiles = new Chunk_tile[1024];
	for (auto& noise_layer : noise_layers) {

		module::Perlin base_terrain_noise_module;
		base_terrain_noise_module.SetSeed(base_seed);

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
				// tiles marked as water, if so, do not place resources
				if (noise_layer->tile_data_category == data::data_category::resource_tile &&
					chunk_tile.is_water)
					continue;

				
				// Add the tile prototype to the Chunk_tile
				prototype_vector.push_back(
					data::data_manager::data_raw_get<data::Tile>(
						noise_layer->tile_data_category, new_tile->name)
				);


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

	world_manager::add_chunk(new Chunk{chunk_x, chunk_y, tiles});
	--*thread_counter;
}
