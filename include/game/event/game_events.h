// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_EVENT_GAME_EVENTS_H
#define JACTORIO_INCLUDE_GAME_EVENT_GAME_EVENTS_H
#pragma once

#include "event_base.h"

namespace jactorio::game
{
	class LogicTickEvent final : public EventBase
	{
	public:
		explicit LogicTickEvent(const unsigned short tick)
			: gameTick(tick) {
		}

		///
		/// 0 - 59
		unsigned short gameTick;

		EVENT_TYPE(logic_tick)
		EVENT_CATEGORY(in_game)
	};

	class RendererTickEvent final : public EventBase
	{
	public:
		RendererTickEvent() = default;

		EVENT_TYPE(renderer_tick)
		EVENT_CATEGORY(application)
	};

	// Gui
	class GuiOpenedEvent : public EventBase
	{
	public:
		EVENT_TYPE(game_gui_open)
		EVENT_CATEGORY(in_game)
	};
}

#endif //JACTORIO_INCLUDE_GAME_EVENT_GAME_EVENTS_H
