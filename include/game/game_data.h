// 
// game.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 03/11/2020
// Last modified: 03/11/2020
// 

#ifndef JACTORIO_GAME_GAME_H
#define JACTORIO_GAME_GAME_H
#pragma once

#include "game/input/mouse_selection.h"
#include "game/player/player_data.h"
#include "game/world/world_data.h"

namespace jactorio::game
{
	struct Game_input
	{
		Mouse_selection mouse{};
	};
	
	///
	/// \brief Holds all data for the runtime of the game (Wrapped with Pybind)
	/// Each sub data has its own mutex enabling concurrency
	struct Game_data
	{
		Game_input input{};
		// Game_event event{};
		Player_data player{};
		World_data world{};
	};

	/// This should only be accessed from logic_loop or render_main,
	/// any other should get references to members to remain testable
	inline Game_data* game_data;
}

#endif // JACTORIO_GAME_GAME_H
