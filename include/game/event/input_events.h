#ifndef GAME_EVENT_INPUT_EVENTS_H
#define GAME_EVENT_INPUT_EVENTS_H

#include "game/event/event_base.h"

namespace jactorio::game
{
	// Keys
	
	/**
	 * Mouse clicks count as key events <br>
	 * For Python API - c++ registering to key presses, use input_manager instead as it is more specialized than this
	 */
	class Key_event : public Event_base
	{
	protected:
		Key_event(const int key, const int action, const int mods)
			: key(key), action(action), mods(mods) {
		};
		
	public:
		int key = -1;
		int action = -1;
		int mods = -1;
		
		EVENT_CATEGORY(input | keyboard)
	};

	class Key_press_event final : public Key_event
	{
	public:
		EVENT_TYPE(key_press)
		EVENT_CATEGORY(input | keyboard)
	};

	class Key_repeat_event final : public Key_event
	{
	public:
		EVENT_TYPE(key_repeat)
		EVENT_CATEGORY(input | keyboard)
	};
	
	class Key_release_event final : public Key_event
	{
	public:
		EVENT_TYPE(key_release)
		EVENT_CATEGORY(input | keyboard)
	};


	// Mouse
}

#endif // GAME_EVENT_INPUT_EVENTS_H
