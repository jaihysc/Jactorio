#ifndef GAME_EVENT_EVENT_H
#define GAME_EVENT_EVENT_H

#include "game/event/event_type.h"

#include <unordered_map>
#include <vector>

// TODO, when adding loading of saves, clear the event data
/**
 * Handles the events of the game
 * !! Ensure that all handlers and raisers of events all provide the same parameters !!
 * !! This is NOT type safe !!
 */
namespace jactorio::game
{
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
		 * Raises an event of event_type, providing the specified args
		 */
		template <typename... ArgsT>
		static void raise(const event_type event_type, const ArgsT& ... args) {
			// All callbacks registered to event
			for (auto& callback : event_handlers_[event_type]) {

				// Cast function pointer to specified args count
				auto fun_ptr =
					reinterpret_cast<void(*)(ArgsT ...)>(callback);

				// Expand args for function ptr
				fun_ptr(args...);
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
