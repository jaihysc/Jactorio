// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/player/player_data.h"

#include <algorithm>
#include <map>

#include "data/prototype/recipe.h"
#include "data/prototype/resource_entity.h"
#include "data/prototype/tile.h"
#include "game/input/mouse_selection.h"
#include "game/logic/inventory_controller.h"
#include "game/logic/placement_controller.h"
#include "game/world/world_data.h"
#include "renderer/rendering/renderer.h"

using namespace jactorio;

void game::PlayerData::World::MouseCalculateSelectedTile(const glm::mat4& mvp_matrix) {
    const auto truncated_player_pos_x = core::SafeCast<float>(core::LossyCast<int>(playerPositionX_));
    const auto truncated_player_pos_y = core::SafeCast<float>(core::LossyCast<int>(playerPositionY_));

    float pixels_from_center_x;
    float pixels_from_center_y;
    {
        const auto window_width  = renderer::Renderer::GetWindowWidth();
        const auto window_height = renderer::Renderer::GetWindowHeight();

        // Account for MVP matrices
        // Normalize to -1 | 1 used by the matrix
        const double norm_x = 2 * (MouseSelection::GetCursorX() / window_width) - 1;
        const double norm_y = 2 * (MouseSelection::GetCursorY() / window_height) - 1;

        // A = C / B
        const glm::vec4 norm_positions = mvp_matrix / glm::vec4(norm_x, norm_y, 1, 1);


        float mouse_x_center;
        float mouse_y_center;
        {
            // Calculate the center tile on screen
            // Calculate number of pixels from center
            const double win_center_norm_x = 2 * (core::SafeCast<double>(window_width) / 2 / window_width) - 1;
            const double win_center_norm_y = 2 * (core::SafeCast<double>(window_height) / 2 / window_height) - 1;

            const glm::vec4 win_center_norm_positions =
                mvp_matrix / glm::vec4(win_center_norm_x, win_center_norm_y, 1, 1);

            mouse_x_center = win_center_norm_positions.x;
            mouse_y_center = win_center_norm_positions.y;
        }

        // If player is standing on a partial tile, adjust the center accordingly to the correct location
        mouse_x_center -=
            core::SafeCast<float>(renderer::Renderer::tileWidth) * (playerPositionX_ - truncated_player_pos_x);

        // This is plus since the y axis is inverted
        mouse_y_center +=
            core::SafeCast<float>(renderer::Renderer::tileWidth) * (playerPositionY_ - truncated_player_pos_y);


        pixels_from_center_x = norm_positions.x - mouse_x_center;
        pixels_from_center_y = mouse_y_center - norm_positions.y;
    }

    // Calculate tile position based on current player position
	float tile_x = truncated_player_pos_x + pixels_from_center_x / core::LossyCast<float>(renderer::Renderer::tileWidth);

	float tile_y = truncated_player_pos_y + pixels_from_center_y / core::LossyCast<float>(renderer::Renderer::tileWidth);

	// Subtract extra tile if negative because no tile exists at -0, -0
	if (tile_x < 0)
		tile_x -= 1.f;
	if (tile_y < 0)
		tile_y -= 1.f;

	mouseSelectedTile_ = {core::LossyCast<WorldCoordAxis>(tile_x), core::LossyCast<WorldCoordAxis>(tile_y)};
}

bool game::PlayerData::World::MouseSelectedTileInRange() const {
    const auto cursor_position = GetMouseTileCoords();

    // Maximum distance of from the player where tiles can be reached
    constexpr unsigned int max_reach = 34;
    const auto tile_dist             = abs(playerPositionX_ - core::SafeCast<float>(cursor_position.x)) +
        abs(playerPositionY_ - core::SafeCast<float>(cursor_position.y));

    return tile_dist <= max_reach;
}

bool game::PlayerData::World::TargetTileValid(WorldData* world_data, const int x, const int y) const {
    assert(world_data != nullptr); // Player is not in a world

    const auto* origin_tile =
        world_data->GetTile(core::LossyCast<int>(playerPositionX_), core::LossyCast<int>(playerPositionY_));

    if (origin_tile == nullptr)
        return false;

    // If the player is on water, they are allowed to walk on water
    if (origin_tile->GetTilePrototype()->isWater)
        return true;

    const ChunkTile* tile = world_data->GetTile(x, y);
    // Chunk not generated yet
    if (tile == nullptr)
        return false;

    return !tile->GetTilePrototype()->isWater;
}

