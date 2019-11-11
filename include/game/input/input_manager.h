#ifndef GAME_INPUT_INPUT_MANAGER_H
#define GAME_INPUT_INPUT_MANAGER_H

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
	unsigned register_input_callback(input_callback, int key, int action, int mods = 0);

	
	/**
	 * Sets the inputs combinations when dispatching the input callback<br>
	 * Callbacks are called when dispatch_input_callbacks() is called
	 */
	void set_input(int key, int action, int mods = 0);

	/**
	 * Calls registered callbacks based on the input set with set_input(...)
	 */
	void dispatch_input_callbacks();


	/**
	 * Removes specified callback at callback_id <br>
	 * The key, action, and mods the callback was registered to must be specified
	 */
	void remove_input_callback(unsigned int callback_id, int key, int action, int mods = 0);
}

#endif // GAME_INPUT_INPUT_MANAGER_H
