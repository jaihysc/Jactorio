// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_GAME_DATA_H
#define JACTORIO_INCLUDE_GAME_GAME_DATA_H
#pragma once

#include "data/prototype_manager.h"
#include "game/event/event.h"
#include "game/input/input_manager.h"
#include "game/input/mouse_selection.h"
#include "game/logic/logic_data.h"
#include "game/player/player_data.h"
#include "game/world/world_data.h"

namespace jactorio::game
{
	///
	/// \brief Does not persist across application restarts
	struct GameDataLocal
	{
		struct GameInput
		{
			MouseSelection mouse{};
			KeyInput key{};
		};

		data::PrototypeManager prototype{};
		GameInput input{};
	};

	///
	/// \brief Serialized runtime data, persists across restarts
	struct GameDataGlobal
	{
		EventData event{};

		PlayerData player{};

		WorldData world{};
		LogicData logic{};
	};

	 // static_assert(std::is_move_constructible_v<GameDataGlobal>);
	 // static_assert(std::is_move_assignable_v<GameDataGlobal>);
	 //
	 // static_assert(std::is_move_constructible_v<EventData>);
	 // static_assert(std::is_move_constructible_v<PlayerData>);
	 // static_assert(std::is_move_constructible_v<WorldData>);
	 // static_assert(std::is_move_constructible_v<LogicData>);
	 //
	 // static_assert(std::is_move_assignable_v<EventData>);
	 // static_assert(std::is_move_assignable_v<PlayerData>);
	 // static_assert(std::is_move_assignable_v<WorldData>);
	 // static_assert(std::is_move_assignable_v<LogicData>);
}

#endif //JACTORIO_INCLUDE_GAME_GAME_DATA_H
