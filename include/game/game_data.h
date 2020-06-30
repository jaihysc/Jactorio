// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#ifndef JACTORIO_INCLUDE_GAME_GAME_DATA_H
#define JACTORIO_INCLUDE_GAME_GAME_DATA_H
#pragma once

#include "game/event/event.h"
#include "game/input/input_manager.h"
#include "game/input/mouse_selection.h"
#include "game/player/player_data.h"
#include "game/world/world_data.h"

namespace jactorio::game
{
	struct GameInput
	{
		MouseSelection mouse{};
		KeyInput key{};
	};

	///
	/// \brief Holds all data for the runtime of the game (Wrapped with Pybind)
	/// Each sub data has its own mutex enabling concurrency
	struct GameData
	{
		data::DataManager prototype{};
		GameInput input{};
		EventData event{};
		PlayerData player{};
		WorldData world{};
	};

	/// This should only be accessed from logic_loop or render_main,
	/// any other should get references to members to remain testable
	inline GameData* game_data;
}

#endif //JACTORIO_INCLUDE_GAME_GAME_DATA_H
