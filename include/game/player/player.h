// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_PLAYER_PLAYER_H
#define JACTORIO_INCLUDE_GAME_PLAYER_PLAYER_H
#pragma once

#include <glm/glm.hpp>
#include <queue>

#include "core/coordinate_tuple.h"
#include "core/orientation.h"
#include "game/logistic/inventory.h"
#include "proto/recipe.h"

namespace jactorio::game
{
    class World;
    class Logic;
    class ChunkTileLayer;

    ///
    /// Stores information & functions regarding a player (Duplicated for multiple players)
    class Player
    {
    public:
        Player()  = default;
        ~Player() = default;

        Player(const Player& other)
            : world{other.world}, inventory{other.inventory}, placement{other.placement}, crafting{other.crafting} {
            placement.playerInv_ = &inventory;
            crafting.playerInv_  = &inventory;
        }

        Player(Player&& other) noexcept
            : world{std::move(other.world)},
              inventory{std::move(other.inventory)},
              placement{std::move(other.placement)},
              crafting{std::move(other.crafting)} {
            placement.playerInv_ = &inventory;
            crafting.playerInv_  = &inventory;
        }

        Player& operator=(Player other) {
            swap(*this, other);
            return *this;
        }

        friend void swap(Player& lhs, Player& rhs) noexcept {
            using std::swap;
            swap(lhs.world, rhs.world);
            swap(lhs.inventory, rhs.inventory);
            swap(lhs.placement, rhs.placement);
            swap(lhs.crafting, rhs.crafting);
        }


        ///
        /// How the player perceives the world, does not modify the world
        class World
        {
            using PlayerPosT = float;

        public:
            ///
            /// Call on game tick to calculate the coordinates of mouse selected tile
            void CalculateMouseSelectedTile(const glm::mat4& mvp_matrix);

            ///
            /// Gets the world X, Y of the tile the mouse is hovered over, computed by calculate_selected_tile(x, y)
            J_NODISCARD WorldCoord GetMouseTileCoords() const {
                return mouseSelectedTile_;
            }

            ///
            /// \return true if selected tile is within placement range
            J_NODISCARD bool MouseSelectedTileInRange() const;

            // ======================================================================

            void SetId(const WorldId world_id) noexcept {
                worldId_ = world_id;
            }
            J_NODISCARD WorldId GetId() const noexcept {
                return worldId_;
            }


            ///
            /// The tile the player is on, decimals indicate partial tile
            J_NODISCARD PlayerPosT GetPositionX() const {
                return positionX_;
            }
            J_NODISCARD PlayerPosT GetPositionY() const {
                return positionY_;
            }

            ///
            /// If the tile at the specified amount is valid, the player will be moved to that tile
            void MovePlayerX(PlayerPosT amount);
            void MovePlayerY(PlayerPosT amount);

            void SetPlayerX(const PlayerPosT x) noexcept {
                positionX_ = x;
            }
            void SetPlayerY(const PlayerPosT y) noexcept {
                positionY_ = y;
            }

            CEREAL_SERIALIZE(archive) {
                archive(worldId_, positionX_, positionY_);
            }

        private:
            ///
            /// \return true if the tile can be walked on
            bool TargetTileValid(game::World* world, const WorldCoord& coord) const;

            WorldCoord mouseSelectedTile_;

            PlayerPosT positionX_ = 0;
            PlayerPosT positionY_ = 0;

            /// The world the player is currently in
            WorldId worldId_ = 0;
        };


        // ============================================================================================


        class Inventory
        {
        public:
            static constexpr std::size_t kDefaultInventorySize = 80;

            ///
            /// High level method for inventory actions, prefer over calls to HandleClick and others
            void HandleInventoryActions(const data::PrototypeManager& proto,
                                        game::Inventory& inv,
                                        size_t index,
                                        bool half_select);

            // ======================================================================

            ///
            /// Interacts with the inventory at index
            /// \param index The inventory index
            /// \param mouse_button Mouse button pressed; 0 - Left, 1 - Right
            /// \param reference_select If true, left clicking will select the item by reference
            void HandleClick(const data::PrototypeManager& proto,
                             uint16_t index,
                             uint16_t mouse_button,
                             bool reference_select,
                             game::Inventory& inv);

