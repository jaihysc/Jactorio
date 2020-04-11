// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 01/20/2020

#include "game/logic/placement_controller.h"

#include "jactorio.h"

#include "data/prototype/tile/tile.h"
#include "game/world/world_data.h"

bool jactorio::game::placement_c::placement_location_valid(World_data& world_data,
                                                           const uint8_t tile_width, const uint8_t tile_height,
                                                           const int x, const int y) {
	for (int offset_y = 0; offset_y < tile_height; ++offset_y) {
		for (int offset_x = 0; offset_x < tile_width; ++offset_x) {
			const auto tile =
				world_data.get_tile_world_coords(x + offset_x, y + offset_y);

			// If the tile proto does not exist, or base tile prototype is water, NOT VALID placement

			const auto* tile_proto = tile->get_tile_prototype(Chunk_tile::chunkLayer::base);
			const auto* entity_proto = tile->get_entity_prototype(Chunk_tile::chunkLayer::entity);

			if (entity_proto != nullptr || tile_proto == nullptr || tile_proto->is_water) {
				return false;
			}
		}
	}

	return true;
}


// ======================================================================
// Entity placement

jactorio::data::Entity* placement_entity;  // This is for the lambda of the place function
bool jactorio::game::placement_c::place_entity_at_coords(World_data& world_data, data::Entity* entity, const int x,
                                                         const int y) {
	const auto tile = world_data.get_tile_world_coords(x, y);

	assert(tile != nullptr);

	// entity is nullptr indicates removing an entity
	if (entity == nullptr) {
		const data::Entity* t_entity = tile->get_entity_prototype(Chunk_tile::chunkLayer::entity);

		if (t_entity == nullptr)  // Already removed
			return false;

		remove_at_coords(
			world_data, Chunk_tile::chunkLayer::entity, t_entity->tile_width,
			t_entity->tile_height, x, y, [](Chunk_tile* chunk_tile) {
				chunk_tile->get_layer(Chunk_tile::chunkLayer::entity).clear();
			});

		return true;
	}

	// Place
	if (!placement_location_valid(world_data, entity->tile_width, entity->tile_height, x, y))
		return false;

	placement_entity = entity;
	place_at_coords(
		world_data, Chunk_tile::chunkLayer::entity, entity->tile_width, entity->tile_height, x,
		y, [](Chunk_tile* chunk_tile) {
			chunk_tile->set_entity_prototype(Chunk_tile::chunkLayer::entity, placement_entity);
		});

	return true;
}


// Sprite placement
jactorio::data::Sprite* placement_sprite;
jactorio::game::Chunk_tile::chunkLayer placement_sprite_layer;

void jactorio::game::placement_c::place_sprite_at_coords(World_data& world_data,
                                                         const Chunk_tile::chunkLayer layer,
                                                         data::Sprite* sprite,
                                                         const uint8_t tile_width, const uint8_t tile_height,
                                                         const int x, const int y) {

	const auto tile = world_data.get_tile_world_coords(x, y);
	assert(tile != nullptr);

	placement_sprite_layer = layer;

	// nullptr indicates removing an entity
	if (sprite == nullptr) {
		remove_at_coords(
			world_data, layer, tile_width, tile_height, x,
			y, [](Chunk_tile* chunk_tile) {
				chunk_tile->get_layer(Chunk_tile::chunkLayer::overlay).clear();
			});
		return;
	}

	// Place
	assert(sprite != nullptr);
	placement_sprite = sprite;
	place_at_coords(
		world_data, layer,
		tile_width, tile_height, x, y,
		[](Chunk_tile* chunk_tile) {
			chunk_tile->set_sprite_prototype(Chunk_tile::chunkLayer::overlay, placement_sprite);
		});
}


// Lower level functions
void jactorio::game::placement_c::place_at_coords(World_data& world_data,
                                                  const Chunk_tile::chunkLayer layer,
                                                  const uint8_t tile_width, const uint8_t tile_height,
                                                  const int x, const int y,
                                                  void (*place_func)(Chunk_tile*)) {
	// Place --- The places tiles are known to be valid
	int entity_index = 0;

	// The top left is handled differently
	Chunk_tile* top_left_tile = world_data.get_tile_world_coords(x, y);
	place_func(top_left_tile);

	Chunk_tile_layer& top_left = top_left_tile->get_layer(layer);
	top_left.multi_tile_index = entity_index++;

	if (tile_width == 1 && tile_height == 1)
		return;

	// Multi tile

	top_left.init_multi_tile_prop(tile_width, tile_height);

	int offset_x = 1;
	for (int offset_y = 0; offset_y < tile_height; ++offset_y) {
		for (; offset_x < tile_width; ++offset_x) {
			Chunk_tile* tile = world_data.get_tile_world_coords(x + offset_x, y + offset_y);
			place_func(tile);

			auto& layer_tile = tile->get_layer(layer);
			layer_tile.multi_tile_index = entity_index++;

			layer_tile.set_multi_tile_parent(&top_left);
		}
		offset_x = 0;
	}
}

void jactorio::game::placement_c::remove_at_coords(World_data& world_data,
                                                   const Chunk_tile::chunkLayer layer,
                                                   const uint8_t tile_width, const uint8_t tile_height,
                                                   int x, int y,
                                                   void (*remove_func)(Chunk_tile*)) {
	// Find top left corner
	{
		const auto tile = world_data.get_tile_world_coords(x, y);
		assert(tile != nullptr);  // Attempted to remove a on a non existent tile

		const auto tile_index = tile->get_layer(layer).multi_tile_index;

		y -= static_cast<int>(tile_index / tile_width);
		x -= tile_index % tile_width;
	}

	// Remove
	for (int offset_y = 0; offset_y < tile_height; ++offset_y) {
		for (int offset_x = 0; offset_x < tile_width; ++offset_x) {
			remove_func(world_data.get_tile_world_coords(x + offset_x, y + offset_y));
		}
	}
}
