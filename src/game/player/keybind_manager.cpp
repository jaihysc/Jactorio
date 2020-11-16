// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/player/keybind_manager.h"

#include "jactorio.h"

using namespace jactorio;

void game::KeybindManager::ChangeActionInput(PlayerAction::Type action_type,
                                             const SDL_KeyCode key,
                                             const InputAction key_action,
                                             const SDL_Keymod mods) {
    const auto i_action = static_cast<int>(action_type);
    assert(i_action < static_cast<int>(PlayerAction::kActionCount_));

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

void game::KeybindManager::ChangeActionInput(PlayerAction::Type action_type,
                                             MouseInput key,
                                             InputAction key_action,
                                             SDL_Keymod mods) {}
