// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_INPUT_INPUT_MANAGER_H
#define JACTORIO_INCLUDE_GAME_INPUT_INPUT_MANAGER_H
#pragma once

#include <SDL_keyboard.h>
#include <functional>
#include <unordered_map>

#include "core/hashers.h"

namespace jactorio::game
{
    enum class InputAction
    {
        none,

        // First pressed down
        key_down,

        // While pressed down, before repeat
        key_pressed,
        // Press down enough to repeat
        key_repeat,

        // pressed and repeat
        key_held,

        // Key lifted
        key_up
    };

    enum class MouseInput
    {
        left,
        middle,
        right,
        x1,
        x2
    };

    class KeyInput
    {
        using InputCallback = std::function<void()>;
        using CallbackId    = uint64_t;

        /// Positive = SDL_KeyCode
        /// Negative = MouseInput * -1
        using IntKeyMouseCodePair = int;

        using InputKeyData = std::tuple<IntKeyMouseCodePair, InputAction, SDL_Keymod>;


        // ======================================================================

        static std::unordered_map<IntKeyMouseCodePair, InputKeyData> activeInputs_;

    public:
        ///
        /// Sets the state of an input
        /// Callbacks for the respective inputs are called when CallCallbacks() is called
        static void SetInput(SDL_KeyCode keycode, InputAction action, SDL_Keymod mod = KMOD_NONE);
        static void SetInput(MouseInput mouse, InputAction action, SDL_Keymod mod = KMOD_NONE);


        // ======================================================================

    private:
        // Increments with each new assigned callback, one is probably not having 4 million registered callbacks
        // so this doesn't need to be decremented
        CallbackId callbackId_ = 1;


        // tuple format: key, action, mods
        // id of callbacks registered to the tuple
        std::unordered_map<InputKeyData, std::vector<CallbackId>, core::hash<InputKeyData>> callbackIds_{};

        std::unordered_map<CallbackId, InputCallback> inputCallbacks_{};


        void CallCallbacks(const InputKeyData& input);

    public:
        ///
        /// Registers a keyboard input callback which will be called when the specified input is activated
        /// \return id of the registered callback, 0 Indicates error
        CallbackId Register(const InputCallback& callback,
                            SDL_KeyCode key,
                            InputAction action,
                            SDL_Keymod mods = KMOD_NONE);

        ///
        /// Registers a mouse input callback which will be called when the specified input is activated
        /// \return id of the registered callback, 0 Indicates error
        CallbackId Register(const InputCallback& callback,
                            MouseInput button,
                            InputAction action,
                            SDL_Keymod mods = KMOD_NONE);


        ///
        /// Calls registered callbacks based on the input set with set_input(...)
        void Raise();


        ///
        /// Removes specified callback at callback_id
        void Unsubscribe(CallbackId callback_id, SDL_KeyCode key, InputAction action, SDL_Keymod mods = KMOD_NONE);

        ///
        /// Deletes all callback data
        void ClearData();

        ///
        static InputAction ToInputAction(int action, bool repeat);
    };
} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_INPUT_INPUT_MANAGER_H
