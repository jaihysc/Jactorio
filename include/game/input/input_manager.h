// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 11/15/2019

#ifndef JACTORIO_INCLUDE_GAME_INPUT_INPUT_MANAGER_H
#define JACTORIO_INCLUDE_GAME_INPUT_INPUT_MANAGER_H
#pragma once

#include <functional>

#include "core/data_type.h"
#include "game/input/input_key.h"

namespace jactorio::game
{
	class Key_input
	{
		using input_callback = std::function<void()>;
		using callback_id_t = uint64_t;

		using input_tuple = std::tuple<inputKey,
		                               inputAction,
		                               inputMod>;

		// ======================================================================
		// Currently set input(s)
		static std::unordered_map<inputKey, input_tuple> active_inputs_;

	public:
		///
		/// \brief Sets the static of an input
		/// \brief Callbacks for the respective inputs are called when dispatch_input_callbacks() is called
		static void set_input(inputKey key, inputAction action, inputMod mods = inputMod::none);

	private:
		// Increments with each new assigned callback, one is probably not having 4 million registered callbacks
		// so this doesn't need to be decremented
		callback_id_t callback_id_ = 1;


		// tuple format: key, action, mods
		// id of callbacks registered to the tuple
		std::unordered_map<input_tuple, std::vector<callback_id_t>,
		                   core::hash<input_tuple>> callback_ids_{};

		std::unordered_map<callback_id_t, input_callback> input_callbacks_{};


		void call_callbacks(const input_tuple& input);

	public:
		///
		/// Registers an input callback which will be called when the specified input is activated
		/// \param key Target key
		/// \param action Key state
		/// \param mods Modifier keys which also have to be pressed
		/// \return id of the registered callback, use it to un-register it - 0 Indicates error
		unsigned subscribe(const input_callback&, inputKey key, inputAction action, inputMod mods = inputMod::none);


		///
		/// \brief Calls registered callbacks based on the input set with set_input(...)
		void raise();


		///
		/// \brief Removes specified callback at callback_id
		void unsubscribe(unsigned int callback_id, inputKey key, inputAction action, inputMod mods = inputMod::none);

		///
		/// \brief Deletes all callback data
		void clear_data();

		// ======================================================================
		// Conversion from GLFW macros

		///
		/// \brief Converts GLFW_ to enum
		static inputKey to_input_key(int key);

		///
		/// \brief Converts GLFW_ to enum
		static inputAction to_input_action(int action);

		///
		/// \brief Converts GLFW_MOD_ to enum
		static inputMod to_input_mod(int mod);
	};
}

#endif //JACTORIO_INCLUDE_GAME_INPUT_INPUT_MANAGER_H
