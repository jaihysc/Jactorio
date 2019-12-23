#ifndef GAME_PLAYER_PLAYER_MANAGER_H
#define GAME_PLAYER_PLAYER_MANAGER_H


#include "data/prototype/item/item.h"

/**
 * Stores information & functions regarding the player
 */
namespace jactorio::game::player_manager
{
	// Inventory
    constexpr unsigned short inventory_size = 80;
	inline data::item_stack player_inventory[inventory_size];  // Holds the internal id of items
	
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
}

#endif // GAME_PLAYER_PLAYER_MANAGER_H
