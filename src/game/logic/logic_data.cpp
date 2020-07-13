// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 07/13/2020

#include "game/logic/logic_data.h"

#include "game/world/world_data.h"

using namespace jactorio;

void game::LogicData::DeferralTimer::DeferralUpdate(WorldData& world_data, const GameTickT game_tick) {
	lastGameTick_ = game_tick;

	// Call callbacks
	for (auto& pair : callbacks_[game_tick]) {
		pair.first.get().OnDeferTimeElapsed(world_data, logicData_, pair.second);
	}

	// Remove used callbacks
	callbacks_.erase(game_tick);
}

game::LogicData::DeferralTimer::DeferralEntry game::LogicData::DeferralTimer::RegisterAtTick(const data::IDeferred& deferred,
                                                                                             data::UniqueDataBase* unique_data,
                                                                                             const GameTickT due_game_tick) {
	assert(due_game_tick > lastGameTick_);

	auto& due_tick_callback = callbacks_[due_game_tick];
	due_tick_callback.emplace_back(std::ref(deferred), unique_data);

	return {due_game_tick, due_tick_callback.size()};
}

game::LogicData::DeferralTimer::DeferralEntry game::LogicData::DeferralTimer::RegisterFromTick(const data::IDeferred& deferred,
                                                                                               data::UniqueDataBase* unique_data,
                                                                                               const GameTickT elapse_game_tick) {

	assert(elapse_game_tick > 0);
	return RegisterAtTick(deferred, unique_data, lastGameTick_ + elapse_game_tick);
}

void game::LogicData::DeferralTimer::RemoveDeferral(DeferralEntry entry) {
	assert(entry.second != 0);  // Invalid callback index

	// due_game_tick does not exist
	if (callbacks_.find(entry.first) == callbacks_.end())
		return;

	auto& due_tick_callback = callbacks_[entry.first];

	// Index is +1 than actual index
	entry.second -= 1;
	assert(entry.second <= due_tick_callback.size());  // Index out of range

	// Instead of erasing, make the callback a blank function so that future remove calls do not go out of range
	due_tick_callback[entry.second].first = blankCallback_;
}

void game::LogicData::DeferralTimer::RemoveDeferralEntry(DeferralEntry& entry) {
	if (entry.second == 0)
		return;

	RemoveDeferral(entry);
	entry.second = 0;
}