            ///
            /// Gets the currently item player is currently holding on the cursor
            /// \return nullptr if there is no item selected
            J_NODISCARD const ItemStack* GetSelectedItem() const;

            ///
            /// Deselects the current item and returns it to its slot ONLY if selected by reference
            /// \return true if successfully deselected
            bool DeselectSelectedItem();

            ///
            /// Increments the selected item to the stack size
            /// \return true if successfully incremented
            bool IncrementSelectedItem();

            ///
            /// Once item count reaches 0, the selected item slot is cleared
            /// \return true if items still remain in stack, false if now empty
            bool DecrementSelectedItem();


#ifdef JACTORIO_BUILD_TEST
            void SetSelectedItem(const ItemStack& item) {
                hasItemSelected_ = true;
                selectedItem_    = item;
            }
#endif


            game::Inventory inventory{kDefaultInventorySize};


            CEREAL_SERIALIZE(archive) {
                archive(inventory, selectedItem_, hasItemSelected_, selectedItemIndex_, selectByReference_);
            }

        private:
            ItemStack selectedItem_;

            bool hasItemSelected_          = false;
            std::size_t selectedItemIndex_ = 0;
            bool selectByReference_        = false;
        };


        // ======================================================================


        class Placement
        {
            friend Player;

        public:
            explicit Placement(Inventory& player_inv) : playerInv_(&player_inv) {}

            friend void swap(Placement& lhs, Placement& rhs) noexcept {
                using std::swap;
                swap(lhs.orientation, rhs.orientation);
                swap(lhs.activatedLayer_, rhs.activatedLayer_);
                swap(lhs.pickupTickCounter_, rhs.pickupTickCounter_);
                swap(lhs.pickupTickTarget_, rhs.pickupTickTarget_);
                swap(lhs.lastSelectedCoord_, rhs.lastSelectedCoord_);
            }


            ///
            /// Rotates placement_orientation clockwise
            void RotateOrientation();
            ///
            /// Rotates placement_orientation counter clockwise
            void CounterRotateOrientation();


            ///
            /// Sets the activated layer, use nullptr to unset
            void SetActivatedLayer(ChunkTileLayer* layer) {
                activatedLayer_ = layer;
            }

            ///
            /// Gets the layer of the entity activated on by the player
            /// \return nullptr If no layer is activated by the player
            J_NODISCARD ChunkTileLayer* GetActivatedLayer() const {
                return activatedLayer_;
            }


            ///
            /// Will place an entity at the location or if an entity does not already exist
            /// \return true if entity was placed
            bool TryPlaceEntity(game::World& world, Logic& logic, const WorldCoord& coord) const;

            ///
            /// Attempts to activate the layer at world coordinates
            /// \return true if layer was activated
            bool TryActivateLayer(game::World& world, const WorldCoord& coord);

            ///
            /// This will either pickup an entity, or mine resources from a resource tile
            /// Call when the key for picking up entities is pressed
            /// If resource + entity exists on one tile, picking up entity takes priority
            void TryPickup(game::World& world, Logic& logic, const WorldCoord& coord, uint16_t ticks = 1);

            ///
            /// \return progress of entity pickup or resource extraction as a fraction between 0 - 1
            J_NODISCARD float GetPickupPercentage() const;


            Orientation orientation = Orientation::up;

        private:
            ChunkTileLayer* activatedLayer_ = nullptr;

            uint16_t pickupTickCounter_ = 0;
            uint16_t pickupTickTarget_  = 1; // Avoids division by zero initially

            WorldCoord lastSelectedCoord_;


            Inventory* playerInv_;
        };


        // ============================================================================================


        class Crafting
        {
            friend Player;

            using RecipeQueueT = std::deque<data::SerialProtoPtr<const proto::Recipe>>;

            using CraftingItemDeductionsT = std::map<std::string, uint16_t>;
            using CraftingItemExtrasT     = std::map<std::string, uint16_t>;

