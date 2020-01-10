#include "game/logic/entity_place_controller.h"

#include "jactorio.h"
#include "game/world/world_manager.h"
#include "game/input/mouse_selection.h"

bool jactorio::game::logic::placement_location_valid(const uint8_t tile_width, const uint8_t tile_height,
                                                     const int x, const int y) {
	for (int offset_y = 0; offset_y < tile_height; ++offset_y) {
		for (int offset_x = 0; offset_x < tile_width; ++offset_x) {
			const auto tile =
				world_manager::get_tile_world_coords(x + offset_x, y + offset_y);

			// If the tile proto does not exist, or base tile prototype is water, NOT VALID placement
			const auto tile_proto = tile->get_tile_layer_tile_prototype(Chunk_tile::chunk_layer::base);
			if (tile->entity != nullptr || tile_proto == nullptr || tile_proto->is_water) {
				return false;
			}
		}
	}

	return true;
}


// Entity placement

bool jactorio::game::logic::place_entity_at_coords_ranged(data::Entity* entity, const int x, const int y) {
	if (!mouse_selection::selected_tile_in_range())
		return false;

	return place_entity_at_coords(entity, x, y);
}

jactorio::data::Entity* placement_entity;  // This is for the lambda of the place function
bool jactorio::game::logic::place_entity_at_coords(data::Entity* entity, const int x, const int y) {
	const auto tile = world_manager::get_tile_world_coords(x, y);
	assert(tile != nullptr);

	// entity is nullptr indicates removing an entity
	if (entity == nullptr) {
		data::Entity* t_entity = tile->entity;
		if (t_entity == nullptr)  // Already removed
			return false;

		remove_at_coords(
			Chunk_tile::chunk_layer::entity, t_entity->tile_width, t_entity->tile_height,
			x, y, [](Chunk_tile* chunk_tile) {
				chunk_tile->entity = nullptr;
				chunk_tile->set_tile_layer_sprite_prototype(Chunk_tile::chunk_layer::entity, nullptr);
			});

		return true;
	}

	// Place
	if (!placement_location_valid(entity->tile_width, entity->tile_height, x, y))
		return false;

	placement_entity = entity;
	place_at_coords(
		Chunk_tile::chunk_layer::entity, entity->tile_width, entity->tile_height, x, y,
		[](Chunk_tile* chunk_tile) {
			chunk_tile->entity = placement_entity;
			chunk_tile->set_tile_layer_sprite_prototype(Chunk_tile::chunk_layer::entity, placement_entity->sprite);
		});

	return true;
}


// Sprite placement
jactorio::data::Sprite* placement_sprite;
jactorio::game::Chunk_tile::chunk_layer placement_sprite_layer;

void jactorio::game::logic::place_sprite_at_coords(const Chunk_tile::chunk_layer layer,
                                                   data::Sprite* sprite,
                                                   const uint8_t tile_width, const uint8_t tile_height,
                                                   const int x, const int y) {

	const auto tile = world_manager::get_tile_world_coords(x, y);
	assert(tile != nullptr);

	placement_sprite_layer = layer;

	// nullptr indicates removing an entity
	if (sprite == nullptr) {
		remove_at_coords(
			layer, tile_width, tile_height, x, y,
			[](Chunk_tile* chunk_tile) {
				chunk_tile->set_tile_layer_sprite_prototype(placement_sprite_layer, nullptr);
			});
	}

	// Place
	placement_sprite = sprite;
	place_at_coords(
		layer, tile_width, tile_height, x, y,
		[](Chunk_tile* chunk_tile) {
			chunk_tile->set_tile_layer_sprite_prototype(placement_sprite_layer, placement_sprite);
		});
}


// Lower level functions
void jactorio::game::logic::place_at_coords(const Chunk_tile::chunk_layer layer,
                                            const uint8_t tile_width, const uint8_t tile_height,
                                            const int x, const int y,
                                            void (*place_func)(Chunk_tile*)) {
	// Place --- The places tiles are known to be valid
	int entity_index = 0;
	for (int offset_y = 0; offset_y < tile_height; ++offset_y) {
		for (int offset_x = 0; offset_x < tile_width; ++offset_x) {
			auto tile = world_manager::get_tile_world_coords(x + offset_x, y + offset_y);
			place_func(tile);

			auto& layer_tile = tile->get_layer(layer);
			layer_tile.multi_tile_index = entity_index++;

			layer_tile.multi_tile_span = tile_width;
			layer_tile.multi_tile_height = tile_height;

		}
	}
}

void jactorio::game::logic::remove_at_coords(const Chunk_tile::chunk_layer layer,
                                             const uint8_t tile_width, const uint8_t tile_height,
                                             int x, int y,
                                             void (*remove_func)(Chunk_tile*)) {
	// If the current sprite is already nullptr, it means the rest of the multi-tile sprite is already removed
	// if (tile->get_tile_layer_sprite_prototype(layer) == nullptr)
	// return false;

	// Find top left corner
	{
		auto tile = world_manager::get_tile_world_coords(x, y);
		assert(tile != nullptr);  // Attempted to remove a on a non existent tile

		const auto tile_index = tile->get_layer(layer).multi_tile_index;

		y -= static_cast<int>(tile_index / tile_width);
		x -= tile_index % tile_width;
	}

	// Remove
	for (int offset_y = 0; offset_y < tile_height; ++offset_y) {
		for (int offset_x = 0; offset_x < tile_width; ++offset_x) {
			remove_func(world_manager::get_tile_world_coords(x + offset_x, y + offset_y));
		}
	}
}
