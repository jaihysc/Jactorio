// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 04/08/2020

#ifndef JACTORIO_GAME_LOGIC_ITEM_INSERT_DESTINATION_H
#define JACTORIO_GAME_LOGIC_ITEM_INSERT_DESTINATION_H
#pragma once

#include "data/prototype/interface/rotatable.h"
#include "data/prototype/item/item.h"
#include "game/world/world_data.h"

namespace jactorio::game
{
	///
	/// \brief Represents a world location where items can be inserted
	class Item_insert_destination
	{
	public:
		/// \brief A function for inserting items into a destination
		/// \param item Item to insert
		/// \param orientation Orientation from origin object 
		///  \return True is successfully inserted
		using insert_func = bool (*)(const data::item_stack& item_stack,
		                             data::Unique_data_base& unique_data,
		                             data::placementOrientation orientation);


		///
		/// \param unique_data Unique data at tile
		/// \param insert_function Obtained from item_logistics::can_accept_item()
		/// \param orientation Orientation from origin prototype, "the destination is <orientation> of prototype"
		Item_insert_destination(data::Unique_data_base& unique_data, const insert_func insert_function,
		                        const data::placementOrientation orientation)
			: destination_unique_data_(unique_data),
			  orientation_(orientation),
			  insertion_function_(insert_function) {
			assert(insert_function != nullptr);
		}


	private:
		/// \brief Tile location where item will be inserted
		data::Unique_data_base& destination_unique_data_;
		data::placementOrientation orientation_;

		/// \brief Function for inserting at destination, from one below
		insert_func insertion_function_;

	public:
		///
		///	 \brief Insert provided item at destination
		bool insert(const data::item_stack& item_stack) const {
			return insertion_function_(item_stack, destination_unique_data_, orientation_);
		}
	};
}

#endif // JACTORIO_GAME_LOGIC_ITEM_INSERT_DESTINATION_H
