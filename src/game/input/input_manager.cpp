// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 11/15/2019

#include "game/input/input_manager.h"

#include <unordered_map>
#include <vector>
#include <GLFW/glfw3.h>

#include "jactorio.h"
#include "core/data_type.h"
#include "game/input/input_key.h"

std::unordered_map<jactorio::game::InputKey,
                   jactorio::game::KeyInput::InputTuple> jactorio::game::KeyInput::activeInputs_{};

unsigned jactorio::game::KeyInput::Subscribe(const InputCallback& callback,
                                             const InputKey key,
                                             const InputAction action,
                                             const InputMod mods) {
	// Assign an id to the callback
	callbackIds_[{key, action, mods}].push_back(callbackId_);

	// Store callback under id
	inputCallbacks_[callbackId_] = callback;

	return callbackId_++;
}

void jactorio::game::KeyInput::SetInput(const InputKey key, InputAction action, const InputMod mods) {
	assert(action != InputAction::key_held);  // Not valid for setting an input, either pressed or repeat
	assert(action != InputAction::key_pressed);  // Not valid for setting an input, use key_down

	activeInputs_[key] = {key, action, mods};
}

void jactorio::game::KeyInput::CallCallbacks(const InputTuple& input) {
	const auto& vector = callbackIds_[input];
	for (unsigned int id : vector) {
		inputCallbacks_[id]();
	}
}

void jactorio::game::KeyInput::Raise() {
	// if (renderer::imgui_manager::input_captured)
	// return;

	// active_input is the keys which are held (active)
	for (auto& active_input : activeInputs_) {
		auto& input = active_input.second;

		switch (std::get<1>(input)) {
		case InputAction::key_down:
			std::get<1>(input) = InputAction::key_down;
			CallCallbacks(input);

			std::get<1>(input) = InputAction::key_held;
			CallCallbacks(input);

			// key_pressed and key_held events also get called
			std::get<1>(input) = InputAction::key_pressed;  // Changed to key_pressed
			CallCallbacks(input);
			break;

		case InputAction::key_pressed:
			// key_held events get called
			std::get<1>(input) = InputAction::key_held;
			CallCallbacks(input);

			std::get<1>(input) = InputAction::key_pressed;  // Change back to original
			CallCallbacks(input);
			break;

		case InputAction::key_repeat:
			// key_held events get called
			std::get<1>(input) = InputAction::key_held;
			CallCallbacks(input);

			std::get<1>(input) = InputAction::key_repeat;  // Change back to original
			CallCallbacks(input);
			break;


			// Event handlers for key_up will only trigger once
		case InputAction::key_up:
			CallCallbacks(input);
			std::get<1>(input) = InputAction::none;
			break;

		default:
			break;
		}
	}
}

void jactorio::game::KeyInput::Unsubscribe(const unsigned callback_id,
                                           const InputKey key,
                                           const InputAction action,
                                           const InputMod mods) {
	auto& id_vector = callbackIds_[{key, action, mods}];

	// Erase the callback id to the callback
	id_vector.erase(
		std::remove(id_vector.begin(), id_vector.end(), callback_id),
		id_vector.end());

	// Erase the callback itself
	inputCallbacks_.erase(callback_id);
}

void jactorio::game::KeyInput::ClearData() {
	inputCallbacks_.clear();
	callbackIds_.clear();
}

// ======================================================================

jactorio::game::InputKey jactorio::game::KeyInput::ToInputKey(const int key) {
	// Mouse
	if (GLFW_MOUSE_BUTTON_1 <= key && key <= GLFW_MOUSE_BUTTON_8)
		return static_cast<InputKey>(key - GLFW_MOUSE_BUTTON_1 + static_cast<int>(InputKey::mouse1));

	// Keyboard
	if (GLFW_KEY_0 <= key && key <= GLFW_KEY_9)
		return static_cast<InputKey>(key - GLFW_KEY_0 + static_cast<int>(InputKey::k0));

	if (GLFW_KEY_A <= key && key <= GLFW_KEY_Z)
		return static_cast<InputKey>(key - GLFW_KEY_A + static_cast<int>(InputKey::a));

	switch (key) {
	case GLFW_KEY_GRAVE_ACCENT:
		return InputKey::grave;
	case GLFW_KEY_TAB:
		return InputKey::tab;
	case GLFW_KEY_ESCAPE:
		return InputKey::escape;
	case GLFW_KEY_SPACE:
		return InputKey::space;

	default:
		break;
	}

	LOG_MESSAGE_f(warning, "Key id %d is not mapped to an inputKey, this input will be ignored", key);
	return InputKey::none;
}

jactorio::game::InputAction jactorio::game::KeyInput::ToInputAction(const int action) {
	switch (action) {
	case GLFW_PRESS:
		return InputAction::key_down;
	case GLFW_REPEAT:
		return InputAction::key_repeat;
	case GLFW_RELEASE:
		return InputAction::key_up;

	default:
		LOG_MESSAGE_f(warning, "Action id %d is not mapped to an inputAction, this input will be ignored", action);
		return InputAction::none;
	}
}

jactorio::game::InputMod jactorio::game::KeyInput::ToInputMod(const int mod) {
	switch (mod) {
	case 0:
		return InputMod::none;

	case GLFW_MOD_ALT:
		return InputMod::alt;
	case GLFW_MOD_CAPS_LOCK:
		return InputMod::caps_lk;
	case GLFW_MOD_CONTROL:
		return InputMod::control;
	case GLFW_MOD_SHIFT:
		return InputMod::shift;
	case GLFW_MOD_SUPER:
		return InputMod::super;

	default:
		LOG_MESSAGE_f(warning, "Modifier id %d is not mapped to an inputMod, this input will be ignored", mod);
		return InputMod::none;
	}
}
