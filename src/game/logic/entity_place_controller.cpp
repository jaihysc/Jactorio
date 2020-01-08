#include "game/logic/entity_place_controller.h"

#include "jactorio.h"
#include "game/world/world_manager.h"
#include "game/input/mouse_selection.h"

bool jactorio::game::logic::place_entity_at_coords(data::Entity* entity, const int x, const int y) {
	if (!mouse_selection::selected_tile_in_range())
		return false;
	
	return place_entity_at_coords_unranged(entity, x, y);
}

bool jactorio::game::logic::place_entity_at_coords_unranged(data::Entity* entity, int x, int y) {
	auto tile = world_manager::get_tile_world_coords(x, y);
	assert(tile != nullptr);

	
	// entity is nullptr indicates removing an entity
	if (entity == nullptr) {		
		// Already removed
		data::Entity* t_entity = tile->entity;
		if (t_entity == nullptr)
			return false;

		// No need to check if the entity is valid, since if an entity was placed here, it was already checked to be valid
		
		// Move to the top left corner of the tile
		y -= static_cast<int>(tile->entity_index / t_entity->tile_width);
		x -= tile->entity_index % t_entity->tile_width;

		for (int offset_y = 0; offset_y < t_entity->tile_height; ++offset_y) {
			for (int offset_x = 0; offset_x < t_entity->tile_width; ++offset_x) {
				auto tile = world_manager::get_tile_world_coords(x + offset_x, y + offset_y);

				tile->entity = nullptr;
				tile->set_tile_layer_sprite_prototype(Chunk_tile::chunk_layer::entity, nullptr);
			}
		}
		
		return true;
	}

	// ===============================
	// Place

	// Validate placement location
	for (int offset_y = 0; offset_y < entity->tile_height; ++offset_y) {
		for (int offset_x = 0; offset_x < entity->tile_width; ++offset_x) {
			auto tile = world_manager::get_tile_world_coords(x + offset_x, y + offset_y);
			
			// If the tile proto does not exist, or tile prototype is water, NOT VALID placement
			const auto tile_proto = tile->get_tile_layer_tile_prototype(Chunk_tile::chunk_layer::base);
			if (tile->entity != nullptr || tile_proto == nullptr || tile_proto->is_water) {
				return false;
			}
		}
	}

	// Place --- The places tiles are known to be valid
	int entity_index = 0;
	for (int offset_y = 0; offset_y < entity->tile_height; ++offset_y) {
		for (int offset_x = 0; offset_x < entity->tile_width; ++offset_x) {
			auto tile = world_manager::get_tile_world_coords(x + offset_x, y + offset_y);

			tile->entity = entity;
			tile->set_tile_layer_sprite_prototype(Chunk_tile::chunk_layer::entity, entity->sprite);
			tile->entity_index = entity_index++;
		}
	}
	
	return true;
}
