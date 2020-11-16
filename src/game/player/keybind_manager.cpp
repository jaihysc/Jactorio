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

void game::KeybindManager::LoadDefaultKeybinds() {
    LOG_MESSAGE(info, "Loading default keybinds");

    ChangeActionInput(PlayerAction::Type::player_move_up, SDLK_w, InputAction::key_held);
    ChangeActionInput(PlayerAction::Type::player_move_right, SDLK_d, InputAction::key_held);
    ChangeActionInput(PlayerAction::Type::player_move_down, SDLK_s, InputAction::key_held);
    ChangeActionInput(PlayerAction::Type::player_move_left, SDLK_a, InputAction::key_held);

    ChangeActionInput(PlayerAction::Type::deselect_held_item, SDLK_q, InputAction::key_down);

    ChangeActionInput(PlayerAction::Type::place_entity, MouseInput::left, InputAction::key_held);
    ChangeActionInput(PlayerAction::Type::activate_layer, MouseInput::left, InputAction::key_down);
    ChangeActionInput(PlayerAction::Type::pickup_or_mine_entity, MouseInput::right, InputAction::key_held);

    ChangeActionInput(PlayerAction::Type::rotate_entity_clockwise, SDLK_r, InputAction::key_up);
    ChangeActionInput(PlayerAction::Type::rotate_entity_counter_clockwise, SDLK_r, InputAction::key_up, KMOD_LSHIFT);

    ChangeActionInput(PlayerAction::Type::toggle_main_menu, SDLK_ESCAPE, InputAction::key_up);
    ChangeActionInput(PlayerAction::Type::toggle_debug_menu, SDLK_BACKQUOTE, InputAction::key_up);
    ChangeActionInput(PlayerAction::Type::toggle_character_menu, SDLK_TAB, InputAction::key_up);
}
