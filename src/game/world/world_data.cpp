// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/world/world_data.h"

#include <algorithm>
#include <future>
#include <noise/noise.h>
#include <noise/noiseutils.h>
#include <set>

#include "data/prototype/noise_layer.h"

using namespace jactorio;

ChunkCoordAxis game::WorldData::WorldCToChunkC(WorldCoordAxis world_coord) {
    ChunkCoordAxis chunk_coord = 0;

    if (world_coord < 0) {
        chunk_coord -= 1;
        world_coord += 1;
    }

    chunk_coord += core::LossyCast<ChunkCoordAxis>(core::LossyCast<float>(world_coord) / Chunk::kChunkWidth);
    return chunk_coord;
}

ChunkCoord game::WorldData::WorldCToChunkC(const WorldCoord& world_coord) {
    return {WorldCToChunkC(world_coord.x), WorldCToChunkC(world_coord.y)};
}


WorldCoordAxis game::WorldData::ChunkCToWorldC(const ChunkCoordAxis chunk_coord) {
    return chunk_coord * kChunkWidth;
}

WorldCoord game::WorldData::ChunkCToWorldC(const ChunkCoord& chunk_coord) {
    return {ChunkCToWorldC(chunk_coord.x), ChunkCToWorldC(chunk_coord.y)};
}


OverlayOffsetAxis game::WorldData::WorldCToOverlayC(const WorldCoordAxis world_coord) {
    WorldCoordAxis val;

    if (world_coord < 0) {
        val = ((world_coord + 1) % kChunkWidth) + kChunkWidth - 1;
    }
    else {
        val = world_coord % kChunkWidth;
    }

    return core::SafeCast<OverlayOffsetAxis>(val);
}

// ======================================================================

game::WorldData::WorldData(const WorldData& other)
    : updateDispatcher{other.updateDispatcher},
      worldChunks_{other.worldChunks_},
      logicChunks_{other.logicChunks_},
      worldGenSeed_{other.worldGenSeed_},
      worldGenChunks_{other.worldGenChunks_} {

    for (auto*& logic_chunk : logicChunks_) {
        logic_chunk = GetChunkC(logic_chunk->GetPosition());
    }
}

void game::WorldData::DeleteChunk(ChunkCoordAxis chunk_x, ChunkCoordAxis chunk_y) {
    worldChunks_.erase(std::make_tuple(chunk_x, chunk_y));
}

void game::WorldData::ClearChunkData() {
    worldChunks_.clear();
    logicChunks_.clear();
}

// ======================================================================

game::Chunk* game::WorldData::GetChunkC(const ChunkCoordAxis chunk_x, const ChunkCoordAxis chunk_y) {
    return const_cast<Chunk*>(static_cast<const WorldData&>(*this).GetChunkC(chunk_x, chunk_y));
}

const game::Chunk* game::WorldData::GetChunkC(const ChunkCoordAxis chunk_x, const ChunkCoordAxis chunk_y) const {
    const auto key = std::tuple<int, int>{chunk_x, chunk_y};

    if (worldChunks_.find(key) == worldChunks_.end())
        return nullptr;

    return &worldChunks_.at(key);
}


game::Chunk* game::WorldData::GetChunkC(const ChunkCoord& chunk_pair) {
    return GetChunkC(chunk_pair.x, chunk_pair.y);
}

const game::Chunk* game::WorldData::GetChunkC(const ChunkCoord& chunk_pair) const {
    return GetChunkC(chunk_pair.x, chunk_pair.y);
}


game::Chunk* game::WorldData::GetChunkW(const WorldCoordAxis world_x, const WorldCoordAxis world_y) {
    return GetChunkC(WorldCToChunkC(world_x), WorldCToChunkC(world_y));
}

const game::Chunk* game::WorldData::GetChunkW(const WorldCoordAxis world_x, const WorldCoordAxis world_y) const {
    return GetChunkC(WorldCToChunkC(world_x), WorldCToChunkC(world_y));
}


game::Chunk* game::WorldData::GetChunkW(const WorldCoord& world_pair) {
    return GetChunkW(world_pair.x, world_pair.y);
}