void game::PlayerData::World::MovePlayerX(const float amount) {
    const float target_x = playerPositionX_ + amount;

//    if (TargetTileValid(&GetWorld(), core::LossyCast<int>(target_x), core::LossyCast<int>(playerPositionY_)))
        playerPositionX_ = target_x;
}

void game::PlayerData::World::MovePlayerY(const float amount) {
    const float target_y = playerPositionY_ + amount;

//    if (TargetTileValid(&GetWorld(), core::LossyCast<int>(playerPositionX_), core::LossyCast<int>(target_y)))
        playerPositionY_ = target_y;
}


// ============================================================================================
// Entity placement / pickup

void game::PlayerData::Placement::RotateOrientation() {
    switch (orientation) {
    case data::Orientation::up:
        orientation = data::Orientation::right;
        break;
    case data::Orientation::right:
        orientation = data::Orientation::down;
        break;
    case data::Orientation::down:
        orientation = data::Orientation::left;
        break;
    case data::Orientation::left:
        orientation = data::Orientation::up;
        break;
    default:
        assert(false); // Missing switch case
        orientation = data::Orientation::up;
    }
}

void game::PlayerData::Placement::CounterRotateOrientation() {
    switch (orientation) {
    case data::Orientation::up:
        orientation = data::Orientation::left;
        break;
    case data::Orientation::left:
        orientation = data::Orientation::down;
        break;
    case data::Orientation::down:
        orientation = data::Orientation::right;
        break;
    case data::Orientation::right:
        orientation = data::Orientation::up;
        break;
    default:
        assert(false); // Missing switch case
        orientation = data::Orientation::up;
    }
}


///
/// \param world_coord Top left tile x, y
void UpdateNeighboringEntities(game::WorldData& world_data,
                               game::LogicData& logic_data,
                               const WorldCoord& world_coord,
                               const data::Entity* entity_ptr) {

    auto call_on_neighbor_update = [&](const WorldCoordAxis emit_x,
                                       const WorldCoordAxis emit_y,
                                       const WorldCoordAxis receive_x,
                                       const WorldCoordAxis receive_y,
                                       const data::Orientation target_orientation) {
        const game::ChunkTile* tile = world_data.GetTile(receive_x, receive_y);
        if (tile != nullptr) {
            const auto& layer = tile->GetLayer(game::TileLayer::entity);

            const auto* entity = static_cast<const data::Entity*>(layer.prototypeData.Get());
            if (entity != nullptr)
                entity->OnNeighborUpdate(
                    world_data, logic_data, {emit_x, emit_y}, {receive_x, receive_y}, target_orientation);
        }
    };

    // Clockwise from top left

	/*
	 *     [1] [2]
	 * [A] [X] [x] [3]
	 * [9] [x] [x] [4]
	 * [8] [x] [x] [5]
	 *     [7] [6]
	 */

	// x and y are receive coordinates
	for (data::ProtoUintT i = 0; i < entity_ptr->tileWidth; ++i) {
		const auto x = world_coord.x + core::SafeCast<WorldCoordAxis>(i);
		const auto y = world_coord.y - 1;

		call_on_neighbor_update(x, y + 1,
		                        x, y,
		                        data::Orientation::down);
	}
	for (data::ProtoUintT i = 0; i < entity_ptr->tileHeight; ++i) {
		const auto x = world_coord.x + core::SafeCast<WorldCoordAxis>(entity_ptr->tileWidth);
		const auto y = world_coord.y + core::SafeCast<WorldCoordAxis>(i);

		call_on_neighbor_update(x - 1, y,
		                        x, y,
		                        data::Orientation::left);
	}
	for (data::ProtoUintT i = 1; i <= entity_ptr->tileWidth; ++i) {
		const auto x = world_coord.x + core::SafeCast<WorldCoordAxis>(entity_ptr->tileWidth) - core::SafeCast<WorldCoordAxis>(i);
		const auto y = world_coord.y + core::SafeCast<WorldCoordAxis>(entity_ptr->tileHeight);

		call_on_neighbor_update(x, y - 1,
		                        x, y,
		                        data::Orientation::up);
	}
	for (data::ProtoUintT i = 1; i <= entity_ptr->tileHeight; ++i) {
		const auto x = world_coord.x - 1;
		const auto y = world_coord.y + core::SafeCast<WorldCoordAxis>(entity_ptr->tileHeight) - core::SafeCast<WorldCoordAxis>(i);

		call_on_neighbor_update(x + 1, y,
		                        x, y,
		                        data::Orientation::right);
	}
}

