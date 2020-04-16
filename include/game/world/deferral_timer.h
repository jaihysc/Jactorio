// 
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
	class Deferral_timer
	{
		game_tick_t last_game_tick_ = 0;

		/// \brief vector of callbacks at game tick
		std::unordered_map<game_tick_t,
		                   std::vector<
			                   std::pair<std::reference_wrapper<const data::Deferred>, data::Unique_data_base*>
		                   >> callbacks_;

		/// \brief 0 indicates invalid callback
		using callback_index = decltype(callbacks_.size());

	public:
		/// \brief Information about the registered deferral for removing
		using deferral_entry = std::pair<game_tick_t, callback_index>;

		///
		/// \brief Calls all deferred callbacks for the current game tick
		/// \param game_tick Current game tick
		void deferral_update(game_tick_t game_tick);

		///
		/// \brief Registers callback which will be called upon the specified game tick
		/// \param deferred Implements virtual function on_defer_time_elapsed
		/// \param due_game_tick Game tick where the callback will be called
		/// \return Index of registered callback, use this to remove the callback later
		deferral_entry register_at_tick(const data::Deferred& deferred, data::Unique_data_base* unique_data,
		                                game_tick_t due_game_tick);

		///
		/// \brief Registers callback which will be called upon the specified game tick
		/// \param deferred Implements virtual function on_defer_time_elapsed
		/// \param elapse_game_tick Callback will be called in game ticks from now
		/// \return Index of registered callback, use this to remove the callback later
		deferral_entry register_from_tick(const data::Deferred& deferred, data::Unique_data_base* unique_data,
		                                  game_tick_t elapse_game_tick);

		///
		/// \brief Removes registered callback at game_tick at index
		void remove_deferral(deferral_entry entry);
	};
}

#endif // JACTORIO_GAME_WORLD_DEFERRAL_TIMER_H
