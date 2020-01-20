#ifndef GAME_EVENT_EVENT_H
#define GAME_EVENT_EVENT_H

#include <unordered_map>
#include <vector>

#include "game/event/game_events.h"
// #include "game/world/chunk_tile.h"  // Use chunk layers from this header

// TODO, when adding loading of saves, clear the event data
namespace jactorio::game
{
	/**
	 * Used for dispatching and listening to events
	 */
	class Event
	{
		using void_ptr = void(*)();
	public:
		Event() = delete;
		~Event() = delete;

		Event(const Event& other) = delete;
		Event(Event&& other) noexcept = delete;
		Event& operator=(const Event& other) = delete;
		Event& operator=(Event&& other) noexcept = delete;
	private:
		static std::unordered_map<event_type, std::vector<void_ptr>> event_handlers_;
		// Handlers will only run once, and will need to be registered again with subscribe_once()
		static std::unordered_map<event_type, std::vector<void_ptr>> event_handlers_once_;

	public:
		/**
		 * Subscribes a callback to an event
		 */
		template <typename T>
		static void subscribe(const event_type event_type, T callback) {
			event_handlers_[event_type]
				.push_back((void_ptr)(callback));
		}

		/**
		 * Subscribes a callback to an event which will only run once
		 */
		template <typename T>
		static void subscribe_once(const event_type event_type, T callback) {
			event_handlers_once_[event_type]
				.push_back((void_ptr)(callback));
		}

		/**
		 * Unsubscribes a callback to an event
		 * @return true if successfully removed, false if callback does not exist
		 */
		template <typename T>
		static bool unsubscribe(const event_type event_type, T callback) {
			bool removed = false;

			auto& handlers = event_handlers_[event_type];  // Event handlers of event_type
			// Find callback in vector and remove
			for (unsigned int i = 0; i < handlers.size(); ++i) {
				if (handlers[i] == (void_ptr)(callback)) {
					handlers.erase(handlers.begin() + i);
					
					removed = true;
					break;
				}
			}

			// Single time events
			auto& handlers_once = event_handlers_once_[event_type];
			// Find callback in vector and remove
			for (unsigned int i = 0; i < handlers_once.size(); ++i) {
				if (handlers_once[i] == (void_ptr)(callback)) {
					handlers_once.erase(handlers_once.begin() + i);
					
					removed = true;
					break;
				}
			}
			
			return removed;
		}

		/**
		 * Raises event of event_type, forwards args to constructor of event class T,
		 * Constructed event is provided by reference to all callbacks
		 */
		template <typename T, typename ... ArgsT>
		static void raise(const event_type event_type, const ArgsT& ... args) {
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
		

		/**
		 * Erases all data held in event_handlers_
		 */
		static void clear_all_data() {
			// All callbacks registered to event
			for (auto& vector : event_handlers_) {
				vector.second.clear();
			}

			event_handlers_.clear();
		}
	};
}

#endif // GAME_EVENT_EVENT_H
