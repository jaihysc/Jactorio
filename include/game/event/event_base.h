// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 01/20/2020

#ifndef JACTORIO_INCLUDE_GAME_EVENT_EVENT_BASE_H
#define JACTORIO_INCLUDE_GAME_EVENT_EVENT_BASE_H
#pragma once

#include "event_type.h"
#include "jactorio.h"

// Fast way to implement pure virtual functions
#define EVENT_TYPE(type) J_NODISCARD EventType GetEventType() const override { return EventType::type; }
#define EVENT_CATEGORY(category) J_NODISCARD int GetCategoryFlags() const override { return category; }

namespace jactorio::game
{
	class EventBase
	{
	protected:
		EventBase()          = default;
		virtual ~EventBase() = default;

	public:
		EventBase(const EventBase& other)                = default;
		EventBase(EventBase&& other) noexcept            = default;
		EventBase& operator=(const EventBase& other)     = default;
		EventBase& operator=(EventBase&& other) noexcept = default;


		bool handled = false;  // Set this to true to prevent this event from being carried further

		J_NODISCARD virtual EventType GetEventType() const = 0;
		J_NODISCARD virtual int GetCategoryFlags() const = 0;

		J_NODISCARD bool InCategory(const EventCategory category) const {
			return GetCategoryFlags() & static_cast<int>(category);
		}
	};
}

#endif //JACTORIO_INCLUDE_GAME_EVENT_EVENT_BASE_H
