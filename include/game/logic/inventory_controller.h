#ifndef GAME_LOGIC_INVENTORY_CONTROLLER_H
#define GAME_LOGIC_INVENTORY_CONTROLLER_H

#include "jactorio.h"

#include "data/prototype/item/item.h"

namespace jactorio::game::logic
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
}

#endif // GAME_LOGIC_INVENTORY_CONTROLLER_H
