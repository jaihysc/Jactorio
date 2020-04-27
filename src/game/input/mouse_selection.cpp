// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 12/21/2019

#include "game/input/mouse_selection.h"

#include "jactorio.h"

#include "data/data_manager.h"
#include "data/prototype/entity/entity.h"
#include "game/logic/placement_controller.h"
#include "game/player/player_data.h"
#include "game/world/chunk_tile.h"
#include "renderer/opengl/shader_manager.h"
#include "renderer/rendering/renderer.h"

double x_position = 0.f;
double y_position = 0.f;

void jactorio::game::set_cursor_position(const double x_pos, const double y_pos) {
	x_position = x_pos;
	y_position = y_pos;
}


double jactorio::game::Mouse_selection::get_cursor_x() {
	return x_position;
}

double jactorio::game::Mouse_selection::get_cursor_y() {
	return y_position;
}


void jactorio::game::Mouse_selection::draw_cursor_overlay(Player_data& player_data) {
	auto* last_tile = player_data.get_player_world().get_tile(last_tile_pos_.first,
	                                                                       last_tile_pos_.second);
	if (last_tile == nullptr)
		return;


	const auto cursor_position = player_data.get_mouse_tile_coords();
	const data::item_stack* ptr;
	if ((ptr = player_data.get_selected_item()) != nullptr)
		draw_overlay(player_data, static_cast<data::Entity*>(ptr->first->entity_prototype),
		             cursor_position.first, cursor_position.second, player_data.placement_orientation);
	else
		draw_overlay(player_data, nullptr,
		             cursor_position.first, cursor_position.second, player_data.placement_orientation);
}

void jactorio::game::Mouse_selection::draw_overlay(Player_data& player_data, data::Entity* const selected_entity,
                                                   const int world_x, const int world_y,
                                                   const data::Orientation placement_orientation) {
	World_data& world_data = player_data.get_player_world();

	auto* last_tile = world_data.get_tile(last_tile_pos_.first,
	                                                   last_tile_pos_.second);
	auto* tile = world_data.get_tile(world_x, world_y);


	// Clear last overlay
	if (!last_tile)
		return;
	place_sprite_at_coords(
		world_data,
		Chunk_tile::chunkLayer::overlay,
		nullptr,
		last_tile_dimensions_.first, last_tile_dimensions_.second,
		last_tile_pos_.first, last_tile_pos_.second);


	// Draw new overlay
	if (!tile)
		return;
	last_tile_pos_ = {world_x, world_y};

	if (selected_entity && selected_entity->placeable) {
		// Has item selected
		place_sprite_at_coords(world_data, Chunk_tile::chunkLayer::overlay, selected_entity->sprite,
		                                    selected_entity->tile_width, selected_entity->tile_height, world_x,
		                                    world_y);

		// Rotatable entities
		if (selected_entity->rotatable) {
			// Create unique data at tile to indicate set / frame to render
			const auto target = selected_entity->map_placement_orientation(placement_orientation,
			                                                               world_data,
			                                                               {world_x, world_y});

			Chunk_tile_layer& target_layer = tile->get_layer(Chunk_tile::chunkLayer::overlay);
			target_layer.unique_data = new data::Renderable_data(target.first, target.second);
			target_layer.prototype_data = selected_entity->on_r_get_sprite(target_layer.unique_data);
		}

		last_tile_dimensions_ = {selected_entity->tile_width, selected_entity->tile_height};
	}
	else {
		// No item selected
		if (tile->get_layer(Chunk_tile::chunkLayer::entity).prototype_data ||
			tile->get_layer(Chunk_tile::chunkLayer::resource).prototype_data) {

			// Is hovering over entity	
			const auto* sprite_ptr = data::data_raw_get<data::Sprite>(
				data::dataCategory::sprite,
				player_data.mouse_selected_tile_in_range() ? "__core__/cursor-select" : "__core__/cursor-invalid");
			assert(sprite_ptr != nullptr);

			tile->set_sprite_prototype(Chunk_tile::chunkLayer::overlay, sprite_ptr);

			auto& layer = tile->get_layer(Chunk_tile::chunkLayer::overlay);
			if (layer.is_multi_tile())
				layer.get_multi_tile_data().multi_tile_span = 1;
		}

		last_tile_dimensions_ = {1, 1};
	}
}
