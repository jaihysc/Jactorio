// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 04/07/2020

#include "game/logic/item_logistics.h"

#include "data/prototype/entity/container_entity.h"
#include "data/prototype/entity/entity.h"
#include "data/prototype/item/item.h"
#include "game/logic/inventory_controller.h"

// ======================================================================

#define ITEM_INSERT_FUNCTION(name_)\
	bool jactorio::game::item_logistics::name_(const jactorio::data::item_stack& item_stack,\
											   jactorio::data::Unique_data_base& unique_data,\
											   const jactorio::data::placementOrientation orientation)

ITEM_INSERT_FUNCTION(insert_container_entity) {
	auto& container_data = static_cast<data::Container_entity_data&>(unique_data);
	if (!inventory_c::can_add_stack(container_data.inventory, container_data.size, item_stack))
		return false;

	inventory_c::add_stack(container_data.inventory, container_data.size, item_stack);
	return true;
}

ITEM_INSERT_FUNCTION(insert_transport_belt) {
	return true;
}

#undef ITEM_INSERT_FUNCTION

jactorio::game::Item_insert_destination::insert_func jactorio::game::item_logistics::can_accept_item(
	const World_data& world_data,
	const World_data::world_coord world_x, const World_data::world_coord world_y) {

	const data::Entity* entity =
		world_data.get_tile_world_coords(world_x, world_y)
		          ->get_entity_prototype(Chunk_tile::chunkLayer::entity);

	if (!entity)
		return nullptr;

	switch (entity->category()) {
	case data::dataCategory::container_entity:
		return &insert_container_entity;
	case data::dataCategory::transport_belt:
		return &insert_transport_belt;

	default:
		return nullptr;
	}
}

// ======================================================================
