#include "game/input/mouse_selection.h"

#include <cmath>

#include "jactorio.h"

#include "data/data_manager.h"
#include "data/prototype/entity/entity.h"
#include "game/logic/entity_place_controller.h"
#include "game/player/player_manager.h"
#include "game/world/chunk_tile.h"
#include "game/world/world_manager.h"
#include "renderer/opengl/shader_manager.h"
#include "renderer/rendering/renderer.h"

double x_position = 0.f;
double y_position = 0.f;

void jactorio::game::mouse_selection::set_cursor_position(const double x_pos, const double y_pos) {
	x_position = x_pos;
	y_position = y_pos;
}

double jactorio::game::mouse_selection::get_position_x() {
	return x_position;
}

double jactorio::game::mouse_selection::get_position_y() {
	return y_position;
}

std::pair<int, int> jactorio::game::mouse_selection::get_mouse_tile_coords() {
	// TODO buffer the mouse selected tile??
	float world_x = player_manager::get_player_position_x();
	float world_y = player_manager::get_player_position_y();

	float pixels_from_center_x;
	float pixels_from_center_y;
	{
		const unsigned short window_width = renderer::Renderer::get_window_width();
		const unsigned short window_height = renderer::Renderer::get_window_height();
		const auto& matrix = renderer::get_mvp_matrix();

		// Account for MVP matrices
		// Normalize to -1 | 1 used by the matrix
		const double norm_x = 2 * (get_position_x() / window_width) - 1;
		const double norm_y = 2 * (get_position_y() / window_height) - 1;

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
			static_cast<float>(renderer::Renderer::tile_width) * (world_x - static_cast<int>(world_x));
		// This is plus since the y axis is inverted
		mouse_y_center +=
			static_cast<float>(renderer::Renderer::tile_width) * (world_y - static_cast<int>(world_y));


		pixels_from_center_x = norm_positions.x - mouse_x_center;
		pixels_from_center_y = mouse_y_center - norm_positions.y;
	}

	// Calculate tile position based on current player position
	world_x = static_cast<int>(world_x) + pixels_from_center_x / static_cast<float>(renderer::Renderer::tile_width);
	world_y = static_cast<int>(world_y) + pixels_from_center_y / static_cast<float>(renderer::Renderer::tile_width);

	// Subtract extra tile if negative because no tile exists at -0, -0
	if (world_x < 0)
		world_x -= 1.f;
	if (world_y < 0)
		world_y -= 1.f;

	return std::pair<int, int>(world_x, world_y);
}

bool jactorio::game::mouse_selection::selected_tile_in_range() {
	const auto cursor_position = get_mouse_tile_coords();

	// Maximum distance of from the player where tiles can be reached
	constexpr unsigned int max_reach = 14;
	const unsigned int tile_dist =
		abs(player_manager::get_player_position_x() - cursor_position.first) +
		abs(player_manager::get_player_position_y() - cursor_position.second);
	
	return tile_dist <= max_reach;
}

// The last tile cannot be stored as a pointer as it can be deleted if the world was regenerated
std::pair<int, int> last_tile_pos;

void jactorio::game::mouse_selection::draw_tile_at_cursor(const std::string& iname,
                                                          const int offset_x = 0, const int offset_y = 0) {
	auto cursor_position = get_mouse_tile_coords();
	cursor_position.first += offset_x;
	cursor_position.second += offset_y;
	
	auto* tile = world_manager::get_tile_world_coords(cursor_position.first, cursor_position.second);

	if (tile == nullptr)
		return;
	
	// Draw tile on the overlay layer
	const auto sprite_ptr = data::data_manager::data_raw_get<data::Sprite>(data::data_category::sprite, iname);
	assert(sprite_ptr != nullptr);
	
	tile->set_tile_layer_sprite_prototype(Chunk_tile::chunk_layer::overlay, sprite_ptr);
	tile->get_layer(Chunk_tile::chunk_layer::overlay).multi_tile_span = 1;
	
	last_tile_pos = cursor_position;
}

std::pair<int, int> last_tile_dimensions;

/**
 * Draws a cursor over the tile currently selected
 */
void draw_selection_box() {
	using namespace jactorio::game;

	const auto cursor_position = mouse_selection::get_mouse_tile_coords();

	// Only draw cursor when over entities
	const auto tile = world_manager::get_tile_world_coords(cursor_position.first, cursor_position.second);
	if (tile == nullptr || tile->entity == nullptr) {
		return;
	}
	
	// Draw invalid cursor if range tis too far
	mouse_selection::draw_tile_at_cursor(
		mouse_selection::selected_tile_in_range() ? "__core__/cursor-select" : "__core__/cursor-invalid");

	last_tile_dimensions.first = 1;
	last_tile_dimensions.second = 1;
}

bool clear_entity_placement_ghost = false;
void jactorio::game::mouse_selection::draw_cursor_overlay() {
	// Remove selection cursor
	{
		auto* last_tile = world_manager::get_tile_world_coords(last_tile_pos.first,
		                                                       last_tile_pos.second);
		if (last_tile == nullptr)
			return;

		last_tile->set_tile_layer_sprite_prototype(Chunk_tile::chunk_layer::overlay,
		                                           nullptr);
	}
	

	// Clear last entity ghost
	if (clear_entity_placement_ghost) {
		logic::place_sprite_at_coords(
			Chunk_tile::chunk_layer::overlay,
			nullptr,
			last_tile_dimensions.first, last_tile_dimensions.second,
			last_tile_pos.first, last_tile_pos.second);

		clear_entity_placement_ghost = false;
	}


	// Draw new overlay
	const auto cursor_position = get_mouse_tile_coords();
	last_tile_pos = cursor_position;
	
	const data::item_stack* ptr;
	if ((ptr = player_manager::get_selected_item()) != nullptr) {
		const auto entity_ptr = static_cast<data::Entity*>(ptr->first->entity_prototype);

		// Ensure selected item is an entity to draw preview
		if (entity_ptr != nullptr) {
			logic::place_sprite_at_coords(
				Chunk_tile::chunk_layer::overlay, 
				entity_ptr->sprite,
				entity_ptr->tile_width, entity_ptr->tile_height, 
				cursor_position.first, cursor_position.second);
			
			last_tile_dimensions.first = entity_ptr->tile_width;
			last_tile_dimensions.second = entity_ptr->tile_height;

			clear_entity_placement_ghost = true;
			return;
		}
	}

	// Draw selection box if no item selected or item selected is not entity
	draw_selection_box();
}
