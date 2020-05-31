// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 04/02/2020

#ifndef JACTORIO_GAME_WORLD_DEFERRAL_TIMER_H
#define JACTORIO_GAME_WORLD_DEFERRAL_TIMER_H
#pragma once

#include <unordered_map>
#include <utility>
#include <vector>

#include "core/data_type.h"
#include "data/prototype/interface/deferred.h"

namespace jactorio::game
{
	///
	/// \brief Manages deferrals, prototypes inheriting 'Deferred'
	class DeferralTimer
	{
		GameTickT lastGameTick_ = 0;

		/// \brief vector of callbacks at game tick
		std::unordered_map<GameTickT,
		                   std::vector<
			                   std::pair<std::reference_wrapper<const data::Deferred>, data::UniqueDataBase*>
		                   >> callbacks_;

		/// \brief 0 indicates invalid callback
		using CallbackIndex = decltype(callbacks_.size());

		// ======================================================================

		///
		/// \brief Used to fill the gap when a callback has been removed
		class BlankCallback final : public data::Deferred
		{
		public:
			void OnDeferTimeElapsed(DeferralTimer&, data::UniqueDataBase*) const override {
			}
		};

		BlankCallback blankCallback_;

	public:
		/// \brief Information about the registered deferral for removing
		///
		/// .second value of 0 indicates invalid callback
		using DeferralEntry = std::pair<GameTickT, CallbackIndex>;

		///
		/// \brief Calls all deferred callbacks for the current game tick
		/// \param game_tick Current game tick
		void DeferralUpdate(GameTickT game_tick);

		///
		/// \brief Registers callback which will be called upon reaching the specified game tick
		/// \param deferred Implements virtual function on_defer_time_elapsed
		/// \param due_game_tick Game tick where the callback will be called
		/// \return Index of registered callback, use this to remove the callback later
		DeferralEntry RegisterAtTick(const data::Deferred& deferred, data::UniqueDataBase* unique_data,
		                             GameTickT due_game_tick);

		///
		/// \brief Registers callback which will be called after the specified game ticks pass
		/// \param deferred Implements virtual function on_defer_time_elapsed
		/// \param elapse_game_tick Callback will be called in game ticks from now
		/// \return Index of registered callback, use this to remove the callback later
		DeferralEntry RegisterFromTick(const data::Deferred& deferred, data::UniqueDataBase* unique_data,
		                               GameTickT elapse_game_tick);

		///
		/// \brief Removes registered callback at game_tick at index
		void RemoveDeferral(DeferralEntry entry);

		///
		/// \brief Removes registered callback and sets entry index to 0
		void RemoveDeferralEntry(DeferralEntry& entry);
	};
}

#endif // JACTORIO_GAME_WORLD_DEFERRAL_TIMER_H
