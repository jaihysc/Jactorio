#include "game/logic/entity_place_controller.h"

#include "jactorio.h"
#include "game/world/world_manager.h"
#include "game/input/mouse_selection.h"

bool jactorio::game::logic::place_entity_at_coords(data::Entity* entity, const int x, const int y) {
	if (!mouse_selection::selected_tile_in_range())
		return false;
	
	return place_entity_at_coords_unranged(entity, x, y);
}

bool jactorio::game::logic::place_entity_at_coords_unranged(data::Entity* entity, const int x, const int y) {
	auto tile = world_manager::get_tile_world_coords(x, y);
	assert(tile != nullptr);

	
	// entity is nullptr indicates removing an entity
	if (entity == nullptr) {		
		// Already removed
		if (tile->entity == nullptr)
			return false;
		
		tile->entity = nullptr;
		tile->set_tile_layer_sprite_prototype(Chunk_tile::chunk_layer::entity, nullptr);
		return true;
	}

	
	// If the tile proto does not exist, or tile prototype is water, NOT VALID placement
	const auto tile_proto = tile->get_tile_layer_tile_prototype(Chunk_tile::chunk_layer::base);
	if (tile_proto == nullptr || tile_proto->is_water) {
		return false;
	}

	// Valid tile
	tile->entity = entity;
	tile->set_tile_layer_sprite_prototype(Chunk_tile::chunk_layer::entity, entity->sprite);
	return true;
}
