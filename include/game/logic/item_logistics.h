// 
// item_logistics.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 04/07/2020
// Last modified: 04/07/2020
// 

#ifndef JACTORIO_GAME_LOGIC_ITEM_LOGISTICS_H
#define JACTORIO_GAME_LOGIC_ITEM_LOGISTICS_H
#pragma once

#include "jactorio.h"

#include "game/world/world_data.h"

namespace jactorio::game
{
	///
	/// \brief Functions for item handling within the game world
	namespace item_logistics
	{
		///
		/// \brief Can the entity at coords accept items: Chests, transport lines, etc
		/// \return Is the tile capable of accepting items of any kind
		J_NODISCARD bool can_accept_item(const World_data& world_data,
		                                 World_data::world_coord world_x, World_data::world_coord world_y);
		
	}
}

#endif // JACTORIO_GAME_LOGIC_ITEM_LOGISTICS_H