bool game::PlayerData::Placement::TryPlaceEntity(WorldData& world_data,
                                                 LogicData& logic_data,
                                                 const WorldCoordAxis world_x,
                                                 const WorldCoordAxis world_y) {
    auto* tile = world_data.GetTile(world_x, world_y);
    if (tile == nullptr)
        return false;

    const auto* stack = playerInv_.GetSelectedItemStack();

    // No selected item or selected item is not placeable and clicked on a entity
    if (stack == nullptr)
        return false;

    // Does not have item, or placeable item
    auto* entity_ptr = static_cast<data::Entity*>(stack->item->entityPrototype);
    if (entity_ptr == nullptr || !entity_ptr->placeable)
        return false;


    assert(entity_ptr != nullptr);
    // Prototypes can perform additional checking on whether the location can be placed on or not
    if (!entity_ptr->OnCanBuild(world_data, {world_x, world_y}))
        return false;

    // Do not take item away from player unless item was successfully placed
    if (!PlaceEntityAtCoords(world_data, entity_ptr, world_x, world_y))
        // Failed to place because an entity already exists
        return false;


    // All validations passed, entity has been placed

    // If item stack was used up, sort player inventory to fill gap
    if (!playerInv_.DecrementSelectedItem()) {
        playerInv_.InventorySort(playerInv_.inventoryPlayer);
    }

    // Call events

    auto& selected_layer = tile->GetLayer(TileLayer::entity);

    entity_ptr->OnBuild(world_data, logic_data, {world_x, world_y}, selected_layer, orientation);
    UpdateNeighboringEntities(world_data, logic_data, {world_x, world_y}, entity_ptr);
    world_data.UpdateDispatch(world_x, world_y, data::UpdateType::place);

    return true;
}

bool game::PlayerData::Placement::TryActivateLayer(WorldData& world_data, const WorldCoord& world_pair) {
    auto* tile = world_data.GetTile(world_pair);
    if (tile == nullptr)
        return false;

    const auto* stack = playerInv_.GetSelectedItemStack();

    // Can activate if: No selected item OR selected item is not placeable
    if (stack != nullptr) {
        // Ensure item attempting to place is an entity
        auto* entity_ptr = static_cast<data::Entity*>(stack->item->entityPrototype);

        if (entity_ptr != nullptr && entity_ptr->placeable)
            return false;
    }

    // Activate the clicked entity / prototype. For example: show the gui
    // Since this is entity layer, everything is guaranteed to be an entity

    constexpr auto select_layer = TileLayer::entity;

    auto& selected_layer = tile->GetLayer(select_layer);
    if (selected_layer.prototypeData.Get() == nullptr)
        return false;

    // // If clicking again on the same entity, deactivate
    // if (activated_layer == &selected_layer)
    // 	activated_layer = nullptr;
    // else

    // Clicking on an existing entity will activate it
    activatedLayer_ = world_data.GetLayerTopLeft(world_pair, select_layer);
    return true;
}


