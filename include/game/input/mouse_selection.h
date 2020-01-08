#ifndef GAME_INPUT_MOUSE_SELECTION_H
#define GAME_INPUT_MOUSE_SELECTION_H

#include <utility>
#include <string>

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
	 * Returns true if selected tile is within placement range
	 */
	bool selected_tile_in_range();

	
	/**
	 * Draws a sprite with specified internal name at the cursor
	 */
	void draw_tile_at_cursor(const std::string& iname, const int offset_x, const int offset_y);

	/**
	 * Clears the last drawn tile with the specified width, height
	 */
	void clear_last_tile(int width, int height);
	
	/**
	 * Draws a selection box if NO entity is selected, otherwise, draws a ghost of the entity selected at the cursor
	 */
	void draw_cursor_overlay();
}

#endif // GAME_INPUT_MOUSE_SELECTION_H
