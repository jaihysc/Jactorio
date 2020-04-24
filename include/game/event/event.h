// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 01/20/2020

#ifndef JACTORIO_INCLUDE_GAME_EVENT_EVENT_H
#define JACTORIO_INCLUDE_GAME_EVENT_EVENT_H
#pragma once

#include <unordered_map>
#include <vector>

#include "game/event/game_events.h"

namespace jactorio::game
{
	///
	/// \brief Used for dispatching and listening to events
	class Event_data
	{
		using callback_func = void(*)();

		std::unordered_map<eventType, std::vector<callback_func>> event_handlers_{};
		// Handlers will only run once, and will need to be registered again with subscribe_once()
		std::unordered_map<eventType, std::vector<callback_func>> event_handlers_once_{};

	public:
		///
		/// \brief Subscribes a callback to an event
		template <typename T>
		void subscribe(const eventType event_type, T callback) {
			event_handlers_[event_type]
				.push_back(reinterpret_cast<callback_func>(+callback));
		}

		///
		/// \brief Subscribes a callback to an event which will only run once
		template <typename T>
		void subscribe_once(const eventType event_type, T callback) {
			event_handlers_once_[event_type]
				.push_back(reinterpret_cast<callback_func>(+callback));
		}

		///
		/// \brief Unsubscribes a callback to an event
		/// \return true if successfully removed, false if callback does not exist
		template <typename T>
		bool unsubscribe(const eventType event_type, T callback) {
			bool removed = false;

			auto& handlers = event_handlers_[event_type];  // Event handlers of event_type
			// Find callback in vector and remove
			for (unsigned int i = 0; i < handlers.size(); ++i) {
				if (handlers[i] == reinterpret_cast<callback_func>(callback)) {
					handlers.erase(handlers.begin() + i);

					removed = true;
					break;
				}
			}

			// Single time events
			auto& handlers_once = event_handlers_once_[event_type];
			// Find callback in vector and remove
			for (unsigned int i = 0; i < handlers_once.size(); ++i) {
				if (handlers_once[i] == reinterpret_cast<callback_func>(callback)) {
					handlers_once.erase(handlers_once.begin() + i);

					removed = true;
					break;
				}
			}

			return removed;
		}

		///
		/// \brief Raises event of event_type, forwards args to constructor of event class T,
		/// \brief Constructed event is provided by reference to all callbacks
		template <typename T, typename ... ArgsT>
		void raise(const eventType event_type, const ArgsT& ... args) {
			// Imgui sets the bool property input_captured
			// This takes priority over all events, and if true no events are allowed to be emitted
			// TODO ability to set if event runs when input is captured by imgui

			// TODO Send events backwards through layers
			for (auto& callback : event_handlers_[event_type]) {
				// Cast function pointer parameter to T
				auto fun_ptr = reinterpret_cast<void(*)(T&)>(callback);

				// Construct T, pass to function ptr
				T event = T(args...);
				fun_ptr(event);
			}

			// Single time events
			for (auto& callback : event_handlers_once_[event_type]) {
				// Cast function pointer parameter to T
				auto fun_ptr = reinterpret_cast<void(*)(T&)>(callback);

				// Construct T, pass to function ptr
				T event = T(args...);
				fun_ptr(event);
			}
			event_handlers_once_[event_type].clear();
		}


		///
		/// \brief Erases all data held in event_handlers_
		void clear_all_data() {
			// All callbacks registered to event
			for (auto& vector : event_handlers_) {
				vector.second.clear();
			}

			event_handlers_.clear();
		}
	};
}

#endif //JACTORIO_INCLUDE_GAME_EVENT_EVENT_H
