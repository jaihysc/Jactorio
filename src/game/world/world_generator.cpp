#include "game/world/world_generator.h"

#include <noise/noise.h>
#include <noise/noiseutils.h>

#include "core/logger.h"
#include "game/world/tile.h"
#include "game/world/world_manager.h"
#include "data/data_manager.h"
#include "data/prototype/noise_layer.h"

namespace
{
	void build_height_map(const module::Perlin& noise_module, utils::NoiseMap& height_map,
	                      const int chunk_x, const int chunk_y) {

		utils::NoiseMapBuilderPlane height_map_builder;
		height_map_builder.SetSourceModule(noise_module);
		height_map_builder.SetDestNoiseMap(height_map);
		height_map_builder.SetDestSize(32, 32);

		// Since x, y represents the center of the chunk, +- 0.5 to get the edges 
		height_map_builder.SetBounds(chunk_x - 0.5, chunk_x + 0.5,
		                             chunk_y - 0.5, chunk_y + 0.5);
		height_map_builder.Build();
	}
}


void jactorio::game::world_generator::generate_chunk(const int chunk_x, const int chunk_y,
                                                     std::atomic<int>* thread_counter) {
	LOG_MESSAGE_f(debug, "Generating new chunk at %d, %d...", chunk_x, chunk_y);

	// The Y axis for libnoise is inverted. It causes no issues as of right now. I am leaving this here
	// In case something happens in the future

	module::Perlin base_terrain_noise_module;
	base_terrain_noise_module.SetOctaveCount(8);
	base_terrain_noise_module.SetFrequency(0.25);
	base_terrain_noise_module.SetPersistence(0.5);

	module::Perlin resource_noise_module;
	resource_noise_module.SetOctaveCount(2);
	resource_noise_module.SetFrequency(0.9);
	resource_noise_module.SetPersistence(0.3);

	utils::NoiseMap base_terrain_height_map;
	utils::NoiseMap resource_height_map;

	build_height_map(base_terrain_noise_module, base_terrain_height_map, chunk_x, chunk_y);
	build_height_map(resource_noise_module, resource_height_map, chunk_x, chunk_y);

	auto* tiles = new Chunk_tile[1024];

	// Call all noise layers to build terrain
	std::vector<data::Noise_layer*> noise_layers = data::data_manager::data_raw_get_all<data::
		Noise_layer>(
		data::data_category::noise_layer);

	for (int y = 0; y < 32; ++y) {
		for (int x = 0; x < 32; ++x) {

			float noise_val = base_terrain_height_map.GetValue(x, y);

			// Libnoise does not guarantee a range of -1 - 1
			for (auto& noise_layer : noise_layers) {
				// Round to range of max-min within noise_layer
				const float start_val = noise_layer->get_start_val();
				const float end_val = noise_layer->get_max_noise_val();
				
				if (noise_val > end_val)
					noise_val = end_val;
				if (noise_val < start_val)
					noise_val = start_val;

				std::string tile_name = noise_layer->get_tile(noise_val)->name;

				tiles[y * 32 + x].tile_prototypes.push_back(
					data::data_manager::data_raw_get
					<data::Tile>(data::data_category::tile, tile_name)
				);
			}
		}
	}

	world_manager::add_chunk(new Chunk{chunk_x, chunk_y, tiles});
	--*thread_counter;
}
