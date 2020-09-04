// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_LOGIC_LOOP_H
#define JACTORIO_INCLUDE_GAME_LOGIC_LOOP_H
#pragma once

#include <mutex>

#include "game/game_data.h"

namespace jactorio
{
    struct LogicRenderLoopCommon
    {
        // Held by the thread which is currently operating on these sets of data

        std::mutex playerDataMutex;
        std::mutex worldDataMutex;

        game::GameDataLocal gameDataLocal;
        game::GameDataGlobal gameDataGlobal;

        bool logicThreadShouldExit             = false;
        bool renderThreadShouldExit            = false;
        volatile bool prototypeLoadingComplete = false;
    };
} // namespace jactorio

namespace jactorio::game
{
    void InitLogicLoop(LogicRenderLoopCommon& common);
}

#endif // JACTORIO_INCLUDE_GAME_LOGIC_LOOP_H
