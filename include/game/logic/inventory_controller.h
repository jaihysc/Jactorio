#ifndef GAME_LOGIC_INVENTORY_CONTROLLER_H
#define GAME_LOGIC_INVENTORY_CONTROLLER_H

#include "data/prototype/item/item.h"

namespace jactorio::game::logic
{
	/**
	 * Moves itemstack from index in origin to target inventory index <br>
	 * When stacking items, the stack prioritizes the target inventory
	 * @return true if origin_inv is empty after the move
	 */
	bool move_itemstack_to_index(data::item_stack* origin_inv, unsigned short origin_inv_index,
	                             data::item_stack* target_inv, unsigned short target_inv_index, 
	                             unsigned short mouse_button);

	// bool move_itemstack(data::item_stack* origin_inv, unsigned short origin_inv_size, unsigned short origin_inv_index,
		// data::item_stack* target_inv, unsigned short target_inv_size, unsigned short target_inv_index);
}

#endif // GAME_LOGIC_INVENTORY_CONTROLLER_H
