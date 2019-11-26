#include "game/world/world_generator.h"

#include <noise/noise.h>
#include <noise/noiseutils.h>

#include "core/logger.h"
#include "game/world/tile.h"
#include "data/data_manager.h"
#include "game/world/world_manager.h"

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
	
	for (int y = 0; y < 32; ++y) {
		for (int x = 0; x < 32; ++x) {
			std::string tile_name = "test_tile";

			// Base terrain
			const auto noise_val = base_terrain_height_map.GetValue(x, y);
			bool in_water = false;
			if (noise_val <= -0.5f) {
				in_water = true;
				tile_name = "deep-water-1";
			}
			else if (noise_val > -0.5f && noise_val <= -0.25f) {
				tile_name = "water-1";
				in_water = true;
			}
			else if (noise_val > -0.25f && noise_val <= -0.2f)
				tile_name = "sand-1";
			else if (noise_val > -0.2f && noise_val <= 0.5f)
				tile_name = "grass-1";
			else if (noise_val > 0.5f)
				tile_name = "dirt-1";


			// Resources
			// std::string resource_tile_name = "test_tile";
			// const auto resource_noise_val = resource_height_map.GetValue(x, y);
			// bool has_resource = false;
			// if (!in_water) {
				// has_resource = true;
				// if (resource_noise_val <= -0.7f)
					// resource_tile_name = "crude-oil";
			// }
			

			tiles[y * 32 + x].tile_prototype = data::data_manager::data_raw_get<data::Tile>(
				data::data_category::tile, tile_name);
			
			// if (has_resource) {
				// tiles[y * 32 + x].tile_resource_prototype = static_cast<data::Tile*>(
					// data::data_manager::data_raw_get(
						// data::data_category::tile, resource_tile_name)
				// );
			// }
		}
	}

	world_manager::add_chunk(new Chunk{chunk_x, chunk_y, tiles});
	--*thread_counter;
}
