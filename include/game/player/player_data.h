// 
// player_data.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 12/21/2019
// Last modified: 03/23/2020
// 

#ifndef JACTORIO_INCLUDE_GAME_PLAYER_PLAYER_DATA_H
#define JACTORIO_INCLUDE_GAME_PLAYER_PLAYER_DATA_H
#pragma once

#include "data/prototype/item/item.h"
#include "data/prototype/item/recipe.h"
#include "game/world/chunk_tile_layer.h"
#include "data/prototype/interface/rotatable.h"
#include "game/world/world_data.h"

#include <queue>


namespace jactorio::game
{
	///
	/// \brief Stores information & functions regarding a player (Duplicated for multiple players)
	class Player_data
	{
		// ============================================================================================
		// Movement
		float player_position_x_ = 0;
		float player_position_y_ = 0;

		/// The world the player is currently in
		World_data* player_world_ = nullptr;

		///
		/// \brief Returns true if the tile can be walked on
		bool target_tile_valid(World_data* world_data, int x, int y) const;

	public:
		void set_player_world(World_data* world_data) { player_world_ = world_data; }
		J_NODISCARD World_data& get_player_world() const {
			assert(player_world_ != nullptr);  // Player is not in a world!
			return *player_world_;
		}


		/// The tile the player is on, decimals indicate partial tile
		J_NODISCARD float get_player_position_x() const { return player_position_x_; }
		J_NODISCARD float get_player_position_y() const { return player_position_y_; }

		///
		/// If the tile at the specified amount is valid, the player will be moved to that tile
		/// \param amount

		void move_player_x(float amount);
		///
		/// If the tile at the specified amount is valid, the player will be moved to that tile
		/// \param amount

		void move_player_y(float amount);


		// ============================================================================================
		// Entity placement / pickup

	private:
		Chunk_tile_layer* activated_layer_ = nullptr;

		uint16_t pickup_tick_counter_ = 0;
		uint16_t pickup_tick_target_ = 0;

		// Do not reference this, this only tracks whether or not a different entity or another tile
		// is selected by comparing pointers
		void* last_selected_ptr_ = nullptr;
		void* last_tile_ptr_ = nullptr;

	public:
		data::placementOrientation placement_orientation = data::placementOrientation::up;

		///
		/// \brief Rotates placement_orientation clockwise 
		void rotate_placement_orientation();
		///
		/// \brief Rotates placement_orientation counter clockwise 
		void counter_rotate_placement_orientation();


		///
		/// \brief Sets the activated layer, use nullptr to unset
		void set_activated_layer(Chunk_tile_layer* layer) { activated_layer_ = layer; }

		///
		/// \brief Gets the layer of the entity activated on by the player
		/// \return nullptr If no layer is activated by the player
		J_NODISCARD Chunk_tile_layer* get_activated_layer() const { return activated_layer_; }


		///
		/// \brief Will place an entity at the location or if an entity does not already exist
		/// \remark Call when the key for placing entities is pressed
		/// \param can_activate_layer will be set activated_layer to the clicked entity's layer if true
		void try_place_entity(World_data& world_data, int world_x, int world_y,
		                      bool can_activate_layer = false);

		///
		/// \brief This will either pickup an entity, or mine resources from a resource tile
		/// 
		/// Call when the key for picking up entities is pressed
		/// If resource + entity exists on one tile, picking up entity takes priority
		void try_pickup(World_data& world_data, int tile_x, int tile_y, uint16_t ticks = 1);

		///
		/// Returns progress of entity pickup or resource extraction as a fraction between 0 - 1
		J_NODISCARD float get_pickup_percentage() const;


		// ============================================================================================
		// Inventory
	private:
		uint16_t selected_recipe_group_ = 0;

		std::deque<data::Recipe*> crafting_queue_;
		uint16_t crafting_ticks_remaining_ = 0;

