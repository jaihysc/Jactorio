#include "game/input/input_manager.h"

#include <unordered_map>
#include <vector>
#include <algorithm>

#include "core/data_type/unordered_map.h"

// Increments with each new assigned callback, one is probably not having 4 million registered callbacks
// so this doesn't need to be decremented
unsigned int callback_id = 1;

// tuple format: key, action, mods
// id of callbacks registered to the tuple
std::unordered_map<std::tuple<int, int, int>, std::vector<unsigned int>,
                   jactorio::core::hash<std::tuple<int, int, int>> > callback_ids;

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

void jactorio::game::input_manager::set_input(const int key, const int action, const int mods) {
	const auto input = std::tuple<int, int, int>{ key, action, mods};
	active_inputs[key] = input;
}

void jactorio::game::input_manager::raise() {
	for (auto& active_input : active_inputs) {
		std::tuple<int, int, int>& input = active_input.second;
		
		// No callbacks registered for input
		if (callback_ids.find(input) == callback_ids.end())
			continue;

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
