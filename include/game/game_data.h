// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_GAME_DATA_H
#define JACTORIO_INCLUDE_GAME_GAME_DATA_H
#pragma once

#include "data/prototype_manager.h"
#include "data/unique_data_manager.h"
#include "game/event/event.h"
#include "game/input/input_manager.h"
#include "game/input/mouse_selection.h"
#include "game/logic/logic_data.h"
#include "game/player/player_data.h"
#include "game/world/world_data.h"

namespace jactorio::game
{
    ///
    /// Does not persist across application restarts
    struct GameDataLocal
    {
        struct GameInput
        {
            MouseSelection mouse;
            KeyInput key;
        };

        data::PrototypeManager prototype;
        data::UniqueDataManager unique;

        GameInput input;
        EventData event;
    };

    ///
    /// Serialized runtime data, persists across restarts
    struct GameDataGlobal
    {
        GameWorlds worlds{1};
        LogicData logic;
        PlayerData player;

        static_assert(std::is_same_v<GameWorlds::size_type, WorldId>);


        CEREAL_SERIALIZE(archive) {
            // Order must be: world, logic, player
            archive(worlds);
            archive(logic);
            archive(player);
        }
    };
} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_GAME_DATA_H