        public:
            explicit Crafting(Inventory& player_inv) : playerInv_(&player_inv) {}

            friend void swap(Crafting& lhs, Crafting& rhs) noexcept {
                using std::swap;
                swap(lhs.recipeSearchText, rhs.recipeSearchText);
                swap(lhs.selectedRecipeGroup_, rhs.selectedRecipeGroup_);
                swap(lhs.craftingQueue_, rhs.craftingQueue_);
                swap(lhs.craftingTicksRemaining_, rhs.craftingTicksRemaining_);
                swap(lhs.craftingItemDeductions_, rhs.craftingItemDeductions_);
                swap(lhs.craftingItemExtras_, rhs.craftingItemExtras_);
                swap(lhs.craftingHeldItem_, rhs.craftingHeldItem_);
            }


            void RecipeGroupSelect(uint16_t index);
            J_NODISCARD uint16_t RecipeGroupGetSelected() const;

            ///
            /// Call every tick to count down the crafting time for the currently queued item (60 ticks = 1 second)
            void RecipeCraftTick(const data::PrototypeManager& proto, uint16_t ticks = 1);

            ///
            /// Queues a recipe to be crafted, this is displayed by the gui is the lower right corner
            void QueueRecipe(const data::PrototypeManager& proto, const proto::Recipe& recipe);

            ///
            /// Returns const reference to recipe queue for rendering in gui
            J_NODISCARD const RecipeQueueT& GetRecipeQueue() const;
            J_NODISCARD uint16_t GetCraftingTicksRemaining() const;

            ///
            /// Recursively depth first crafts the recipe
            /// \remark WILL NOT check that the given recipe is valid or required ingredients are present and assumes is
            void RecipeCraftR(const data::PrototypeManager& proto, const proto::Recipe& recipe);

            ///
            /// Recursively steps through a recipe and sub-recipes to determine if it is craftable
            /// \param batches How many runs of the recipe
            J_NODISCARD bool RecipeCanCraft(const data::PrototypeManager& proto,
                                            const proto::Recipe& recipe,
                                            uint16_t batches) const;

#ifdef JACTORIO_BUILD_TEST
            CraftingItemDeductionsT& GetCraftingItemDeductions() {
                return craftingItemDeductions_;
            }
            CraftingItemExtrasT& GetCraftingItemExtras() {
                return craftingItemExtras_;
            }
#endif


            /// Current text in recipe search menu
            std::string recipeSearchText;


            CEREAL_SERIALIZE(archive) {
                archive(craftingQueue_,
                        craftingTicksRemaining_,
                        craftingItemDeductions_,
                        craftingItemExtras_,
                        craftingHeldItem_);
            }

        private:
            ///
            /// The actual recursive function for RecipeCraftR
            /// \param used_items Tracks amount of an item that has already been used,
            /// so 2 recipes sharing one ingredient will be correctly accounted for in recursion when counting from the
            /// inventory
            bool RecipeCanCraftR(const data::PrototypeManager& proto,
                                 std::map<const proto::Item*, uint32_t>& used_items,
                                 const proto::Recipe& recipe,
                                 unsigned batches) const;

            uint16_t selectedRecipeGroup_ = 0;

            RecipeQueueT craftingQueue_;
            uint16_t craftingTicksRemaining_ = 0;

            /// Items to be deducted away during crafting and not returned to the player inventory
            /// Used for recipes requiring sub-recipes, where intermediate items must be satisfied first
            CraftingItemDeductionsT craftingItemDeductions_;

            /// Excess items which queued recipes will return to the player inventory
            CraftingItemExtrasT craftingItemExtras_;

            /// Item which is held until there is space in the player inventory to return
            ItemStack craftingHeldItem_ = {nullptr, 0};


            Inventory* playerInv_;
        };


        World world;
        Inventory inventory;
        Placement placement{inventory};
        Crafting crafting{inventory};


        CEREAL_SERIALIZE(archive) {
            archive(world, inventory, crafting);
        }
    };
} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_PLAYER_PLAYER_H
