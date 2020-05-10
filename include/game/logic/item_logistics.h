// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 04/07/2020

#ifndef JACTORIO_GAME_LOGIC_ITEM_LOGISTICS_H
#define JACTORIO_GAME_LOGIC_ITEM_LOGISTICS_H
#pragma once

#include "jactorio.h"

#include "game/logic/item_insert_destination.h"
#include "game/logic/item_logistics.h"
#include "game/world/world_data.h"

namespace jactorio::game
{
	///
	/// \brief Functions for item handling within the game world
	namespace item_logistics
	{
#define ITEM_INSERT_FUNCTION(name_)\
	bool name_(const jactorio::data::ItemStack& item_stack,\
			   jactorio::data::UniqueDataBase& unique_data,\
		       const jactorio::data::Orientation orientation)

		// Item insertion functions, returned by can_accept_item
		ITEM_INSERT_FUNCTION(InsertContainerEntity);
		ITEM_INSERT_FUNCTION(InsertTransportBelt);

#undef ITEM_INSERT_FUNCTION

		///
		/// \brief Can the entity at coords accept items: Chests, transport lines, etc
		/// \return nullptr if tile is not capable of accepting items, otherwise a function pointer to insert items 
		J_NODISCARD ItemInsertDestination::InsertFunc CanAcceptItem(const WorldData& world_data,
		                                                            WorldData::WorldCoord world_x,
		                                                            WorldData::WorldCoord world_y);
	}
}

#endif // JACTORIO_GAME_LOGIC_ITEM_LOGISTICS_H
