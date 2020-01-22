#ifndef GAME_LOGIC_INVENTORY_CONTROLLER_H
#define GAME_LOGIC_INVENTORY_CONTROLLER_H

#include "jactorio.h"

#include "data/prototype/item/item.h"

namespace jactorio::game
{
	namespace inventory_c
	{		
		/**
		 * Moves itemstack from index in origin to target inventory index <br>
		 * When stacking items, the stack prioritizes the target inventory <br>
		 * @remarks This should be only used by the player, machines should use add_itemstack_to_inv instead
		 * @return true if origin_inv is empty after the move
		 */
		bool move_itemstack_to_index(data::item_stack* origin_inv, uint16_t origin_inv_index,
		                             data::item_stack* target_inv, uint16_t target_inv_index,
		                             unsigned short mouse_button);

		// Non-player inventories
		
		/**
		 * Attempts to add the specified item stack to the target_inv <br>
		 * Will not set item_stack to nullptr
		 * @details Add to the inventory left to right, top to bottom | Fill slots of same type if it encounters them
		 * @return true if item successfully added to target_inv <br>
		 * false if failed, amount left in item_stack.second is remaining amount
		 */
		bool add_itemstack_to_inv(data::item_stack* target_inv, uint16_t target_inv_size,
		                          data::item_stack& item_stack);

		/**
		 * Gets count of items matching item in inv of inv_size
		 * @return Item count with internal id iid
		 */
		uint32_t get_inv_item_count(data::item_stack* inv, uint16_t inv_size, const data::Item* item);

		
		/**
		 * Removes remove_amount of specified item from inventory inv <br>
		 * @return false if there is insufficient items to remove remove_amount
		 */
		bool remove_inv_item_s(data::item_stack* inv, uint16_t inv_size,
		                       const data::Item* item, uint32_t remove_amount);

		/**
		 * Removes remove_amount of specified item from inventory inv <br>
		 * Sets inv[i].first to nullptr
		 * !! DOES NOT check that there is sufficient items to remove, use remove_inv_item_s() if checking is needed
		 */
		void remove_inv_item(data::item_stack* inv, uint16_t inv_size,
		                     const data::Item* item, uint32_t remove_amount);
	}
}

#endif // GAME_LOGIC_INVENTORY_CONTROLLER_H
