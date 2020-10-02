// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_LOGIC_LOOP_H
#define JACTORIO_INCLUDE_GAME_LOGIC_LOOP_H
#pragma once

namespace jactorio
{
    class ThreadedLoopCommon;
}

namespace jactorio::game
{
    void InitLogicLoop(ThreadedLoopCommon& common);
}

#endif // JACTORIO_INCLUDE_GAME_LOGIC_LOOP_H
