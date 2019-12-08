#ifndef GAME_INPUT_MOUSE_SELECTION_H
#define GAME_INPUT_MOUSE_SELECTION_H

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

	
}

#endif // GAME_INPUT_MOUSE_SELECTION_H
