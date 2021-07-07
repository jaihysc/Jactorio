// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/world/world.h"

#include <algorithm>
#include <future>
#include <noise/noise.h>
#include <noise/noiseutils.h>
#include <set>

#include "proto/noise_layer.h"
#include "proto/sprite.h"

using namespace jactorio;

ChunkCoordAxis game::World::WorldCToChunkC(WorldCoordAxis coord) {
    ChunkCoordAxis chunk_coord = 0;

    if (coord < 0) {
        chunk_coord -= 1;
        coord += 1;
    }

    chunk_coord += LossyCast<ChunkCoordAxis>(LossyCast<float>(coord) / Chunk::kChunkWidth);
    return chunk_coord;
}

ChunkCoord game::World::WorldCToChunkC(const WorldCoord& coord) {
    return {WorldCToChunkC(coord.x), WorldCToChunkC(coord.y)};
}


WorldCoordAxis game::World::ChunkCToWorldC(const ChunkCoordAxis chunk_coord) {
    return chunk_coord * Chunk::kChunkWidth;
}

WorldCoord game::World::ChunkCToWorldC(const ChunkCoord& chunk_coord) {
    return {ChunkCToWorldC(chunk_coord.x), ChunkCToWorldC(chunk_coord.y)};
}


OverlayOffsetAxis game::World::WorldCToOverlayC(const WorldCoordAxis coord) {
    WorldCoordAxis val;

    if (coord < 0) {
        val = ((coord + 1) % Chunk::kChunkWidth) + Chunk::kChunkWidth - 1;
    }
    else {
        val = coord % Chunk::kChunkWidth;
    }

    return SafeCast<OverlayOffsetAxis>(val);
}

Position2<OverlayOffsetAxis> game::World::WorldCToOverlayC(const WorldCoord& coord) {
    return {WorldCToOverlayC(coord.x), WorldCToOverlayC(coord.y)};
}

// ======================================================================

game::World::World(const World& other)
    : updateDispatcher{other.updateDispatcher},
      chunkTexCoordIds_{other.chunkTexCoordIds_},
      worldChunks_{other.worldChunks_},
      logicChunks_{other.logicChunks_},
      worldGenSeed_{other.worldGenSeed_},
      worldGenChunks_{other.worldGenChunks_} {

    for (auto*& logic_chunk : logicChunks_) {
        logic_chunk = GetChunkC(logic_chunk->GetPosition());
    }
}

void game::World::DeleteChunk(const ChunkCoord& c_coord) {
    worldChunks_.erase(std::make_tuple(c_coord.x, c_coord.y));

    auto [tex_ids, readable_chunks] = GetChunkTexCoordIds(c_coord);
    if (readable_chunks > 0) {
        for (std::size_t i = 0; i < SafeCast<std::size_t>(Chunk::kChunkArea) * kTileLayerCount; ++i) {
            tex_ids[i] = 0;
        }
    }
}

void game::World::Clear() {
    worldChunks_.clear();
    logicChunks_.clear();
    worldGenChunks_.clear();
}

// ======================================================================

game::Chunk* game::World::GetChunkC(const ChunkCoord& c_coord) {
    return const_cast<Chunk*>(static_cast<const World*>(this)->GetChunkC(c_coord));
}

const game::Chunk* game::World::GetChunkC(const ChunkCoord& c_coord) const {
    const auto key = std::tuple<int, int>{c_coord.x, c_coord.y};

    if (worldChunks_.find(key) == worldChunks_.end())
        return nullptr;

    return &worldChunks_.at(key);
}


game::Chunk* game::World::GetChunkW(const WorldCoord& coord) {
    return const_cast<Chunk*>(static_cast<const World*>(this)->GetChunkW(coord));
}

const game::Chunk* game::World::GetChunkW(const WorldCoord& coord) const {
    return GetChunkC(WorldCToChunkC(coord));
}

// ======================================================================

