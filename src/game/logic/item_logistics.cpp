// 
// item_logistics.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 04/07/2020
// Last modified: 04/07/2020
// 

#include "data/prototype/entity/entity.h"
#include "game/logic/item_logistics.h"

bool jactorio::game::item_logistics::can_accept_item(const World_data& world_data,
                                                     const World_data::world_coord world_x,
                                                     const World_data::world_coord world_y) {
	const data::Entity* entity =
		world_data.get_tile_world_coords(world_x, world_y)->get_entity_prototype(Chunk_tile::chunkLayer::entity);

	if (!entity)
		return false;

	switch (entity->category()) {
	case data::dataCategory::container_entity:
	case data::dataCategory::transport_belt:
		return true;

	default:
		return false;
	}
}
