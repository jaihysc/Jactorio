// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_GAME_CONTROLLER_H
#define JACTORIO_INCLUDE_GAME_GAME_CONTROLLER_H
#pragma once

#include "data/prototype_manager.h"
#include "data/unique_data_manager.h"
#include "game/event/event.h"
#include "game/input/input_manager.h"
#include "game/input/mouse_selection.h"
#include "game/logic/logic.h"
#include "game/player/player.h"
#include "game/world/world.h"

namespace jactorio::game
{
    ///
    /// Top level class for controlling game simulation
    class GameController
    {
        static constexpr auto kDefaultWorldCount = 1;

    public:
        ///
        /// Clears worlds, logic, player
        void ResetGame();

        ///
        /// Allows worlds to be cleared
        void ClearRefsToWorld();

        // Non serialized

        struct GameInput
        {
            MouseSelection mouse;
            InputManager key;
        };

        data::PrototypeManager proto;
        data::UniqueDataManager unique;

        GameInput input;
        EventData event;


        // Serialized

        GameWorlds worlds{kDefaultWorldCount};
        Logic logic;
        Player player;

        static_assert(std::is_same_v<GameWorlds::size_type, WorldId>);

        CEREAL_SERIALIZE(archive) {
            // Order must be: world, logic, player
            archive(worlds);
            archive(logic);
            archive(player);
        }
    };
} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_GAME_CONTROLLER_H