const game::Chunk* game::WorldData::GetChunkW(const WorldCoord& world_pair) const {
    return GetChunkW(world_pair.x, world_pair.y);
}

// ======================================================================

game::ChunkTile* game::WorldData::GetTile(const WorldCoordAxis world_x, const WorldCoordAxis world_y) {
    return const_cast<ChunkTile*>(static_cast<const WorldData&>(*this).GetTile(world_x, world_y));
}

const game::ChunkTile* game::WorldData::GetTile(WorldCoordAxis world_x, WorldCoordAxis world_y) const {
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

    chunk_index_x += core::SafeCast<float>(world_x) / Chunk::kChunkWidth;
    chunk_index_y += core::SafeCast<float>(world_y) / Chunk::kChunkWidth;


    const auto* chunk = GetChunkC(core::LossyCast<int>(chunk_index_x), core::LossyCast<int>(chunk_index_y));

    if (chunk != nullptr) {
        auto tile_index_x = world_x % Chunk::kChunkWidth;
        auto tile_index_y = world_y % Chunk::kChunkWidth;

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


game::ChunkTile* game::WorldData::GetTile(const WorldCoord& world_pair) {
    return GetTile(world_pair.x, world_pair.y);
}

const game::ChunkTile* game::WorldData::GetTile(const WorldCoord& world_pair) const {
    return GetTile(world_pair.x, world_pair.y);
}


// ======================================================================


game::ChunkTile* game::WorldData::GetTileTopLeft(const WorldCoord& world_coord, const TileLayer layer) {
    auto* tile = GetTile(world_coord);
    if (tile == nullptr)
        return nullptr;

    return GetTileTopLeft(world_coord, tile->GetLayer(layer));
}

const game::ChunkTile* game::WorldData::GetTileTopLeft(const WorldCoord& world_coord, const TileLayer layer) const {
    return const_cast<WorldData*>(this)->GetTileTopLeft(world_coord, layer);
}

game::ChunkTile* game::WorldData::GetTileTopLeft(WorldCoord world_coord, const ChunkTileLayer& chunk_tile_layer) {
    chunk_tile_layer.AdjustToTopLeft(world_coord.x, world_coord.y);
    return GetTile(world_coord);
}

const game::ChunkTile* game::WorldData::GetTileTopLeft(const WorldCoord& world_coord,
                                                       const ChunkTileLayer& chunk_tile_layer) const {
    return const_cast<WorldData*>(this)->GetTileTopLeft(world_coord, chunk_tile_layer);
}


game::ChunkTileLayer* game::WorldData::GetLayerTopLeft(const WorldCoord& world_coord,
                                                       const TileLayer& tile_layer) noexcept {
    auto* tile = GetTileTopLeft(world_coord, tile_layer);
    if (tile == nullptr)
        return nullptr;

    return &tile->GetLayer(tile_layer);
}

const game::ChunkTileLayer* game::WorldData::GetLayerTopLeft(const WorldCoord& world_coord,
                                                             const TileLayer& tile_layer) const noexcept {
    return const_cast<WorldData*>(this)->GetLayerTopLeft(world_coord, tile_layer);
}


// ======================================================================
// Logic chunks

void game::WorldData::LogicRegister(const Chunk::LogicGroup group,
                                    const WorldCoord& world_pair,
                                    const TileLayer layer) {
    assert(group != Chunk::LogicGroup::count_);
    assert(layer != TileLayer::count_);

    auto* chunk = GetChunkW(world_pair);
    assert(chunk);

    auto* tile_layer  = &GetTile(world_pair)->GetLayer(layer);
    auto& logic_group = chunk->GetLogicGroup(group);

    // Already added to logic group at tile layer
    if (std::find(logic_group.begin(), logic_group.end(), tile_layer) != logic_group.end())
        return;

    LogicAddChunk(*chunk);
    chunk->GetLogicGroup(group).push_back(tile_layer);
}

void game::WorldData::LogicRemove(const Chunk::LogicGroup group,
                                  const WorldCoord& world_pair,
                                  const std::function<bool(ChunkTileLayer*)>& pred) {
    auto* chunk = GetChunkW(world_pair);
    assert(chunk);

    auto& logic_group = chunk->GetLogicGroup(group);

    logic_group.erase(std::remove_if(logic_group.begin(), logic_group.end(), pred), logic_group.end());

    // Remove from logic chunks if now empty
    for (auto& i_group : chunk->logicGroups) {
        if (!i_group.empty())
            return;
    }

    logicChunks_.erase(std::remove(logicChunks_.begin(), logicChunks_.end(), chunk), logicChunks_.end());
}

void game::WorldData::LogicRemove(const Chunk::LogicGroup group, const WorldCoord& world_pair, const TileLayer layer) {
    auto* tile_layer = &GetTile(world_pair)->GetLayer(layer);

    LogicRemove(group, world_pair, [&](ChunkTileLayer* t_layer) { return t_layer == tile_layer; });
}

void game::WorldData::LogicAddChunk(Chunk& chunk) {
    // Only add a chunk for logic updates once
    if (std::find(logicChunks_.begin(), logicChunks_.end(), &chunk) == logicChunks_.end()) {
        logicChunks_.emplace_back(&chunk);
    }
}

game::WorldData::LogicChunkContainerT& game::WorldData::LogicGetChunks() {
    return logicChunks_;
}

// ======================================================================

// T is value stored in noise_layer at data_category
template <typename T>
void GenerateChunk(game::WorldData& world_data,
                   const data::PrototypeManager& data_manager,
                   const ChunkCoord& chunk_coord,
                   const data::DataCategory data_category,
                   void (*func)(game::ChunkTile& target_tile,
                                void* prototype,
                                const data::NoiseLayer<T>& noise_layer,
                                float noise_val)) {
    using namespace jactorio;

    // The Y axis for libnoise is inverted. It causes no issues as of right now. I am leaving this here
    // In case something happens in the future

    // Get all TILE noise layers for building terrain
    auto noise_layers = data_manager.DataRawGetAll<data::NoiseLayer<T>>(data_category);

    // Sort Noise layers, the one with the highest order takes priority if tiles overlap
    std::sort(
        noise_layers.begin(), noise_layers.end(), [](auto* left, auto* right) { return left->order < right->order; });


    auto* chunk = world_data.GetChunkC(chunk_coord);

    // Allocate new tiles if chunk has not been generated yet
    if (chunk == nullptr) {
        chunk = &world_data.EmplaceChunk(chunk_coord);
    }

    int seed_offset = 0; // Incremented every time a noise layer generates to keep terrain unique
    for (const auto* noise_layer : noise_layers) {
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
        height_map_builder.SetBounds(
            chunk_coord.x - 0.5, chunk_coord.x + 0.5, chunk_coord.y - 0.5, chunk_coord.y + 0.5);
        height_map_builder.Build();


        // Transfer noise values from height map to chunk tiles
        for (int y = 0; y < game::Chunk::kChunkWidth; ++y) {
            for (int x = 0; x < game::Chunk::kChunkWidth; ++x) {
                float noise_val = base_terrain_height_map.GetValue(x, y);
                auto* new_tile  = noise_layer->Get(noise_val);

                func(chunk->GetCTile(x, y), new_tile, *noise_layer, noise_val);
            }
        }
    }
}

///
/// Generates a chunk and adds it to the world when done <br>
/// Call this with a std::thread to to this in async
void Generate(game::WorldData& world_data,
              const data::PrototypeManager& data_manager,
              const int chunk_x,
              const int chunk_y) {
    using namespace jactorio;

    LOG_MESSAGE_F(debug, "Generating new chunk at %d, %d...", chunk_x, chunk_y);

    // Base
    GenerateChunk<data::Tile>(
        world_data,
        data_manager,
        {chunk_x, chunk_y},
        data::DataCategory::noise_layer_tile,
        [](game::ChunkTile& target, void* tile, const auto& /*noise_layer*/, float /*noise_val*/) {
            assert(tile != nullptr); // Base tile should never generate nullptr

            auto* new_tile = static_cast<data::Tile*>(tile);
            target.SetTilePrototype(new_tile);
        });

    // Resources
    GenerateChunk<data::ResourceEntity>(
        world_data,
        data_manager,
        {chunk_x, chunk_y},
        data::DataCategory::noise_layer_entity,
        [](game::ChunkTile& target, void* tile, const auto& noise_layer, float noise_val) {
            if (tile == nullptr) // Do not override existing tiles
                return;

            // Do not place resources on water since they cannot be mined by entities
            const auto* base_layer = target.GetTilePrototype();
            if (base_layer != nullptr && base_layer->isWater)
                return;

            // Already has a resource
            if (target.GetLayer(game::TileLayer::resource).prototypeData != nullptr)
                return;


            // For resource amount, scale noise value up by richness
            const auto noise_range = noise_layer.GetValNoiseRange(noise_val);
            const auto noise_min   = noise_range.first;
            const auto noise_max   = noise_range.second;
            auto resource_amount =
                core::LossyCast<uint16_t>((noise_val - noise_min) * noise_layer.richness / (noise_max - noise_min));

            if (resource_amount <= 0)
                resource_amount = 1;

            // Place new tile
            auto* new_tile = static_cast<data::ResourceEntity*>(tile);

            auto& layer         = target.GetLayer(game::TileLayer::resource);
            layer.prototypeData = new_tile;

            assert(resource_amount > 0);
            layer.MakeUniqueData<data::ResourceEntityData>(resource_amount);
        });
}


void game::WorldData::QueueChunkGeneration(const ChunkCoordAxis chunk_x, const ChunkCoordAxis chunk_y) const {
    // .find is not needed to check for duplicates as insert already does that

    worldGenChunks_.insert({chunk_x, chunk_y});
}

void game::WorldData::GenChunk(const data::PrototypeManager& data_manager, uint8_t amount) {
    assert(amount > 0);

    // https://stackoverflow.com/questions/8234779/how-to-remove-from-a-map-while-iterating-it

    for (auto it = worldGenChunks_.cbegin(); it != worldGenChunks_.cend() /* not hoisted */; /* no increment */) {
        const auto& coords = *it;

        assert(worldGenChunks_.count(coords) == 1);
        Generate(*this, data_manager, std::get<0>(coords), std::get<1>(coords));

        worldGenChunks_.erase(it++);

        if (--amount == 0)
            break;
    }
}


void game::WorldData::DeserializePostProcess() {
    for (auto& [c_coord, chunk] : worldChunks_) {

        for (uint32_t y = 0; y < kChunkWidth; ++y) { // x, y is position within current chunk
            for (uint32_t x = 0; x < kChunkWidth; ++x) {

                auto world_coord = ChunkCToWorldC({std::get<0>(c_coord), std::get<1>(c_coord)});
                world_coord.x += x;
                world_coord.y += y;

                auto* tile = GetTile(world_coord);
                assert(tile != nullptr);

                for (uint8_t layer_i = 0; layer_i < ChunkTile::kTileLayerCount; ++layer_i) {
                    auto& layer = tile->layers[layer_i];

                    if (layer.GetMultiTileIndex() != 0) {
                        layer.AdjustToTopLeft(world_coord.x, world_coord.y);
                        auto* tl_tile = GetTile(world_coord); // Now adjusted to top left
                        assert(tl_tile != nullptr);

                        layer.SetTopLeftLayer(tl_tile->GetLayer(layer_i));
                    }

                    if (layer.prototypeData != nullptr) {
                        layer.prototypeData->OnDeserialize(*this, world_coord, layer);
                    }
                }
            }
        }
    }
}


// ======================================================================


game::WorldData::SerialLogicChunkContainerT game::WorldData::ToSerializeLogicChunkContainer() const {
    SerialLogicChunkContainerT serial_logic;

    serial_logic.reserve(logicChunks_.size());

    for (const auto& logic_chunk : logicChunks_) {
        serial_logic.push_back(logic_chunk->GetPosition());
    }

    return serial_logic;
}

void game::WorldData::FromSerializeLogicChunkContainer(const SerialLogicChunkContainerT& serial_logic) {
    assert(logicChunks_.empty());

    for (const auto& logic_chunk : serial_logic) {
        logicChunks_.push_back(GetChunkC(logic_chunk));
    }

    assert(logicChunks_.size() == serial_logic.size());
}
