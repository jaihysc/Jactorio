// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 11/15/2019

#include "game/input/input_manager.h"

#include <unordered_map>
#include <vector>
#include <GLFW/glfw3.h>

#include "jactorio.h"
#include "core/data_type.h"
#include "game/input/input_key.h"

std::unordered_map<jactorio::game::inputKey,
                   jactorio::game::Key_input::input_tuple> jactorio::game::Key_input::active_inputs_{};

unsigned jactorio::game::Key_input::subscribe(const input_callback& callback,
                                              const inputKey key,
                                              const inputAction action,
                                              const inputMod mods) {
	// Assign an id to the callback
	callback_ids_[{key, action, mods}].push_back(callback_id_);

	// Store callback under id
	input_callbacks_[callback_id_] = callback;

	return callback_id_++;
}

void jactorio::game::Key_input::set_input(const inputKey key, inputAction action, const inputMod mods) {
	assert(action != inputAction::key_held);  // Not valid for setting an input, either pressed or repeat
	assert(action != inputAction::key_pressed);  // Not valid for setting an input, use key_down

	active_inputs_[key] = {key, action, mods};
}

void jactorio::game::Key_input::call_callbacks(const input_tuple& input) {
	const auto& vector = callback_ids_[input];
	for (unsigned int id : vector) {
		input_callbacks_[id]();
	}
}

void jactorio::game::Key_input::raise() {
	// if (renderer::imgui_manager::input_captured)
	// return;

	// active_input is the keys which are held (active)
	for (auto& active_input : active_inputs_) {
		auto& input = active_input.second;

		switch (std::get<1>(input)) {
		case inputAction::key_down:
			std::get<1>(input) = inputAction::key_down;
			call_callbacks(input);

			std::get<1>(input) = inputAction::key_held;
			call_callbacks(input);

			// key_pressed and key_held events also get called
			std::get<1>(input) = inputAction::key_pressed;  // Changed to key_pressed
			call_callbacks(input);
			break;

		case inputAction::key_pressed:
			// key_held events get called
			std::get<1>(input) = inputAction::key_held;
			call_callbacks(input);

			std::get<1>(input) = inputAction::key_pressed;  // Change back to original
			call_callbacks(input);
			break;

		case inputAction::key_repeat:
			// key_held events get called
			std::get<1>(input) = inputAction::key_held;
			call_callbacks(input);

			std::get<1>(input) = inputAction::key_repeat;  // Change back to original
			call_callbacks(input);
			break;


			// Event handlers for key_up will only trigger once
		case inputAction::key_up:
			call_callbacks(input);
			std::get<1>(input) = inputAction::none;
			break;

		default:
			break;
		}
	}
}

void jactorio::game::Key_input::unsubscribe(const unsigned callback_id,
                                            const inputKey key,
                                            const inputAction action,
                                            const inputMod mods) {
	auto& id_vector = callback_ids_[{key, action, mods}];

	// Erase the callback id to the callback
	id_vector.erase(
		std::remove(id_vector.begin(), id_vector.end(), callback_id),
		id_vector.end());

	// Erase the callback itself
	input_callbacks_.erase(callback_id);
}

void jactorio::game::Key_input::clear_data() {
	input_callbacks_.clear();
	callback_ids_.clear();
}

// ======================================================================

jactorio::game::inputKey jactorio::game::Key_input::to_input_key(const int key) {
	// Mouse
	if (GLFW_MOUSE_BUTTON_1 <= key && key <= GLFW_MOUSE_BUTTON_8)
		return static_cast<inputKey>(key - GLFW_MOUSE_BUTTON_1 + static_cast<int>(inputKey::mouse1));

	// Keyboard
	if (GLFW_KEY_0 <= key && key <= GLFW_KEY_9)
		return static_cast<inputKey>(key - GLFW_KEY_0 + static_cast<int>(inputKey::k0));

	if (GLFW_KEY_A <= key && key <= GLFW_KEY_Z)
		return static_cast<inputKey>(key - GLFW_KEY_A + static_cast<int>(inputKey::a));

	switch (key) {
	case GLFW_KEY_GRAVE_ACCENT:
		return inputKey::grave;
	case GLFW_KEY_TAB:
		return inputKey::tab;
	case GLFW_KEY_ESCAPE:
		return inputKey::escape;
	case GLFW_KEY_SPACE:
		return inputKey::space;

	default:
		break;
	}

	LOG_MESSAGE_f(warning, "Key id %d is not mapped to an inputKey, this input will be ignored", key);
	return inputKey::none;
}

jactorio::game::inputAction jactorio::game::Key_input::to_input_action(const int action) {
	switch (action) {
	case GLFW_PRESS:
		return inputAction::key_down;
	case GLFW_REPEAT:
		return inputAction::key_repeat;
	case GLFW_RELEASE:
		return inputAction::key_up;

	default:
		LOG_MESSAGE_f(warning, "Action id %d is not mapped to an inputAction, this input will be ignored", action);
		return inputAction::none;
	}
}

jactorio::game::inputMod jactorio::game::Key_input::to_input_mod(const int mod) {
	switch (mod) {
	case 0:
		return inputMod::none;

	case GLFW_MOD_ALT:
		return inputMod::alt;
	case GLFW_MOD_CAPS_LOCK:
		return inputMod::caps_lk;
	case GLFW_MOD_CONTROL:
		return inputMod::control;
	case GLFW_MOD_SHIFT:
		return inputMod::shift;
	case GLFW_MOD_SUPER:
		return inputMod::super;

	default:
		LOG_MESSAGE_f(warning, "Modifier id %d is not mapped to an inputMod, this input will be ignored", mod);
		return inputMod::none;
	}
}