void game::PlayerData::Placement::TryPickup(
    WorldData& world_data, LogicData& logic_data, WorldCoordAxis tile_x, WorldCoordAxis tile_y, const uint16_t ticks) {
    auto* tile = world_data.GetTile(tile_x, tile_y);

    const data::Entity* chosen_ptr;
    bool is_resource_ptr = true;
    {
        const auto* entity_ptr   = tile->GetEntityPrototype();
        const auto* resource_ptr = tile->GetEntityPrototype(TileLayer::resource);

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
        pickupTickTarget_  = core::LossyCast<uint16_t>(chosen_ptr->pickupTime * kGameHertz); // Seconds to ticks
    }
    // Remember the entity + tile which was selected
    lastSelectedPtr_ = chosen_ptr;
    lastTilePtr_     = tile;

    pickupTickCounter_ += ticks;
    if (pickupTickCounter_ >= pickupTickTarget_) {
        // Entity picked up
        LOG_MESSAGE(debug, "Player picked up entity");

        // Give picked up item to player
        const auto item_stack = data::ItemStack{chosen_ptr->GetItem(), 1};

        // Failed to add item, likely because the inventory is full
        if (!CanAddStack(playerInv_.inventoryPlayer, item_stack).first)
            return;

        AddStack(playerInv_.inventoryPlayer, item_stack);
        playerInv_.InventorySort(playerInv_.inventoryPlayer);

        pickupTickCounter_ = 0;
        // Resource entity
        if (is_resource_ptr) {
            auto& layer         = tile->GetLayer(TileLayer::resource);
            auto* resource_data = layer.GetUniqueData<data::ResourceEntityData>();

            assert(resource_data != nullptr); // Resource tiles should have valid data

            // Delete resource tile if it is empty after extracting
            if (--resource_data->resourceAmount == 0) {
                layer.Clear();
            }
        }
        // Is normal entity
        else {
            constexpr auto select_layer = TileLayer::entity;

            auto& layer = tile->GetLayer(select_layer);

            // User may have hovered on another tile other than the top left
            auto tl_tile_x = tile_x;
            auto tl_tile_y = tile_y;
            layer.AdjustToTopLeft(tl_tile_x, tl_tile_y);


            // Picking up an entity which is set in activated_layer will unset activated_layer
            if (activatedLayer_ == world_data.GetLayerTopLeft({tile_x, tile_y}, select_layer))
                activatedLayer_ = nullptr;

            // Call events
            const auto* entity = static_cast<const data::Entity*>(layer.prototypeData.Get());

            entity->OnRemove(world_data, logic_data, {tile_x, tile_y}, layer);

            const bool result = PlaceEntityAtCoords(world_data, nullptr, tile_x, tile_y);
            assert(result); // false indicates failed to remove entity

            UpdateNeighboringEntities(world_data, logic_data, {tl_tile_x, tl_tile_y}, entity);

			world_data.UpdateDispatch(tile_x, tile_y, data::UpdateType::remove);
		}
	}
}

float game::PlayerData::Placement::GetPickupPercentage() const {
    if (lastSelectedPtr_ == nullptr) // Not initialized yet
        return 0.f;

    return core::SafeCast<float>(pickupTickCounter_) / core::SafeCast<float>(pickupTickTarget_);
}

// ============================================================================================
// Inventory

void game::PlayerData::Inventory::HandleInventoryActions(const data::PrototypeManager& data_manager,
                                                         data::Item::Inventory& inv,
                                                         const size_t index,
                                                         const bool half_select) {
    const bool is_player_inv = &inv == &inventoryPlayer;


    InventoryClick(data_manager, core::SafeCast<uint16_t>(index), half_select ? 1 : 0, is_player_inv, inv);
    InventorySort(inventoryPlayer);
}

