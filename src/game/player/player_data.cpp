// 
// player_data.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 12/21/2019
// Last modified: 03/21/2020
// 

#include "game/player/player_data.h"

#include <algorithm>
#include <cassert>
#include <map>

#include "data/data_manager.h"
#include "data/prototype/entity/resource_entity.h"
#include "game/input/mouse_selection.h"
#include "game/logic/inventory_controller.h"
#include "game/logic/placement_controller.h"
#include "game/world/world_data.h"
#include "game/world/chunk_tile_getters.h"

bool jactorio::game::Player_data::target_tile_valid(World_data* world_data, const int x, const int y) const {
	assert(world_data != nullptr);  // Player is not in a world

	const auto origin_tile =
		world_data->get_tile_world_coords(
			static_cast<int>(player_position_x_),
			static_cast<int>(player_position_y_));

	if (origin_tile == nullptr)
		return false;

	// If the player is on water, they are allowed to walk on water
	if (chunk_tile_getter::get_tile_prototype(*origin_tile, Chunk_tile::chunkLayer::base)->is_water)
		return true;

	const Chunk_tile* tile = world_data->get_tile_world_coords(x, y);
	// Chunk not generated yet
	if (tile == nullptr)
		return false;

	return !chunk_tile_getter::get_tile_prototype(*tile, Chunk_tile::chunkLayer::base)->is_water;
}

void jactorio::game::Player_data::move_player_x(const float amount) {
	const float target_x = player_position_x_ + amount;

	if (target_tile_valid(player_world_, static_cast<int>(target_x), static_cast<int>(player_position_y_)))
		player_position_x_ = target_x;
}

void jactorio::game::Player_data::move_player_y(const float amount) {
	const float target_y = player_position_y_ + amount;

	if (target_tile_valid(player_world_, static_cast<int>(player_position_x_), static_cast<int>(target_y)))
		player_position_y_ = target_y;
}


// ============================================================================================
// Entity placement / pickup

void jactorio::game::Player_data::rotate_placement_orientation() {
	switch (placement_orientation) {
	case data::placementOrientation::up:
		placement_orientation = data::placementOrientation::right;
		break;
	case data::placementOrientation::right:
		placement_orientation = data::placementOrientation::down;
		break;
	case data::placementOrientation::down:
		placement_orientation = data::placementOrientation::left;
		break;
	case data::placementOrientation::left:
		placement_orientation = data::placementOrientation::up;
		break;
	default:
		assert(false);  // Missing switch case
	}
}

void jactorio::game::Player_data::counter_rotate_placement_orientation() {
	switch (placement_orientation) {
	case data::placementOrientation::up:
		placement_orientation = data::placementOrientation::left;
		break;
	case data::placementOrientation::left:
		placement_orientation = data::placementOrientation::down;
		break;
	case data::placementOrientation::down:
		placement_orientation = data::placementOrientation::right;
		break;
	case data::placementOrientation::right:
		placement_orientation = data::placementOrientation::up;
		break;
	default:
		assert(false);  // Missing switch case
	}
}

void jactorio::game::Player_data::try_place_entity(World_data& world_data,
                                                   const int world_x, const int world_y,
                                                   const bool can_activate_layer) {
	auto* tile = world_data.get_tile_world_coords(world_x, world_y);
	if (tile == nullptr)
		return;

	auto& selected_layer = tile->get_layer(Chunk_tile::chunkLayer::entity);

	const data::item_stack* item = get_selected_item();
	// Ensure item attempting to place is an entity
	data::Entity* entity_ptr = nullptr;

	// No selected item or selected item is not placeable and clicked on a entity
	bool check_selection = false;
	if (item == nullptr)
		check_selection = true;
	else {
		entity_ptr = static_cast<data::Entity*>(item->first->entity_prototype);
		if (entity_ptr == nullptr || !entity_ptr->placeable)
			check_selection = true;
	}

	if (check_selection) {
		if (!can_activate_layer)
			return;

		// Since this is entity layer, everything is guaranteed to be an entity
		if (selected_layer.prototype_data) {
			// // If clicking again on the same entity, deactivate
			// if (activated_layer == &selected_layer)
			// 	activated_layer = nullptr;
			// else

			// Clicking on an existing entity will activate it
			activated_layer_ = &selected_layer;
		}

		return;
	}


	assert(entity_ptr != nullptr);
	// Do not take item away from player unless item was successfully placed
	if (! placement_c::place_entity_at_coords(world_data, entity_ptr, world_x, world_y)) {
		// Failed to place because an entity already exists
		return;
	}

	// If item stack was used up, sort player inventory to fill gap
	if (!decrement_selected_item()) {
		inventory_sort();
	}

	// Call events
	std::pair<uint16_t, uint16_t> set_frame = {0, 0};

	// Calculate correct sprite set / frame if rotatable
	const data::Rotatable_entity* rotatable_entity;
	if (entity_ptr->rotatable && (rotatable_entity = dynamic_cast<data::Rotatable_entity*>(entity_ptr)) != nullptr) {
		set_frame = rotatable_entity->map_placement_orientation(
			placement_orientation,
			&world_data.get_tile_world_coords(
				           world_x,
				           world_y - 1)
			           ->get_layer(Chunk_tile::chunkLayer::entity),
			&world_data.get_tile_world_coords(
				           world_x + 1,
				           world_y)
			           ->get_layer(Chunk_tile::chunkLayer::entity),
			&world_data.get_tile_world_coords(
				           world_x,
				           world_y + 1)
			           ->get_layer(Chunk_tile::chunkLayer::entity),
			&world_data.get_tile_world_coords(
				           world_x - 1,
				           world_y)
			           ->get_layer(Chunk_tile::chunkLayer::entity));
	}

	entity_ptr->on_build(&selected_layer, set_frame);
}


