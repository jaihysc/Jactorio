#include "game/player/player_manager.h"

#include <cassert>

#include "game/logic/inventory_controller.h"
#include "game/world/world_manager.h"

float player_position_x = 0;
float player_position_y = 0;

// Returns true if the tile can be walked on and is not walter
bool target_tile_valid(const float x, const float y) {
    const auto origin_tile = jactorio::game::world_manager::get_tile_world_coords(player_position_x, player_position_y);
    if (origin_tile == nullptr)
        return false;
	
	// If the player is on water, they are allowed to walk on water
    if (origin_tile->is_water)
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


void jactorio::game::player_manager::move_player_x(const float amount) {
    float target_x = player_position_x + amount;

    if (target_tile_valid(target_x, player_position_y))
        player_position_x = target_x;
}

void jactorio::game::player_manager::move_player_y(const float amount) {
    float target_y = player_position_y + amount;

    if (target_tile_valid(player_position_x, target_y))
        player_position_y = target_y;
}

// Inventory

jactorio::data::item_stack selected_item;

void jactorio::game::player_manager::set_clicked_inventory(const unsigned short index) {
    assert(index < inventory_size);

    inventory_controller::move_itemstack_to_index(&selected_item, 0, 
                                                  player_inventory, index);
}

jactorio::data::item_stack* jactorio::game::player_manager::get_selected_item() {
    if (selected_item.first == nullptr)
        return nullptr;

    return &selected_item;
}
