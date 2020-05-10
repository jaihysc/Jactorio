// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 04/02/2020

#ifndef JACTORIO_DATA_PROTOTYPE_ENTITY_DEFERRED_ENTITY_H
#define JACTORIO_DATA_PROTOTYPE_ENTITY_DEFERRED_ENTITY_H
#pragma once

namespace jactorio
{
	namespace data
	{
		struct UniqueDataBase;
	}

	namespace game
	{
		class DeferralTimer;
	}
}


namespace jactorio::data
{
	/// \brief Has actions which completes at a later game tick
	class Deferred
	{
	public:
		Deferred()          = default;
		virtual ~Deferred() = default;

		Deferred(const Deferred& other)                = default;
		Deferred(Deferred&& other) noexcept            = default;
		Deferred& operator=(const Deferred& other)     = default;
		Deferred& operator=(Deferred&& other) noexcept = default;

		///
		/// \brief The callback requested at the specified game tick was reached
		/// \param timer Deferred timer which dispatched this callback
		/// \param unique_data Unique data the callback was registered with
		virtual void OnDeferTimeElapsed(game::DeferralTimer& timer, UniqueDataBase* unique_data) const = 0;
	};
}

#endif // JACTORIO_DATA_PROTOTYPE_ENTITY_DEFERRED_ENTITY_H