void jactorio::game::Player_data::try_pickup(World_data& world_data,
                                             const int tile_x, const int tile_y,
                                             const uint16_t ticks) {
	auto* tile = world_data.get_tile_world_coords(tile_x, tile_y);

	data::Entity* chosen_ptr;
	bool is_resource_ptr = true;
	{
		const auto entity_ptr =
			chunk_tile_getter::get_entity_prototype(*tile, Chunk_tile::chunkLayer::entity);

		const auto resource_ptr =
			chunk_tile_getter::get_entity_prototype(*tile, Chunk_tile::chunkLayer::resource);

		// Picking up entities takes priority since it is higher on the layer
		if (entity_ptr != nullptr) {
			is_resource_ptr = false;
			chosen_ptr = entity_ptr;
		}
		else if (resource_ptr != nullptr)
			chosen_ptr = resource_ptr;
		else
			// No valid pointers
			return;
	}

	// Selecting a new tile different from the last selected tile will reset the counter
	if (last_selected_ptr_ != chosen_ptr || last_tile_ptr_ != tile) {
		pickup_tick_counter_ = 0;
		pickup_tick_target_ = chosen_ptr->pickup_time * 60;  // Seconds to ticks
	}
	// Remember the entity + tile which was selected
	last_selected_ptr_ = chosen_ptr;
	last_tile_ptr_ = tile;

	pickup_tick_counter_ += ticks;
	if (pickup_tick_counter_ >= pickup_tick_target_) {
		// Entity picked up
		LOG_MESSAGE(debug, "Player picked up entity");

		// Give picked up item to player
		auto item_stack = data::item_stack(chosen_ptr->get_item(), 1);
		const bool added_item = inventory_c::add_itemstack_to_inv(inventory_player, inventory_size, item_stack);

		if (!added_item) {  // Failed to add item, likely because the inventory is full
			return;
		}
		inventory_sort();

		pickup_tick_counter_ = 0;
		// Resource entity
		if (is_resource_ptr) {
			auto& layer = tile->get_layer(Chunk_tile::chunkLayer::resource);
			auto* resource_data = static_cast<data::Resource_entity_data*>(layer.unique_data);

			assert(resource_data != nullptr);  // Resource tiles should have valid data

			// Delete resource tile if it is empty after extracting
			if (--resource_data->resource_amount == 0) {
				layer.clear();
			}
		}
			// Is normal entity
		else {
			// Picking up an entity which is set in activated_layer will unset activated_layer
			if (activated_layer_ == &tile->get_layer(Chunk_tile::chunkLayer::entity))
				activated_layer_ = nullptr;

			const bool result = placement_c::place_entity_at_coords(world_data, nullptr, tile_x, tile_y);
			assert(result);  // false indicates failed to remove entity
		}
	}
}

float jactorio::game::Player_data::get_pickup_percentage() const {
	if (last_selected_ptr_ == nullptr)  // Not initialized yet
		return 0.f;

	return static_cast<float>(pickup_tick_counter_) / static_cast<float>(pickup_tick_target_);
}

// ============================================================================================
// Inventory

