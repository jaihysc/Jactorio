#ifndef GAME_INPUT_INPUT_MANAGER_H
#define GAME_INPUT_INPUT_MANAGER_H

#include <GLFW/glfw3.h>

// Different from GLFW_PRESS, PRESS_FIRST is only raised once whereas GLFW_PRESS is repeated
#define GLFW_PRESS_FIRST -2

using input_callback = void(*)();

namespace jactorio::game::input_manager
{
	/**
	 * Registers an input callback which will be called when the specified input is activated
	 * @param key Target key
	 * @param action Key state
	 * @param mods Modifier keys which also have to be pressed
	 * @return id of the registered callback, use it to un-register it - 0 Indicates error
	 */
	unsigned subscribe(input_callback, int key, int action, int mods = 0);

	
	/**
	 * Sets the static of an input<br>
	 * Callbacks for the respective inputs are called when dispatch_input_callbacks() is called
	 */
	void set_input(int key, int action, int mods = 0);

	/**
	 * Calls registered callbacks based on the input set with set_input(...)
	 */
	void raise();


	/**
	 * Removes specified callback at callback_id <br>
	 * The key, action, and mods the callback was registered to must be specified
	 */
	void unsubscribe(unsigned int callback_id, int key, int action, int mods = 0);

	/**
	 * Deletes all callback data
	 */
	void clear_data();
}

#endif // GAME_INPUT_INPUT_MANAGER_H
