// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_PLAYER_KEYBIND_MANAGER_H
#define JACTORIO_INCLUDE_GAME_PLAYER_KEYBIND_MANAGER_H
#pragma once

#include <array>

#include "jactorio.h"

#include "game/input/input_manager.h"
#include "game/player/player_action.h"

namespace jactorio::game
{
    class InputManager;
    struct GameDataGlobal;

    class KeybindManager
    {
    public:
        ///
        /// \remark Provided parameter's lifetime must exceed this object
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


        ///
        /// Performs actual change of input for action
        /// \tparam TKey SDL_KeyCode or MouseInput
        template <typename TKey>
        void DoChangeActionInput(PlayerAction::Type action_type, TKey key, InputAction key_action, SDL_Keymod mods);
    };

    template <typename TKey>
    void KeybindManager::DoChangeActionInput(const PlayerAction::Type action_type,
                                             const TKey key,
                                             const InputAction key_action,
                                             const SDL_Keymod mods) {

        const auto i_action = static_cast<int>(action_type);
        assert(i_action < static_cast<int>(game::PlayerAction::kActionCount_));

        auto& callback_id = actionCallbackId_[i_action];
        if (callback_id != 0) {
            inputManager_.Unsubscribe(callback_id);
        }

        callback_id = inputManager_.Register(
            [this, action_type]() {
                PlayerAction::GetExecutor(action_type)(dataGlobal_); //
            },
            key,
            key_action,
            mods);
    }
} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_PLAYER_KEYBIND_MANAGER_H
