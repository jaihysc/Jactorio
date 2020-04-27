// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 04/07/2020

#ifndef JACTORIO_GAME_LOGIC_ITEM_LOGISTICS_H
#define JACTORIO_GAME_LOGIC_ITEM_LOGISTICS_H
#pragma once

#include "jactorio.h"

#include "game/logic/item_logistics.h"
#include "game/logic/item_insert_destination.h"
#include "game/world/world_data.h"

namespace jactorio::game
{
	///
	/// \brief Functions for item handling within the game world
	namespace item_logistics
	{
#define ITEM_INSERT_FUNCTION(name_)\
	bool name_(const jactorio::data::item_stack& item_stack,\
			   jactorio::data::Unique_data_base& unique_data,\
		       const jactorio::data::Orientation orientation)

		// Item insertion functions, returned by can_accept_item
		ITEM_INSERT_FUNCTION(insert_container_entity);
		ITEM_INSERT_FUNCTION(insert_transport_belt);

#undef ITEM_INSERT_FUNCTION

		///
		/// \brief Can the entity at coords accept items: Chests, transport lines, etc
		/// \return nullptr if tile is not capable of accepting items, otherwise a function pointer to insert items 
		J_NODISCARD Item_insert_destination::insert_func can_accept_item(const World_data& world_data,
		                                                                 World_data::world_coord world_x,
		                                                                 World_data::world_coord world_y);
	}
}

#endif // JACTORIO_GAME_LOGIC_ITEM_LOGISTICS_H
