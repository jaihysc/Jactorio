// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 11/15/2019

#ifndef JACTORIO_INCLUDE_GAME_INPUT_INPUT_MANAGER_H
#define JACTORIO_INCLUDE_GAME_INPUT_INPUT_MANAGER_H
#pragma once

#include <functional>
#include <unordered_map>

#include "core/data_type.h"
#include "game/input/input_key.h"

namespace jactorio::game
{
	class KeyInput
	{
		using InputCallback = std::function<void()>;
		using CallbackId = uint64_t;

		using InputTuple = std::tuple<InputKey,
		                              InputAction,
		                              InputMod>;

		// ======================================================================
		// Currently set input(s)
		static std::unordered_map<InputKey, InputTuple> activeInputs_;

	public:
		///
		/// \brief Sets the static of an input
		/// \brief Callbacks for the respective inputs are called when dispatch_input_callbacks() is called
		static void SetInput(InputKey key, InputAction action, InputMod mods = InputMod::none);

	private:
		// Increments with each new assigned callback, one is probably not having 4 million registered callbacks
		// so this doesn't need to be decremented
		CallbackId callbackId_ = 1;


		// tuple format: key, action, mods
		// id of callbacks registered to the tuple
		std::unordered_map<InputTuple, std::vector<CallbackId>,
		                   core::hash<InputTuple>> callbackIds_{};

		std::unordered_map<CallbackId, InputCallback> inputCallbacks_{};


		void CallCallbacks(const InputTuple& input);

	public:
		///
		/// Registers an input callback which will be called when the specified input is activated
		/// \param key Target key
		/// \param action Key state
		/// \param mods Modifier keys which also have to be pressed
		/// \return id of the registered callback, use it to un-register it - 0 Indicates error
		unsigned Subscribe(const InputCallback&, InputKey key, InputAction action, InputMod mods = InputMod::none);


		///
		/// \brief Calls registered callbacks based on the input set with set_input(...)
		void Raise();


		///
		/// \brief Removes specified callback at callback_id
		void Unsubscribe(unsigned int callback_id, InputKey key, InputAction action, InputMod mods = InputMod::none);

		///
		/// \brief Deletes all callback data
		void ClearData();

		// ======================================================================
		// Conversion from GLFW macros

		///
		/// \brief Converts GLFW_ to enum
		static InputKey ToInputKey(int key);

		///
		/// \brief Converts GLFW_ to enum
		static InputAction ToInputAction(int action);

		///
		/// \brief Converts GLFW_MOD_ to enum
		static InputMod ToInputMod(int mod);
	};
}

#endif //JACTORIO_INCLUDE_GAME_INPUT_INPUT_MANAGER_H
