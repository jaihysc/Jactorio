// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/player/player.h"

#include <map>

#include "game/input/mouse_selection.h"
#include "game/logistic/inventory.h"
#include "game/world/world.h"
#include "proto/recipe.h"
#include "proto/resource_entity.h"
#include "proto/tile.h"

using namespace jactorio;

void game::Player::World::SetMouseSelectedTile(const WorldCoord& coord) noexcept {
    mouseSelectedTile_ = coord;
}

WorldCoord game::Player::World::GetMouseTileCoords() const noexcept {
    return mouseSelectedTile_;
}

bool game::Player::World::MouseSelectedTileInRange() const {
    const auto cursor_position = GetMouseTileCoords();

    // Maximum distance of from the player where tiles can be reached
    constexpr unsigned int max_reach = 34;
    const auto tile_dist =
        abs(position_.x - SafeCast<float>(cursor_position.x)) + abs(position_.y - SafeCast<float>(cursor_position.y));

    return tile_dist <= max_reach;
}

Position2<game::Player::World::PlayerPosT> game::Player::World::GetPosition() const noexcept {
    return position_;
}

bool game::Player::World::TargetTileValid(game::World* world, const WorldCoord& coord) const {
    assert(world != nullptr); // Player is not in a world

    const auto* origin_tile = world->GetTile(
        {LossyCast<WorldCoordAxis>(position_.x), LossyCast<WorldCoordAxis>(position_.y)}, TileLayer::base);

    if (origin_tile == nullptr)
        return false;

    // If the player is on water, they are allowed to walk on water
    if (origin_tile->GetPrototype<proto::Tile>()->isWater)
        return true;


    const auto* target_tile = world->GetTile(coord, TileLayer::base);
    // Chunk not generated yet
    if (target_tile == nullptr)
        return false;

    return !target_tile->GetPrototype<proto::Tile>()->isWater;
}

void game::Player::World::MovePlayerX(const float amount) {
    const float target_x = position_.x + amount;

    //    if (TargetTileValid(&GetWorld(), LossyCast<int>(target_x), LossyCast<int>(playerPositionY_)))
    position_.x = target_x;
}

void game::Player::World::MovePlayerY(const float amount) {
    const float target_y = position_.y + amount;

    //    if (TargetTileValid(&GetWorld(), LossyCast<int>(playerPositionX_), LossyCast<int>(target_y)))
    position_.y = target_y;
}

void game::Player::World::SetPlayerX(const PlayerPosT x) noexcept {
    position_.x = x;
}

void game::Player::World::SetPlayerY(const PlayerPosT y) noexcept {
    position_.y = y;
}


// ============================================================================================
// Entity placement / pickup

void game::Player::Placement::RotateOrientation() {
    switch (orientation) {
    case Orientation::up:
        orientation = Orientation::right;
        break;
    case Orientation::right:
        orientation = Orientation::down;
        break;
    case Orientation::down:
        orientation = Orientation::left;
        break;
    case Orientation::left:
        orientation = Orientation::up;
        break;
    default:
        assert(false); // Missing switch case
        orientation = Orientation::up;
    }
}

void game::Player::Placement::CounterRotateOrientation() {
    switch (orientation) {
    case Orientation::up:
        orientation = Orientation::left;
        break;
    case Orientation::left:
        orientation = Orientation::down;
        break;
    case Orientation::down:
        orientation = Orientation::right;
        break;
    case Orientation::right:
        orientation = Orientation::up;
        break;
    default:
        assert(false); // Missing switch case
        orientation = Orientation::up;
    }
}

void game::Player::Placement::DeactivateTile() {
    activatedTile_ = nullptr;
}

std::pair<game::ChunkTile*, WorldCoord> game::Player::Placement::GetActivatedTile() const {
    return {activatedTile_, activatedCoord_};
}

