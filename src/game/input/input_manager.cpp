// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 11/15/2019

#include "game/input/input_manager.h"

#include <algorithm>
#include <unordered_map>
#include <vector>

#include "core/data_type.h"
#include "renderer/gui/imgui_manager.h"

// Increments with each new assigned callback, one is probably not having 4 million registered callbacks
// so this doesn't need to be decremented
unsigned int callback_id = 1;

// tuple format: key, action, mods
// id of callbacks registered to the tuple
std::unordered_map<std::tuple<int, int, int>, std::vector<unsigned int>,
                   jactorio::core::hash<std::tuple<int, int, int>>> callback_ids;

std::unordered_map<unsigned int, input_callback> input_callbacks;


// Currently set input(s)
std::unordered_map<int, std::tuple<int, int, int>> active_inputs;

unsigned jactorio::game::input_manager::subscribe(const input_callback callback,
                                                  const int key, const int action,
                                                  const int mods) {
	// // Retrieve scancode of specified key
	// const int key = glfwGetKeyScancode(key);
	// if (key == -1)
	// 	return 0;

	// Assign an id to the callback
	callback_ids[std::tuple<int, int, int>{key, action, mods}]
		.push_back(callback_id);

	// Store callback under id
	input_callbacks[callback_id] = callback;

	return callback_id++;
}

void jactorio::game::input_manager::set_input(const int key, int action, const int mods) {
	// GLFW_PRESS becomes GLFW_PRESS_FIRST
	if (action == GLFW_PRESS)
		action = GLFW_PRESS_FIRST;

	const auto input = std::tuple<int, int, int>{key, action, mods};
	active_inputs[key] = input;
}

void jactorio::game::input_manager::raise() {
	// if (renderer::imgui_manager::input_captured)
	// return;

	for (auto& active_input : active_inputs) {
		std::tuple<int, int, int>& input = active_input.second;

		// No callbacks registered for input
		// if (callback_ids.find(input) == callback_ids.end())
		// continue;

		if (std::get<1>(input) == GLFW_PRESS_FIRST) {
			// Dispatch callbacks for GLFW_PRESS_FIRST
			const auto& vector = callback_ids[input];
			for (unsigned int id : vector) {
				input_callbacks[id]();
			}

			// Then dispatch callbacks for GLFW_PRESS
			std::get<1>(input) = GLFW_PRESS;
		}

		// Call callbacks registered to the input
		const auto& vector = callback_ids[input];
		for (unsigned int id : vector) {
			input_callbacks[id]();
		}

		// Event handlers for release will only trigger once
		if (std::get<1>(input) == GLFW_RELEASE) {
			std::get<1>(input) = -1;
		}
	}
}

void jactorio::game::input_manager::unsubscribe(const unsigned callback_id, const int key,
                                                const int action, const int mods) {
	auto& id_vector = callback_ids[
		std::tuple<int, int, int>{key, action, mods}
	];

	// Erase the callback id to the callback
	id_vector.erase(
		std::remove(id_vector.begin(), id_vector.end(), callback_id),
		id_vector.end());

	// Erase the callback itself
	input_callbacks.erase(callback_id);
}

void jactorio::game::input_manager::clear_data() {
	input_callbacks.clear();
	callback_ids.clear();
}
