// 
// mouse_selection.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 12/21/2019
// Last modified: 03/21/2020
// 

#include "game/input/mouse_selection.h"

#include <cmath>

#include "jactorio.h"

#include "data/data_manager.h"
#include "data/prototype/entity/entity.h"
#include "game/logic/placement_controller.h"
#include "game/player/player_data.h"
#include "game/world/chunk_tile.h"
#include "game/world/chunk_tile_getters.h"
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


void jactorio::game::Mouse_selection::calculate_selected_tile(float player_x, float player_y) {
	last_player_x_ = player_x;
	last_player_y_ = player_y;

	float pixels_from_center_x;
	float pixels_from_center_y;
	{
		const unsigned short window_width = renderer::Renderer::get_window_width();
		const unsigned short window_height = renderer::Renderer::get_window_height();
		const auto& matrix = renderer::get_mvp_matrix();

		// Account for MVP matrices
		// Normalize to -1 | 1 used by the matrix
		const double norm_x = 2 * (get_cursor_x() / window_width) - 1;
		const double norm_y = 2 * (get_cursor_y() / window_height) - 1;

		// A = C / B
		const glm::vec4 norm_positions = matrix / glm::vec4(norm_x, norm_y, 1, 1);


		float mouse_x_center;
		float mouse_y_center;
		{
			// Calculate the center tile on screen
			// Calculate number of pixels from center
			const double win_center_norm_x = 2 * (static_cast<double>(window_width) / 2 / window_width) - 1;
			const double win_center_norm_y = 2 * (static_cast<double>(window_height) / 2 / window_height) - 1;

			const glm::vec4 win_center_norm_positions =
				matrix / glm::vec4(win_center_norm_x, win_center_norm_y, 1, 1);

			mouse_x_center = win_center_norm_positions.x;
			mouse_y_center = win_center_norm_positions.y;
		}

		// If player is standing on a partial tile, adjust the center accordingly to the correct location
		mouse_x_center -=
			static_cast<float>(renderer::Renderer::tile_width) * (player_x - static_cast<int>(player_x));
		// This is plus since the y axis is inverted
		mouse_y_center +=
			static_cast<float>(renderer::Renderer::tile_width) * (player_y - static_cast<int>(player_y));


		pixels_from_center_x = norm_positions.x - mouse_x_center;
		pixels_from_center_y = mouse_y_center - norm_positions.y;
	}

	// Calculate tile position based on current player position
	player_x = static_cast<int>(player_x) + pixels_from_center_x / static_cast<float>(renderer::Renderer::tile_width);
	player_y = static_cast<int>(player_y) + pixels_from_center_y / static_cast<float>(renderer::Renderer::tile_width);

	// Subtract extra tile if negative because no tile exists at -0, -0
	if (player_x < 0)
		player_x -= 1.f;
	if (player_y < 0)
		player_y -= 1.f;

	mouse_selected_tile_ = std::pair<int, int>(player_x, player_y);
}


// ======================================================================

bool jactorio::game::Mouse_selection::selected_tile_in_range() const {
	const auto cursor_position = get_mouse_tile_coords();

	// Maximum distance of from the player where tiles can be reached
	constexpr unsigned int max_reach = 14;
	const unsigned int tile_dist =
		abs(last_player_x_ - cursor_position.first) +
		abs(last_player_y_ - cursor_position.second);

	return tile_dist <= max_reach;
}

void jactorio::game::Mouse_selection::draw_cursor_overlay(Player_data& player_data) {
	auto* last_tile = player_data.get_player_world().get_tile_world_coords(last_tile_pos_.first,
	                                                                       last_tile_pos_.second);
	if (last_tile == nullptr)
		return;


	const auto cursor_position = get_mouse_tile_coords();
	const data::item_stack* ptr;
	if ((ptr = player_data.get_selected_item()) != nullptr)
		draw_overlay(player_data.get_player_world(), static_cast<data::Entity*>(ptr->first->entity_prototype),
		             cursor_position.first, cursor_position.second, player_data.placement_orientation);
	else
		draw_overlay(player_data.get_player_world(), nullptr,
		             cursor_position.first, cursor_position.second, player_data.placement_orientation);
}

void jactorio::game::Mouse_selection::draw_overlay(World_data& world_data, data::Entity* const selected_entity,
                                                   const int world_x, const int world_y,
                                                   const data::placementOrientation placement_orientation) {
	auto* last_tile = world_data.get_tile_world_coords(last_tile_pos_.first,
	                                                   last_tile_pos_.second);
	auto* tile = world_data.get_tile_world_coords(world_x, world_y);


	// Clear last overlay
	if (!last_tile)
		return;
	placement_c::place_sprite_at_coords(
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
		placement_c::place_sprite_at_coords(world_data, Chunk_tile::chunkLayer::overlay, selected_entity->sprite,
		                                    selected_entity->tile_width, selected_entity->tile_height, world_x,
		                                    world_y);

		// Rotatable entities
		const data::Rotatable_entity* rotatable_entity;
		if (selected_entity->rotatable &&
			(rotatable_entity = dynamic_cast<data::Rotatable_entity*>(selected_entity)) != nullptr) {
			const auto target = rotatable_entity->map_placement_orientation(
				placement_orientation,
				&world_data.get_tile_world_coords(
					           world_x,
					           world_y - 1)
				           ->get_layer(Chunk_tile::chunkLayer::entity),
				&world_data.get_tile_world_coords(
					           world_x + 1,
					           world_y)
				           ->get_layer(Chunk_tile::chunkLayer::entity),
				&world_data.get_tile_world_coords(
					           world_x,
					           world_y + 1)
				           ->get_layer(Chunk_tile::chunkLayer::entity),
				&world_data.get_tile_world_coords(
					           world_x - 1,
					           world_y)
				           ->get_layer(Chunk_tile::chunkLayer::entity));

			tile->get_layer(Chunk_tile::chunkLayer::overlay).prototype_data = selected_entity->sprite;
			tile->get_layer(Chunk_tile::chunkLayer::overlay).unique_data =
				new data::Renderable_data(target.first, target.second);
		}

		last_tile_dimensions_ = {selected_entity->tile_width, selected_entity->tile_height};
	}
	else {
		// No item selected
		if (tile->get_layer(Chunk_tile::chunkLayer::entity).prototype_data ||
			tile->get_layer(Chunk_tile::chunkLayer::resource).prototype_data) {

			// Is hovering over entity	
			const auto sprite_ptr = data::data_manager::data_raw_get<data::Sprite>(
				data::data_category::sprite,
				selected_tile_in_range() ? "__core__/cursor-select" : "__core__/cursor-invalid");
			assert(sprite_ptr != nullptr);

			chunk_tile_getter::set_sprite_prototype(*tile, Chunk_tile::chunkLayer::overlay, sprite_ptr);
			tile->get_layer(Chunk_tile::chunkLayer::overlay).multi_tile_span = 1;
		}
	}
}