void jactorio::game::Player_data::inventory_sort() {
	// The inventory must be sorted without moving the selected cursor

	LOG_MESSAGE(debug, "Sorting player inventory");

	// Copy non-cursor into a new array, sort it, copy it back minding the selection cursor
	std::vector<data::item_stack> inv_temp;
	inv_temp.reserve(inventory_size);
	for (const auto& i : inventory_player) {
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
	for (long long i = inv_temp.size() - 1; i >= 0; --i) {
		uint16_t buffer_item_count = inv_temp[i].second;
		const uint16_t stack_size = inv_temp[i].first->stack_size;

		// Find index which the same item type begins
		auto j = i;
		while (inv_temp[j].first == inv_temp[i].first) {
			if (j == 0) {
				j = -1;
				break;
			}
			j--;
		}
		// Ends 1 before, shift 1 ahead
		j++;

		for (; j < i; ++j) {

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
	int start = -1;  // The index of the first blank slot post sorting
	unsigned int inv_temp_index = 0;
	for (auto i = 0; i < inventory_size; ++i) {
		// Skip the cursor
		if (inventory_player[i].first != nullptr &&
			inventory_player[i].first->get_localized_name() == inventory_selected_cursor_iname)
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
		}

		inventory_player[i] = inv_temp[inv_temp_index++];
	}
loop_exit:

	if (start == -1)  // Start being -1 means that there is no empty slots
		return;

	// Copy empty spaces into the remainder of the slots
	for (auto i = start; i < inventory_size; ++i) {
		// Skip the cursor
		if (inventory_player[i].first != nullptr &&
			inventory_player[i].first->get_localized_name() == inventory_selected_cursor_iname)
			continue;

		inventory_player[i] = {nullptr, 0};
	}
}

// LEFT CLICK - Select by reference, the item in the cursor mirrors the inventory item
// RIGHT CLICK - Select unique, the item in the cursor exists independently of the inventory item

void jactorio::game::Player_data::inventory_click(const unsigned short index,
                                                  const unsigned short mouse_button,
                                                  const bool allow_reference_select,
                                                  data::item_stack* inv) {
	assert(index < inventory_size);
	assert(mouse_button == 0 || mouse_button == 1);  // Only left + right click supported

	// Clicking on the same location + inventory, selecting by reference: deselect
	if (has_item_selected_ && select_by_reference_ &&
		selected_item_index_ == index && selected_item_inventory_ == inv) {
		has_item_selected_ = false;

		// Remove selection cursor from inventory_player
		inventory_player[selected_item_index_].first = nullptr;
		inventory_player[selected_item_index_].second = 0;

		// Add referenced item to slot
		inv[selected_item_index_] = selected_item_;
		return;
	}


	// Selection mode can only be set upon first item selection
	if (!has_item_selected_) {
		// Clicking empty slot
		if (inv[index].first == nullptr)
			return;

		has_item_selected_ = true;
		selected_item_index_ = index;
		selected_item_inventory_ = inv;

		// Reference
		if (allow_reference_select && mouse_button == 0) {
			assert(inv == inventory_player);  // Select by reference only allowed for player inventory
			select_by_reference_ = true;
			selected_item_ = inv[index];

			// Swap icon out for a cursor indicating the current index is selected
			inventory_player[index].first = data::data_manager::data_raw_get<data::Item>(
				data::data_category::item, inventory_selected_cursor_iname);
			inventory_player[index].second = 0;

			// Return is necessary when selecting by reference
			// The item needs to be moved after selecting the next inventory slot
			return;
		}

		// Unique
		select_by_reference_ = false;

		// Clear the cursor inventory so half can be moved into it
		selected_item_.first = nullptr;
		selected_item_.second = 0;
		// DO NOT return for it to move the item into the new inventory
	}

	const bool cursor_empty =
		inventory_c::move_itemstack_to_index(&selected_item_, 0,
		                                     inv, index,
		                                     mouse_button);
	// Cursor slot is empty
	if (cursor_empty) {
		has_item_selected_ = false;

		if (select_by_reference_) {
			// Remove cursor icon
			assert(selected_item_index_ < inventory_size);
			// Select by reference is only for inventory_player
			inventory_player[selected_item_index_].first = nullptr;
			inventory_player[selected_item_index_].second = 0;
		}
	}
}

const jactorio::data::item_stack* jactorio::game::Player_data::get_selected_item() const {
	if (!has_item_selected_)
		return nullptr;

	return &selected_item_;
}

bool jactorio::game::Player_data::increment_selected_item() {
	assert(has_item_selected_);

	// DO not increment if it will exceed the stack size
	if (selected_item_.second < selected_item_.first->stack_size) {
		selected_item_.second++;
		return true;
	}

	return false;
}

bool jactorio::game::Player_data::decrement_selected_item() {
	assert(has_item_selected_);

	if (--selected_item_.second == 0) {
		// Item stack now empty
		has_item_selected_ = false;
		// Remove selection cursor
		inventory_player[selected_item_index_].first = nullptr;
		inventory_player[selected_item_index_].second = 0;

		return false;
	}
	return true;
}

// ============================================================================================
// Recipe

void jactorio::game::Player_data::recipe_group_select(const uint16_t index) {
	selected_recipe_group_ = index;
}

uint16_t jactorio::game::Player_data::recipe_group_get_selected() const {
	return selected_recipe_group_;
}

void jactorio::game::Player_data::recipe_craft_tick(uint16_t ticks) {
	while (ticks != 0 && !crafting_queue_.empty()) {
		// Ticks available greater than or equal to crafting ticks remaining
		if (ticks >= crafting_ticks_remaining_) {
			ticks -= crafting_ticks_remaining_;

			auto* recipe = crafting_queue_.front();
			crafting_queue_.pop_front();

			// Return product
			data::recipe_item recipe_item = recipe->get_product();
			auto* product_item = data::data_manager::data_raw_get<data::Item>(
				data::data_category::item, recipe_item.first);

			data::item_stack i = {product_item, recipe_item.second};

			// Deduct based on the deductions
			std::map<std::string, uint16_t>::iterator element;
			if ((element = crafting_item_deductions_.find(recipe_item.first)) != crafting_item_deductions_.end()) {
				auto& deduct_amount = element->second;

				if (i.second >= deduct_amount) {
					i.second -= deduct_amount;

					LOG_MESSAGE_f(debug, "Crafting return deducting %d of '%s'",
					              deduct_amount, recipe_item.first.c_str());

					crafting_item_deductions_.erase(recipe_item.first);  // Now empty
				}
					// Deduct amount greater than i.second
				else {
					deduct_amount -= i.second;
					i.second = 0;

					LOG_MESSAGE_f(debug, "Crafting return deducting %d of '%s', no items returned",
					              deduct_amount, recipe_item.first.c_str());
				}
			}

			// Still has items to return to player inventory
			if (i.second != 0) {
				// Extra not available in queue anymore since it has been returned to the player
				auto& queue_extras = crafting_item_extras_[recipe_item.first];
				if (queue_extras > i.second)
					queue_extras -= i.second;
				else
					// If entry is 0, erase it
					crafting_item_extras_.erase(recipe_item.first);

				inventory_c::add_itemstack_to_inv(inventory_player, inventory_size, i);
			}

			// Set crafting ticks remaining to the next item
			if (!crafting_queue_.empty())
				crafting_ticks_remaining_ = crafting_queue_.front()->crafting_time * 60;
		}
			// Crafting ticks remaining is greater, decrement ticks remaining
		else {
			crafting_ticks_remaining_ -= ticks;
			break;
		}
	}

}

void jactorio::game::Player_data::recipe_queue(data::Recipe* recipe) {
	assert(recipe != nullptr);  // Invalid recipe given

	LOG_MESSAGE_f(debug, "Queuing recipe: '%s'", recipe->get_product().first.c_str());

	// Remove ingredients
	for (auto& ingredient : recipe->ingredients) {
		auto* item = data::data_manager::data_raw_get<data::Item>(
			data::data_category::item, ingredient.first);

		inventory_c::remove_inv_item(inventory_player, inventory_size, item, ingredient.second);
	}

	// Queue is empty, crafting time for the first item in queue must be set here
	if (crafting_queue_.empty())
		crafting_ticks_remaining_ = recipe->crafting_time * 60;

	crafting_queue_.push_back(recipe);
}

const std::deque<jactorio::data::Recipe*>& jactorio::game::Player_data::get_recipe_queue() const {
	return crafting_queue_;
}

uint16_t jactorio::game::Player_data::get_crafting_ticks_remaining() const {
	return crafting_ticks_remaining_;
}

void jactorio::game::Player_data::recipe_craft_r(data::Recipe* recipe) {
	assert(recipe != nullptr);  // Invalid recipe given

	for (auto& ingredient : recipe->ingredients) {
		const auto ingredient_proto = data::data_manager::data_raw_get<data::Item>(
			data::data_category::item, ingredient.first);

		const uint32_t possess_amount =
			inventory_c::get_inv_item_count(inventory_player, inventory_size, ingredient_proto);

		// Insufficient ingredient amount in player inventory
		if (possess_amount < ingredient.second) {
			auto amount_needed = ingredient.second - possess_amount;
			// If there any items being queued to be returned to the player, do not return and instead
			// use it to craft
			auto& queued_available = crafting_item_extras_[ingredient.first];
			auto& return_deductions = crafting_item_deductions_[ingredient.first];

			if (amount_needed < queued_available) {
				// Available through what is queued
				queued_available -= possess_amount;
				return_deductions += possess_amount;  // Not available anymore, so deduct it after crafting

				LOG_MESSAGE_f(debug, "Increasing crafting deductions of '%s' by %d",
				              ingredient.first.c_str(), possess_amount);
			}
			else {
				// More than what is queued

				// Use what is available from queue
				amount_needed -= queued_available;

				// amount_needed: Deduct from returning to player inventory as it is used for crafting
				// queued_available: Queued available items not available anymore, so deduct it after crafting
				LOG_MESSAGE_f(debug, "Increasing crafting deductions of '%s' by %d",
				              ingredient.first.c_str(), amount_needed + queued_available);
				return_deductions += amount_needed + queued_available;


				auto* ingredient_recipe = data::Recipe::get_item_recipe(ingredient.first);

				// Round up to always ensure enough is crafted
				const unsigned int yield = ingredient_recipe->get_product().second;
				const auto batches = (amount_needed + yield - 1) / yield;

				// Keep track of excess amounts
				const auto excess = batches * yield - amount_needed;
				if (excess > 0)
					crafting_item_extras_[ingredient.first] += excess;
				else
					// All available from queue used up, delete entry
					crafting_item_extras_.erase(ingredient.first);

				// Craft sub-recipes recursively until met
				for (unsigned int i = 0; i < batches; ++i) {
					recipe_craft_r(ingredient_recipe);
				}
			}

		}
	}

	// Ingredients met - Queue crafting recipe
	recipe_queue(recipe);
}


bool jactorio::game::Player_data::recipe_can_craft_r(std::map<data::Item*, uint32_t>& used_items,
                                                     const data::Recipe* recipe, const uint16_t batches) {
	assert(recipe != nullptr);  // Invalid recipe given

	for (auto& ingredient : recipe->ingredients) {
		const auto ingredient_proto = data::data_manager::data_raw_get<data::Item>(
			data::data_category::item, ingredient.first);

		// If item has already been counted, use the map used_items. Otherwise, count from inventory
		uint32_t possess_amount;
		if (used_items.find(ingredient_proto) != used_items.end()) {
			possess_amount = used_items[ingredient_proto];
		}
		else {
			possess_amount = inventory_c::get_inv_item_count(inventory_player,
			                                                 inventory_size,
			                                                 ingredient_proto);
			used_items[ingredient_proto] = possess_amount;
		}

		// Ingredient met, subtract from used_items, check others
		if (possess_amount >= ingredient.second * batches) {
			used_items[ingredient_proto] -= ingredient.second * batches;
			continue;
		}

		const auto* ingredient_recipe = data::Recipe::get_item_recipe(ingredient.first);
		// Ingredient cannot be crafted
		if (ingredient_recipe == nullptr)
			return false;

		// Ingredient craftable but not met

		// Amount still needed to be crafted
		unsigned int ingredient_required_batches;
		{
			used_items[ingredient_proto] = 0;  // Use up amount available + craft to reach desired amount

			const unsigned int x = ingredient.second * batches - possess_amount;
			const unsigned int y = ingredient_recipe->get_product().second;
			// Round up to always ensure enough is crafted
			ingredient_required_batches = (x + y - 1) / y;
		}

		// Is able to craft desired amount of ingredient recursively?
		if (!recipe_can_craft_r(used_items, ingredient_recipe, ingredient_required_batches)) {
			return false;
		}
	}

	return true;

}

bool jactorio::game::Player_data::recipe_can_craft(const data::Recipe* recipe, const uint16_t batches) {
	std::map<data::Item*, uint32_t> used_items;
	return recipe_can_craft_r(used_items, recipe, batches);
}

// ============================================================================================
// TEST use only

void jactorio::game::Player_data::clear_player_inventory() {
	for (auto& i : inventory_player) {
		i.first = nullptr;
		i.second = 0;
	}
}

void jactorio::game::Player_data::reset_inventory_variables() {
	has_item_selected_ = false;
	select_by_reference_ = false;
}

std::map<std::string, uint16_t>& jactorio::game::Player_data::get_crafting_item_deductions() {
	return crafting_item_deductions_;
}

std::map<std::string, uint16_t>& jactorio::game::Player_data::get_crafting_item_extras() {
	return crafting_item_extras_;
}

void jactorio::game::Player_data::set_selected_item(data::item_stack& item) {
	has_item_selected_ = true;
	selected_item_ = item;
}
