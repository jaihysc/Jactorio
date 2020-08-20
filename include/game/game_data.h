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
		EventData event{};
	};

	///
	/// \brief Serialized runtime data, persists across restarts
	struct GameDataGlobal
	{
		PlayerData player{};

		WorldData world{};
		LogicData logic{};


		CEREAL_SERIALIZE(archive) {
			// TODO player and logic
			archive(world);
		}
	};
}

#endif //JACTORIO_INCLUDE_GAME_GAME_DATA_H