void game::PlayerData::Inventory::InventorySort(data::Item::Inventory& inv) {
    // The inventory must be sorted without moving the selected cursor

    // Copy non-cursor into a new array, sort it, copy it back minding the selection cursor
    std::vector<data::ItemStack> sorted_inv;
    sorted_inv.reserve(inv.size());
    for (const auto& stack : inv) {
        // Skip the cursor
        if (stack.item == nullptr || stack.item->GetLocalizedName() == data::Item::kInventorySelectedCursor) {
            continue;
        }

        sorted_inv.push_back(stack);
    }

    // Sort temp inventory (does not contain cursor)
    std::sort(sorted_inv.begin(), sorted_inv.end(), [](const data::ItemStack a, const data::ItemStack b) {
        const auto& a_name = a.item->GetLocalizedName();
        const auto& b_name = b.item->GetLocalizedName();

        return a_name < b_name;
    });

    // Compress item stacks
    if (sorted_inv.size() > 1) {
        // Cannot compress slot 0 with anything before it
        for (auto sort_inv_index = sorted_inv.size() - 1; sort_inv_index > 0; --sort_inv_index) {

            auto buffer_item_count = sorted_inv[sort_inv_index].count;
            const auto stack_size  = sorted_inv[sort_inv_index].item->stackSize;

            // Find index which the same item type begins
            auto stack_compress_begin = sort_inv_index;
            while (sorted_inv[stack_compress_begin - 1].item == sorted_inv[sort_inv_index].item) {
                stack_compress_begin--;
                if (stack_compress_begin == 0)
                    break;
            }


            // Compress stacks
            for (; stack_compress_begin < sort_inv_index; ++stack_compress_begin) {

                // If item somehow exceeds stack do not attempt to stack into it
                if (sorted_inv[stack_compress_begin].count > stack_size)
                    continue;

                // Amount which can be dropped is space left until reaching stack size
                const uint16_t max_drop_amount = stack_size - sorted_inv[stack_compress_begin].count;

				// Has enough to max current stack and move on
				if (buffer_item_count > max_drop_amount) {
					sorted_inv[stack_compress_begin].count = stack_size;
					buffer_item_count -= max_drop_amount;
				}
					// Not enough to drop and move on
				else {
					sorted_inv[stack_compress_begin].count += buffer_item_count;
					sorted_inv[sort_inv_index].item = nullptr;
					buffer_item_count               = 0;
					break;
				}
			}
			// Did not drop all items off
			if (buffer_item_count > 0) {
				sorted_inv[sort_inv_index].count = buffer_item_count;
			}

		}
	}


	// Copy sorted inventory back into origin inventory
	bool has_empty_slot = false;

	std::size_t start          = 0;   // The index of the first blank slot post sorting
	std::size_t inv_temp_index = 0;

	for (size_t i = 0; i < inv.size(); ++i) {
		// Skip the cursor
		if (inv[i].item != nullptr &&
			inv[i].item->GetLocalizedName() == data::Item::kInventorySelectedCursor)
			continue;

		// Iterated through every item in the sorted inventory
		if (inv_temp_index >= sorted_inv.size()) {
			has_empty_slot = true;
			start          = i;
			goto loop_exit;
		}
		// Omit empty gaps in sorted inv from the compressing process
		while (sorted_inv[inv_temp_index].item == nullptr) {
			inv_temp_index++;
			if (inv_temp_index >= sorted_inv.size()) {
				has_empty_slot = true;
				start          = i;
				goto loop_exit;
			}
		}

		inv[i] = sorted_inv[inv_temp_index++];
	}
loop_exit:

	if (!has_empty_slot)
		return;

	// Copy empty spaces into the remainder of the slots
	for (auto i = start; i < inv.size(); ++i) {
		// Skip the cursor
		if (inv[i].item != nullptr &&
			inv[i].item->GetLocalizedName() == data::Item::kInventorySelectedCursor)
			continue;

		inv[i] = {nullptr, 0};
	}
}

// LEFT CLICK - Select by reference, the item in the cursor mirrors the inventory item
// RIGHT CLICK - Select unique, the item in the cursor exists independently of the inventory item

void game::PlayerData::Inventory::InventoryClick(const data::PrototypeManager& data_manager,
                                                 const uint16_t index,
                                                 const uint16_t mouse_button,
                                                 const bool allow_reference_select,
                                                 data::Item::Inventory& inv) {
    assert(index < inventoryPlayer.size());
    assert(mouse_button == 0 || mouse_button == 1); // Only left + right click supported

    // Clicking on the same location + inventory, selecting by reference: deselect
    if (hasItemSelected_ && selectByReference_ && selectedItemIndex_ == index && selectedItemInventory_ == &inv) {
        hasItemSelected_ = false;

        // Add referenced item to slot
        inv[selectedItemIndex_] = selectedItem_;
        return;
    }


    // Selection mode can only be set upon first item selection
    if (!hasItemSelected_) {
        // Clicking empty slot
        if (inv[index].item == nullptr)
            return;

        hasItemSelected_       = true;
        selectedItemIndex_     = index;
        selectedItemInventory_ = &inv;

        // Reference
        if (allow_reference_select && mouse_button == 0) {
            assert(&inv == &inventoryPlayer); // Select by reference only allowed for player inventory
            selectByReference_ = true;
            selectedItem_      = inv[index];

            // Swap icon out for a cursor indicating the current index is selected
            inventoryPlayer[index].item  = data_manager.DataRawGet<data::Item>(data::Item::kInventorySelectedCursor);
            inventoryPlayer[index].count = 0;

            // Return is necessary when selecting by reference
            // The item needs to be moved after selecting the next inventory slot
            return;
        }

        // Unique
        selectByReference_ = false;

		// Clear the cursor inventory so half can be moved into it
		selectedItem_.item  = nullptr;
		selectedItem_.count = 0;
		// DO NOT return for it to move the item into the new inventory
	}

	const bool cursor_empty =
		MoveItemstackToIndex(selectedItem_,
		                     inv[index],
		                     mouse_button);
	// Cursor slot is empty
	if (cursor_empty) {
		hasItemSelected_ = false;

		if (selectByReference_) {
			// Remove cursor icon
			assert(selectedItemIndex_ < inventoryPlayer.size());
			// Select by reference is only for inventory_player
			inventoryPlayer[selectedItemIndex_].item  = nullptr;
			inventoryPlayer[selectedItemIndex_].count = 0;
		}
	}
}

