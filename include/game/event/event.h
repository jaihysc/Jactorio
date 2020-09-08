// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_EVENT_EVENT_H
#define JACTORIO_INCLUDE_GAME_EVENT_EVENT_H
#pragma once

#include <unordered_map>
#include <vector>

#include "game/event/game_events.h"

namespace jactorio::game
{
    ///
    /// Used for dispatching and listening to events
    class EventData
    {
        using CallbackFunc = void (*)(EventBase&);

        std::unordered_map<EventType, std::vector<CallbackFunc>> eventHandlers_{};
        // Handlers will only run once, and will need to be registered again with subscribeOnce()
        std::unordered_map<EventType, std::vector<CallbackFunc>> eventHandlersOnce_{};

    public:
        ///
        /// Subscribes a callback to an event
        /// \param callback Should accept single parameter EventBase by reference
        template <typename T>
        void Subscribe(const EventType event_type, T callback) {
            eventHandlers_[event_type].push_back(reinterpret_cast<CallbackFunc>(+callback));
        }

        ///
        /// Subscribes a callback to an event which will only run once
        /// \param callback Should accept single parameter EventBase by reference
        template <typename T>
        void SubscribeOnce(const EventType event_type, T callback) {
            eventHandlersOnce_[event_type].push_back(reinterpret_cast<CallbackFunc>(+callback));
        }

        ///
        /// Unsubscribes a callback to an event
        /// \return true if successfully removed, false if callback does not exist
        template <typename T>
        bool Unsubscribe(const EventType event_type, T callback) {
            bool removed = false;

            auto& handlers = eventHandlers_[event_type]; // Event handlers of event_type
            // Find callback in vector and remove
            for (unsigned int i = 0; i < handlers.size(); ++i) {
                if (handlers[i] == reinterpret_cast<CallbackFunc>(+callback)) {
                    handlers.erase(handlers.begin() + i);

                    removed = true;
                    break;
                }
            }

            // Single time events
            auto& handlers_once = eventHandlersOnce_[event_type];
            // Find callback in vector and remove
            for (unsigned int i = 0; i < handlers_once.size(); ++i) {
                if (handlers_once[i] == reinterpret_cast<CallbackFunc>(+callback)) {
                    handlers_once.erase(handlers_once.begin() + i);

                    removed = true;
                    break;
                }
            }

            return removed;
        }

        ///
        /// Raises event of EventType, forwards args to constructor of TEvent inheriting EventBase,
        /// Constructed event is provided by reference to all callbacks
        template <typename TEvent, typename... Args>
        void Raise(const EventType event_type, Args&&... args) {

            for (auto& callback : eventHandlers_[event_type]) {
                // Cast function pointer parameter to T
                auto fun_ptr = reinterpret_cast<void (*)(TEvent&)>(callback);

                // Construct EventBase, pass to function ptr
                TEvent event = TEvent(std::forward<Args>(args)...);
                fun_ptr(event);
            }

            // Single time events
            for (auto& callback : eventHandlersOnce_[event_type]) {
                auto fun_ptr = reinterpret_cast<void (*)(TEvent&)>(callback);

                TEvent event = TEvent(std::forward<Args>(args)...);
                fun_ptr(event);
            }
            eventHandlersOnce_[event_type].clear();
        }


        ///
        /// Erases all data held
        void ClearAllData() {
            // All callbacks registered to event
            for (auto& vector : eventHandlers_) {
                vector.second.clear();
            }

            eventHandlers_.clear();
        }
    };
} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_EVENT_EVENT_H
