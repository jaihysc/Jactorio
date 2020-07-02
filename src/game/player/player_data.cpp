// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#include "game/player/player_data.h"

#include <algorithm>
#include <cassert>
#include <map>

#include "data/data_manager.h"
#include "data/prototype/entity/resource_entity.h"
#include "data/prototype/interface/update_listener.h"
#include "data/prototype/tile/tile.h"
#include "game/input/mouse_selection.h"
#include "game/logic/inventory_controller.h"
#include "game/logic/placement_controller.h"
#include "game/world/world_data.h"
#include "renderer/opengl/shader_manager.h"
#include "renderer/rendering/renderer.h"

void jactorio::game::PlayerData::MouseCalculateSelectedTile() {
	float pixels_from_center_x;
	float pixels_from_center_y;
	{
		const unsigned short window_width  = renderer::Renderer::GetWindowWidth();
		const unsigned short window_height = renderer::Renderer::GetWindowHeight();
		const auto& matrix                 = renderer::GetMvpMatrix();

		// Account for MVP matrices
		// Normalize to -1 | 1 used by the matrix
		const double norm_x = 2 * (MouseSelection::GetCursorX() / window_width) - 1;
		const double norm_y = 2 * (MouseSelection::GetCursorY() / window_height) - 1;

		// A = C / B
		const glm::vec4 norm_positions = matrix / glm::vec4(norm_x, norm_y, 1, 1);


		float mouse_x_center;
		float mouse_y_center;
		{
			// Calculate the center tile on screen
			// Calculate number of pixels from center
			const double win_center_norm_x = 2 * (static_cast<double>(window_width) / 2 / window_width) - 1;
			const double win_center_norm_y = 2 * (static_cast<double>(window_height) / 2 / window_height) - 1;

			const glm::vec4 win_center_norm_positions =
				matrix / glm::vec4(win_center_norm_x, win_center_norm_y, 1, 1);

			mouse_x_center = win_center_norm_positions.x;
			mouse_y_center = win_center_norm_positions.y;
		}

		// If player is standing on a partial tile, adjust the center accordingly to the correct location
		mouse_x_center -=
			static_cast<float>(renderer::Renderer::tileWidth) *
			(playerPositionX_ - static_cast<float>(static_cast<int>(playerPositionX_)));

		// This is plus since the y axis is inverted
		mouse_y_center +=
			static_cast<float>(renderer::Renderer::tileWidth) *
			(playerPositionY_ - static_cast<float>(static_cast<int>(playerPositionY_)));


		pixels_from_center_x = norm_positions.x - mouse_x_center;
		pixels_from_center_y = mouse_y_center - norm_positions.y;
	}

	// Calculate tile position based on current player position
	float tile_x = static_cast<float>(static_cast<int>(playerPositionX_)) +
		pixels_from_center_x / static_cast<float>(renderer::Renderer::tileWidth);

	float tile_y = static_cast<float>(static_cast<int>(playerPositionY_)) +
		pixels_from_center_y / static_cast<float>(renderer::Renderer::tileWidth);

	// Subtract extra tile if negative because no tile exists at -0, -0
	if (tile_x < 0)
		tile_x -= 1.f;
	if (tile_y < 0)
		tile_y -= 1.f;

	mouseSelectedTile_ = std::pair<int, int>(tile_x, tile_y);
}

bool jactorio::game::PlayerData::MouseSelectedTileInRange() const {
	const auto cursor_position = GetMouseTileCoords();

	// Maximum distance of from the player where tiles can be reached
	constexpr unsigned int max_reach = 34;
	const auto tile_dist             =
		abs(playerPositionX_ - cursor_position.first) + abs(playerPositionY_ - cursor_position.second);

	return tile_dist <= max_reach;
}

bool jactorio::game::PlayerData::TargetTileValid(WorldData* world_data, const int x, const int y) const {
	assert(world_data != nullptr);  // Player is not in a world

	const auto* origin_tile =
		world_data->GetTile(
			static_cast<int>(playerPositionX_),
			static_cast<int>(playerPositionY_));

	if (origin_tile == nullptr)
		return false;

	// If the player is on water, they are allowed to walk on water
	if (origin_tile->GetTilePrototype(ChunkTile::ChunkLayer::base)->isWater)
		return true;

	const ChunkTile* tile = world_data->GetTile(x, y);
	// Chunk not generated yet
	if (tile == nullptr)
		return false;

	return !tile->GetTilePrototype(ChunkTile::ChunkLayer::base)->isWater;
}

