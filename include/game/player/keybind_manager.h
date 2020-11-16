// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_PLAYER_KEYBIND_MANAGER_H
#define JACTORIO_INCLUDE_GAME_PLAYER_KEYBIND_MANAGER_H
#pragma once

#include <array>

#include "game/input/input_manager.h"
#include "game/player/player_action.h"

namespace jactorio::game
{
    class InputManager;
    struct GameDataGlobal;

    class KeybindManager
    {
    public:
        explicit KeybindManager(InputManager& input, GameDataGlobal& data_global)
            : inputManager_(input), dataGlobal_(data_global) {}

        ///
        /// Modifies the keyboard input which correlates to the provided action
        void ChangeActionInput(PlayerAction::Type action_type,
                               SDL_KeyCode key,
                               InputAction key_action,
                               SDL_Keymod mods = KMOD_NONE);

        ///
        /// Modifies the mouse input which correlates to the provided action
        void ChangeActionInput(PlayerAction::Type action_type,
                               MouseInput key,
                               InputAction key_action,
                               SDL_Keymod mods = KMOD_NONE);

    private:
        InputManager& inputManager_;
        GameDataGlobal& dataGlobal_;

        /// Id of each action's executor in InputManager
        std::array<InputManager::CallbackId, PlayerAction::kActionCount_> actionCallbackId_{};
    };
} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_PLAYER_KEYBIND_MANAGER_H
