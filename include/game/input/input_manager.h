// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_INPUT_INPUT_MANAGER_H
#define JACTORIO_INCLUDE_GAME_INPUT_INPUT_MANAGER_H
#pragma once

#include <functional>
#include <unordered_map>
#include <vector>

#include "core/hashers.h"
#include "game/input/input_type.h"

namespace jactorio::game
{
    class InputManager
    {
        using InputCallback = std::function<void()>;

    public:
        /// Positive = SDL_KeyCode
        /// Negative = MouseInput * -1 (Should never be 0)
        using IntKeyMouseCodePair = int;

        using InputKeyData = std::tuple<IntKeyMouseCodePair, InputAction, SDL_Keymod>;

        /// 0 indicates invalid ID
        using CallbackId = uint64_t;

        ///
        /// Sets the state of an input
        /// Callbacks for the respective inputs are called when CallCallbacks() is called
        static void SetInput(SDL_KeyCode keycode, InputAction action, SDL_Keymod mod = KMOD_NONE);
        static void SetInput(MouseInput mouse, InputAction action, SDL_Keymod mod = KMOD_NONE);


        // ======================================================================

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
        void Unsubscribe(CallbackId callback_id);

        ///
        /// Deletes all callback data
        void Clear();

        ///
        static InputAction ToInputAction(int action, bool repeat);

    private:
        static std::unordered_map<IntKeyMouseCodePair, InputKeyData> activeInputs_;


        // Increments with each new assigned callback, one is probably not having 4 million registered callbacks
        // so this doesn't need to be decremented
        CallbackId callbackId_ = 1;


        // tuple format: key, action, mods
        // id of callbacks registered to the tuple
        std::unordered_map<InputKeyData, std::vector<CallbackId>, hash<InputKeyData>> callbackIds_{};

        std::unordered_map<CallbackId, InputCallback> inputCallbacks_{};


        void CallCallbacks(const InputKeyData& input);
    };
} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_INPUT_INPUT_MANAGER_H
