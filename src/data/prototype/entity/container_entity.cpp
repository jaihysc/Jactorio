// 
// container_entity.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 02/07/2020
// Last modified: 03/22/2020
// 

#include "data/prototype/entity/container_entity.h"

#include "renderer/gui/gui_menus.h"

void jactorio::data::Container_entity::delete_unique_data(void* ptr) const {
	delete static_cast<Container_entity_data*>(ptr);
}

void* jactorio::data::Container_entity::copy_unique_data(void* ptr) const {
	auto* data = new Container_entity_data(inventory_size);

	// Copy inventory contents over
	for (int i = 0; i < inventory_size; ++i) {
		data->inventory[i] = static_cast<Container_entity_data*>(ptr)->inventory[i];
	}

	return data;
}


void jactorio::data::Container_entity::on_build(game::World_data& world_data, std::pair<int, int> world_coords,
                                                game::Chunk_tile_layer* tile_layer, uint16_t frame,
                                                placementOrientation orientation) const {
	tile_layer->unique_data = new Container_entity_data(inventory_size);
}

void jactorio::data::Container_entity::on_r_show_gui(game::Player_data& player_data,
                                                     game::Chunk_tile_layer* tile_layer) const {
	renderer::gui::container_entity(player_data,
	                                static_cast<Container_entity_data*>(tile_layer->unique_data)->inventory,
	                                inventory_size);
}
