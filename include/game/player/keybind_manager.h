// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_PLAYER_KEYBIND_MANAGER_H
#define JACTORIO_INCLUDE_GAME_PLAYER_KEYBIND_MANAGER_H
#pragma once

#include <array>
#include <functional>
#include <type_traits>

#include "jactorio.h"

#include "core/resource_guard.h"
#include "data/cereal/serialize.h"
#include "game/input/input_manager.h"
#include "game/player/player_action.h"

#include <cereal/types/array.hpp>

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
            : input_(input), dataGlobal_(data_global) {}


        KeybindManager(const KeybindManager& other)     = delete;
        KeybindManager(KeybindManager&& other) noexcept = default;

        KeybindManager& operator=(KeybindManager other) {
            swap(*this, other);
            return *this;
        }


        friend void swap(KeybindManager& lhs, KeybindManager& rhs) noexcept {
            using std::swap;
            swap(lhs.input_, rhs.input_);
            swap(lhs.dataGlobal_, rhs.dataGlobal_);
            swap(lhs.actionCallbackId_, rhs.actionCallbackId_);
            swap(lhs.actionKeyData_, rhs.actionKeyData_);
        }


        ///
        /// Modifies the keyboard input which correlates to the provided action
        void ChangeActionInput(PlayerAction::Type player_action,
                               SDL_KeyCode key,
                               InputAction key_action,
                               SDL_Keymod mods = KMOD_NONE);

        ///
        /// Modifies the mouse input which correlates to the provided action
        void ChangeActionInput(PlayerAction::Type player_action,
                               MouseInput key,
                               InputAction key_action,
                               SDL_Keymod mods = KMOD_NONE);


        ///
        /// Modifies keyboard button for provided player action
        void ChangeActionKey(PlayerAction::Type player_action, SDL_KeyCode key);

        ///
        /// Modifies mouse button for provided player action
        void ChangeActionKey(PlayerAction::Type player_action, MouseInput key);

        ///
        /// Modifies the key's action for provided player action
        void ChangeActionKeyAction(PlayerAction::Type player_action, InputAction key_action);

        ///
        /// Modifies the modifiers for provided player action
        void ChangeActionMods(PlayerAction::Type player_action, SDL_Keymod mods);


        ///
        /// Uses pre-determined default keybinds for actions
        void LoadDefaultKeybinds();

        ///
        /// Calls ChangeActionInput for all player actions with stored key bindings
        /// \remark Only has effect after this object was deserialized to register all key bindings
        void RegisterAllKeyData();


        ///
        /// Returns key information for each keybind
        /// Index each PlayerAction::Type by its value when cast to int
        J_NODISCARD auto& GetKeybindInfo() const {
            return actionKeyData_;
        }


        CEREAL_SERIALIZE(archive) {
            for (auto& [key_code, key_action, mods] : actionKeyData_) {
                archive.startNode();
                CapturingGuard<void()> guard([&archive]() { archive.finishNode(); });

                archive(cereal::make_nvp("key_code", key_code),
                        cereal::make_nvp("key_action", key_action),
                        cereal::make_nvp("key_mods", mods));
            }
        }

    private:
        std::reference_wrapper<InputManager> input_;
        std::reference_wrapper<GameDataGlobal> dataGlobal_;

        /// Id of each action's executor in InputManager
        /// Index by numerical value of PlayerAction::Type
        std::array<InputManager::CallbackId, PlayerAction::kActionCount_> actionCallbackId_{};

        /// Keybind info of each action
        /// Index by numerical value of PlayerAction::Type
        std::array<InputManager::InputKeyData, PlayerAction::kActionCount_> actionKeyData_;


        ///
        /// Performs actual change of input for action
        /// Do not directly call, call ChangeActionInput
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
            input_.get().Unsubscribe(callback_id);
        }

        callback_id = input_.get().Register(
            [this, action_type]() {
                PlayerAction::GetExecutor(action_type)(dataGlobal_.get()); //
            },
            key,
            key_action,
            mods);
    }

    static_assert(std::is_move_constructible_v<KeybindManager>);
    static_assert(std::is_move_assignable_v<KeybindManager>);
} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_PLAYER_KEYBIND_MANAGER_H
