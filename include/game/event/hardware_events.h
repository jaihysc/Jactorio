// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_EVENT_HARDWARE_EVENTS_H
#define JACTORIO_INCLUDE_GAME_EVENT_HARDWARE_EVENTS_H
#pragma once

#include <variant>

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

    ///
    /// Keyboard or mouse activity
    class InputActivityEvent final : public EventBase
    {
    public:
        InputActivityEvent(const SDL_KeyCode key, const InputAction key_action, const SDL_Keymod mods)
            : input(std::in_place_type<KeyboardActivityEvent>, key, key_action, mods) {}

        InputActivityEvent(const MouseInput key, const InputAction key_action, const SDL_Keymod mods)
            : input(std::in_place_type<MouseActivityEvent>, key, key_action, mods) {}

        std::variant<KeyboardActivityEvent, MouseActivityEvent> input;

        EVENT_TYPE(input_activity)
        EVENT_CATEGORY(application)
    };

} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_EVENT_HARDWARE_EVENTS_H
