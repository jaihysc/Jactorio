#ifndef GAME_PLAYER_PLAYER_MANAGER_H
#define GAME_PLAYER_PLAYER_MANAGER_H

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
}

#endif // GAME_PLAYER_PLAYER_MANAGER_H