game::ChunkTile* game::World::GetTile(const WorldCoord& coord, const TileLayer tlayer) {
    return const_cast<ChunkTile*>(static_cast<const World*>(this)->GetTile(coord, tlayer));
}
const game::ChunkTile* game::World::GetTile(const WorldCoord& coord, const TileLayer tlayer) const {
    const auto* chunk = GetChunkC(WorldCToChunkC(coord));

    if (chunk != nullptr) {
        return &chunk->GetCTile(Chunk::WorldCToChunkTileC(coord), tlayer);
    }

    return nullptr;
}

std::pair<SpriteTexCoordIndexT*, int> game::World::GetChunkTexCoordIds(const ChunkCoord& c_coord) noexcept {
    const auto pair = const_cast<const World*>(this)->GetChunkTexCoordIds(c_coord);
    return {const_cast<SpriteTexCoordIndexT*>(pair.first), pair.second};
}


std::pair<const SpriteTexCoordIndexT*, int> game::World::GetChunkTexCoordIds(const ChunkCoord& c_coord) const noexcept {
    const auto& x_dvector = chunkTexCoordIds_[c_coord.y];

    // Check y dvector in range
    if (-c_coord.y > SafeCast<ChunkCoordAxis>(chunkTexCoordIds_.size_front()) ||
        c_coord.y >= SafeCast<ChunkCoordAxis>(chunkTexCoordIds_.size_back())) {
        return {nullptr, 0};
    }

    const auto size_front = SafeCast<ChunkCoordAxis>(x_dvector.size_front());
    const auto size_back  = SafeCast<ChunkCoordAxis>(x_dvector.size_back());

    int readable_amount = 0;
    // Check x dvector in range
    if (c_coord.x < 0) {
        if (-c_coord.x <= size_front) {
            readable_amount = -c_coord.x + size_back;
        }
    }
    else if (c_coord.x >= 0) {
        if (c_coord.x < size_back) {
            readable_amount = size_back - c_coord.x;
        }
    }

    return {x_dvector[c_coord.x].data(), readable_amount};
}

SpriteTexCoordIndexT game::World::GetTexCoordId(const WorldCoord& coord, const TileLayer layer) const noexcept {
    const auto c_coord  = WorldCToChunkC(coord);
    const auto ct_coord = Chunk::WorldCToChunkTileC(coord);
    return chunkTexCoordIds_[c_coord.y][c_coord.x][(ct_coord.y * Chunk::kChunkWidth + ct_coord.x) * kTileLayerCount +
                                                   static_cast<int>(layer)];
}

void game::World::SetTexCoordId(const WorldCoord& coord,
                                const TileLayer layer,
                                const SpriteTexCoordIndexT id) noexcept {
    SetTexCoordId(WorldCToChunkC(coord), Chunk::WorldCToChunkTileC(coord), layer, id);
}

void game::World::SetTexCoordId(const ChunkCoord& c_coord,
                                const ChunkTileCoord& ct_coord,
                                TileLayer layer,
                                const SpriteTexCoordIndexT id) noexcept {
    chunkTexCoordIds_[c_coord.y][c_coord.x]
                     [(ct_coord.y * Chunk::kChunkWidth + ct_coord.x) * kTileLayerCount + static_cast<int>(layer)] = id;
}


// ======================================================================
// Placement

bool game::World::PlaceLocationValid(const WorldCoord& coord, const Dimension dimensions) const {
    for (int offset_y = 0; offset_y < dimensions.y; ++offset_y) {
        for (int offset_x = 0; offset_x < dimensions.x; ++offset_x) {

            const WorldCoord i_coord(coord.x + offset_x, coord.y + offset_y);

            // If the tile proto does not exist, or base tile prototype is water, NOT VALID placement

            const auto* tile_proto   = GetTile(i_coord, TileLayer::base)->GetPrototype<proto::Tile>();
            const auto* entity_proto = GetTile(i_coord, TileLayer::entity)->GetPrototype<proto::Entity>();

            if (entity_proto != nullptr || tile_proto == nullptr || tile_proto->isWater) {
                return false;
            }
        }
    }

    return true;
}

