// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_EVENT_HARDWARE_EVENTS_H
#define JACTORIO_INCLUDE_GAME_EVENT_HARDWARE_EVENTS_H
#pragma once

#include "game/event/event_base.h"
#include "game/input/input_type.h"

namespace jactorio::game
{
    class KeyboardActivityEvent final : public EventBase
    {
    public:
        KeyboardActivityEvent(const SDL_KeyCode key, const InputAction key_action, const SDL_Keymod mods)
            : key(key), keyAction(key_action), mods(mods) {}

        SDL_KeyCode key;
        InputAction keyAction;
        SDL_Keymod mods;

        EVENT_TYPE(keyboard_activity)
        EVENT_CATEGORY(application)
    };

    class MouseActivityEvent final : public EventBase
    {
    public:
        MouseActivityEvent(const MouseInput key, const InputAction key_action, const SDL_Keymod mods)
            : key(key), keyAction(key_action), mods(mods) {}

        MouseInput key;
        InputAction keyAction;
        SDL_Keymod mods;

        EVENT_TYPE(mouse_activity)
        EVENT_CATEGORY(application)
    };

} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_EVENT_HARDWARE_EVENTS_H