void jactorio::game::PlayerData::MovePlayerX(const float amount) {
	const float target_x = playerPositionX_ + amount;

	if (TargetTileValid(playerWorld_, static_cast<int>(target_x), static_cast<int>(playerPositionY_)))
		playerPositionX_ = target_x;
}

void jactorio::game::PlayerData::MovePlayerY(const float amount) {
	const float target_y = playerPositionY_ + amount;

	if (TargetTileValid(playerWorld_, static_cast<int>(playerPositionX_), static_cast<int>(target_y)))
		playerPositionY_ = target_y;
}


// ============================================================================================
// Entity placement / pickup

void jactorio::game::PlayerData::RotatePlacementOrientation() {
	switch (placementOrientation) {
	case data::Orientation::up:
		placementOrientation = data::Orientation::right;
		break;
	case data::Orientation::right:
		placementOrientation = data::Orientation::down;
		break;
	case data::Orientation::down:
		placementOrientation = data::Orientation::left;
		break;
	case data::Orientation::left:
		placementOrientation = data::Orientation::up;
		break;
	default:
		assert(false);  // Missing switch case
	}
}

void jactorio::game::PlayerData::CounterRotatePlacementOrientation() {
	switch (placementOrientation) {
	case data::Orientation::up:
		placementOrientation = data::Orientation::left;
		break;
	case data::Orientation::left:
		placementOrientation = data::Orientation::down;
		break;
	case data::Orientation::down:
		placementOrientation = data::Orientation::right;
		break;
	case data::Orientation::right:
		placementOrientation = data::Orientation::up;
		break;
	default:
		assert(false);  // Missing switch case
	}
}

void CallOnNeighborUpdate(jactorio::game::WorldData& world_data,
                          const jactorio::game::WorldData::WorldPair emit_pair,
                          const jactorio::game::WorldData::WorldCoord world_x,
                          const jactorio::game::WorldData::WorldCoord world_y,
                          const jactorio::data::Orientation target_orientation) {
	using namespace jactorio;

	const game::ChunkTile* tile = world_data.GetTile(world_x, world_y);
	if (tile) {
		auto& layer = tile->GetLayer(game::ChunkTile::ChunkLayer::entity);

		const auto* entity = static_cast<const data::Entity*>(layer.prototypeData);
		if (entity)
			entity->OnNeighborUpdate(world_data,
			                         emit_pair,
			                         {world_x, world_y},
			                         target_orientation);
	}
}

void UpdateNeighboringEntities(jactorio::game::WorldData& world_data,
                               const jactorio::game::WorldData::WorldCoord world_x,
                               const jactorio::game::WorldData::WorldCoord world_y,
                               const jactorio::data::Entity* entity_ptr) {
	// Clockwise from top left

	/*
	 *     [1] [2]
	 * [A] [X] [x] [3]
	 * [9] [x] [x] [4]
	 * [8] [x] [x] [5]
	 *     [7] [6]
	 */
	using namespace jactorio;

	const game::WorldData::WorldPair emit_coords = {world_x, world_y};
	for (int x = world_x; x < world_x + entity_ptr->tileWidth; ++x) {
		CallOnNeighborUpdate(world_data,
		                     emit_coords,
		                     x,
		                     world_y - 1,
		                     data::Orientation::down);
	}
	for (int y = world_y; y < world_y + entity_ptr->tileHeight; ++y) {
		CallOnNeighborUpdate(world_data,
		                     emit_coords,
		                     world_x + entity_ptr->tileWidth,
		                     y,
		                     data::Orientation::left);
	}
	for (int x = world_x + entity_ptr->tileWidth - 1; x >= world_x; --x) {
		CallOnNeighborUpdate(world_data,
		                     emit_coords,
		                     x,
		                     world_y + entity_ptr->tileHeight,
		                     data::Orientation::up);
	}
	for (int y = world_y + entity_ptr->tileHeight - 1; y >= world_y; --y) {
		CallOnNeighborUpdate(world_data,
		                     emit_coords,
		                     world_x - 1,
		                     y,
		                     data::Orientation::right);
	}
}

