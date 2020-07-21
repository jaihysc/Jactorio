// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/world/world_data.h"

#include <algorithm>
#include <future>
#include <mutex>
#include <set>
#include <noise/noise.h>
#include <noise/noiseutils.h>

#include "data/prototype_manager.h"
#include "data/prototype/entity/resource_entity.h"
#include "data/prototype/interface/update_listener.h"
#include "data/prototype/tile/noise_layer.h"
#include "data/prototype/tile/tile.h"
#include "game/world/chunk_tile.h"

using namespace jactorio;

game::Chunk::ChunkCoord game::WorldData::ToChunkCoord(WorldCoord world_coord) {
	Chunk::ChunkCoord chunk_coord = 0;

	if (world_coord < 0) {
		chunk_coord -= 1;
		world_coord += 1;
	}

	chunk_coord += static_cast<float>(world_coord) / Chunk::kChunkWidth;
	return chunk_coord;
}

game::Chunk* game::WorldData::AddChunk(const Chunk& chunk) {
	const auto position = chunk.GetPosition();

	auto conditional = worldChunks_.emplace(std::make_tuple(position.first, position.second), chunk);
	return &conditional.first->second;
}

void game::WorldData::DeleteChunk(Chunk::ChunkCoord chunk_x, Chunk::ChunkCoord chunk_y) {
	worldChunks_.erase(std::make_tuple(chunk_x, chunk_y));
}

void game::WorldData::ClearChunkData() {
	worldChunks_.clear();
	logicChunks_.clear();
}

// ======================================================================

game::Chunk* game::WorldData::GetChunkC(const Chunk::ChunkCoord chunk_x, const Chunk::ChunkCoord chunk_y) {
	return const_cast<Chunk*>(static_cast<const WorldData&>(*this).GetChunkC(chunk_x, chunk_y));
}

const game::Chunk* game::WorldData::GetChunkC(const Chunk::ChunkCoord chunk_x,
                                              const Chunk::ChunkCoord chunk_y) const {
	const auto key = std::tuple<int, int>{chunk_x, chunk_y};

	if (worldChunks_.find(key) == worldChunks_.end())
		return nullptr;

	return &worldChunks_.at(key);
}


game::Chunk* game::WorldData::GetChunkC(const Chunk::ChunkPair& chunk_pair) {
	return GetChunkC(chunk_pair.first, chunk_pair.second);
}

const game::Chunk* game::WorldData::GetChunkC(const Chunk::ChunkPair& chunk_pair) const {
	return GetChunkC(chunk_pair.first, chunk_pair.second);
}


game::Chunk* game::WorldData::GetChunk(const WorldCoord world_x, const WorldCoord world_y) {
	return GetChunkC(ToChunkCoord(world_x), ToChunkCoord(world_y));
}

const game::Chunk* game::WorldData::GetChunk(const WorldCoord world_x, const WorldCoord world_y) const {
	return GetChunkC(ToChunkCoord(world_x), ToChunkCoord(world_y));
}


game::Chunk* game::WorldData::GetChunk(const WorldPair& world_pair) {
	return GetChunk(world_pair.first, world_pair.second);
}

const game::Chunk* game::WorldData::GetChunk(const WorldPair& world_pair) const {
	return GetChunk(world_pair.first, world_pair.second);
}

// ======================================================================

game::ChunkTile* game::WorldData::GetTile(const WorldCoord world_x, const WorldCoord world_y) {
	return const_cast<ChunkTile*>(static_cast<const WorldData&>(*this).GetTile(world_x, world_y));
}

const game::ChunkTile* game::WorldData::GetTile(WorldCoord world_x, WorldCoord world_y) const {
	// The negative chunks start at -1, unlike positive chunks at 0
	// Thus add 1 to become 0 so the calculations can be performed
	bool negative_x = false;
	bool negative_y = false;

	float chunk_index_x = 0;
	float chunk_index_y = 0;

	if (world_x < 0) {
		negative_x = true;
		chunk_index_x -= 1;
		world_x += 1;
	}
	if (world_y < 0) {
		negative_y = true;
		chunk_index_y -= 1;
		world_y += 1;
	}

	chunk_index_x += static_cast<float>(world_x) / Chunk::kChunkWidth;
	chunk_index_y += static_cast<float>(world_y) / Chunk::kChunkWidth;


	const auto* chunk = GetChunkC(static_cast<int>(chunk_index_x), static_cast<int>(chunk_index_y));

	if (chunk != nullptr) {
		int tile_index_x = static_cast<int>(world_x) % Chunk::kChunkWidth;
		int tile_index_y = static_cast<int>(world_y) % Chunk::kChunkWidth;

		if (negative_x) {
			tile_index_x = Chunk::kChunkWidth - 1 - tile_index_x * -1;
		}
		if (negative_y) {
			tile_index_y = Chunk::kChunkWidth - 1 - tile_index_y * -1;
		}

		return &chunk->Tiles()[Chunk::kChunkWidth * tile_index_y + tile_index_x];
	}

	return nullptr;
}


