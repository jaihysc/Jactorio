#include "game/player/player_manager.h"

#include "game/world/world_manager.h"

float player_position_x = 0;
float player_position_y = 0;

// Returns true if the tile can be walked on and is not walter
bool target_tile_valid(float x, float y) {
    // If the player is on water, they are allowed to walk on water
    if (jactorio::game::world_manager::get_tile_world_coords(player_position_x, player_position_y)->is_water)
        return true;

    const jactorio::game::Chunk_tile* tile = jactorio::game::world_manager::get_tile_world_coords(x, y);
    // Chunk not generated yet
    if (tile == nullptr)
        return false;

    return !tile->is_water;
}


float jactorio::game::player_manager::get_player_position_x() {
    return player_position_x;
}

float jactorio::game::player_manager::get_player_position_y() {
    return player_position_y;
}


void jactorio::game::player_manager::move_player_x(float amount) {
    float target_x = player_position_x + amount;

    if (target_tile_valid(target_x, player_position_y))
        player_position_x = target_x;
}

void jactorio::game::player_manager::move_player_y(float amount) {
    float target_y = player_position_y + amount;

    if (target_tile_valid(player_position_x, target_y))
        player_position_y = target_y;
}