const data::ItemStack* game::PlayerData::Inventory::GetSelectedItemStack() const {
    if (!hasItemSelected_)
        return nullptr;

    return &selectedItem_;
}

bool game::PlayerData::Inventory::DeselectSelectedItem() {
    if (!hasItemSelected_ || !selectByReference_)
        return false;

    // Add referenced item to slot
    inventoryPlayer[selectedItemIndex_] = selectedItem_;
    hasItemSelected_                    = false;
    return true;
}

bool game::PlayerData::Inventory::IncrementSelectedItem() {
    assert(hasItemSelected_);

    // DO not increment if it will exceed the stack size
    if (selectedItem_.count < selectedItem_.item->stackSize) {
        selectedItem_.count++;
        return true;
    }

    return false;
}

bool game::PlayerData::Inventory::DecrementSelectedItem() {
    assert(hasItemSelected_);

    if (--selectedItem_.count == 0) {
        // Item stack now empty
        hasItemSelected_ = false;
        // Remove selection cursor
        inventoryPlayer[selectedItemIndex_].item  = nullptr;
        inventoryPlayer[selectedItemIndex_].count = 0;

        return false;
    }
    return true;
}

// ============================================================================================
// Recipe

void game::PlayerData::Crafting::RecipeGroupSelect(const uint16_t index) {
    selectedRecipeGroup_ = index;
}

uint16_t game::PlayerData::Crafting::RecipeGroupGetSelected() const {
    return selectedRecipeGroup_;
}

void game::PlayerData::Crafting::RecipeCraftTick(const data::PrototypeManager& data_manager, uint16_t ticks) {
    // Attempt to return held item if inventory is full
    if (craftingHeldItem_.count != 0) {
        const auto extra_items  = AddStack(playerInv_.inventoryPlayer, craftingHeldItem_);
        craftingHeldItem_.count = extra_items;
        return;
    }


    while (ticks != 0 && !craftingQueue_.empty()) {
        // Ticks available greater than or equal to crafting ticks remaining
        if (ticks >= craftingTicksRemaining_) {
            ticks -= craftingTicksRemaining_;

            const auto* recipe = craftingQueue_.front();
            craftingQueue_.pop_front();

            // Return product
            data::RecipeItem recipe_item = recipe->product;
            const auto* product_item     = data_manager.DataRawGet<data::Item>(recipe_item.first);

            data::ItemStack item = {product_item, recipe_item.second};

            // Deduct based on the deductions
            std::map<std::string, uint16_t>::iterator element;
            if ((element = craftingItemDeductions_.find(recipe_item.first)) != craftingItemDeductions_.end()) {
                auto& deduct_amount = element->second;

                if (item.count >= deduct_amount) {
                    item.count -= deduct_amount;

                    LOG_MESSAGE_F(
                        debug, "Crafting return deducting %d of '%s'", deduct_amount, recipe_item.first.c_str());

                    craftingItemDeductions_.erase(recipe_item.first); // Now empty
                }
                // Deduct amount greater than i.count
                else {
                    deduct_amount -= item.count;
                    item.count = 0;

                    LOG_MESSAGE_F(debug,
                                  "Crafting return deducting %d of '%s', no items returned",
                                  deduct_amount,
                                  recipe_item.first.c_str());
                }
            }

			// Still has items to return to player inventory
			if (item.count != 0) {
                // Extra not available in queue anymore since it has been returned to the player
                auto& queue_extras = craftingItemExtras_[recipe_item.first];
                if (queue_extras > item.count)
                    queue_extras -= item.count;
                else
                    // If entry is 0, erase it
                    craftingItemExtras_.erase(recipe_item.first);

                const auto extra_items = AddStack(playerInv_.inventoryPlayer, item);
                if (extra_items != 0)
                    craftingHeldItem_ = {item.item, extra_items};
            }

            // Set crafting ticks remaining to the next item
            if (!craftingQueue_.empty())
                craftingTicksRemaining_ = core::LossyCast<uint16_t>(craftingQueue_.front()->craftingTime * kGameHertz);
        }
        // Crafting ticks remaining is greater, decrement ticks remaining
		else {
			craftingTicksRemaining_ -= ticks;
			break;
		}
	}

}