/// \param coord Top left tile x, y
/// \param orientation Orientation of placed / removed entity
void UpdateNeighboringEntities(game::World& world,
                               game::Logic& logic,
                               const WorldCoord& coord,
                               const Orientation orientation,
                               const proto::Entity* entity_ptr) {

    auto call_on_neighbor_update =
        [&](const WorldCoord& emit_coord, const WorldCoord& receive_coord, const Orientation target_orientation) {
            const auto* tile = world.GetTile(receive_coord, game::TileLayer::entity);
            if (tile != nullptr) {
                const auto* entity = tile->GetPrototype<proto::Entity>();
                if (entity != nullptr)
                    entity->OnNeighborUpdate(world, logic, emit_coord, receive_coord, target_orientation);
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
    for (proto::ProtoUintT i = 0; i < entity_ptr->GetWidth(orientation); ++i) {
        const auto x = coord.x + SafeCast<WorldCoordAxis>(i);
        const auto y = coord.y - 1;

        call_on_neighbor_update({x, y + 1}, {x, y}, Orientation::down);
    }
    for (proto::ProtoUintT i = 0; i < entity_ptr->GetHeight(orientation); ++i) {
        const auto x = coord.x + SafeCast<WorldCoordAxis>(entity_ptr->GetWidth(orientation));
        const auto y = coord.y + SafeCast<WorldCoordAxis>(i);

        call_on_neighbor_update({x - 1, y}, {x, y}, Orientation::left);
    }
    for (proto::ProtoUintT i = 1; i <= entity_ptr->GetWidth(orientation); ++i) {
        const auto x =
            coord.x + SafeCast<WorldCoordAxis>(entity_ptr->GetWidth(orientation)) - SafeCast<WorldCoordAxis>(i);
        const auto y = coord.y + SafeCast<WorldCoordAxis>(entity_ptr->GetHeight(orientation));

        call_on_neighbor_update({x, y - 1}, {x, y}, Orientation::up);
    }
    for (proto::ProtoUintT i = 1; i <= entity_ptr->GetHeight(orientation); ++i) {
        const auto x = coord.x - 1;
        const auto y =
            coord.y + SafeCast<WorldCoordAxis>(entity_ptr->GetHeight(orientation)) - SafeCast<WorldCoordAxis>(i);

        call_on_neighbor_update({x + 1, y}, {x, y}, Orientation::right);
    }
}

bool game::Player::Placement::TryPlaceEntity(game::World& world, Logic& logic, const WorldCoord& coord) const {
    auto* tile = world.GetTile(coord, TileLayer::entity);
    if (tile == nullptr)
        return false;

    const auto* stack = playerInv_->GetSelectedItem();

    // No selected item or selected item is not placeable and clicked on a entity
    if (stack == nullptr)
        return false;

    // Does not have item, or placeable item
    auto* entity = stack->item->entityPrototype;
    if (entity == nullptr || !entity->placeable)
        return false;


    assert(entity != nullptr);
    // Prototypes can perform additional checking on whether the location can be placed on or not
    if (!entity->OnCanBuild(world, coord, orientation))
        return false;

    // Do not take item away from player unless item was successfully placed
    if (!world.Place(coord, orientation, *entity))
        // Failed to place because an entity already exists
        return false;


    // All validations passed, entity has been placed

    // If item stack was used up, sort player inventory to fill gap
    if (!playerInv_->DecrementSelectedItem()) {
        playerInv_->inventory.Sort();
    }

    // Call events

    entity->OnBuild(world, logic, coord, orientation);
    UpdateNeighboringEntities(world, logic, coord, orientation, entity);

    for (DimensionAxis y_offset = 0; y_offset < entity->GetHeight(orientation); ++y_offset) {
        for (DimensionAxis x_offset = 0; x_offset < entity->GetWidth(orientation); ++x_offset) {
            world.UpdateDispatch({coord.x + x_offset, coord.y + y_offset}, proto::UpdateType::place);
        }
    }

    return true;
}

bool game::Player::Placement::TryActivateTile(game::World& world, const WorldCoord& coord) {
    auto* tile = world.GetTile(coord, TileLayer::entity);
    if (tile == nullptr)
        return false;

    const auto* stack = playerInv_->GetSelectedItem();

    // Can activate if: No selected item OR selected item is not placeable
    if (stack != nullptr) {
        assert(!stack->Empty());
        // Ensure item attempting to place is an entity
        auto* entity_ptr = stack->item->entityPrototype;

        if (entity_ptr != nullptr && entity_ptr->placeable)
            return false;
    }

    // Activate the clicked entity / prototype. For example: show the gui
    if (tile->GetPrototype() == nullptr)
        return false;

    // // If clicking again on the same entity, deactivate
    // if (activated_layer == &selected_layer)
    // 	activated_layer = nullptr;
    // else

    // Clicking on an existing entity will activate it
    activatedTile_  = tile->GetTopLeft();
    activatedCoord_ = coord;
    return true;
}


void game::Player::Placement::TryPickup(game::World& world,
                                        Logic& logic,
                                        const WorldCoord& coord,
                                        const uint16_t ticks) {
    auto* entity_tile   = world.GetTile(coord, TileLayer::entity);
    auto* resource_tile = world.GetTile(coord, TileLayer::resource);

    if (entity_tile == nullptr || resource_tile == nullptr) {
        return;
    }

    const auto* entity_proto   = entity_tile->GetPrototype<proto::Entity>();
    const auto* resource_proto = resource_tile->GetPrototype<proto::ResourceEntity>();


    const proto::Entity* chosen_ptr = resource_proto;
    bool is_resource_ptr            = true;
    {
        // Picking up entities takes priority since it is higher on the layer
        if (entity_proto != nullptr) {
            is_resource_ptr = false;
            chosen_ptr      = entity_proto;
        }
        else if (resource_proto != nullptr) {
        }
        else {
            // No valid pointers
            return;
        }
    }


    auto give_item = [this, &chosen_ptr]() {
        assert(chosen_ptr->GetItem() != nullptr); // Entity prototype does not have an item prototype
        const ItemStack item_stack{chosen_ptr->GetItem(), 1};

        // Failed to add item, likely because the inventory is full
        if (!playerInv_->inventory.CanAdd(item_stack).first) {
            return;
        }

        playerInv_->inventory.Add(item_stack);
        playerInv_->inventory.Sort();

        pickupTickCounter_ = 0;
    };

    if (is_resource_ptr) {
        // Selecting a new tile different from the last selected tile will reset the counter
        if (lastSelectedCoord_ != coord) {
            lastSelectedCoord_ = coord;
            pickupTickCounter_ = 0;
        }

        pickupTickCounter_ += ticks;
        pickupTickTarget_ = LossyCast<uint16_t>(SafeCast<const proto::ResourceEntity*>(chosen_ptr)->pickupTime *
                                                kGameHertz); // Seconds to ticks
        if (pickupTickCounter_ >= pickupTickTarget_) {
            auto* resource_data = resource_tile->GetUniqueData<proto::ResourceEntityData>();
            assert(resource_data != nullptr); // Resource tiles should have valid data

            // Delete resource tile if it is empty after extracting
            if (--resource_data->resourceAmount == 0) {
                resource_tile->Clear();
                world.SetTexCoordId(coord, TileLayer::resource, 0);
            }

            give_item();
        }
    }
    else {
        PickupEntity(world, logic, coord, entity_tile, entity_proto);
        give_item();
    }
}

float game::Player::Placement::GetPickupPercentage() const {
    return SafeCast<float>(pickupTickCounter_) / SafeCast<float>(pickupTickTarget_);
}

void game::Player::Placement::PickupEntity(
    game::World& world, Logic& logic, const WorldCoord& coord, ChunkTile* entity_tile, const proto::Entity* entity) {
    // User may have hovered on another tile other than the top left
    auto tl_coord = coord.Incremented(*entity_tile);

    // Picking up an entity which is set in activated_layer will unset activated_layer
    if (activatedTile_ == entity_tile->GetTopLeft())
        activatedTile_ = nullptr;


    // Call events
    entity->OnRemove(world, logic, tl_coord);

    const bool result = world.Remove(tl_coord, entity_tile->GetOrientation());
    assert(result); // false indicates failed to remove entity

    UpdateNeighboringEntities(world, logic, tl_coord, entity_tile->GetOrientation(), entity);

    world.UpdateDispatch(tl_coord, proto::UpdateType::remove);

    for (DimensionAxis y_offset = 0; y_offset < entity->GetHeight(orientation); ++y_offset) {
        for (DimensionAxis x_offset = 0; x_offset < entity->GetWidth(orientation); ++x_offset) {
            world.UpdateDispatch({tl_coord.x + x_offset, tl_coord.y + y_offset}, proto::UpdateType::remove);
        }
    }
}

// ============================================================================================
// Inventory

void game::Player::Inventory::HandleInventoryActions(const data::PrototypeManager& proto,
                                                     game::Inventory& inv,
                                                     const size_t index,
                                                     const bool half_select) {
    const bool is_player_inv = &inv == &inventory;


    HandleClick(proto, SafeCast<uint16_t>(index), half_select ? 1 : 0, is_player_inv, inv);
    inventory.Sort();
}

// LEFT CLICK - Select by reference, the item in the cursor mirrors the inventory item
// RIGHT CLICK - Select unique, the item in the cursor exists independently of the inventory item

void game::Player::Inventory::HandleClick(const data::PrototypeManager& proto,
                                          const uint16_t index,
                                          const uint16_t mouse_button,
                                          const bool reference_select,
                                          game::Inventory& inv) {
    assert(index < inventory.Size());
    assert(mouse_button == 0 || mouse_button == 1); // Only left + right click supported

    if (reference_select)
        assert(&inv == &inventory); // Can only reference select player inventory to allow serialization support

    // Clicking on the same location + inventory, selecting by reference: deselect
    if (hasItemSelected_ && selectByReference_ && selectedItemIndex_ == index) {
        hasItemSelected_ = false;

        // Add referenced item to slot
        inv[selectedItemIndex_] = selectedItem_;
        return;
    }


    // Selection mode can only be set upon first item selection
    if (!hasItemSelected_) {
        // Clicking empty slot
        if (inv[index].Empty())
            return;

        hasItemSelected_   = true;
        selectedItemIndex_ = index;

        // Reference
        if (reference_select && mouse_button == 0) {
            assert(&inv == &inventory); // Select by reference only allowed for player inventory
            selectByReference_ = true;
            selectedItem_      = inv[index];

            // Swap icon out for a cursor indicating the current index is selected
            inventory[index].item  = proto.Get<proto::Item>(proto::Item::kInventorySelectedCursor);
            inventory[index].count = 0;

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

    const bool cursor_empty = MoveItemstackToIndex(selectedItem_, inv[index], mouse_button);

    if (cursor_empty) {
        hasItemSelected_ = false;

        if (selectByReference_) {
            // Remove cursor icon
            assert(selectedItemIndex_ < inventory.Size());
            // Select by reference is only for inventory_player
            inventory[selectedItemIndex_].item  = nullptr;
            inventory[selectedItemIndex_].count = 0;
        }
    }
}

const game::ItemStack* game::Player::Inventory::GetSelectedItem() const {
    if (!hasItemSelected_)
        return nullptr;

    return &selectedItem_;
}

bool game::Player::Inventory::DeselectSelectedItem() {
    if (!hasItemSelected_ || !selectByReference_)
        return false;

    // Add referenced item to slot
    inventory[selectedItemIndex_] = selectedItem_;
    hasItemSelected_              = false;

    inventory.Sort();

    return true;
}

bool game::Player::Inventory::IncrementSelectedItem() {
    assert(hasItemSelected_);

    // DO not increment if it will exceed the stack size
    if (selectedItem_.count < selectedItem_.item->stackSize) {
        selectedItem_.count++;
        return true;
    }

    return false;
}

bool game::Player::Inventory::DecrementSelectedItem() {
    assert(hasItemSelected_);

    if (--selectedItem_.count == 0) {
        // Item stack now empty
        hasItemSelected_ = false;
        // Remove selection cursor
        inventory[selectedItemIndex_].item  = nullptr;
        inventory[selectedItemIndex_].count = 0;

        return false;
    }
    return true;
}

// ============================================================================================
// Recipe

void game::Player::Crafting::RecipeGroupSelect(const uint16_t index) {
    selectedRecipeGroup_ = index;
}

uint16_t game::Player::Crafting::RecipeGroupGetSelected() const {
    return selectedRecipeGroup_;
}

void game::Player::Crafting::RecipeCraftTick(const data::PrototypeManager& proto, uint16_t ticks) {
    // Attempt to return held item if inventory is full
    if (craftingHeldItem_.count != 0) {
        const auto extra_items  = playerInv_->inventory.Add(craftingHeldItem_);
        craftingHeldItem_.count = extra_items;
        return;
    }


    while (ticks != 0 && !craftingQueue_.empty()) {
        // Ticks available greater than or equal to crafting ticks remaining
        if (ticks >= craftingTicksRemaining_) {
            ticks -= craftingTicksRemaining_;

            const auto* recipe = craftingQueue_.front().Get();
            craftingQueue_.pop_front();

            // Return product
            proto::RecipeItem recipe_item = recipe->product;
            const auto* product_item      = proto.Get<proto::Item>(recipe_item.first);

            ItemStack item = {product_item, recipe_item.second};

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

                const auto extra_items = playerInv_->inventory.Add(item);
                if (extra_items != 0)
                    craftingHeldItem_ = {item.item, extra_items};
            }

            // Set crafting ticks remaining to the next item
            if (!craftingQueue_.empty())
                craftingTicksRemaining_ = LossyCast<uint16_t>(craftingQueue_.front()->craftingTime * kGameHertz);
        }
        // Crafting ticks remaining is greater, decrement ticks remaining
        else {
            craftingTicksRemaining_ -= ticks;
            break;
        }
    }
}

void game::Player::Crafting::QueueRecipe(const data::PrototypeManager& proto, const proto::Recipe& recipe) {
    LOG_MESSAGE_F(debug, "Queuing recipe: '%s'", recipe.product.first.c_str());

    // Remove ingredients
    for (const auto& ingredient : recipe.ingredients) {
        const auto* item = proto.Get<proto::Item>(ingredient.first);

        playerInv_->inventory.Delete(*item, ingredient.second);
    }

    // Queue is empty, crafting time for the first item in queue must be set here
    if (craftingQueue_.empty())
        craftingTicksRemaining_ = LossyCast<uint16_t>(recipe.craftingTime * kGameHertz);

    craftingQueue_.emplace_back(&recipe);
}

const game::Player::Crafting::RecipeQueueT& game::Player::Crafting::GetRecipeQueue() const {
    return craftingQueue_;
}

uint16_t game::Player::Crafting::GetCraftingTicksRemaining() const {
    return craftingTicksRemaining_;
}

void game::Player::Crafting::RecipeCraftR(const data::PrototypeManager& proto, const proto::Recipe& recipe) {
    for (const auto& ingredient : recipe.ingredients) {
        const auto* ingredient_proto = proto.Get<proto::Item>(ingredient.first);

        const uint32_t possess_amount = playerInv_->inventory.Count(*ingredient_proto);

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


                const auto* ingredient_recipe = proto::Recipe::GetItemRecipe(proto, ingredient.first);

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
                    RecipeCraftR(proto, *ingredient_recipe);
                }
            }
        }
    }

    // Ingredients met - Queue crafting recipe
    QueueRecipe(proto, recipe);
}


