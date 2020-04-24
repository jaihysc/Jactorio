// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 01/20/2020

#ifndef JACTORIO_INCLUDE_GAME_EVENT_GAME_EVENTS_H
#define JACTORIO_INCLUDE_GAME_EVENT_GAME_EVENTS_H
#pragma once

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

	class Renderer_tick_event final : public Event_base
	{
	public:
		Renderer_tick_event() = default;

		EVENT_TYPE(renderer_tick)
		EVENT_CATEGORY(application)
	};

	// Gui
	class Gui_opened_event : public Event_base
	{
	public:
		EVENT_TYPE(game_gui_open)
		EVENT_CATEGORY(in_game)
	};
}

#endif //JACTORIO_INCLUDE_GAME_EVENT_GAME_EVENTS_H
