#include "game/input/mouse_selection.h"

#include <cmath>

#include "data/data_manager.h"
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

std::pair<int, int> jactorio::game::mouse_selection::get_mouse_selected_tile() {
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

// The last tile cannot be stored as a pointer as it can be deleted if the world was regenerated
std::pair<int, int> last_tile_pos;
void jactorio::game::mouse_selection::draw_cursor_selected_tile() {
	// Maximum distance of from the player where tiles can be reached
	constexpr unsigned int max_reach = 14;
	
	const auto cursor_position = get_mouse_selected_tile();
	
	auto* tile = world_manager::get_tile_world_coords(cursor_position.first, cursor_position.second);
	auto* last_tile = world_manager::get_tile_world_coords(last_tile_pos.first, last_tile_pos.second);

	if (tile == nullptr)
		return;

	// Delete the cursor at the last_tile if exists
	if (last_tile != nullptr) {
		last_tile->set_tile_prototype(Chunk_tile::prototype_category::overlay, nullptr);
	}

	// Draw invalid cursor if range tis too far
	const unsigned int tile_dist = 
		abs(player_manager::get_player_position_x() - cursor_position.first) + 
		abs(player_manager::get_player_position_y() - cursor_position.second);

	std::string cursor_iname = "__core__/cursor-select";
	if (tile_dist > max_reach)
		cursor_iname = "__core__/cursor-invalid";
	
	// Draw cursor on the overlay layer
	tile->set_tile_prototype(
		Chunk_tile::prototype_category::overlay,
		data::data_manager::data_raw_get<data::Tile>(data::data_category::tile, cursor_iname)
	);
	last_tile_pos = cursor_position;
}
