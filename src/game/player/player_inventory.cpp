#include "game/player/player_inventory.h"

#include <cassert>

#include "core/logger.h"

unsigned short selected_inventory_index;
bool selected_slot_is_empty;

// Item selection
jactorio::data::item_stack selected_item;
bool is_item_selected = false;

void jactorio::game::player_inventory::set_clicked_inventory(const unsigned short index, const bool is_empty) {
	assert(index < inventory_size);
	
	selected_inventory_index = index;
	selected_slot_is_empty = is_empty;

	if (!is_empty) {
		// No item selected, pick up the item
		if (!is_item_selected) {
			is_item_selected = true;
			selected_item = player_inventory[index];
			player_inventory[index].second = 0;
		}
			// Has item selected, clicked on another item; distribute stack / swap places
		else {
			const auto temp = player_inventory[index];
			
			player_inventory[index] = selected_item;
			selected_item = temp;
		}
	}
	else {
		// Click empty location with item selected
		if (is_item_selected) {
			player_inventory[index] = selected_item;
			is_item_selected = false;
		}
	}
}

jactorio::data::item_stack* jactorio::game::player_inventory::get_selected_item() {
	if (!is_item_selected)
		return nullptr;
	
	return &selected_item;
}
