// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/logic/placement_controller.h"

#include <functional>

#include "jactorio.h"

#include "game/world/world_data.h"
#include "proto/abstract/entity.h"
#include "proto/tile.h"

using namespace jactorio;

bool game::PlacementLocationValid(WorldData& world,
                                  const WorldCoord& coord,
                                  const uint8_t tile_width,
                                  const uint8_t tile_height) {
    for (int offset_y = 0; offset_y < tile_height; ++offset_y) {
        for (int offset_x = 0; offset_x < tile_width; ++offset_x) {
            const ChunkTile* tile = world.GetTile(coord.x + offset_x, coord.y + offset_y);

            // If the tile proto does not exist, or base tile prototype is water, NOT VALID placement

            const auto* tile_proto   = tile->GetTilePrototype();
            const auto* entity_proto = tile->GetEntityPrototype();

            if (entity_proto != nullptr || tile_proto == nullptr || tile_proto->isWater) {
                return false;
            }
        }
    }

    return true;
}

// ======================================================================
// Entity placement

///
/// \remark Assumes tiles are valid for placement
void PlaceAtCoords(game::WorldData& world,
                   const WorldCoord& coord,
                   const game::TileLayer layer,
                   const uint8_t tile_width,
                   const uint8_t tile_height,
                   const std::function<void(game::ChunkTile*)>& place_func) {


    // The top left is handled differently
    auto* top_left_tile = world.GetTile(coord);
    place_func(top_left_tile);

    auto& top_left = top_left_tile->GetLayer(layer);

    if (tile_width == 1 && tile_height == 1)
        return;

    // Multi tile

    game::MultiTileData::ValueT entity_index = 1;
    int offset_x                             = 1;

    for (int offset_y = 0; offset_y < tile_height; ++offset_y) {
        for (; offset_x < tile_width; ++offset_x) {
            auto* tile = world.GetTile(coord.x + offset_x, coord.y + offset_y);
            place_func(tile);

            auto& layer_tile = tile->GetLayer(layer);

            layer_tile.SetupMultiTile(entity_index++, top_left);
        }
        offset_x = 0;
    }
}

void RemoveAtCoords(game::WorldData& world,
                    WorldCoord coord,
                    const game::TileLayer layer,
                    const uint8_t tile_width,
                    const uint8_t tile_height,
                    void (*remove_func)(game::ChunkTile*)) {

    // Find top left corner
    const auto* tile = world.GetTile(coord);
    assert(tile != nullptr); // Attempted to remove a on a non existent tile

    tile->GetLayer(layer).AdjustToTopLeft(coord);


    // Remove
    for (int offset_y = 0; offset_y < tile_height; ++offset_y) {
        for (int offset_x = 0; offset_x < tile_width; ++offset_x) {
            remove_func(world.GetTile(coord.x + offset_x, coord.y + offset_y));
        }
    }
}

bool game::PlaceEntityAtCoords(WorldData& world,
                               const WorldCoord& coord,
                               const Orientation orien,
                               const proto::Entity* entity) {
    const ChunkTile* tile = world.GetTile(coord.x, coord.y);
    assert(tile != nullptr);

    // entity is nullptr indicates removing an entity
    if (entity == nullptr) {
        const proto::Entity* t_entity = tile->GetEntityPrototype();

        if (t_entity == nullptr) // Already removed
            return false;

        RemoveAtCoords(world,
                       coord,
                       TileLayer::entity,
                       t_entity->GetWidth(orien),
                       t_entity->GetHeight(orien),
                       [](ChunkTile* chunk_tile) { chunk_tile->GetLayer(TileLayer::entity).Clear(); });

        return true;
    }

    // Place
    if (!PlacementLocationValid(world, coord, entity->GetWidth(orien), entity->GetHeight(orien)))
        return false;

    PlaceAtCoords(
        world, coord, TileLayer::entity, entity->GetWidth(orien), entity->GetHeight(orien), [&](ChunkTile* chunk_tile) {
            chunk_tile->SetEntityPrototype(orien, entity);
        });

    return true;
}
