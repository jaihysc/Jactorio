// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_GAME_LOGIC_LOGIC_DATA_H
#define JACTORIO_GAME_LOGIC_LOGIC_DATA_H
#pragma once

#include <atomic>
#include <unordered_map>
#include <vector>

#include "core/data_type.h"
#include "data/prototype/interface/deferred.h"

namespace jactorio::game
{
	class LogicData
	{
		// ======================================================================
		// World properties
		std::atomic<GameTickT> gameTick_ = 0;

	public:
		/// \brief Called by the logic loop every update
		void GameTickAdvance() { ++gameTick_; }

		///
		/// \brief Number of logic updates since the world was created
		J_NODISCARD GameTickT GameTick() const { return gameTick_; }

		// ======================================================================

		///
		/// \brief Manages deferrals, prototypes inheriting 'Deferred'
		class DeferralTimer
		{
		public:
			explicit DeferralTimer(LogicData& logic_data)
				: logicData_(logic_data) {
			}

		private:
			/// \brief vector of callbacks at game tick
			std::unordered_map<GameTickT,
			                   std::vector<
				                   std::pair<std::reference_wrapper<const data::IDeferred>, data::UniqueDataBase*>
			                   >> callbacks_;

			/// \brief 0 indicates invalid callback
			using CallbackIndex = decltype(callbacks_.size());

		public:
			/// \brief Information about the registered deferral for removing
			///
			/// .second value of 0 indicates invalid callback
			using DeferralEntry = std::pair<GameTickT, CallbackIndex>;

			///
			/// \brief Calls all deferred callbacks for the current game tick
			/// \param game_tick Current game tick
			void DeferralUpdate(WorldData& world_data, GameTickT game_tick);

			///
			/// \brief Registers callback which will be called upon reaching the specified game tick
			/// \param deferred Implements virtual function on_defer_time_elapsed
			/// \param due_game_tick Game tick where the callback will be called
			/// \return Index of registered callback, use this to remove the callback later
			DeferralEntry RegisterAtTick(const data::IDeferred& deferred, data::UniqueDataBase* unique_data,
			                             GameTickT due_game_tick);

			///
			/// \brief Registers callback which will be called after the specified game ticks pass
			/// \param deferred Implements virtual function on_defer_time_elapsed
			/// \param elapse_game_tick Callback will be called in game ticks from now
			/// \return Index of registered callback, use this to remove the callback later
			DeferralEntry RegisterFromTick(const data::IDeferred& deferred, data::UniqueDataBase* unique_data,
			                               GameTickT elapse_game_tick);

			///
			/// \brief Removes registered callback at game_tick at index
			void RemoveDeferral(DeferralEntry entry);

			///
			/// \brief Removes registered callback and sets entry index to 0
			void RemoveDeferralEntry(DeferralEntry& entry);

		private:
			GameTickT lastGameTick_ = 0;
			LogicData& logicData_;

			///
			/// \brief Used to fill the gap when a callback has been removed
			class BlankCallback final : public data::IDeferred
			{
			public:
				void OnDeferTimeElapsed(WorldData&, LogicData&, data::UniqueDataBase*) const override {
				}
			} blankCallback_;
		} deferralTimer{*this};
	};
}

#endif // JACTORIO_GAME_LOGIC_LOGIC_DATA_H
