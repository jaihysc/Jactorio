// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_EVENT_EVENT_H
#define JACTORIO_INCLUDE_GAME_EVENT_EVENT_H
#pragma once

#include <functional>
#include <unordered_map>
#include <vector>

#include "game/event/event_base.h"

namespace jactorio::game
{
    ///
    /// Used for dispatching and listening to events
    class EventData
    {
        using CallbackFunc = std::function<void(EventBase& e)>;

        std::unordered_map<EventType, std::vector<CallbackFunc>> eventHandlers_{};
        // Handlers will only run once, and will need to be registered again with subscribeOnce()
        std::unordered_map<EventType, std::vector<CallbackFunc>> eventHandlersOnce_{};

    public:
        ///
        /// Subscribes a callback to an event
        /// \param callback Should accept single parameter EventBase by reference
        void Subscribe(EventType event_type, CallbackFunc callback);

        ///
        /// Subscribes a callback to an event which will only run once
        /// \param callback Should accept single parameter EventBase by reference
        void SubscribeOnce(EventType event_type, CallbackFunc callback);

        ///
        /// Unsubscribes a callback to an event
        /// \return true if successfully removed, false if callback does not exist
        bool Unsubscribe(EventType event_type, CallbackFunc callback);


        ///
        /// Raises event of EventType, forwards args to constructor of TEvent inheriting EventBase,
        /// Constructed event is provided by reference to all callbacks
        template <typename TEvent, typename... Args>
        void Raise(EventType event_type, Args&&... args);


        ///
        /// Erases all data held
        void ClearAllData();
    };

    template <typename TEvent, typename... Args>
    void EventData::Raise(const EventType event_type, Args&&... args) {

        for (auto& callback : eventHandlers_[event_type]) {
            // Construct EventBase, pass to callback
            TEvent event = TEvent(std::forward<Args>(args)...);
            callback(event);
        }

        // Single time events
        for (auto& callback : eventHandlersOnce_[event_type]) {
            TEvent event = TEvent(std::forward<Args>(args)...);
            callback(event);
        }
        eventHandlersOnce_[event_type].clear();
    }
} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_EVENT_EVENT_H
