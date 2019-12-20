#ifndef GAME_INPUT_MOUSE_SELECTION_H
#define GAME_INPUT_MOUSE_SELECTION_H

#include <utility>

/**
 * Handles mouse selection within the game
 */
namespace jactorio::game::mouse_selection
{
	/**
	 * Callback provided to glfwSetCursorPosCallback to set mouse position <br>
	 */
	void set_cursor_position(double x_pos, double y_pos);

	[[nodiscard]] double get_position_x();
	[[nodiscard]] double get_position_y();

	/**
	 * Gets the world X, Y of the tile the mouse is hovered over
	 */
	std::pair<int, int> get_mouse_selected_tile();

	/**
	 * Draws a cursor over the tile currently selected
	 */
	void draw_cursor_selected_tile();
	
}

#endif // GAME_INPUT_MOUSE_SELECTION_H
