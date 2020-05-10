// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 04/08/2020

#ifndef JACTORIO_GAME_LOGIC_ITEM_INSERT_DESTINATION_H
#define JACTORIO_GAME_LOGIC_ITEM_INSERT_DESTINATION_H
#pragma once

#include "data/prototype/orientation.h"
#include "data/prototype/item/item.h"

namespace jactorio::game
{
	///
	/// \brief Represents a world location where items can be inserted
	class ItemInsertDestination
	{
	public:
		/// \brief A function for inserting items into a destination
		/// \param item Item to insert
		/// \param orientation Orientation from origin object 
		///  \return True is successfully inserted
		using InsertFunc = bool (*)(const data::ItemStack& item_stack,
		                            data::UniqueDataBase& unique_data,
		                            data::Orientation orientation);


		///
		/// \param unique_data Unique data at tile
		/// \param insert_function Obtained from item_logistics::can_accept_item()
		/// \param orientation Orientation from origin prototype, "the destination is <orientation> of prototype"
		ItemInsertDestination(data::UniqueDataBase& unique_data, const InsertFunc insert_function,
		                      const data::Orientation orientation)
			: destinationUniqueData_(unique_data),
			  orientation_(orientation),
			  insertionFunction(insert_function) {
			assert(insert_function != nullptr);
		}


	private:
		/// \brief Tile location where item will be inserted
		data::UniqueDataBase& destinationUniqueData_;
		data::Orientation orientation_;

		/// \brief Function for inserting at destination, from one below
		InsertFunc insertionFunction;

	public:
		///
		///	 \brief Insert provided item at destination
		bool Insert(const data::ItemStack& item_stack) const {
			return insertionFunction(item_stack, destinationUniqueData_, orientation_);
		}
	};
}

#endif // JACTORIO_GAME_LOGIC_ITEM_INSERT_DESTINATION_H
