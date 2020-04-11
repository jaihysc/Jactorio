// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 04/02/2020

#ifndef JACTORIO_DATA_PROTOTYPE_ENTITY_DEFERRED_ENTITY_H
#define JACTORIO_DATA_PROTOTYPE_ENTITY_DEFERRED_ENTITY_H
#pragma once

#include "data/prototype/prototype_base.h"

namespace jactorio::game
{
	class Deferral_timer;
}


namespace jactorio::data
{
	/// \brief Has actions which completes at a later game tick
	class Deferred
	{
	public:
		Deferred() = default;
		virtual ~Deferred() = default;

		Deferred(const Deferred& other) = default;
		Deferred(Deferred&& other) noexcept = default;
		Deferred& operator=(const Deferred& other) = default;
		Deferred& operator=(Deferred&& other) noexcept = default;

		///
		/// \brief The callback requested at the specified game tick was reached
		/// \param timer Deferred timer which dispatched this callback
		/// \param unique_data Unique data the callback was registered with
		virtual void on_defer_time_elapsed(game::Deferral_timer& timer, Unique_data_base* unique_data) const = 0;
	};
}

#endif // JACTORIO_DATA_PROTOTYPE_ENTITY_DEFERRED_ENTITY_H
