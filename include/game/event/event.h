#ifndef GAME_EVENT_EVENT_H
#define GAME_EVENT_EVENT_H

#include <unordered_map>
#include <vector>

#include "jactorio.h"
#include "game/event/game_events.h"

// TODO, when adding loading of saves, clear the event data
namespace jactorio::game
{
	/**
	 * Used for dispatching and listening to events
	 */
	class Event
	{
	public:
		Event() = delete;
		~Event() = delete;

		Event(const Event& other) = delete;
		Event(Event&& other) noexcept = delete;
		Event& operator=(const Event& other) = delete;
		Event& operator=(Event&& other) noexcept = delete;
	private:
		static std::unordered_map<event_type, std::vector<void*>> event_handlers_;

	public:
		/**
		 * Subscribes a callback to an event
		 */
		template <typename T>
		static void subscribe(const event_type event_type, T callback) {
			event_handlers_[event_type]
				.push_back(reinterpret_cast<void*>(callback));
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
				if (handlers[i] == reinterpret_cast<void*>(callback)) {
					handlers.erase(handlers.begin() + i);
					
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
			// TODO raise not implemented
			for (auto& callback : event_handlers_[event_type]) {
				// Cast function pointer parameter to T
				auto fun_ptr = reinterpret_cast<void(*)(T&)>(callback);

				// Construct T, pass to function ptr
				fun_ptr(T(args...));
			}
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
