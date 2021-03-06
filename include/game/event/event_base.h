// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_EVENT_EVENT_BASE_H
#define JACTORIO_INCLUDE_GAME_EVENT_EVENT_BASE_H
#pragma once

#include "jactorio.h"

#include "event_type.h"

// Fast way to implement pure virtual functions
#define EVENT_TYPE(type)                                  \
    J_NODISCARD EventType GetEventType() const override { \
        return EventType::type;                           \
    }
#define EVENT_CATEGORY(category)                                          \
    J_NODISCARD int GetCategoryFlags() const override {                   \
        return static_cast<int>(jactorio::game::EventCategory::category); \
    }

namespace jactorio::game
{
    class EventBase
    {
    protected:
        EventBase()          = default;
        virtual ~EventBase() = default;

    public:
        EventBase(const EventBase& other)     = default;
        EventBase(EventBase&& other) noexcept = default;
        EventBase& operator=(const EventBase& other) = default;
        EventBase& operator=(EventBase&& other) noexcept = default;


        bool handled = false; // Set this to true to prevent this event from being carried further

        J_NODISCARD virtual EventType GetEventType() const = 0;
        J_NODISCARD virtual int GetCategoryFlags() const   = 0;

        J_NODISCARD bool InCategory(const EventCategory category) const {
            return GetCategoryFlags() & static_cast<int>(category);
        }
    };
} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_EVENT_EVENT_BASE_H
