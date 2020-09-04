// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_EVENT_GAME_EVENTS_H
#define JACTORIO_INCLUDE_GAME_EVENT_GAME_EVENTS_H
#pragma once

#include <functional>
#include <vector>

#include "game/event/event_base.h"
#include "renderer/display_window.h"

namespace jactorio::game
{
    class LogicTickEvent final : public EventBase
    {
    public:
        explicit LogicTickEvent(const unsigned short tick) : gameTick(tick) {}

        ///
        /// 0 - 59
        unsigned short gameTick;

        EVENT_TYPE(logic_tick)
        EVENT_CATEGORY(in_game)
    };

    class RendererTickEvent final : public EventBase
    {
    public:
        using DisplayWindowContainerT = std::vector<std::reference_wrapper<renderer::DisplayWindow>>;

        explicit RendererTickEvent(const DisplayWindowContainerT& window) : windows(window) {}

        const DisplayWindowContainerT& windows;

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
} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_EVENT_GAME_EVENTS_H