		/// Items to be deducted away during crafting and not returned to the player inventory
		/// Used for recipes requiring subrecipes, where intermediate items must be satisfied first
		std::map<std::string, uint16_t> crafting_item_deductions_;

		/// Excess items which queued recipes will return to the player inventory
		std::map<std::string, uint16_t> crafting_item_extras_;


		data::item_stack selected_item_;

		bool has_item_selected_ = false;
		unsigned short selected_item_index_ = 0;
		data::item_stack* selected_item_inventory_ = nullptr;
		bool select_by_reference_ = false;

	public:
		static constexpr char inventory_selected_cursor_iname[] = "__core__/inventory-selected-cursor";

		///
		/// Sorts inventory items by internal name, grouping multiples of the same item into one stack, obeying stack size
		void inventory_sort();

		static constexpr unsigned short inventory_size = 80;
		data::item_stack inventory_player[inventory_size];  // Holds the internal id of items

		///
		/// \brief Interacts with the player inventory at index
		/// \param index The player inventory index
		/// \param mouse_button Mouse button pressed; 0 - Left, 1 - Right
		/// \param allow_reference_select If true, left clicking will select the item by reference
		void inventory_click(unsigned short index, unsigned short mouse_button, bool allow_reference_select,
		                     data::item_stack* inv);

		///
		/// \brief Gets the currently item player is currently holding on the cursor
		/// \return nullptr if there is no item selected
		J_NODISCARD const data::item_stack* get_selected_item() const;


		///
		/// \brief Increments the selected item to the stack size
		/// \return true if successfully incremented
		bool increment_selected_item();

		///
		/// \brief Once item count reaches 0, the selected item slot is cleared
		/// \return true if items still remain in stack, false if now empty
		bool decrement_selected_item();


		// ============================================================================================
		// Recipe

		void recipe_group_select(uint16_t index);
		J_NODISCARD uint16_t recipe_group_get_selected() const;

		///
		/// \brief Call every tick to count down the crafting time for the currently queued item (60 ticks = 1 second)

		void recipe_craft_tick(uint16_t ticks = 1);

		///
		/// \brief Queues a recipe to be crafted, this is displayed by the gui is the lower right corner
		void recipe_queue(data::Recipe* recipe);

		///
		/// \brief Returns const reference to recipe queue for rendering in gui
		J_NODISCARD const std::deque<data::Recipe*>& get_recipe_queue() const;
		J_NODISCARD uint16_t get_crafting_ticks_remaining() const;

	private:
		///
		/// \brief The actual recursive function for recipe_craft_r
		/// \param used_items Tracks amount of an item that has already been used,
		/// so 2 recipes sharing one ingredient will be correctly accounted for in recursion when counting from the inventory
		bool recipe_can_craft_r(std::map<data::Item*, uint32_t>& used_items,
		                        const data::Recipe* recipe, uint16_t batches);
	public:
		///
		/// \brief Recursively depth first crafts the recipe
		/// !! This WILL NOT check that the given recipe is valid or required ingredients are present and assumes it is!!
		void recipe_craft_r(data::Recipe* recipe);

		///
		/// \brief Recursively steps through a recipe and subrecipies to determine if it is craftable
		/// \param recipe
		/// \param batches How many runs of the recipe
		bool recipe_can_craft(const data::Recipe* recipe, uint16_t batches);


		// ============================================================================================
		// TEST only

		J_TEST_USE_ONLY void clear_player_inventory();
		J_TEST_USE_ONLY void reset_inventory_variables();

		J_TEST_USE_ONLY std::map<std::string, uint16_t>& get_crafting_item_deductions();
		J_TEST_USE_ONLY std::map<std::string, uint16_t>& get_crafting_item_extras();

		J_TEST_USE_ONLY void set_selected_item(data::item_stack& item);
	};
}

#endif //JACTORIO_INCLUDE_GAME_PLAYER_PLAYER_DATA_H
