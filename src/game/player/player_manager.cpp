#include "game/player/player_manager.h"

#include <cassert>

#include "game/logic/inventory_controller.h"
#include "game/world/world_manager.h"
#include "data/data_manager.h"

float player_position_x = 0;
float player_position_y = 0;

// Returns true if the tile can be walked on and is not walter
bool target_tile_valid(const float x, const float y) {
	const auto origin_tile = jactorio::game::world_manager::get_tile_world_coords(player_position_x, player_position_y);
	if (origin_tile == nullptr)
		return false;

	// If the player is on water, they are allowed to walk on water
	if (origin_tile->is_water)
		return true;

	const jactorio::game::Chunk_tile* tile = jactorio::game::world_manager::get_tile_world_coords(x, y);
	// Chunk not generated yet
	if (tile == nullptr)
		return false;

	return !tile->is_water;
}


float jactorio::game::player_manager::get_player_position_x() {
	return player_position_x;
}

float jactorio::game::player_manager::get_player_position_y() {
	return player_position_y;
}


void jactorio::game::player_manager::move_player_x(const float amount) {
	const float target_x = player_position_x + amount;

	if (target_tile_valid(target_x, player_position_y))
		player_position_x = target_x;
}

void jactorio::game::player_manager::move_player_y(const float amount) {
	const float target_y = player_position_y + amount;

	if (target_tile_valid(player_position_x, target_y))
		player_position_y = target_y;
}

// Inventory

// LEFT CLICK - Select by reference, the item in the cursor mirrors the inventory item
// RIGHT CLICK - Select unique, the item in the cursor exists independently of the inventory item
jactorio::data::item_stack selected_item;

bool has_item_selected = false;
unsigned short selected_item_index;
bool select_by_reference = false;

void jactorio::game::player_manager::set_clicked_inventory(const unsigned short index,
                                                           const unsigned short mouse_button) {
	assert(index < inventory_size);
	assert(mouse_button == 0 || mouse_button == 1);  // Only left + right click supported
	
	// Clicking on the same location, selecting by reference: deselect
	if (has_item_selected && selected_item_index == index && select_by_reference) {
		has_item_selected = false;
		player_inventory[selected_item_index] = selected_item;

		return;
	}
	
	
	// Selection mode can only be set upon first item selection
	if (!has_item_selected) {
		has_item_selected = true;
		selected_item_index = index;

		// Reference
		if (mouse_button == 0) {
			select_by_reference = true;
			selected_item = player_inventory[index];

			// Swap icon out for a cursor indicating the current index is selected
			player_inventory[index].first = data::data_manager::data_raw_get<data::Item>(
				data::data_category::item, "__core__/inventory-selected-cursor");
			player_inventory[index].second = 0;

			// Return is necessary when selecting by reference
			// The item needs to be moved after selecting the next inventory slot
			return;
		}

		// Unique
		select_by_reference = false;

		// Clear the cursor inventory so half can be moved into it
		selected_item.first = nullptr;
		selected_item.second = 0;
		// DO NOT return for it to move the item into the new inventory
	}

	const bool cursor_empty =
		inventory_controller::move_itemstack_to_index(&selected_item, 0,
		                                              player_inventory, index,
		                                              mouse_button);
	// Cursor slot is empty
	if (cursor_empty) {
		has_item_selected = false;
		
		if (select_by_reference) {
			// Remove cursor icon
			assert(selected_item_index < inventory_size);

			player_inventory[selected_item_index].first = nullptr;
			player_inventory[selected_item_index].second = 0;
		}
	}
}

jactorio::data::item_stack* jactorio::game::player_manager::get_selected_item() {
	if (!has_item_selected)
		return nullptr;
	
	return &selected_item;
}

void jactorio::game::player_manager::reset_inventory_variables() {
	has_item_selected = false;
	select_by_reference = false;
}