game::ChunkTile* game::WorldData::GetTile(const WorldPair& world_pair) {
	return GetTile(world_pair.first, world_pair.second);
}

const game::ChunkTile* game::WorldData::GetTile(const WorldPair& world_pair) const {
	return GetTile(world_pair.first, world_pair.second);
}


// ======================================================================
// Logic chunks

void game::WorldData::LogicRegister(const Chunk::LogicGroup group, const WorldPair& world_pair,
                                    const ChunkTile::ChunkLayer layer) {
	assert(group != Chunk::LogicGroup::count_);
	assert(layer != ChunkTile::ChunkLayer::count_);

	auto* chunk = GetChunk(world_pair);
	assert(chunk);

	logicChunks_.emplace(chunk);

	auto* tile_layer = &GetTile(world_pair)->GetLayer(layer);
	chunk->GetLogicGroup(group)
	     .push_back(tile_layer);
}

void game::WorldData::LogicRemove(const Chunk::LogicGroup group, const WorldPair& world_pair,
                                  const std::function<bool(ChunkTileLayer*)>& pred) {
	auto* chunk = GetChunk(world_pair);
	assert(chunk);

	auto& logic_group = chunk->GetLogicGroup(group);

	logic_group.erase(
		std::remove_if(logic_group.begin(), logic_group.end(), pred),
		logic_group.end());

	// Remove from logic chunks if now empty
	for (auto& i_group : chunk->logicGroups) {
		if (!i_group.empty())
			return;
	}

	logicChunks_.erase(chunk);
}

void game::WorldData::LogicRemove(const Chunk::LogicGroup group, const WorldPair& world_pair,
                                  const ChunkTile::ChunkLayer layer) {
	auto* tile_layer = &GetTile(world_pair)->GetLayer(layer);

	LogicRemove(group, world_pair, [&](ChunkTileLayer* t_layer) {
		return t_layer == tile_layer;
	});
}

void game::WorldData::LogicAddChunk(Chunk* chunk) {
	assert(chunk != nullptr);
	logicChunks_.emplace(chunk);
}

std::set<game::Chunk*>& game::WorldData::LogicGetChunks() {
	return logicChunks_;
}

// ======================================================================

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


	const auto* chunk = world_data.GetChunkC(chunk_x, chunk_y);

	// Allocate new tiles if chunk has not been generated yet
	if (chunk == nullptr) {
		chunk = world_data.EmplaceChunk(chunk_x, chunk_y);
	}

	game::ChunkTile* tiles = chunk->Tiles();

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
		height_map_builder.SetDestSize(game::Chunk::kChunkWidth, game::Chunk::kChunkWidth);

		// Since x, y represents the center of the chunk, +- 0.5 to get the edges 
		height_map_builder.SetBounds(chunk_x - 0.5, chunk_x + 0.5,
		                             chunk_y - 0.5, chunk_y + 0.5);
		height_map_builder.Build();


		// Transfer noise values from height map to chunk tiles
		for (int y = 0; y < game::Chunk::kChunkWidth; ++y) {
			for (int x = 0; x < game::Chunk::kChunkWidth; ++x) {
				float noise_val = base_terrain_height_map.GetValue(x, y);
				auto* new_tile  = noise_layer->Get(noise_val);

				func(tiles[y * game::Chunk::kChunkWidth + x], new_tile, noise_val, noise_layer->richness);
			}
		}

	}
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

			target.SetTilePrototype(new_tile);
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
			const auto* base_layer = target.GetTilePrototype();
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

// ======================================================================

game::WorldData::UpdateDispatcher::ListenerEntry game::WorldData::UpdateDispatcher::Register(
	WorldCoord current_world_x, WorldCoord current_world_y,
	WorldCoord target_world_x, WorldCoord target_world_y, const data::IUpdateListener& proto_listener) {

	return Register({current_world_x, current_world_y},
	                {target_world_x, target_world_y}, proto_listener);
}

game::WorldData::UpdateDispatcher::ListenerEntry game::WorldData::UpdateDispatcher::Register(
	const WorldPair& current_coords, const WorldPair& target_coords, const data::IUpdateListener& proto_listener) {

	auto& collection = container_[target_coords];
	collection.emplace_back(std::make_pair(current_coords, &proto_listener));

	return {current_coords, target_coords};
}

bool game::WorldData::UpdateDispatcher::Unregister(const ListenerEntry& entry) {
	auto& collection = container_[entry.second];

	for (decltype(collection.size()) i = 0; i < collection.size(); ++i) {
		auto& element = collection[i];

		if (element.first == entry.first) {
			collection.erase(collection.begin() + i);
		}
	}

	return false;
}

void game::WorldData::UpdateDispatcher::Dispatch(WorldCoord world_x, WorldCoord world_y, const data::UpdateType type) {
	Dispatch({world_x, world_y}, type);
}

void game::WorldData::UpdateDispatcher::Dispatch(const WorldPair& world_pair, const data::UpdateType type) {
	auto& collection = container_[world_pair];

	for (auto& pair : collection) {
		pair.second->OnTileUpdate(worldData_, world_pair, pair.first, type);
	}
}
