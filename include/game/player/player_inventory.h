#ifndef GAME_PLAYER_PLAYER_INVENTORY_H
#define GAME_PLAYER_PLAYER_INVENTORY_H

#include "data/prototype/item/item.h"

/**
 * Manages the logic behind the player inventory
 */
namespace jactorio::game::player_inventory
{
	// Inventory
	constexpr unsigned short inventory_size = 80;
	inline data::item_stack player_inventory[inventory_size];  // Holds the internal id of items

	/**
	 * @param index The player inventory index
	 * @param is_empty Is the clicked inventory empty?
	 */
    void set_clicked_inventory(unsigned short index, bool is_empty);

	/**
	 * Whether or not the player has something selected in the inventory
	 * @return nullptr if there is no item selected
	 */
	[[nodiscard]] data::item_stack* get_selected_item();
}

#endif // GAME_PLAYER_PLAYER_INVENTORY_H
