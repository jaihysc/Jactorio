// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 01/20/2020

#include "game/logic/placement_controller.h"

#include "jactorio.h"

#include "data/prototype/tile/tile.h"
#include "game/world/world_data.h"

bool jactorio::game::PlacementLocationValid(WorldData& world_data,
                                            const uint8_t tile_width,
                                            const uint8_t tile_height,
                                            const int x,
                                            const int y) {
	for (int offset_y = 0; offset_y < tile_height; ++offset_y) {
		for (int offset_x = 0; offset_x < tile_width; ++offset_x) {
			const ChunkTile* tile =
				world_data.GetTile(x + offset_x, y + offset_y);

			// If the tile proto does not exist, or base tile prototype is water, NOT VALID placement

			const auto* tile_proto   = tile->GetTilePrototype(ChunkTile::ChunkLayer::base);
			const auto* entity_proto = tile->GetEntityPrototype(ChunkTile::ChunkLayer::entity);

			if (entity_proto != nullptr || tile_proto == nullptr || tile_proto->isWater) {
				return false;
			}
		}
	}

	return true;
}


// ======================================================================
// Entity placement

jactorio::data::Entity* placement_entity;  // This is for the lambda of the place function
bool jactorio::game::PlaceEntityAtCoords(WorldData& world_data,
                                         data::Entity* entity,
                                         const int x,
                                         const int y) {
	const ChunkTile* tile = world_data.GetTile(x, y);
	assert(tile != nullptr);

	// entity is nullptr indicates removing an entity
	if (entity == nullptr) {
		const data::Entity* t_entity = tile->GetEntityPrototype(ChunkTile::ChunkLayer::entity);

		if (t_entity == nullptr)  // Already removed
			return false;

		RemoveAtCoords(
			world_data,
			ChunkTile::ChunkLayer::entity,
			t_entity->tileWidth,
			t_entity->tileHeight,
			x,
			y,
			[](ChunkTile* chunk_tile) {
				chunk_tile->GetLayer(ChunkTile::ChunkLayer::entity).Clear();
			});

		return true;
	}

	// Place
	if (!PlacementLocationValid(world_data, entity->tileWidth, entity->tileHeight, x, y))
		return false;

	placement_entity = entity;
	PlaceAtCoords(
		world_data,
		ChunkTile::ChunkLayer::entity,
		entity->tileWidth,
		entity->tileHeight,
		x,
		y,
		[](ChunkTile* chunk_tile) {
			chunk_tile->SetEntityPrototype(ChunkTile::ChunkLayer::entity, placement_entity);
		});

	return true;
}


// Sprite placement
jactorio::data::Sprite* placement_sprite;
jactorio::game::ChunkTile::ChunkLayer placement_sprite_layer;

void jactorio::game::PlaceSpriteAtCoords(WorldData& world_data,
                                         const ChunkTile::ChunkLayer layer,
                                         data::Sprite* sprite,
                                         const uint8_t tile_width,
                                         const uint8_t tile_height,
                                         const int x,
                                         const int y) {

	const ChunkTile* tile = world_data.GetTile(x, y);
	assert(tile != nullptr);

	placement_sprite_layer = layer;

	// nullptr indicates removing an entity
	if (sprite == nullptr) {
		RemoveAtCoords(
			world_data,
			layer,
			tile_width,
			tile_height,
			x,
			y,
			[](ChunkTile* chunk_tile) {
				chunk_tile->GetLayer(ChunkTile::ChunkLayer::overlay).Clear();
			});
		return;
	}

	// Place
	assert(sprite != nullptr);
	placement_sprite = sprite;
	PlaceAtCoords(
		world_data,
		layer,
		tile_width,
		tile_height,
		x,
		y,
		[](ChunkTile* chunk_tile) {
			chunk_tile->SetSpritePrototype(ChunkTile::ChunkLayer::overlay, placement_sprite);
		});
}


// Lower level functions
void jactorio::game::PlaceAtCoords(WorldData& world_data,
                                   const ChunkTile::ChunkLayer layer,
                                   const uint8_t tile_width,
                                   const uint8_t tile_height,
                                   const int x,
                                   const int y,
                                   void (*place_func)(ChunkTile*)) {
	// Place --- The places tiles are known to be valid
	int entity_index = 0;

	// The top left is handled differently
	ChunkTile* top_left_tile = world_data.GetTile(x, y);
	place_func(top_left_tile);

	ChunkTileLayer& top_left = top_left_tile->GetLayer(layer);
	top_left.multiTileIndex  = entity_index++;

	if (tile_width == 1 && tile_height == 1)
		return;

	// Multi tile

	top_left.InitMultiTileProp(tile_width, tile_height);

	int offset_x = 1;
	for (int offset_y = 0; offset_y < tile_height; ++offset_y) {
		for (; offset_x < tile_width; ++offset_x) {
			ChunkTile* tile = world_data.GetTile(x + offset_x, y + offset_y);
			place_func(tile);

			auto& layer_tile          = tile->GetLayer(layer);
			layer_tile.multiTileIndex = entity_index++;

			layer_tile.SetMultiTileParent(&top_left);
		}
		offset_x = 0;
	}
}

void jactorio::game::RemoveAtCoords(WorldData& world_data,
                                    const ChunkTile::ChunkLayer layer,
                                    const uint8_t tile_width,
                                    const uint8_t tile_height,
                                    int x,
                                    int y,
                                    void (*remove_func)(ChunkTile*)) {
	// Find top left corner
	{
		const ChunkTile* tile = world_data.GetTile(x, y);
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