void jactorio::game::PlayerData::TryPlaceEntity(WorldData& world_data,
                                                const int world_x, const int world_y,
                                                const bool can_activate_layer) {
	auto* tile = world_data.GetTile(world_x, world_y);
	if (tile == nullptr)
		return;

	auto& selected_layer = tile->GetLayer(ChunkTile::ChunkLayer::entity);

	const data::ItemStack* item = GetSelectedItem();
	// Ensure item attempting to place is an entity
	data::Entity* entity_ptr = nullptr;

	bool activate_selection = false;
	// No selected item or selected item is not placeable and clicked on a entity
	if (item == nullptr)
		activate_selection = true;
	else {
		entity_ptr = static_cast<data::Entity*>(item->first->entityPrototype);
		if (entity_ptr == nullptr || !entity_ptr->placeable)
			activate_selection = true;
	}

	// Activate the clicked entity / prototype. For example: show the gui
	if (activate_selection) {
		if (!can_activate_layer)
			return;

		// Since this is entity layer, everything is guaranteed to be an entity
		if (selected_layer.prototypeData) {
			// // If clicking again on the same entity, deactivate
			// if (activated_layer == &selected_layer)
			// 	activated_layer = nullptr;
			// else

			// Clicking on an existing entity will activate it
			activatedLayer_ = &selected_layer;
		}

		return;
	}


	assert(entity_ptr != nullptr);
	// Prototypes can perform additional checking on whether the location can be placed on or not
	if (!entity_ptr->OnCanBuild(world_data, {world_x, world_y}))
		return;

	// Do not take item away from player unless item was successfully placed
	if (!PlaceEntityAtCoords(world_data, entity_ptr, world_x, world_y))
		// Failed to place because an entity already exists
		return;


	// All validations passed, entity has been placed

	// If item stack was used up, sort player inventory to fill gap
	if (!DecrementSelectedItem()) {
		InventorySort();
	}

	// Call events

	entity_ptr->OnBuild(world_data, {world_x, world_y}, selected_layer, placementOrientation);
	UpdateNeighboringEntities(world_data, world_x, world_y, entity_ptr);
	world_data.updateDispatcher.Dispatch(world_x, world_y, data::UpdateType::place);
}


void jactorio::game::PlayerData::TryPickup(WorldData& world_data,
                                           const int tile_x, const int tile_y,
                                           const uint16_t ticks) {
	auto* tile = world_data.GetTile(tile_x, tile_y);

	const data::Entity* chosen_ptr;
	bool is_resource_ptr = true;
	{
		const auto* entity_ptr   = tile->GetEntityPrototype(ChunkTile::ChunkLayer::entity);
		const auto* resource_ptr = tile->GetEntityPrototype(ChunkTile::ChunkLayer::resource);

		// Picking up entities takes priority since it is higher on the layer
		if (entity_ptr != nullptr) {
			is_resource_ptr = false;
			chosen_ptr      = entity_ptr;
		}
		else if (resource_ptr != nullptr)
			chosen_ptr = resource_ptr;
		else
			// No valid pointers
			return;
	}

	// Selecting a new tile different from the last selected tile will reset the counter
	if (lastSelectedPtr_ != chosen_ptr || lastTilePtr_ != tile) {
		pickupTickCounter_ = 0;
		pickupTickTarget_  = static_cast<uint16_t>(chosen_ptr->pickupTime * JC_GAME_HERTZ);  // Seconds to ticks
	}
	// Remember the entity + tile which was selected
	lastSelectedPtr_ = chosen_ptr;
	lastTilePtr_     = tile;

	pickupTickCounter_ += ticks;
	if (pickupTickCounter_ >= pickupTickTarget_) {
		// Entity picked up
		LOG_MESSAGE(debug, "Player picked up entity");

		// Give picked up item to player
		const auto item_stack = data::ItemStack(chosen_ptr->GetItem(), 1);

		// Failed to add item, likely because the inventory is full
		if (!CanAddStack(inventoryPlayer, kInventorySize, item_stack))
			return;

		AddStack(inventoryPlayer, kInventorySize, item_stack);
		InventorySort();

		pickupTickCounter_ = 0;
		// Resource entity
		if (is_resource_ptr) {
			auto& layer         = tile->GetLayer(ChunkTile::ChunkLayer::resource);
			auto* resource_data = layer.GetUniqueData<data::ResourceEntityData>();

			assert(resource_data != nullptr);  // Resource tiles should have valid data

			// Delete resource tile if it is empty after extracting
			if (--resource_data->resourceAmount == 0) {
				layer.Clear();
			}
		}
			// Is normal entity
		else {
			auto& layer = tile->GetLayer(ChunkTile::ChunkLayer::entity);

			// Picking up an entity which is set in activated_layer will unset activated_layer
			if (activatedLayer_ == &layer)
				activatedLayer_ = nullptr;

			// Call events
			const auto* entity = static_cast<const data::Entity*>(layer.prototypeData);

			entity->OnRemove(world_data, {tile_x, tile_y}, layer);

			const bool result = PlaceEntityAtCoords(world_data, nullptr, tile_x, tile_y);
			assert(result);  // false indicates failed to remove entity

			UpdateNeighboringEntities(world_data, tile_x, tile_y, entity);
			world_data.updateDispatcher.Dispatch(tile_x, tile_y, data::UpdateType::remove);
		}
	}
}

