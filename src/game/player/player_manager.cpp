#include "game/player/player_manager.h"

#include <cassert>
#include <algorithm>

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
	if (origin_tile->get_tile_layer_tile_prototype(jactorio::game::Chunk_tile::chunk_layer::base)->is_water)
		return true;

	const jactorio::game::Chunk_tile* tile = jactorio::game::world_manager::get_tile_world_coords(x, y);
	// Chunk not generated yet
	if (tile == nullptr)
		return false;

	return !tile->get_tile_layer_tile_prototype(jactorio::game::Chunk_tile::chunk_layer::base)->is_water;
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

void jactorio::game::player_manager::player_inventory_sort() {
	// The inventory must be sorted without moving the selected cursor

	// Copy non-cursor into a new array, sort it, copy it back minding the selection cursor
	std::vector<data::item_stack> inv_temp;
	inv_temp.reserve(player_inventory_size);
	for (const auto& i : player_inventory) {
		// Skip the cursor
		if (i.first == nullptr ||
			i.first->get_localized_name() == inventory_selected_cursor_iname) {
			continue;
		}
		
		inv_temp.push_back(i);
	}
	
	// Sort temp inventory (does not contain cursor)
	std::sort(inv_temp.begin(), inv_temp.end(),
	          [](const data::item_stack a, const data::item_stack b) {
		          auto& a_name = a.first->get_localized_name();
		          auto& b_name = b.first->get_localized_name();

		          return a_name < b_name;
	          });

	// Compress item stacks
	for (int i = inv_temp.size() - 1; i >= 0; --i) {
		uint16_t buffer_item_count = inv_temp[i].second;
		const uint16_t stack_size = inv_temp[i].first->stack_size;

		// Find index which the same item type begins
		int j = i;
		while (inv_temp[j].first == inv_temp[i].first) {
			if (j == 0) {
				j = -1;
				break;
			}
			j--;
		}
		// Ends 1 before, shift 1 ahead
		j++;

		for (j; j < i; ++j) {

			// If item somehow exceeds stack do not attempt to stack into it
			if (inv_temp[j].second > stack_size)
				continue;

			// Amount which can be dropped is space left until reaching stack size
			const uint16_t max_drop_amount = stack_size - inv_temp[j].second;

			// Has enough to max current stack and move on
			if (buffer_item_count > max_drop_amount) {
				inv_temp[j].second = stack_size;
				buffer_item_count -= max_drop_amount;
			}
				// Not enough to drop and move on
			else {
				inv_temp[j].second += buffer_item_count;
				inv_temp[i].first = nullptr;
				buffer_item_count = 0;
				break;
			}
		}
		// Did not drop all items off
		if (buffer_item_count > 0) {
			inv_temp[i].second = buffer_item_count;
		}

	}
	
	// Copy back into origin inventory
	unsigned int start = 0;  // The index of the first blank slot post sorting
	unsigned int inv_temp_index = 0;
	for (auto i = 0; i < player_inventory_size; ++i) {
		// Skip the cursor
		if (player_inventory[i].first != nullptr && 
			player_inventory[i].first->get_localized_name() == inventory_selected_cursor_iname)
			continue;
		
		if (inv_temp_index >= inv_temp.size()) {
			start = i;
			break;
		}
		// Omit empty gaps in inv_temp from the compressing process
		while (inv_temp[inv_temp_index].first == nullptr) {
			inv_temp_index++;
			if (inv_temp_index >= inv_temp.size()) {
				start = i;
				goto loop_exit;
			}
		};
		
		player_inventory[i] = inv_temp[inv_temp_index++];
	}
loop_exit:
	
	// Copy empty spaces into the remainder of the slots
	for (auto i = start; i < player_inventory_size; ++i) {
		// Skip the cursor
		if (player_inventory[i].first != nullptr &&
			player_inventory[i].first->get_localized_name() == inventory_selected_cursor_iname)
			continue;
		
		player_inventory[i] = {nullptr, 0};
	}
}

