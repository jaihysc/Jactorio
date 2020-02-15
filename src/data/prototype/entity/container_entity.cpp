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


void jactorio::data::Container_entity::on_build(game::Chunk_tile_layer* tile_layer) const {
	tile_layer->unique_data = new Container_entity_data(inventory_size);
}

void jactorio::data::Container_entity::on_show_gui(game::Chunk_tile_layer* tile_layer) const {
	renderer::gui::container_entity(
		static_cast<Container_entity_data*>(tile_layer->unique_data)->inventory, inventory_size);
}