bool game::World::Place(const WorldCoord& coord, const Orientation orien, const proto::Entity& entity) {
    constexpr auto place_layer = TileLayer::entity;

    auto* provided_tile = GetTile(coord, place_layer);
    assert(provided_tile != nullptr);

    const auto dimension = entity.GetDimension(orien);

    if (!PlaceLocationValid(coord, dimension))
        return false;

    // The top left is handled differently
    provided_tile->SetPrototype(orien, entity);
    assert(entity.sprite != nullptr);
    SetTexCoordId(coord, place_layer, entity.sprite->texCoordId);

    if (dimension.x != 1 || dimension.y != 1) {
        // Multi tile

        int entity_index = 1;
        int offset_x     = 1;

        for (int offset_y = 0; offset_y < dimension.y; ++offset_y) {
            for (; offset_x < dimension.x; ++offset_x) {
                const auto current_coord = WorldCoord(coord.x + offset_x, coord.y + offset_y);

                auto* tile = GetTile(current_coord, place_layer);
                assert(tile != nullptr);

                tile->SetPrototype(orien, entity);
                tile->SetupMultiTile(entity_index++, *provided_tile);

                // entity_index - 1 has the same effect as adding 1 each iteration, starting with adding 1
                SetTexCoordId(current_coord, place_layer, entity.sprite->texCoordId + entity_index - 1);
            }
            offset_x = 0;
        }
    }

    return true;
}

bool game::World::Remove(const WorldCoord& coord, const Orientation orien) {
    constexpr auto remove_layer = TileLayer::entity;

    auto* provided_tile = GetTile(coord, remove_layer);
    assert(provided_tile != nullptr);

    const auto* t_entity = provided_tile->GetPrototype<proto::Entity>();

    if (t_entity == nullptr) // Already removed
        return false;

    // Remove starting from top left corner
    const auto tl_coord = coord.Incremented(*provided_tile);

    for (int offset_y = 0; offset_y < t_entity->GetHeight(orien); ++offset_y) {
        for (int offset_x = 0; offset_x < t_entity->GetWidth(orien); ++offset_x) {
            const auto current_coord = WorldCoord(tl_coord.x + offset_x, tl_coord.y + offset_y);

            auto* tile = GetTile(current_coord, remove_layer);
            assert(tile != nullptr);

            tile->Clear();
            SetTexCoordId(current_coord, remove_layer, 0);
        }
    }

    return true;
}


// ======================================================================
// Logic chunks

void game::World::LogicRegister(const LogicGroup group, const WorldCoord& coord, const TileLayer tlayer) {
    assert(group != LogicGroup::count_);
    assert(tlayer != TileLayer::count_);

    auto* chunk = GetChunkW(coord);
    assert(chunk != nullptr);

    auto* tile        = GetTile(coord, tlayer);
    auto& logic_group = chunk->GetLogicGroup(group);

    // Already added to logic group at tile
    if (std::find(logic_group.begin(), logic_group.end(), tile) != logic_group.end())
        return;

    LogicAddChunk(*chunk);
    chunk->GetLogicGroup(group).push_back(tile);
}

