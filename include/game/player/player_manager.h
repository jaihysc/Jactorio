#ifndef GAME_PLAYER_PLAYER_MANAGER_H
#define GAME_PLAYER_PLAYER_MANAGER_H

#include "data/prototype/item/item.h"
#include "data/prototype/item/recipe.h"

#include <vector>

/**
 * Stores information & functions regarding the player
 */
namespace jactorio::game::player_manager
{
    // Movement functions
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

	
    // Inventory

    constexpr char inventory_selected_cursor_iname[] = "__core__/inventory-selected-cursor";

    /**
     * Sorts inventory items by internal name, grouping multiples of the same item into one stack, obeying stack size
     */
    void player_inventory_sort();
	
    constexpr unsigned short player_inventory_size = 80;
    inline data::item_stack player_inventory[player_inventory_size];  // Holds the internal id of items
	
    /**
     * Interacts with the player inventory at index
     * @param index The player inventory index
     * @param mouse_button Mouse button pressed; 0 - Left, 1 - Right
     */
    void set_clicked_inventory(unsigned short index, unsigned short mouse_button);

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
     * @return true if successfully decremented, false if slot is empty
     */
    bool decrement_selected_item();


    // Recipe
    void select_recipe_group(uint16_t index);
    J_NODISCARD uint16_t get_selected_recipe_group();
	
	
    // Reserved
	
    /**
     * !! Tests use only !!
     */
    void reset_inventory_variables();
}

#endif // GAME_PLAYER_PLAYER_MANAGER_H
