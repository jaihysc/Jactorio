// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#ifndef JACTORIO_INCLUDE_GAME_PLAYER_PLAYER_DATA_H
#define JACTORIO_INCLUDE_GAME_PLAYER_PLAYER_DATA_H
#pragma once

#include <queue>

#include "data/prototype/interface/rotatable.h"
#include "data/prototype/item/item.h"
#include "data/prototype/item/recipe.h"
#include "game/world/chunk_tile_layer.h"
#include "game/world/world_data.h"

namespace jactorio::game
{
	///
	/// \brief Stores information & functions regarding a player (Duplicated for multiple players)
	class PlayerData
	{
	public:
		/// Used when concurrently accessing player data, such as for rendering
		std::mutex mutex;

	private:
		// ======================================================================
		// Player specific mouse selection

		std::pair<int, int> mouseSelectedTile_;

	public:
		///
		/// \brief Call on game tick to calculate the coordinates of mouse selected tile
		/// Cached in mouse_selected_tile_
		void MouseCalculateSelectedTile();

		///
		/// Gets the world X, Y of the tile the mouse is hovered over, computed by calculate_selected_tile(x, y)
		J_NODISCARD std::pair<int, int> GetMouseTileCoords() const { return mouseSelectedTile_; }

		///
		/// \return true if selected tile is within placement range
		J_NODISCARD bool MouseSelectedTileInRange() const;


	private:
		// ============================================================================================
		// Movement
		float playerPositionX_ = 0;
		float playerPositionY_ = 0;

		/// The world the player is currently in
		WorldData* playerWorld_ = nullptr;

		///
		/// \brief Returns true if the tile can be walked on
		bool TargetTileValid(WorldData* world_data, int x, int y) const;

	public:
		void SetPlayerWorld(WorldData* world_data) { playerWorld_ = world_data; }
		J_NODISCARD WorldData& GetPlayerWorld() const {
			assert(playerWorld_ != nullptr);  // Player is not in a world!
			return *playerWorld_;
		}


		/// The tile the player is on, decimals indicate partial tile
		J_NODISCARD float GetPlayerPositionX() const { return playerPositionX_; }
		J_NODISCARD float GetPlayerPositionY() const { return playerPositionY_; }

		///
		/// If the tile at the specified amount is valid, the player will be moved to that tile
		/// \param amount
		void MovePlayerX(float amount);

		///
		/// If the tile at the specified amount is valid, the player will be moved to that tile
		/// \param amount
		void MovePlayerY(float amount);


		// ============================================================================================
		// Entity placement / pickup

	private:
		ChunkTileLayer* activatedLayer_ = nullptr;

		uint16_t pickupTickCounter_ = 0;
		uint16_t pickupTickTarget_  = 0;

		// Do not reference this, this only tracks whether or not a different entity or another tile
		// is selected by comparing pointers
		const void* lastSelectedPtr_ = nullptr;
		const void* lastTilePtr_     = nullptr;

	public:
		data::Orientation placementOrientation = data::Orientation::up;

		///
		/// \brief Rotates placement_orientation clockwise 
		void RotatePlacementOrientation();
		///
		/// \brief Rotates placement_orientation counter clockwise 
		void CounterRotatePlacementOrientation();


		///
		/// \brief Sets the activated layer, use nullptr to unset
		void SetActivatedLayer(ChunkTileLayer* layer) { activatedLayer_ = layer; }

		///
		/// \brief Gets the layer of the entity activated on by the player
		/// \return nullptr If no layer is activated by the player
		J_NODISCARD ChunkTileLayer* GetActivatedLayer() const { return activatedLayer_; }


		///
		/// \brief Will place an entity at the location or if an entity does not already exist
		/// \remark Call when the key for placing entities is pressed
		/// \param can_activate_layer will be set activated_layer to the clicked entity's layer if true
		void TryPlaceEntity(WorldData& world_data, int world_x, int world_y,
		                    bool can_activate_layer = false);

		///
		/// \brief This will either pickup an entity, or mine resources from a resource tile
		/// Call when the key for picking up entities is pressed
		/// If resource + entity exists on one tile, picking up entity takes priority
		void TryPickup(WorldData& world_data, int tile_x, int tile_y, uint16_t ticks = 1);

		///
		/// \return progress of entity pickup or resource extraction as a fraction between 0 - 1
		J_NODISCARD float GetPickupPercentage() const;


		// ============================================================================================
		// Inventory
	private:
		uint16_t selectedRecipeGroup_ = 0;

		std::deque<const data::Recipe*> craftingQueue_;
		uint16_t craftingTicksRemaining_ = 0;

