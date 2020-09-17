// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_CORE_LOOP_COMMON_H
#define JACTORIO_INCLUDE_CORE_LOOP_COMMON_H
#pragma once

#include <mutex>

#include "game/game_data.h"

namespace jactorio
{
    ///
    /// Used between threaded loops
    struct ThreadedLoopCommon
    {
        enum class GameState
        {
            main_menu,
            in_world,
            quit
        };

        std::mutex playerDataMutex;
        std::mutex worldDataMutex;

        game::GameDataLocal gameDataLocal;
        game::GameDataGlobal gameDataGlobal;

        GameState gameState = GameState::main_menu;

        bool logicThreadShouldExit             = false;
        volatile bool prototypeLoadingComplete = false;
    };
} // namespace jactorio


#endif // JACTORIO_INCLUDE_CORE_LOOP_COMMON_H
