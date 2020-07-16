// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/world/world_data.h"

#include <algorithm>
#include <mutex>
#include <set>
#include <noise/noise.h>
#include <noise/noiseutils.h>

#include "core/logger.h"
#include "data/prototype_manager.h"
#include "data/prototype/noise_layer.h"
#include "data/prototype/entity/resource_entity.h"
#include "data/prototype/tile/tile.h"
#include "game/world/chunk_tile.h"

constexpr auto kChunkWidth = jactorio::game::WorldData::kChunkWidth;

using namespace jactorio;

// T is value stored in noise_layer at data_category
template <typename T>
void GenerateChunk(game::WorldData& world_data,
                   const data::PrototypeManager& data_manager,
                   const int chunk_x, const int chunk_y,
                   const data::DataCategory data_category,
                   void (*func)(game::ChunkTile&, void*, float, double)) {
	using namespace jactorio;

	// The Y axis for libnoise is inverted. It causes no issues as of right now. I am leaving this here
	// In case something happens in the future

	// Get all TILE noise layers for building terrain
	auto noise_layers = data_manager.DataRawGetAll<data::NoiseLayer<T>>(data_category);

	// Sort Noise layers, the one with the highest order takes priority if tiles overlap
	std::sort(noise_layers.begin(), noise_layers.end(),
	          [](auto* left, auto* right) {
		          return left->order < right->order;
	          }
	);


	// Allocate new tiles if chunk has not been generated yet
	const auto* chunk = world_data.GetChunkC(chunk_x, chunk_y);
	game::ChunkTile* tiles;

	if (chunk == nullptr)
		tiles = new game::ChunkTile[kChunkWidth * kChunkWidth];  // This will be deleted by the world_manager
	else
		tiles = chunk->Tiles();

	int seed_offset = 0;  // Incremented every time a noise layer generates to keep terrain unique
	for (const auto& noise_layer : noise_layers) {
		module::Perlin base_terrain_noise_module;
		base_terrain_noise_module.SetSeed(world_data.GetWorldGeneratorSeed() + seed_offset++);

		// Load properties of each noise layer
		base_terrain_noise_module.SetOctaveCount(noise_layer->octaveCount);
		base_terrain_noise_module.SetFrequency(noise_layer->frequency);
		base_terrain_noise_module.SetPersistence(noise_layer->persistence);

		utils::NoiseMap base_terrain_height_map;
		utils::NoiseMapBuilderPlane height_map_builder;
		height_map_builder.SetSourceModule(base_terrain_noise_module);
		height_map_builder.SetDestNoiseMap(base_terrain_height_map);
		height_map_builder.SetDestSize(kChunkWidth, kChunkWidth);

		// Since x, y represents the center of the chunk, +- 0.5 to get the edges 
		height_map_builder.SetBounds(chunk_x - 0.5, chunk_x + 0.5,
		                             chunk_y - 0.5, chunk_y + 0.5);
		height_map_builder.Build();


		// Transfer noise values from height map to chunk tiles
		for (int y = 0; y < kChunkWidth; ++y) {
			for (int x = 0; x < kChunkWidth; ++x) {
				float noise_val = base_terrain_height_map.GetValue(x, y);
				auto* new_tile  = noise_layer->Get(noise_val);

				func(tiles[y * kChunkWidth + x], new_tile, noise_val, noise_layer->richness);
			}
		}

	}

	// Because the renderer is async, a new chunk is only added at the very end to renderer attempting to render nullptr
	if (chunk == nullptr)
		world_data.EmplaceChunk(chunk_x, chunk_y, tiles);
}

///
/// \brief Generates a chunk and adds it to the world when done <br>
/// Call this with a std::thread to to this in async
void Generate(game::WorldData& world_data, const data::PrototypeManager& data_manager,
              const int chunk_x, const int chunk_y) {
	using namespace jactorio;

	LOG_MESSAGE_F(debug, "Generating new chunk at %d, %d...", chunk_x, chunk_y);

	// Base
	GenerateChunk<data::Tile>(
		world_data, data_manager,
		chunk_x, chunk_y,
		data::DataCategory::noise_layer_tile,
		[](game::ChunkTile& target, void* tile, float, double) {
			assert(tile != nullptr);  // Base tile should never generate nullptr
			// Add the tile prototype to the Chunk_tile
			auto* new_tile = static_cast<data::Tile*>(tile);

			target.SetTilePrototype(game::ChunkTile::ChunkLayer::base, new_tile);
		});

	// Resources
	GenerateChunk<data::ResourceEntity>(
		world_data, data_manager,
		chunk_x, chunk_y,
		data::DataCategory::noise_layer_entity,
		[](game::ChunkTile& target, void* tile, const float val, const double richness) {
			if (tile == nullptr)  // Do not override existing tiles with nullptr
				return;

			// Do not place resource on water
			const auto* base_layer = target.GetTilePrototype(game::ChunkTile::ChunkLayer::base);
			if (base_layer != nullptr && base_layer->isWater)
				return;

			// Add the tile prototype to the Chunk_tile
			auto* new_tile = static_cast<data::ResourceEntity*>(tile);

			auto& layer         = target.GetLayer(game::ChunkTile::ChunkLayer::resource);
			layer.prototypeData = new_tile;

			// For resource amount, multiply by arbitrary number to scale noise val (0 - 1) to a reasonable number
			layer.MakeUniqueData<data::ResourceEntityData>(static_cast<uint16_t>(val * 7823 * richness));
		});
}


void game::WorldData::QueueChunkGeneration(const Chunk::ChunkCoord chunk_x,
                                           const Chunk::ChunkCoord chunk_y) const {
	const auto chunk_key = std::make_pair(chunk_x, chunk_y);

	// Is the chunk already under generation? If so return
	if (worldGenChunks_.find(chunk_key) != worldGenChunks_.end())
		return;

	// Writing
	std::lock_guard<std::mutex> lk{worldGenQueueMutex_};
	worldGenChunks_.insert(std::pair{chunk_x, chunk_y});
}

void game::WorldData::GenChunk(const data::PrototypeManager& data_manager, uint8_t amount) {
	assert(amount > 0);

	// Generate a chunk
	// Find the first chunk which has yet been generated, ->second is true indicates it NEEDS generation
	for (const auto& coords : worldGenChunks_) {
		Generate(*this, data_manager, std::get<0>(coords), std::get<1>(coords));

		// Mark the chunk as done generating
		worldGenChunks_.erase(coords);

		if (--amount == 0)
			break;
	}

}
