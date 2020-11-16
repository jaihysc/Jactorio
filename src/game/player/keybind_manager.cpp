// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/player/keybind_manager.h"

using namespace jactorio;

void game::KeybindManager::ChangeActionInput(const PlayerAction::Type action_type,
                                             const SDL_KeyCode key,
                                             const InputAction key_action,
                                             const SDL_Keymod mods) {
    DoChangeActionInput(action_type, key, key_action, mods);
}

void game::KeybindManager::ChangeActionInput(const PlayerAction::Type action_type,
                                             const MouseInput key,
                                             const InputAction key_action,
                                             const SDL_Keymod mods) {
    DoChangeActionInput(action_type, key, key_action, mods);
}
