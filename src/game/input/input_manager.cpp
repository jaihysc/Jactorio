// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/input/input_manager.h"

#include <SDL.h>
#include <unordered_map>
#include <vector>

#include "jactorio.h"
#include "core/data_type.h"

using namespace jactorio;

std::unordered_map<game::KeyInput::IntKeyMouseCodePair,
                   game::KeyInput::InputKeyData> game::KeyInput::activeInputs_{};

void game::KeyInput::SetInput(SDL_KeyCode keycode, InputAction action, SDL_Keymod mod) {
	assert(action != InputAction::key_held);  // Not valid for setting an input, either pressed or repeat
	assert(action != InputAction::key_pressed);  // Not valid for setting an input, use key_down

	activeInputs_[keycode] = InputKeyData{keycode, action, mod};
}

void game::KeyInput::SetInput(MouseInput mouse, InputAction action, SDL_Keymod mod) {
	assert(action != InputAction::key_held);  // Not valid for setting an input, either pressed or repeat
	assert(action != InputAction::key_pressed);  // Not valid for setting an input, use key_down

	auto mouse_code           = static_cast<IntKeyMouseCodePair>(mouse) * -1;
	activeInputs_[mouse_code] = InputKeyData{mouse_code, action, mod};
}

// ======================================================================

void game::KeyInput::CallCallbacks(const InputKeyData& input) {
	const auto& vector = callbackIds_[input];
	for (unsigned int id : vector) {
		inputCallbacks_[id]();
	}
}

game::KeyInput::CallbackId game::KeyInput::Register(const InputCallback& callback,
                                                    SDL_KeyCode key, InputAction action, SDL_Keymod mods) {
	// Keyboard
	// Assign an id to the callback
	callbackIds_[{key, action, mods}].push_back(callbackId_);

	// Store callback under id
	inputCallbacks_[callbackId_] = callback;

	return callbackId_++;
}

game::KeyInput::CallbackId game::KeyInput::Register(const InputCallback& callback,
                                                    MouseInput button, InputAction action, SDL_Keymod mods) {
	// Mouse
	// Assign an id to the callback
	callbackIds_[{static_cast<IntKeyMouseCodePair>(button) * -1, action, mods}].push_back(callbackId_);

	// Store callback under id
	inputCallbacks_[callbackId_] = callback;

	return callbackId_++;
}

void game::KeyInput::Raise() {
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

void game::KeyInput::Unsubscribe(const unsigned callback_id, SDL_KeyCode key, InputAction action, SDL_Keymod mods) {
	auto& id_vector = callbackIds_[{key, action, mods}];

	// Erase the callback id to the callback
	id_vector.erase(
		std::remove(id_vector.begin(), id_vector.end(), callback_id),
		id_vector.end());

	// Erase the callback itself
	inputCallbacks_.erase(callback_id);
}

void game::KeyInput::ClearData() {
	inputCallbacks_.clear();
	callbackIds_.clear();
}

game::InputAction game::KeyInput::ToInputAction(const int action, const bool repeat) {
	switch (action) {
	case SDL_KEYDOWN:
		if (repeat)
			return InputAction::key_repeat;
		return InputAction::key_down;

	case SDL_KEYUP:
		return InputAction::key_up;

	default:
		assert(false);
		break;
	}

	return InputAction::none;
}
