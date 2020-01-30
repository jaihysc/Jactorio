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
#include "data/prototype/entity/resource_entity.h"

// Prevents constant erasing of buffers
int world_gen_seed = 1001;

// Stores whether or not a chunk is being generated, this gets cleared once all world generation is done
std::set<std::tuple<int, int>> world_gen_chunks;

// T is value stored in noise_layer at data_category
template <typename T>
void generate_chunk(const int chunk_x, const int chunk_y,
                    const jactorio::data::data_category data_category,
                    void (*func)(jactorio::game::Chunk_tile&, void*, float)) {
	using namespace jactorio;

	// The Y axis for libnoise is inverted. It causes no issues as of right now. I am leaving this here
	// In case something happens in the future

	// Get all TILE noise layers for building terrain
	std::vector<data::Noise_layer<T>*> noise_layers =
		data::data_manager::data_raw_get_all<data::Noise_layer<T>>(data_category);

	// Sort Noise layers, the one with the highest order takes priority if tiles overlap
	std::sort(noise_layers.begin(), noise_layers.end(),
	          [](auto* left, auto* right) {
		          return left->order < right->order;
	          }
	);


	// Allocate new tiles if chunk has not been generated yet
	const auto chunk = game::world_manager::get_chunk(chunk_x, chunk_y);
	game::Chunk_tile* tiles;

	if (chunk == nullptr)
		tiles = new game::Chunk_tile[1024];  // This will be deleted by the world_manager
	else
		tiles = chunk->tiles_ptr();

	int seed_offset = 0;  // Incremented every time a noise layer generates to keep terrain unique
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
				float noise_val = base_terrain_height_map.GetValue(x, y);
				auto* new_tile = noise_layer->get(noise_val);

				func(tiles[y * 32 + x], new_tile, noise_val);
			}
		}

	}

	// Because the renderer is async, a new chunk is only added at the very end to renderer attempting to render nullptr
	if (chunk == nullptr)
		game::world_manager::add_chunk(new game::Chunk{chunk_x, chunk_y, tiles});
}

/**
 * Generates a chunk and adds it to the world when done <br>
 * Call this with a std::thread to to this in async
 * @param chunk_x X position of chunk to generate
 * @param chunk_y Y position of chunk to generate
 */
void generate(const int chunk_x, const int chunk_y) {
	using namespace jactorio;

	LOG_MESSAGE_f(debug, "Generating new chunk at %d, %d...", chunk_x, chunk_y);

	// Base
	generate_chunk<data::Tile>(
		chunk_x, chunk_y, data::data_category::noise_layer_tile, [](game::Chunk_tile& target, void* tile, float) {
			assert(tile != nullptr);  // Base tile should never generate nullptr
			// Add the tile prototype to the Chunk_tile
			auto* new_tile = static_cast<data::Tile*>(tile);

			target.set_layer_tile_prototype(game::Chunk_tile::chunk_layer::base, new_tile);
		});

	// Resources
	generate_chunk<data::Resource_entity>(
		chunk_x, chunk_y, data::data_category::noise_layer_entity, [](game::Chunk_tile& target, void* tile, float val) {
			if (tile == nullptr)  // Do not override existing tiles with nullptr
				return;
		
			// Add the tile prototype to the Chunk_tile
			auto* new_tile = static_cast<data::Resource_entity*>(tile);

			auto& layer = target.get_layer(game::Chunk_tile::chunk_layer::resource);
			layer.set_data(new_tile);

			// For resource amount, multiply by arbitrary number to scale noise val (0 - 1) to a reasonable number
			layer.unique_data = new data::Resource_entity_data(val * 7823);
		});
}

void jactorio::game::world_generator::set_world_generator_seed(const int seed) {
	world_gen_seed = seed;
}

int jactorio::game::world_generator::get_world_generator_seed() {
	return world_gen_seed;
}

std::mutex m;

void jactorio::game::world_generator::queue_chunk_generation(const int chunk_x, const int chunk_y) {
	const auto chunk_key = std::tuple<int, int>{+ chunk_x, chunk_y};

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