float jactorio::game::PlayerData::GetPickupPercentage() const {
	if (lastSelectedPtr_ == nullptr)  // Not initialized yet
		return 0.f;

	return static_cast<float>(pickupTickCounter_) / static_cast<float>(pickupTickTarget_);
}

// ============================================================================================
// Inventory

void jactorio::game::PlayerData::InventorySort() {
	// The inventory must be sorted without moving the selected cursor

	LOG_MESSAGE(debug, "Sorting player inventory");

	// Copy non-cursor into a new array, sort it, copy it back minding the selection cursor
	std::vector<data::ItemStack> inv_temp;
	inv_temp.reserve(kInventorySize);
	for (const auto& i : inventoryPlayer) {
		// Skip the cursor
		if (i.first == nullptr ||
			i.first->GetLocalizedName() == kInventorySelectedCursorIname) {
			continue;
		}

		inv_temp.push_back(i);
	}

	// Sort temp inventory (does not contain cursor)
	std::sort(inv_temp.begin(), inv_temp.end(),
	          [](const data::ItemStack a, const data::ItemStack b) {
		          const auto& a_name = a.first->GetLocalizedName();
		          const auto& b_name = b.first->GetLocalizedName();

		          return a_name < b_name;
	          });

	// Compress item stacks
	for (int64_t i = inv_temp.size() - 1; i >= 0; --i) {
		uint16_t buffer_item_count = inv_temp[i].second;
		const uint16_t stack_size  = inv_temp[i].first->stackSize;

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
	int start                   = -1;  // The index of the first blank slot post sorting
	unsigned int inv_temp_index = 0;
	for (auto i = 0; i < kInventorySize; ++i) {
		// Skip the cursor
		if (inventoryPlayer[i].first != nullptr &&
			inventoryPlayer[i].first->GetLocalizedName() == kInventorySelectedCursorIname)
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

		inventoryPlayer[i] = inv_temp[inv_temp_index++];
	}
loop_exit:

	if (start == -1)  // Start being -1 means that there is no empty slots
		return;

	// Copy empty spaces into the remainder of the slots
	for (auto i = start; i < kInventorySize; ++i) {
		// Skip the cursor
		if (inventoryPlayer[i].first != nullptr &&
			inventoryPlayer[i].first->GetLocalizedName() == kInventorySelectedCursorIname)
			continue;

		inventoryPlayer[i] = {nullptr, 0};
	}
}

// LEFT CLICK - Select by reference, the item in the cursor mirrors the inventory item
// RIGHT CLICK - Select unique, the item in the cursor exists independently of the inventory item

void jactorio::game::PlayerData::InventoryClick(const data::DataManager& data_manager,
                                                const unsigned short index,
                                                const unsigned short mouse_button,
                                                const bool allow_reference_select,
                                                data::ItemStack* inv) {
	assert(index < kInventorySize);
	assert(mouse_button == 0 || mouse_button == 1);  // Only left + right click supported

	// Clicking on the same location + inventory, selecting by reference: deselect
	if (hasItemSelected_ && selectByReference_ &&
		selectedItemIndex_ == index && selectedItemInventory_ == inv) {
		hasItemSelected_ = false;

		// Add referenced item to slot
		inv[selectedItemIndex_] = selectedItem_;
		return;
	}


	// Selection mode can only be set upon first item selection
	if (!hasItemSelected_) {
		// Clicking empty slot
		if (inv[index].first == nullptr)
			return;

		hasItemSelected_       = true;
		selectedItemIndex_     = index;
		selectedItemInventory_ = inv;

		// Reference
		if (allow_reference_select && mouse_button == 0) {
			assert(inv == inventoryPlayer);  // Select by reference only allowed for player inventory
			selectByReference_ = true;
			selectedItem_      = inv[index];

			// Swap icon out for a cursor indicating the current index is selected
			inventoryPlayer[index].first = data_manager.DataRawGet<data::Item>(data::DataCategory::item,
			                                                                   kInventorySelectedCursorIname);
			inventoryPlayer[index].second = 0;

			// Return is necessary when selecting by reference
			// The item needs to be moved after selecting the next inventory slot
			return;
		}

		// Unique
		selectByReference_ = false;

		// Clear the cursor inventory so half can be moved into it
		selectedItem_.first  = nullptr;
		selectedItem_.second = 0;
		// DO NOT return for it to move the item into the new inventory
	}

	const bool cursor_empty =
		MoveItemstackToIndex(&selectedItem_, 0,
		                     inv, index,
		                     mouse_button);
	// Cursor slot is empty
	if (cursor_empty) {
		hasItemSelected_ = false;

		if (selectByReference_) {
			// Remove cursor icon
			assert(selectedItemIndex_ < kInventorySize);
			// Select by reference is only for inventory_player
			inventoryPlayer[selectedItemIndex_].first  = nullptr;
			inventoryPlayer[selectedItemIndex_].second = 0;
		}
	}
}

const jactorio::data::ItemStack* jactorio::game::PlayerData::GetSelectedItem() const {
	if (!hasItemSelected_)
		return nullptr;

	return &selectedItem_;
}

bool jactorio::game::PlayerData::DeselectSelectedItem() {
	if (!hasItemSelected_ || !selectByReference_)
		return false;

	// Add referenced item to slot
	inventoryPlayer[selectedItemIndex_] = selectedItem_;
	hasItemSelected_                    = false;
	return true;
}

bool jactorio::game::PlayerData::IncrementSelectedItem() {
	assert(hasItemSelected_);

	// DO not increment if it will exceed the stack size
	if (selectedItem_.second < selectedItem_.first->stackSize) {
		selectedItem_.second++;
		return true;
	}

	return false;
}

bool jactorio::game::PlayerData::DecrementSelectedItem() {
	assert(hasItemSelected_);

	if (--selectedItem_.second == 0) {
		// Item stack now empty
		hasItemSelected_ = false;
		// Remove selection cursor
		inventoryPlayer[selectedItemIndex_].first  = nullptr;
		inventoryPlayer[selectedItemIndex_].second = 0;

		return false;
	}
	return true;
}

// ============================================================================================
// Recipe

void jactorio::game::PlayerData::RecipeGroupSelect(const uint16_t index) {
	selectedRecipeGroup_ = index;
}

uint16_t jactorio::game::PlayerData::RecipeGroupGetSelected() const {
	return selectedRecipeGroup_;
}

void jactorio::game::PlayerData::RecipeCraftTick(const data::DataManager& data_manager, uint16_t ticks) {
	// Attempt to return held item if inventory is full
	if (craftingHeldItem_.second != 0) {
		const auto extra_items   = AddStack(inventoryPlayer, kInventorySize, craftingHeldItem_);
		craftingHeldItem_.second = extra_items;
		return;
	}


	while (ticks != 0 && !craftingQueue_.empty()) {
		// Ticks available greater than or equal to crafting ticks remaining
		if (ticks >= craftingTicksRemaining_) {
			ticks -= craftingTicksRemaining_;

			const auto* recipe = craftingQueue_.front();
			craftingQueue_.pop_front();

			// Return product
			data::RecipeItem recipe_item = recipe->GetProduct();
			const auto* product_item     = data_manager.DataRawGet<data::Item>(data::DataCategory::item, recipe_item.first);

			data::ItemStack item = {product_item, recipe_item.second};

			// Deduct based on the deductions
			std::map<std::string, uint16_t>::iterator element;
			if ((element = craftingItemDeductions_.find(recipe_item.first)) != craftingItemDeductions_.end()) {
				auto& deduct_amount = element->second;

				if (item.second >= deduct_amount) {
					item.second -= deduct_amount;

					LOG_MESSAGE_F(debug, "Crafting return deducting %d of '%s'",
					              deduct_amount, recipe_item.first.c_str());

					craftingItemDeductions_.erase(recipe_item.first);  // Now empty
				}
					// Deduct amount greater than i.second
				else {
					deduct_amount -= item.second;
					item.second = 0;

					LOG_MESSAGE_F(debug, "Crafting return deducting %d of '%s', no items returned",
					              deduct_amount, recipe_item.first.c_str());
				}
			}

			// Still has items to return to player inventory
			if (item.second != 0) {
				// Extra not available in queue anymore since it has been returned to the player
				auto& queue_extras = craftingItemExtras_[recipe_item.first];
				if (queue_extras > item.second)
					queue_extras -= item.second;
				else
					// If entry is 0, erase it
					craftingItemExtras_.erase(recipe_item.first);

				const auto extra_items = AddStack(inventoryPlayer, kInventorySize, item);
				if (extra_items != 0)
					craftingHeldItem_ = {item.first, extra_items};
			}

			// Set crafting ticks remaining to the next item
			if (!craftingQueue_.empty())
				craftingTicksRemaining_ = static_cast<uint16_t>(craftingQueue_.front()->craftingTime * JC_GAME_HERTZ);
		}
			// Crafting ticks remaining is greater, decrement ticks remaining
		else {
			craftingTicksRemaining_ -= ticks;
			break;
		}
	}

}

void jactorio::game::PlayerData::RecipeQueue(const data::DataManager& data_manager, const data::Recipe& recipe) {
	LOG_MESSAGE_F(debug, "Queuing recipe: '%s'", recipe.GetProduct().first.c_str());

	// Remove ingredients
	for (const auto& ingredient : recipe.ingredients) {
		const auto* item = data_manager.DataRawGet<data::Item>(data::DataCategory::item, ingredient.first);

		DeleteInvItem(inventoryPlayer, kInventorySize, item, ingredient.second);
	}

	// Queue is empty, crafting time for the first item in queue must be set here
	if (craftingQueue_.empty())
		craftingTicksRemaining_ = static_cast<uint16_t>(recipe.craftingTime * JC_GAME_HERTZ);

	craftingQueue_.push_back(&recipe);
}

const std::deque<const jactorio::data::Recipe*>& jactorio::game::PlayerData::GetRecipeQueue() const {
	return craftingQueue_;
}

uint16_t jactorio::game::PlayerData::GetCraftingTicksRemaining() const {
	return craftingTicksRemaining_;
}

void jactorio::game::PlayerData::RecipeCraftR(const data::DataManager& data_manager, const data::Recipe& recipe) {
	for (const auto& ingredient : recipe.ingredients) {
		const auto* ingredient_proto = data_manager.DataRawGet<data::Item>(
			data::DataCategory::item, ingredient.first);

		const uint32_t possess_amount =
			GetInvItemCount(inventoryPlayer, kInventorySize, ingredient_proto);

		// Insufficient ingredient amount in player inventory
		if (possess_amount < ingredient.second) {
			auto amount_needed = ingredient.second - possess_amount;
			// If there any items being queued to be returned to the player, do not return and instead
			// use it to craft
			auto& queued_available  = craftingItemExtras_[ingredient.first];
			auto& return_deductions = craftingItemDeductions_[ingredient.first];

			if (amount_needed < queued_available) {
				// Available through what is queued
				queued_available -= possess_amount;
				return_deductions += possess_amount;  // Not available anymore, so deduct it after crafting

				LOG_MESSAGE_F(debug, "Increasing crafting deductions of '%s' by %d",
				              ingredient.first.c_str(), possess_amount);
			}
			else {
				// More than what is queued

				// Use what is available from queue
				amount_needed -= queued_available;

				// amount_needed: Deduct from returning to player inventory as it is used for crafting
				// queued_available: Queued available items not available anymore, so deduct it after crafting
				LOG_MESSAGE_F(debug, "Increasing crafting deductions of '%s' by %d",
				              ingredient.first.c_str(), amount_needed + queued_available);
				return_deductions += amount_needed + queued_available;


				auto* ingredient_recipe = data::Recipe::GetItemRecipe(ingredient.first);

				// Round up to always ensure enough is crafted
				const unsigned int yield = ingredient_recipe->GetProduct().second;
				const auto batches       = (amount_needed + yield - 1) / yield;

				// Keep track of excess amounts
				const auto excess = batches * yield - amount_needed;
				if (excess > 0)
					craftingItemExtras_[ingredient.first] += excess;
				else
					// All available from queue used up, delete entry
					craftingItemExtras_.erase(ingredient.first);

				// Craft sub-recipes recursively until met
				for (unsigned int i = 0; i < batches; ++i) {
					assert(ingredient_recipe);
					RecipeCraftR(data_manager, *ingredient_recipe);
				}
			}

		}
	}

	// Ingredients met - Queue crafting recipe
	RecipeQueue(data_manager, recipe);
}


bool jactorio::game::PlayerData::RecipeCanCraftR(const data::DataManager& data_manager,
                                                 std::map<const data::Item*, uint32_t>& used_items,
                                                 const data::Recipe& recipe, const uint16_t batches) const {
	for (const auto& ingredient : recipe.ingredients) {
		const auto* ingredient_proto = data_manager.DataRawGet<data::Item>(
			data::DataCategory::item, ingredient.first);

		// If item has already been counted, use the map used_items. Otherwise, count from inventory
		unsigned int possess_amount;
		if (used_items.find(ingredient_proto) != used_items.end()) {
			possess_amount = used_items[ingredient_proto];
		}
		else {
			possess_amount = GetInvItemCount(inventoryPlayer,
			                                 kInventorySize,
			                                 ingredient_proto);
			used_items[ingredient_proto] = possess_amount;
		}

		// Ingredient met, subtract from used_items, check others
		if (possess_amount >= ingredient.second * batches) {
			used_items[ingredient_proto] -= ingredient.second * batches;
			continue;
		}

		const auto* ingredient_recipe = data::Recipe::GetItemRecipe(ingredient.first);
		// Ingredient cannot be crafted
		if (ingredient_recipe == nullptr)
			return false;

		// Ingredient craftable but not met

		// Amount still needed to be crafted
		unsigned int ingredient_required_batches;
		{
			used_items[ingredient_proto] = 0;  // Use up amount available + craft to reach desired amount

			const unsigned int x = ingredient.second * batches - possess_amount;
			const unsigned int y = ingredient_recipe->GetProduct().second;
			// Round up to always ensure enough is crafted
			ingredient_required_batches = (x + y - 1) / y;
		}

		// Is able to craft desired amount of ingredient recursively?
		assert(ingredient_recipe);
		if (!RecipeCanCraftR(data_manager, used_items, *ingredient_recipe, ingredient_required_batches)) {
			return false;
		}
	}

	return true;

}

bool jactorio::game::PlayerData::RecipeCanCraft(const data::DataManager& data_manager, const data::Recipe& recipe,
                                                const uint16_t batches) const {
	std::map<const data::Item*, uint32_t> used_items;
	return RecipeCanCraftR(data_manager, used_items, recipe, batches);
}
