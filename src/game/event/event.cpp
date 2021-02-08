// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/event/event.h"

using namespace jactorio;

void game::EventData::Subscribe(const EventType event_type, CallbackFunc callback) {
    eventHandlers_[event_type].push_back(std::move(callback));
}

void game::EventData::SubscribeOnce(const EventType event_type, CallbackFunc callback) {
    eventHandlersOnce_[event_type].push_back(std::move(callback));
}

bool game::EventData::Unsubscribe(const EventType event_type, CallbackFunc callback) {
    bool removed = false;

    auto& handlers = eventHandlers_[event_type]; // Event handlers of event_type
    // Find callback in vector and remove
    for (unsigned int i = 0; i < handlers.size(); ++i) {
        if (handlers[i].target<EventBase*>() == callback.target<EventBase*>()) {
            handlers.erase(handlers.begin() + i);

            removed = true;
            break;
        }
    }

    // Single time events
    auto& handlers_once = eventHandlersOnce_[event_type];
    // Find callback in vector and remove
    for (unsigned int i = 0; i < handlers_once.size(); ++i) {
        if (handlers_once[i].target<EventBase*>() == callback.target<EventBase*>()) {
            handlers_once.erase(handlers_once.begin() + i);

            removed = true;
            break;
        }
    }

    return removed;
}

void game::EventData::Clear() {
    // All callbacks registered to event
    for (auto& [event_type, callbacks] : eventHandlers_) {
        callbacks.clear();
    }

    eventHandlers_.clear();
}
