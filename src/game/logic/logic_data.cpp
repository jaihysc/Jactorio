// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/logic/logic_data.h"

#include "game/world/world_data.h"

using namespace jactorio;

void game::LogicData::DeferralTimer::DeferralUpdate(WorldData& world_data, const GameTickT game_tick) {
	if (game_tick > 0)
		assert(game_tick > lastGameTick_); // assertion would fail on game tick 0, since lastGameTick would be 0
	else
		assert(game_tick >= lastGameTick_);

	lastGameTick_ = game_tick;

	// Call callbacks
	for (auto& pair : callbacks_[game_tick]) {
		pair.prototype.get().OnDeferTimeElapsed(world_data, logicData_, pair.uniqueData);
	}

	// Remove used callbacks
	callbacks_.erase(game_tick);
}

game::LogicData::DeferralTimer::DeferralEntry game::LogicData::DeferralTimer::RegisterAtTick(const data::IDeferred& deferred,
	data::UniqueDataBase* unique_data,
	const GameTickT due_game_tick) {
	assert(due_game_tick > lastGameTick_);

	auto& due_tick_callback = callbacks_[due_game_tick];
	due_tick_callback.emplace_back(CallbackContainerEntry{std::ref(deferred), unique_data});

	return {due_game_tick, due_tick_callback.size()};
}

game::LogicData::DeferralTimer::DeferralEntry game::LogicData::DeferralTimer::RegisterFromTick(const data::IDeferred& deferred,
	data::UniqueDataBase* unique_data,
	const GameTickT elapse_game_tick) {

	assert(elapse_game_tick > 0);
	return RegisterAtTick(deferred, unique_data, lastGameTick_ + elapse_game_tick);
}

void game::LogicData::DeferralTimer::RemoveDeferral(DeferralEntry entry) {
	assert(entry.callbackIndex != 0);  // Invalid callback index

	// due_game_tick does not exist
	if (callbacks_.find(entry.dueTick) == callbacks_.end())
		return;

	auto& due_tick_callback = callbacks_[entry.dueTick];

	// Index is +1 than actual index
	entry.callbackIndex -= 1;
	assert(entry.callbackIndex <= due_tick_callback.size());  // Index out of range

	// Instead of erasing, make the callback a blank function so that future remove calls do not go out of range
	due_tick_callback[entry.callbackIndex].prototype = blankCallback_;
}

void game::LogicData::DeferralTimer::RemoveDeferralEntry(DeferralEntry& entry) {
	if (!entry.Valid())
		return;

	RemoveDeferral(entry);
	entry.Invalidate();
}

game::LogicData::DeferralTimer::DebugInfo game::LogicData::DeferralTimer::GetDebugInfo() const {
	return {callbacks_};
}
