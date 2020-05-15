// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 01/20/2020

#ifndef JACTORIO_INCLUDE_GAME_LOGIC_INVENTORY_CONTROLLER_H
#define JACTORIO_INCLUDE_GAME_LOGIC_INVENTORY_CONTROLLER_H
#pragma once

#include "data/prototype/item/item.h"

namespace jactorio::game
{
	/// 
	/// Moves itemstack from index in origin to target inventory index <br>
	/// When stacking items, the stack prioritizes the target inventory <br>
	/// \remarks This should be only used by the player, machines should use add_itemstack_to_inv instead
	/// \return true if origin_inv is empty after the move
	bool MoveItemstackToIndex(data::ItemStack* origin_inv, uint16_t origin_inv_index,
	                          data::ItemStack* target_inv, uint16_t target_inv_index,
	                          unsigned short mouse_button);

	// ======================================================================
	// Non-player inventories

	///
	/// \brief Returns whether or not item stack can be added to the target_inv 
	bool CanAddStack(const data::ItemStack* target_inv, uint16_t target_inv_size,
	                 const data::ItemStack& item_stack);

	///
	/// \brief Attempts to add item_stack to target_inv
	/// \return Number of items which were NOT added 
	decltype(data::ItemStack::second) AddStack(data::ItemStack* target_inv, uint16_t target_inv_size,
	                                           const data::ItemStack& item_stack);

	///
	/// \brief Attempts to add the specified item stack to the target_inv and subtracts item_stack 
	/// \details Add to the inventory left to right, top to bottom | Fill slots of same type if it encounters them
	/// \return false if failed, amount left in item_stack.second is remaining amount
	bool AddStackSub(data::ItemStack* target_inv, uint16_t target_inv_size,
	                 data::ItemStack& item_stack);

	/// 
	/// Gets count of items matching item in inv of inv_size
	/// \return Item count with internal id iid
	uint32_t GetInvItemCount(data::ItemStack* inv, uint16_t inv_size, const data::Item* item);


	/// 
	/// Removes remove_amount of specified item from inventory inv <br>
	/// \return false if there is insufficient items to remove remove_amount
	bool RemoveInvItemS(data::ItemStack* inv, uint16_t inv_size,
	                    const data::Item* item, uint32_t remove_amount);

	/// 
	/// Removes remove_amount of specified item from inventory inv <br>
	/// Sets inv[i].first to nullptr
	/// \remark DOES NOT check that there is sufficient items to remove, use remove_inv_item_s() if checking is needed
	void RemoveInvItem(data::ItemStack* inv, uint16_t inv_size,
	                   const data::Item* item, uint32_t remove_amount);
}

#endif //JACTORIO_INCLUDE_GAME_LOGIC_INVENTORY_CONTROLLER_H
