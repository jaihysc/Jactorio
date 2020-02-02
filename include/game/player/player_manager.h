#ifndef GAME_PLAYER_PLAYER_MANAGER_H
#define GAME_PLAYER_PLAYER_MANAGER_H

#include "data/prototype/item/item.h"
#include "data/prototype/item/recipe.h"
#include "game/world/chunk_tile_layer.h"

#include <vector>
#include <queue>

/**
 * Stores information & functions regarding the player
 */
namespace jactorio::game::player_manager
{
	// ============================================================================================
	// Movement
	
	/**
	 * The tile the player is on, decimals indicate partial tile
	 */
	float get_player_position_x();
	float get_player_position_y();

	/**
	 * If the tile at the specified amount is valid, the player will be moved to that tile
	 * @param amount
	 */
	void move_player_x(float amount);
	/**
	 * If the tile at the specified amount is valid, the player will be moved to that tile
	 * @param amount
	 */
	void move_player_y(float amount);

	// ============================================================================================
	// Entity placement / pickup

	/**
	 * Gets the layer of the entity activated on by the player
	 * @return nullptr If no layer is activated by the player
	 */
	J_NODISCARD Chunk_tile_layer* get_activated_layer();
	
	/**
	 * Call when the key for placing entities is pressed <br>
	 * Will place am entity at the location or if an entity already exists <br>
	 * Activated_layer will be set to the clicked entity's layer IF mouse_release is true <br>
	 * 
	 * Everything occurs in a single tick
	 */
	void try_place(int tile_x, int tile_y, bool mouse_release = false);

	/**
	 * Call when the key for picking up entities is pressed <br>
	 * This will either pickup an entity, or mine resources from a resource tile
	 * If 2 exists on one tile, picking up entity takes priority
	 */
	void try_pickup(int tile_x, int tile_y, uint16_t ticks = 1);

	/**
	 * Returns progress of entity pickup or resource extraction as a fraction between 0 - 1
	 */
	float get_pickup_percentage();
	
	// ============================================================================================
	// Inventory

	constexpr char inventory_selected_cursor_iname[] = "__core__/inventory-selected-cursor";

	/**
	 * Sorts inventory items by internal name, grouping multiples of the same item into one stack, obeying stack size
	 */
	void inventory_sort();
	
	constexpr unsigned short inventory_size = 80;
	inline data::item_stack inventory_player[inventory_size];  // Holds the internal id of items
	
	/**
	 * Interacts with the player inventory at index
	 * @param index The player inventory index
	 * @param mouse_button Mouse button pressed; 0 - Left, 1 - Right
	 * @param allow_reference_select If true, left clicking will select the item by reference
	 */
	void inventory_click(unsigned short index, unsigned short mouse_button, bool allow_reference_select, data::item_stack* inv);

	/**
	 * Gets the currently item player is currently holding on the cursor
	 * @return nullptr if there is no item selected
	 */
	J_NODISCARD const data::item_stack* get_selected_item();


	/**
	 * Increments the selected item to the stack size
	 * @return true if successfully incremented
	 */
	bool increment_selected_item();

	/**
	 * Once item count reaches 0, the selected item slot is cleared
	 * @return true if items still remain in stack, false if now empty
	 */
	bool decrement_selected_item();


	// ============================================================================================
	// Recipe

	void recipe_group_select(uint16_t index);
	J_NODISCARD uint16_t recipe_group_get_selected();

	/**
	 * Call every tick to count down the crafting time for the currently queued item (60 ticks = 1 second)
	 */
	void recipe_craft_tick(uint16_t ticks = 1);

	/**
	 * Queues a recipe to be crafted, this is displayed by the gui is the lower right corner
	 */
	void recipe_queue(data::Recipe* recipe);

	/**
	 * Returns const reference to recipe queue for rendering in gui
	 */
	J_NODISCARD const std::deque<data::Recipe*>& get_recipe_queue();
	J_NODISCARD uint16_t get_crafting_ticks_remaining();
	
	/**
	 * Recursively depth first crafts the recipe
	 * !! This WILL NOT check that the given recipe is valid or required ingredients are present and assumes it is!!
	 */
	void recipe_craft_r(data::Recipe* recipe);
	
	/**
	 * Recursively steps through a recipe and subrecipies to determine if it is craftable
	 * @param recipe
	 * @param batches How many runs of the recipe
	 */
	bool recipe_can_craft(const data::Recipe* recipe, uint16_t batches);

	// ============================================================================================
	// Reserved
	
	J_TEST_USE_ONLY void reset_inventory_variables();

	J_TEST_USE_ONLY std::map<std::string, uint16_t>& get_crafting_item_deductions();
	J_TEST_USE_ONLY std::map<std::string, uint16_t>& get_crafting_item_extras();

	J_TEST_USE_ONLY void set_selected_item(data::item_stack& item);
}

#endif // GAME_PLAYER_PLAYER_MANAGER_H
