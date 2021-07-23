// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_CORE_LOOP_COMMON_H
#define JACTORIO_INCLUDE_CORE_LOOP_COMMON_H
#pragma once

#include <mutex>

#include "game/game_controller.h"
#include "gui/main_menu_data.h"
#include "render/render_controller.h"

namespace jactorio
{
    /// Used between threaded loops
    class ThreadedLoopCommon
    {
    public:
        enum class GameState
        {
            main_menu,
            in_world,
            quit
        };


        std::mutex playerDataMutex;
        std::mutex worldDataMutex;


        GameState gameState = GameState::main_menu;
        gui::MainMenuData mainMenuData;


        volatile bool prototypeLoadingComplete = false;


        game::GameController gameController{&this->renderController};
        /// Because OpenGL does not allow calls from different threads,
        /// this must be created and destroyed in the renderer thread
        render::RenderController* renderController = nullptr;
    };
} // namespace jactorio


#endif // JACTORIO_INCLUDE_CORE_LOOP_COMMON_H
