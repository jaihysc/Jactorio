#include "game/input/mouse_selection.h"

#include "renderer/rendering/renderer.h"
#include "renderer/opengl/shader_manager.h"
#include "game/player/player_manager.h"

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
	int pixels_from_center_x;
	int pixels_from_center_y;
	{
		// Account for MVP matrices
		// Normalize to -1 | 1 used by the matrix
		const double norm_x = 2 * (get_position_x() / renderer::Renderer::get_window_width()) - 1;
		const double norm_y = 2 * (get_position_y() / renderer::Renderer::get_window_height()) - 1;

		// A = C / B
		const glm::vec4 norm_positions = renderer::get_mvp_matrix() / glm::vec4(norm_x, norm_y, 1, 1);

		// Calculate number of pixels from center
		const int mouse_x_center = renderer::Renderer::get_window_width() / 2;
		const int mouse_y_center = renderer::Renderer::get_window_height() / 2;

		pixels_from_center_x = static_cast<int>(norm_positions.x - static_cast<float>(mouse_x_center));
		pixels_from_center_y = static_cast<int>(static_cast<float>(mouse_y_center) - norm_positions.y);
	}

	float world_x = player_manager::get_player_position_x();
	float world_y = player_manager::get_player_position_y();

	// Calculate tile position based on current player position
	world_x += pixels_from_center_x / renderer::Renderer::tile_width;
	world_y += pixels_from_center_y / renderer::Renderer::tile_width;

	return std::pair<int, int>(world_x, world_y);
}
