// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_LOGIC_INVENTORY_CONTROLLER_H
#define JACTORIO_INCLUDE_GAME_LOGIC_INVENTORY_CONTROLLER_H
#pragma once

#include "data/prototype/item.h"

namespace jactorio::game
{
    ///
    /// \return true if origin stack matches the filter of target stack and can thus be inserted into it
    bool StackMatchesFilter(const data::ItemStack& origin_stack, const data::ItemStack& target_stack);
    ///
    /// Moves itemstack origin to target
    /// When stacking items, the stack prioritizes the target inventory
    /// \remarks This should be only used by the player, machines should use AddStack instead
    /// \return true if origin_stack is empty after the move
    bool MoveItemstackToIndex(data::ItemStack& origin_stack,
                              data::ItemStack& target_stack,
                              unsigned short mouse_button);

    // ======================================================================
    // Non-player inventories

    ///
    /// Returns whether or not item stack can be added to the target_inv
    /// \return success, first index which can be added at
    std::pair<bool, size_t> CanAddStack(const data::Item::Inventory& inv, const data::ItemStack& item_stack);

    ///
    /// Attempts to add item_stack to target_inv
    /// \return Number of items which were NOT added
    data::Item::StackCount AddStack(data::Item::Inventory& inv, const data::ItemStack& item_stack);

    ///
    /// Attempts to add the specified item stack to the target_inv and subtracts item_stack
    /// \details Add to the inventory left to right, top to bottom | Fill slots of same type if it encounters them
    /// \return false if failed, amount left in item_stack.second is remaining amount
    bool AddStackSub(data::Item::Inventory& inv, data::ItemStack& item_stack);

    ///
    /// Gets count of items matching item in inv of inv_size
    /// \return Item count with internal id iid
    uint32_t GetInvItemCount(const data::Item::Inventory& inv, const data::Item* item);

    ///
    /// Gets the first item within the inventory
    /// \return nullptr if no items were found
    const data::Item* GetFirstItem(const data::Item::Inventory& inv);


    ///
    /// Removes remove_amount of specified item from inventory inv
    /// \return false if there is insufficient items to remove remove_amount
    bool RemoveInvItem(data::Item::Inventory& inv, const data::Item* item, uint32_t remove_amount);

    ///
    /// Removes remove_amount of specified item from inventory inv
    /// \remark DOES NOT check that there is sufficient items to remove, use RemoveInvItem() if checking is needed
    void DeleteInvItem(data::Item::Inventory& inv, const data::Item* item, uint32_t remove_amount);
} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_LOGIC_INVENTORY_CONTROLLER_H