		/// Items to be deducted away during crafting and not returned to the player inventory
		/// Used for recipes requiring subrecipes, where intermediate items must be satisfied first
		std::map<std::string, uint16_t> craftingItemDeductions_;

		/// Excess items which queued recipes will return to the player inventory
		std::map<std::string, uint16_t> craftingItemExtras_;

		/// Item which is held until there is space in the player inventory to return
		data::ItemStack craftingHeldItem_ = {nullptr, 0};


		data::ItemStack selectedItem_;

		bool hasItemSelected_                   = false;
		unsigned short selectedItemIndex_       = 0;
		data::ItemStack* selectedItemInventory_ = nullptr;
		bool selectByReference_                 = false;

	public:
		static constexpr char kInventorySelectedCursorIname[] = "__core__/inventory-selected-cursor";

		///
		/// Sorts inventory items by internal name, grouping multiples of the same item into one stack, obeying stack size
		void InventorySort();

		static constexpr unsigned short kInventorySize = 80;
		data::ItemStack inventoryPlayer[kInventorySize];  // Holds the internal id of items

		///
		/// \brief Interacts with the player inventory at index
		/// \param index The player inventory index
		/// \param mouse_button Mouse button pressed; 0 - Left, 1 - Right
		/// \param allow_reference_select If true, left clicking will select the item by reference
		void InventoryClick(const data::DataManager& data_manager,
		                    unsigned short index, unsigned short mouse_button, bool allow_reference_select,
		                    data::ItemStack* inv);

		///
		/// \brief Gets the currently item player is currently holding on the cursor
		/// \return nullptr if there is no item selected
		J_NODISCARD const data::ItemStack* GetSelectedItem() const;

		///
		/// \brief Deselects the current item and returns it to its slot ONLY if selected by reference
		/// \return true if successfully deselected
		bool DeselectSelectedItem();

		///
		/// \brief Increments the selected item to the stack size
		/// \return true if successfully incremented
		bool IncrementSelectedItem();

		///
		/// \brief Once item count reaches 0, the selected item slot is cleared
		/// \return true if items still remain in stack, false if now empty
		bool DecrementSelectedItem();


		// ============================================================================================
		// Recipe

		void RecipeGroupSelect(uint16_t index);
		J_NODISCARD uint16_t RecipeGroupGetSelected() const;

		///
		/// \brief Call every tick to count down the crafting time for the currently queued item (60 ticks = 1 second)

		void RecipeCraftTick(const data::DataManager& data_manager, uint16_t ticks = 1);

		///
		/// \brief Queues a recipe to be crafted, this is displayed by the gui is the lower right corner
		void RecipeQueue(const data::DataManager& data_manager, const data::Recipe& recipe);

		///
		/// \brief Returns const reference to recipe queue for rendering in gui
		J_NODISCARD const std::deque<const data::Recipe*>& GetRecipeQueue() const;
		J_NODISCARD uint16_t GetCraftingTicksRemaining() const;

	private:
		///
		/// \brief The actual recursive function for recipe_craft_r
		/// \param used_items Tracks amount of an item that has already been used,
		/// so 2 recipes sharing one ingredient will be correctly accounted for in recursion when counting from the inventory
		bool RecipeCanCraftR(const data::DataManager& data_manager,
		                     std::map<const data::Item*, uint32_t>& used_items,
		                     const data::Recipe& recipe, uint16_t batches) const;
	public:
		///
		/// \brief Recursively depth first crafts the recipe
		/// !! This WILL NOT check that the given recipe is valid or required ingredients are present and assumes it is!!
		void RecipeCraftR(const data::DataManager& data_manager, const data::Recipe& recipe);

		///
		/// \brief Recursively steps through a recipe and subrecipies to determine if it is craftable
		/// \param recipe
		/// \param batches How many runs of the recipe
		J_NODISCARD bool RecipeCanCraft(const data::DataManager& data_manager, const data::Recipe& recipe,
		                                uint16_t batches) const;


		// ============================================================================================
#ifdef JACTORIO_BUILD_TEST
		void ClearPlayerInventory() {
			for (auto& i : inventoryPlayer) {
				i.first = nullptr;
				i.second = 0;
			}
		}

		void ResetInventoryVariables() {
			hasItemSelected_ = false;
			selectByReference_ = false;
		}

		std::map<std::string, uint16_t>& GetCraftingItemDeductions() {
			return craftingItemDeductions_;
		}

		std::map<std::string, uint16_t>& GetCraftingItemExtras() {
			return craftingItemExtras_;
		}

		void SetSelectedItem(data::ItemStack& item) {
			hasItemSelected_ = true;
			selectedItem_ = item;
		}
#endif
	};
}

#endif //JACTORIO_INCLUDE_GAME_PLAYER_PLAYER_DATA_H
