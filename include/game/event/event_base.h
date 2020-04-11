// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 01/20/2020

#ifndef JACTORIO_INCLUDE_GAME_EVENT_EVENT_BASE_H
#define JACTORIO_INCLUDE_GAME_EVENT_EVENT_BASE_H
#pragma once

#include "event_type.h"
#include "jactorio.h"

// Fast way to implement pure virtual functions
#define EVENT_TYPE(type) J_NODISCARD event_type get_event_type() const override { return event_type::type; }
#define EVENT_CATEGORY(category) J_NODISCARD int get_category_flags() const override { return category; }

namespace jactorio::game
{
	class Event_base
	{
	protected:
		Event_base() = default;
		virtual ~Event_base() = default;

	public:
		Event_base(const Event_base& other) = default;
		Event_base(Event_base&& other) noexcept = default;
		Event_base& operator=(const Event_base& other) = default;
		Event_base& operator=(Event_base&& other) noexcept = default;


		bool handled = false;  // Set this to true to prevent this event from being carried further

		J_NODISCARD virtual event_type get_event_type() const = 0;
		J_NODISCARD virtual int get_category_flags() const = 0;

		J_NODISCARD bool in_category(const event_category category) const {
			return get_category_flags() & static_cast<int>(category);
		}
	};
}

#endif //JACTORIO_INCLUDE_GAME_EVENT_EVENT_BASE_H
