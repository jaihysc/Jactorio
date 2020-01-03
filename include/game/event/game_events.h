#ifndef GAME_EVENT_GAME_EVENTS_H
#define GAME_EVENT_GAME_EVENTS_H

#include "event_base.h"

namespace jactorio::game
{
	class Logic_tick_event final : public Event_base
	{
	public:
		explicit Logic_tick_event(const unsigned short tick)
			: game_tick(tick) {
		}
		
		unsigned short game_tick;
		
		EVENT_TYPE(logic_tick)
		EVENT_CATEGORY(in_game)
	};


	// Gui
	class Gui_opened_event : public Event_base
	{
	public:
		EVENT_TYPE(game_gui_open)
		EVENT_CATEGORY(in_game)
	};

}

#endif // GAME_EVENT_GAME_EVENTS_H
