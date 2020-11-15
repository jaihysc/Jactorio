// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/player/keybind_manager.h"

using namespace jactorio;

game::KeybindManager::KeybindManager(KeyInput& input) {}

void game::KeybindManager::ChangeActionInput(PlayerAction action,
                                             SDL_KeyCode key,
                                             InputAction key_action,
                                             SDL_Keymod mods) {}

void game::KeybindManager::ChangeActionInput(PlayerAction action,
                                             MouseInput key,
                                             InputAction key_action,
                                             SDL_Keymod mods) {}