void game::PlayerData::Crafting::QueueRecipe(const data::PrototypeManager& data_manager, const data::Recipe& recipe) {
    LOG_MESSAGE_F(debug, "Queuing recipe: '%s'", recipe.product.first.c_str());

    // Remove ingredients
    for (const auto& ingredient : recipe.ingredients) {
        const auto* item = data_manager.DataRawGet<data::Item>(ingredient.first);

        DeleteInvItem(playerInv_.inventoryPlayer, item, ingredient.second);
    }

    // Queue is empty, crafting time for the first item in queue must be set here
    if (craftingQueue_.empty())
        craftingTicksRemaining_ = core::LossyCast<uint16_t>(recipe.craftingTime * kGameHertz);

    craftingQueue_.push_back(&recipe);
}

const game::PlayerData::Crafting::RecipeQueueT& game::PlayerData::Crafting::GetRecipeQueue() const {
    return craftingQueue_;
}

uint16_t game::PlayerData::Crafting::GetCraftingTicksRemaining() const {
    return craftingTicksRemaining_;
}

void game::PlayerData::Crafting::RecipeCraftR(const data::PrototypeManager& data_manager, const data::Recipe& recipe) {
    for (const auto& ingredient : recipe.ingredients) {
        const auto* ingredient_proto = data_manager.DataRawGet<data::Item>(ingredient.first);

        const uint32_t possess_amount = GetInvItemCount(playerInv_.inventoryPlayer, ingredient_proto);

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
                return_deductions += possess_amount; // Not available anymore, so deduct it after crafting

                LOG_MESSAGE_F(
                    debug, "Increasing crafting deductions of '%s' by %d", ingredient.first.c_str(), possess_amount);
            }
            else {
                // More than what is queued

                // Use what is available from queue
                amount_needed -= queued_available;

                // amount_needed: Deduct from returning to player inventory as it is used for crafting
                // queued_available: Queued available items not available anymore, so deduct it after crafting
                LOG_MESSAGE_F(debug,
                              "Increasing crafting deductions of '%s' by %d",
                              ingredient.first.c_str(),
                              amount_needed + queued_available);
                return_deductions += amount_needed + queued_available;


                const auto* ingredient_recipe = data::Recipe::GetItemRecipe(data_manager, ingredient.first);

                // Round up to always ensure enough is crafted
                const unsigned int yield = ingredient_recipe->product.second;
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
    QueueRecipe(data_manager, recipe);
}


bool game::PlayerData::Crafting::RecipeCanCraftR(const data::PrototypeManager& data_manager,
                                                 std::map<const data::Item*, uint32_t>& used_items,
                                                 const data::Recipe& recipe,
                                                 const unsigned batches) const {
    for (const auto& [ing_name, ing_amount_to_craft] : recipe.ingredients) {
        const auto* ing_item = data_manager.DataRawGet<data::Item>(ing_name);

        // If item has already been counted, use the map used_items. Otherwise, count from inventory
        unsigned int possess_amount;
        if (used_items.find(ing_item) != used_items.end()) {
            possess_amount = used_items[ing_item];
        }
        else {
            possess_amount       = GetInvItemCount(playerInv_.inventoryPlayer, ing_item);
            used_items[ing_item] = possess_amount;
        }

        // Ingredient met, subtract from used_items, check others
        if (possess_amount >= core::SafeCast<unsigned>(ing_amount_to_craft * batches)) {
            used_items[ing_item] -= ing_amount_to_craft * batches;
            continue;
        }

        const auto* ingredient_recipe = data::Recipe::GetItemRecipe(data_manager, ing_name);
        // Ingredient cannot be crafted
        if (ingredient_recipe == nullptr)
            return false;

        // Ingredient craftable but not met

        // Amount still needed to be crafted
        unsigned int ingredient_required_batches;
        {
            used_items[ing_item] = 0; // Use up amount available + craft to reach desired amount

            const unsigned int x = ing_amount_to_craft * batches - possess_amount;
            const unsigned int y = ingredient_recipe->product.second;
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

bool game::PlayerData::Crafting::RecipeCanCraft(const data::PrototypeManager& data_manager,
                                                const data::Recipe& recipe,
                                                const uint16_t batches) const {
    std::map<const data::Item*, uint32_t> used_items;
    return RecipeCanCraftR(data_manager, used_items, recipe, batches);
}