bool game::Player::Crafting::RecipeCanCraftR(const data::PrototypeManager& proto,
                                             std::map<const proto::Item*, uint32_t>& used_items,
                                             const proto::Recipe& recipe,
                                             const unsigned batches) const {
    for (const auto& [ing_name, ing_amount_to_craft] : recipe.ingredients) {
        const auto* ing_item = proto.Get<proto::Item>(ing_name);

        // If item has already been counted, use the map used_items. Otherwise, count from inventory
        unsigned int possess_amount;
        if (used_items.find(ing_item) != used_items.end()) {
            possess_amount = used_items[ing_item];
        }
        else {
            possess_amount       = playerInv_->inventory.Count(*ing_item);
            used_items[ing_item] = possess_amount;
        }

        // Ingredient met, subtract from used_items, check others
        if (possess_amount >= SafeCast<unsigned>(ing_amount_to_craft * batches)) {
            used_items[ing_item] -= ing_amount_to_craft * batches;
            continue;
        }

        const auto* ingredient_recipe = proto::Recipe::GetItemRecipe(proto, ing_name);
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
        if (!RecipeCanCraftR(proto, used_items, *ingredient_recipe, ingredient_required_batches)) {
            return false;
        }
    }

    return true;
}

bool game::Player::Crafting::RecipeCanCraft(const data::PrototypeManager& proto,
                                            const proto::Recipe& recipe,
                                            const uint16_t batches) const {
    std::map<const proto::Item*, uint32_t> used_items;
    return RecipeCanCraftR(proto, used_items, recipe, batches);
}
