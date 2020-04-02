// 
// deferral_timer.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 04/02/2020
// Last modified: 04/02/2020
// 

#include "game/world/deferral_timer.h"

void jactorio::game::Deferral_timer::deferral_update(const game_tick_t game_tick) {
	// Call callbacks
	for (std::pair<data::Deferred&, data::Unique_data_base*> pair : callbacks_[game_tick]) {
		pair.first.on_defer_time_elapsed(pair.second);
	}

	// Remove used callbacks
	callbacks_.erase(game_tick);
}

jactorio::game::Deferral_timer::callback_index jactorio::game::Deferral_timer::register_deferral(
	data::Deferred& deferred, data::Unique_data_base* unique_data,
	const game_tick_t due_game_tick) {

	auto& callbacks = callbacks_[due_game_tick];
	callbacks.emplace_back(deferred, unique_data);

	return callbacks.size() - 1;
}

void jactorio::game::Deferral_timer::remove_deferral(const game_tick_t due_game_tick, const callback_index index) {
	// due_game_tick does not exist
	if (callbacks_.find(due_game_tick) == callbacks_.end())
		return;

	auto& callbacks = callbacks_[due_game_tick];

	// Index out of range
	assert(index < callbacks.size());
	callbacks.erase(callbacks.begin() + index);
}