void game::World::LogicRemove(const LogicGroup group,
                              const WorldCoord& coord,
                              const std::function<bool(ChunkTile*)>& pred) {
    auto* chunk = GetChunkW(coord);
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

void game::World::LogicRemove(const LogicGroup group, const WorldCoord& coord, const TileLayer tlayer) {
    auto* tile = GetTile(coord, tlayer);
    LogicRemove(group, coord, [&](ChunkTile* i_tile) { return i_tile == tile; });
}

void game::World::LogicAddChunk(Chunk& chunk) {
    // Only add a chunk for logic updates once
    if (std::find(logicChunks_.begin(), logicChunks_.end(), &chunk) == logicChunks_.end()) {
        logicChunks_.emplace_back(&chunk);
    }
}

game::World::LogicChunkContainerT& game::World::LogicGetChunks() {
    return const_cast<LogicChunkContainerT&>(static_cast<const World*>(this)->LogicGetChunks());
}

const game::World::LogicChunkContainerT& game::World::LogicGetChunks() const {
    return logicChunks_;
}

// ======================================================================

// T is value stored in noise_layer at data_category
template <typename T>
void GenerateChunk(game::World& world,
                   const data::PrototypeManager& proto,
                   const ChunkCoord& chunk_coord,
                   const proto::Category data_category,
                   void (*func)(game::World& l_world,
                                game::Chunk& chunk,
                                ChunkTileCoord ct_coord,
                                const T* prototype,
                                const proto::NoiseLayer<T>& noise_layer,
                                float noise_val)) {

    // The Y axis for libnoise is inverted. It causes no issues as of right now. I am leaving this here
    // In case something happens in the future

    // Get all TILE noise layers for building terrain
    auto noise_layers = proto.GetAll<proto::NoiseLayer<T>>(data_category);

    // Sort Noise layers, the one with the highest order takes priority if tiles overlap
    std::sort(
        noise_layers.begin(), noise_layers.end(), [](auto* left, auto* right) { return left->order < right->order; });


    auto* chunk = world.GetChunkC(chunk_coord);

    // Allocate new tiles if chunk has not been generated yet
    if (chunk == nullptr) {
        chunk = &world.EmplaceChunk(chunk_coord);
    }

    int seed_offset = 0; // Incremented every time a noise layer generates to keep terrain unique
    for (const auto* noise_layer : noise_layers) {
        module::Perlin base_terrain_noise_module;
        base_terrain_noise_module.SetSeed(world.GetWorldGeneratorSeed() + seed_offset++);

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
        for (ChunkTileCoordAxis y = 0; y < game::Chunk::kChunkWidth; ++y) {
            for (ChunkTileCoordAxis x = 0; x < game::Chunk::kChunkWidth; ++x) {
                float noise_val = base_terrain_height_map.GetValue(x, y);
                auto* prototype = noise_layer->Get(noise_val);

                assert(chunk != nullptr);
                assert(noise_layer != nullptr);

                func(world, *chunk, {x, y}, prototype, *noise_layer, noise_val);
            }
        }
    }
}

/// Generates a chunk and adds it to the world when done
/// Call this with a std::thread to to this in async
void Generate(game::World& world, const data::PrototypeManager& proto, const int chunk_x, const int chunk_y) {
    // Base
    GenerateChunk<proto::Tile>(
        world,
        proto,
        {chunk_x, chunk_y},
        proto::Category::noise_layer_tile,
        [](auto& l_world,
           auto& chunk,
           auto ct_coord,
           const auto* prototype,
           const auto& /*noise_layer*/,
           float /*noise_val*/) {
            if (prototype == nullptr)
                return;

            auto& tile = chunk.GetCTile(ct_coord, game::TileLayer::base);
            tile.SetPrototype(Orientation::up, prototype);
            l_world.SetTexCoordId(chunk.GetPosition(), ct_coord, game::TileLayer::base, prototype->sprite->texCoordId);
        });

    // Resources
    GenerateChunk<proto::Entity>(
        world,
        proto,
        {chunk_x, chunk_y},
        proto::Category::noise_layer_entity,
        [](auto& l_world, auto& chunk, auto ct_coord, auto* prototype, const auto& noise_layer, float noise_val) {
            if (prototype == nullptr)
                return;

            auto& tile_base     = chunk.GetCTile(ct_coord, game::TileLayer::base);
            auto& tile_resource = chunk.GetCTile(ct_coord, game::TileLayer::resource);

            // Do not place resources on water since they cannot be mined by entities
            const auto* base_proto = tile_base.template GetPrototype<proto::Tile>();
            if (base_proto != nullptr && base_proto->isWater)
                return;

            // Already has a resource
            if (tile_resource.GetPrototype() != nullptr)
                return;


            // For resource amount, scale noise value up by richness
            const auto noise_range = noise_layer.GetValNoiseRange(noise_val);
            const auto noise_min   = noise_range.first;
            const auto noise_max   = noise_range.second;
            auto resource_amount =
                LossyCast<uint16_t>((noise_val - noise_min) * noise_layer.richness / (noise_max - noise_min));

            if (resource_amount <= 0)
                resource_amount = 1;

            // Place new resource
            tile_resource.SetPrototype(Orientation::up, prototype);
            l_world.SetTexCoordId(
                chunk.GetPosition(), ct_coord, game::TileLayer::resource, prototype->sprite->texCoordId);

            assert(resource_amount > 0);
            tile_resource.template MakeUniqueData<proto::ResourceEntityData>(resource_amount);
        });
}


void game::World::QueueChunkGeneration(const ChunkCoord& c_coord) const {
    // NO need to regenerate existing chunks
    if (GetChunkC(c_coord) == nullptr) {
        // .find is not needed to check for duplicates as insert already does that
        worldGenChunks_.insert({c_coord.x, c_coord.y});
    }
}

void game::World::GenChunk(const data::PrototypeManager& proto, uint8_t amount) {
    assert(amount > 0);

    // https://stackoverflow.com/questions/8234779/how-to-remove-from-a-map-while-iterating-it

    for (auto it = worldGenChunks_.cbegin(); it != worldGenChunks_.cend() /* not hoisted */; /* no increment */) {
        const auto& coords = *it;

        assert(worldGenChunks_.count(coords) == 1);
        Generate(*this, proto, std::get<0>(coords), std::get<1>(coords));

        worldGenChunks_.erase(it++);

        if (--amount == 0)
            break;
    }
}


void game::World::DeserializePostProcess() {
    auto iterate_world_chunks =
        [&](const std::function<void(const WorldCoord& coord, ChunkTile& tile, TileLayer tlayer)>& callback) {
            for (auto& [c_coord, chunk] : worldChunks_) {

                for (uint32_t y = 0; y < Chunk::kChunkWidth; ++y) { // x, y is position within current chunk
                    for (uint32_t x = 0; x < Chunk::kChunkWidth; ++x) {

                        auto coord = ChunkCToWorldC({std::get<0>(c_coord), std::get<1>(c_coord)});
                        coord.x += x;
                        coord.y += y;

                        for (uint8_t layer_i = 0; layer_i < kTileLayerCount; ++layer_i) {
                            const auto tlayer = static_cast<TileLayer>(layer_i);

                            auto* tile = GetTile(coord, tlayer);
                            assert(tile != nullptr);

                            callback(coord, *tile, tlayer);
                        }
                    }
                }
            }
        };

    // Resolve multi tiles
    iterate_world_chunks([this](const auto& coord, auto& tile, auto tlayer) {
        if (tile.GetMultiTileIndex() != 0) {
            auto* tl_tile = GetTile(coord.Incremented(tile), tlayer); // Now adjusted to top left
            assert(tl_tile != nullptr);

            tile.SetupMultiTile(tile.GetMultiTileIndex(), *tl_tile);
        }
    });

    // OnDeserialize
    iterate_world_chunks([this](const auto& coord, auto& tile, auto /*tlayer*/) {
        if (tile.GetPrototype() != nullptr) {
            tile.GetPrototype()->OnDeserialize(*this, coord, tile);
        }
    });
}


// ======================================================================


game::World::SerialLogicChunkContainerT game::World::ToSerializeLogicChunkContainer() const {
    SerialLogicChunkContainerT serial_logic;

    serial_logic.reserve(logicChunks_.size());

    for (const auto& logic_chunk : logicChunks_) {
        serial_logic.push_back(logic_chunk->GetPosition());
    }

    return serial_logic;
}

void game::World::FromSerializeLogicChunkContainer(const SerialLogicChunkContainerT& serial_logic) {
    assert(logicChunks_.empty());

    for (const auto& logic_chunk : serial_logic) {
        logicChunks_.push_back(GetChunkC(logic_chunk));
    }

    assert(logicChunks_.size() == serial_logic.size());
}
