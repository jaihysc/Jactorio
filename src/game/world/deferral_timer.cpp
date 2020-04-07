// 
// deferral_timer.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 04/02/2020
// Last modified: 04/06/2020
// 

#include "game/world/deferral_timer.h"

void jactorio::game::Deferral_timer::deferral_update(const game_tick_t game_tick) {
	last_game_tick_ = game_tick;

	// Call callbacks
	for (auto& pair : callbacks_[game_tick]) {
		pair.first.get().on_defer_time_elapsed(*this, pair.second);
	}

	// Remove used callbacks
	callbacks_.erase(game_tick);
}

jactorio::game::Deferral_timer::callback_index jactorio::game::Deferral_timer::register_at_tick(
	const data::Deferred& deferred, data::Unique_data_base* unique_data,
	const game_tick_t due_game_tick) {
	assert(due_game_tick > last_game_tick_);

	auto& callbacks = callbacks_[due_game_tick];
	callbacks.emplace_back(std::ref(deferred), unique_data);

	return callbacks.size() - 1;
}

jactorio::game::Deferral_timer::callback_index jactorio::game::Deferral_timer::register_from_tick(const data::Deferred& deferred,
                                                                                                  data::Unique_data_base*
                                                                                                  unique_data,
                                                                                                  const game_tick_t
                                                                                                  elapse_game_tick) {
	return register_at_tick(deferred, unique_data, last_game_tick_ + elapse_game_tick);
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
