#ifndef GAME_EVENT_GAME_EVENTS_H
#define GAME_EVENT_GAME_EVENTS_H

#include "jactorio.h"
#include "event_type.h"

namespace jactorio::game
{
	class Event_base
	{
	protected:
		Event_base() = default;
		virtual ~Event_base() = default;
		
	public:
		Event_base(const Event_base& other) = default;
		Event_base(Event_base&& other) noexcept = default;
		Event_base& operator=(const Event_base& other) = default;
		Event_base& operator=(Event_base&& other) noexcept = default;

		
		bool handled = false;  // Set this to true to prevent this event from being carried further

		J_NO_DISCARD virtual event_type get_event_type() const = 0;
		J_NO_DISCARD virtual int get_category_flags() const = 0;

		J_NO_DISCARD bool in_category(const event_category category) const {
			return get_category_flags() & static_cast<int>(category);
		}
	};


	// Inheritors
	
	// Fast way to implement virtual functions
#define EVENT_TYPE(type) J_NO_DISCARD event_type get_event_type() const override { return event_type::type; }
#define EVENT_CATEGORY(category) J_NO_DISCARD int get_category_flags() const override { return category; }
	
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

	
	class Gui_opened : public Event_base
	{
	public:
		EVENT_TYPE(game_gui_open)
		EVENT_CATEGORY(in_game)
	};
	
#undef EVENT_TYPE
#undef EVENT_CATEGORY
}

#endif // GAME_EVENT_GAME_EVENTS_H
