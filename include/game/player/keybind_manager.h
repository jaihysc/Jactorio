// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_PLAYER_KEYBIND_MANAGER_H
#define JACTORIO_INCLUDE_GAME_PLAYER_KEYBIND_MANAGER_H
#pragma once

#include "game/input/input_type.h"
#include "game/player/player_action.h"

namespace jactorio::game
{
    class InputManager;
    struct GameDataGlobal;

    class KeybindManager
    {
    public:
        explicit KeybindManager(InputManager& input);

        ///
        /// Modifies the keyboard input which correlates to the provided action
        void ChangeActionInput(PlayerAction action,
                               SDL_KeyCode key,
                               InputAction key_action,
                               SDL_Keymod mods = KMOD_NONE);

        ///
        /// Modifies the mouse input which correlates to the provided action
        void ChangeActionInput(PlayerAction action,
                               MouseInput key,
                               InputAction key_action,
                               SDL_Keymod mods = KMOD_NONE);
    };
} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_PLAYER_KEYBIND_MANAGER_H
