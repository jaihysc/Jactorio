// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/player/keybind_manager.h"

using namespace jactorio;

void game::KeybindManager::ChangeActionInput(const PlayerAction::Type action_type,
                                             const SDL_KeyCode key,
                                             const InputAction key_action,
                                             const SDL_Keymod mods) {
    assert(key >= 0); // Accidental call with MouseInput?

    DoChangeActionInput(action_type, key, key_action, mods);

    // Store new key info
    auto& [stored_key, stored_key_action, stored_mod] = actionKeyData_[static_cast<int>(action_type)];

    stored_key        = key;
    stored_key_action = key_action;
    stored_mod        = mods;
}

void game::KeybindManager::ChangeActionInput(const PlayerAction::Type action_type,
                                             const MouseInput key,
                                             const InputAction key_action,
                                             const SDL_Keymod mods) {
    assert(static_cast<int>(key) > 0); // MouseInput should start at 1 to avoid conflict with SDL_KeyCode

    DoChangeActionInput(action_type, key, key_action, mods);

    auto& [stored_key, stored_key_action, stored_mod] = actionKeyData_[static_cast<int>(action_type)];

    stored_key        = static_cast<int>(key) * -1;
    stored_key_action = key_action;
    stored_mod        = mods;
}


void game::KeybindManager::ChangeActionKey(PlayerAction::Type player_action, const SDL_KeyCode key) {
    auto& [_, key_action, mod] = actionKeyData_[static_cast<int>(player_action)];

    ChangeActionInput(player_action, key, key_action, mod);
}

void game::KeybindManager::ChangeActionKey(PlayerAction::Type player_action, const MouseInput key) {
    auto& [_, key_action, mod] = actionKeyData_[static_cast<int>(player_action)];

    ChangeActionInput(player_action, key, key_action, mod);
}

void game::KeybindManager::ChangeActionKeyAction(PlayerAction::Type player_action, const InputAction key_action) {
    auto& [key, _, mod] = actionKeyData_[static_cast<int>(player_action)];

    if (key >= 0) {
        ChangeActionInput(player_action, static_cast<SDL_KeyCode>(key), key_action, mod);
    }
    else {
        ChangeActionInput(player_action, static_cast<MouseInput>(key * -1), key_action, mod);
    }
}

void game::KeybindManager::ChangeActionMod(PlayerAction::Type player_action, const SDL_Keymod mod) {
    auto& [key, key_action, _] = actionKeyData_[static_cast<int>(player_action)];

    if (key >= 0) {
        ChangeActionInput(player_action, static_cast<SDL_KeyCode>(key), key_action, mod);
    }
    else {
        ChangeActionInput(player_action, static_cast<MouseInput>(key * -1), key_action, mod);
    }
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