// LEFT CLICK - Select by reference, the item in the cursor mirrors the inventory item
// RIGHT CLICK - Select unique, the item in the cursor exists independently of the inventory item
jactorio::data::item_stack selected_item;

bool has_item_selected = false;
unsigned short selected_item_index;
bool select_by_reference = false;

void jactorio::game::player_manager::set_clicked_inventory(const unsigned short index,
                                                           const unsigned short mouse_button) {
	assert(index < player_inventory_size);
	assert(mouse_button == 0 || mouse_button == 1);  // Only left + right click supported
	
	// Clicking on the same location, selecting by reference: deselect
	if (has_item_selected && selected_item_index == index && select_by_reference) {
		has_item_selected = false;
		player_inventory[selected_item_index] = selected_item;

		return;
	}
	
	
	// Selection mode can only be set upon first item selection
	if (!has_item_selected) {
		// Clicking empty slot
		if (player_inventory[index].first == nullptr)
			return;
		
		has_item_selected = true;
		selected_item_index = index;

		// Reference
		if (mouse_button == 0) {
			select_by_reference = true;
			selected_item = player_inventory[index];

			// Swap icon out for a cursor indicating the current index is selected
			player_inventory[index].first = data::data_manager::data_raw_get<data::Item>(
				data::data_category::item, inventory_selected_cursor_iname);
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
		inventory_c::move_itemstack_to_index(&selected_item, 0,
		                                     player_inventory, index,
		                                     mouse_button);
	// Cursor slot is empty
	if (cursor_empty) {
		has_item_selected = false;
		
		if (select_by_reference) {
			// Remove cursor icon
			assert(selected_item_index < player_inventory_size);

			player_inventory[selected_item_index].first = nullptr;
			player_inventory[selected_item_index].second = 0;
		}
	}
}

const jactorio::data::item_stack* jactorio::game::player_manager::get_selected_item() {
	if (!has_item_selected)
		return nullptr;
	
	return &selected_item;
}

bool jactorio::game::player_manager::increment_selected_item() {
	assert(has_item_selected);

	// DO not increment if it will exceed the stack size
	if (selected_item.second < selected_item.first->stack_size) {
		selected_item.second++;
		return true;
	}
	
	return false;
}

bool jactorio::game::player_manager::decrement_selected_item() {
	assert(has_item_selected);

	if (--selected_item.second == 0) {
		// Item stack now empty
		has_item_selected = false;
		// Remove selection cursor
		player_inventory[selected_item_index].first = nullptr;
		player_inventory[selected_item_index].second = 0;
		
		return false;
	}
	return true;
}


// Recipe
uint16_t selected_recipe_group = 0;

void jactorio::game::player_manager::select_recipe_group(const uint16_t index) {
	selected_recipe_group = index;
}

uint16_t jactorio::game::player_manager::get_selected_recipe_group() {
	return selected_recipe_group;
}

bool jactorio::game::player_manager::recipe_craft(data::Recipe* recipe) {
	// Ensure ingredients are met
	for (auto& ingredient : recipe->ingredients) {
		auto* item = data::data_manager::data_raw_get<data::Item>(
			data::data_category::item, ingredient.first);

		if (inventory_c::get_inv_item_count(player_inventory,  player_inventory_size, item) < ingredient.second) {
			return false;
		}
	}

	// Ingredients exist, remove them
	for (auto& ingredient : recipe->ingredients) {
		auto* item = data::data_manager::data_raw_get<data::Item>(
			data::data_category::item, ingredient.first);

		inventory_c::remove_inv_item(player_inventory, player_inventory_size, item, ingredient.second);
	}

	// Return product
	auto* product_item = data::data_manager::data_raw_get<data::Item>(
		data::data_category::item, recipe->get_product().first);
	data::item_stack i = {product_item, recipe->get_product().second};
	
	inventory_c::add_itemstack_to_inv(player_inventory, player_inventory_size, i);
	return true;
}


// Reserved

void jactorio::game::player_manager::reset_inventory_variables() {
	has_item_selected = false;
	select_by_reference = false;
}
