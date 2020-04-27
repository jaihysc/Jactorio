// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 04/07/2020

#include "game/logic/item_logistics.h"

#include "data/prototype/entity/container_entity.h"
#include "data/prototype/entity/entity.h"
#include "data/prototype/entity/transport/transport_line.h"
#include "data/prototype/item/item.h"
#include "game/logic/inventory_controller.h"

// ======================================================================

#define ITEM_INSERT_FUNCTION(name_)\
	bool jactorio::game::item_logistics::name_(const jactorio::data::item_stack& item_stack,\
											   jactorio::data::Unique_data_base& unique_data,\
											   const jactorio::data::Orientation orientation)

ITEM_INSERT_FUNCTION(insert_container_entity) {
	auto& container_data = static_cast<data::Container_entity_data&>(unique_data);
	if (!can_add_stack(container_data.inventory, container_data.size, item_stack))
		return false;

	add_stack(container_data.inventory, container_data.size, item_stack);
	return true;
}

ITEM_INSERT_FUNCTION(insert_transport_belt) {
	assert(item_stack.second == 1);  // Can only insert 1 at a time

	auto& line_data = static_cast<data::Transport_line_data&>(unique_data);

	bool use_line_left = false;
	// Decide whether to add item to left side or right side
	switch (line_data.line_segment.direction) {
	case data::Orientation::up:
		switch (orientation) {
		case data::Orientation::up:
			break;
		case data::Orientation::right:
			use_line_left = true;
			break;
		case data::Orientation::down:
			return false;
		case data::Orientation::left:
			break;

		default:
			assert(false);
			break;
		}
		break;

	case data::Orientation::right:
		switch (orientation) {
		case data::Orientation::up:
			break;
		case data::Orientation::right:
			break;
		case data::Orientation::down:
			use_line_left = true;
			break;
		case data::Orientation::left:
			return false;

		default:
			assert(false);
			break;
		}
		break;

	case data::Orientation::down:
		switch (orientation) {
		case data::Orientation::up:
			return false;
		case data::Orientation::right:
			break;
		case data::Orientation::down:
			break;
		case data::Orientation::left:
			use_line_left = true;
			break;

		default:
			assert(false);
			break;
		}
		break;

	case data::Orientation::left:
		switch (orientation) {
		case data::Orientation::up:
			use_line_left = true;
			break;
		case data::Orientation::right:
			return false;
		case data::Orientation::down:
			break;
		case data::Orientation::left:
			break;

		default:
			assert(false);
			break;
		}
		break;

	default:
		assert(false);
		break;
	}

	// TODO offset needs to be calculated correctly if segment is not 1 long
	constexpr double insertion_offset = 0.5;
	return line_data.line_segment.try_insert_item(use_line_left, insertion_offset, item_stack.first);
}

#undef ITEM_INSERT_FUNCTION

jactorio::game::Item_insert_destination::insert_func jactorio::game::item_logistics::can_accept_item(
	const World_data& world_data,
	const World_data::world_coord world_x, const World_data::world_coord world_y) {

	const data::Entity* entity =
		world_data.get_tile(world_x, world_y)
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
