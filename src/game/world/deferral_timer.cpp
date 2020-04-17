// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 04/02/2020

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

jactorio::game::Deferral_timer::deferral_entry jactorio::game::Deferral_timer::register_at_tick(
	const data::Deferred& deferred, data::Unique_data_base* unique_data,
	const game_tick_t due_game_tick) {
	assert(due_game_tick > last_game_tick_);

	auto& callbacks = callbacks_[due_game_tick];
	callbacks.emplace_back(std::ref(deferred), unique_data);

	return {due_game_tick, callbacks.size()};
}

jactorio::game::Deferral_timer::deferral_entry jactorio::game::Deferral_timer::register_from_tick(const data::Deferred& deferred,
                                                                                                  data::Unique_data_base*
                                                                                                  unique_data,
                                                                                                  const game_tick_t
                                                                                                  elapse_game_tick) {
	assert(elapse_game_tick > 0);
	return register_at_tick(deferred, unique_data, last_game_tick_ + elapse_game_tick);
}

void jactorio::game::Deferral_timer::remove_deferral(deferral_entry entry) {
	assert(entry.second != 0);  // Invalid callback index

	// due_game_tick does not exist
	if (callbacks_.find(entry.first) == callbacks_.end())
		return;

	auto& callbacks = callbacks_[entry.first];

	// Index is +1 than actual index
	entry.second -= 1;
	assert(entry.second <= callbacks.size());  // Index out of range

	// Instead of erasing, make the callback a blank function so that future remove calls do not go out of range
	callbacks[entry.second].first = blank_callback_;
}
