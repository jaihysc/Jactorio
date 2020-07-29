// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/logic/placement_controller.h"

#include <functional>

#include "jactorio.h"

#include "data/prototype/tile/tile.h"
#include "game/world/world_data.h"

using namespace jactorio;

bool game::PlacementLocationValid(WorldData& world_data,
                                  const uint8_t tile_width,
                                  const uint8_t tile_height,
                                  const WorldData::WorldCoord x,
                                  const WorldData::WorldCoord y) {
	for (int offset_y = 0; offset_y < tile_height; ++offset_y) {
		for (int offset_x = 0; offset_x < tile_width; ++offset_x) {
			const ChunkTile* tile =
				world_data.GetTile(x + offset_x, y + offset_y);

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

void PlaceAtCoords(game::WorldData& world_data,
                   const game::ChunkTile::ChunkLayer layer,
                   const uint8_t tile_width,
                   const uint8_t tile_height,
                   const game::WorldData::WorldCoord x,
                   const game::WorldData::WorldCoord y,
                   const std::function<void(game::ChunkTile*)>& place_func) {
	// Place --- The places tiles are known to be valid
	int entity_index = 0;

	// The top left is handled differently
	auto* top_left_tile = world_data.GetTile(x, y);
	place_func(top_left_tile);

	auto& top_left          = top_left_tile->GetLayer(layer);
	top_left.multiTileIndex = entity_index++;

	if (tile_width == 1 && tile_height == 1)
		return;

	// Multi tile

	top_left.InitMultiTileProp(tile_width, tile_height);

	int offset_x = 1;
	for (int offset_y = 0; offset_y < tile_height; ++offset_y) {
		for (; offset_x < tile_width; ++offset_x) {
			auto* tile = world_data.GetTile(x + offset_x, y + offset_y);
			place_func(tile);

			auto& layer_tile          = tile->GetLayer(layer);
			layer_tile.multiTileIndex = entity_index++;

			layer_tile.SetMultiTileParent(&top_left);
		}
		offset_x = 0;
	}
}

void RemoveAtCoords(game::WorldData& world_data,
                    const game::ChunkTile::ChunkLayer layer,
                    const uint8_t tile_width,
                    const uint8_t tile_height,
                    game::WorldData::WorldCoord x,
                    game::WorldData::WorldCoord y,
                    void (*remove_func)(game::ChunkTile*)) {
	// Find top left corner
	{
		const auto* tile = world_data.GetTile(x, y);
		assert(tile != nullptr);  // Attempted to remove a on a non existent tile

		const auto tile_index = tile->GetLayer(layer).multiTileIndex;

		y -= static_cast<int>(tile_index / tile_width);
		x -= tile_index % tile_width;
	}

	// Remove
	for (int offset_y = 0; offset_y < tile_height; ++offset_y) {
		for (int offset_x = 0; offset_x < tile_width; ++offset_x) {
			remove_func(world_data.GetTile(x + offset_x, y + offset_y));
		}
	}
}

bool game::PlaceEntityAtCoords(WorldData& world_data,
                               const data::Entity* entity,
                               const WorldData::WorldCoord x,
                               const WorldData::WorldCoord y) {
	const ChunkTile* tile = world_data.GetTile(x, y);
	assert(tile != nullptr);

	// entity is nullptr indicates removing an entity
	if (entity == nullptr) {
		const data::Entity* t_entity = tile->GetEntityPrototype();

		if (t_entity == nullptr)  // Already removed
			return false;

		RemoveAtCoords(
			world_data,
			ChunkTile::ChunkLayer::entity,
			t_entity->tileWidth,
			t_entity->tileHeight,
			x, y,
			[](ChunkTile* chunk_tile) {
				chunk_tile->GetLayer(ChunkTile::ChunkLayer::entity).Clear();
			});

		return true;
	}

	// Place
	if (!PlacementLocationValid(world_data, entity->tileWidth, entity->tileHeight, x, y))
		return false;

	PlaceAtCoords(
		world_data,
		ChunkTile::ChunkLayer::entity,
		entity->tileWidth, entity->tileHeight,
		x, y,
		[&](ChunkTile* chunk_tile) {
			chunk_tile->SetEntityPrototype(entity);
		});

	return true;
}

bool game::PlaceEntityAtCoords(WorldData& world_data, const data::Entity* entity, const WorldData::WorldPair& world_pair) {
	return PlaceEntityAtCoords(world_data, entity, world_pair.first, world_pair.second);
}
