// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_CORE_LOOP_COMMON_H
#define JACTORIO_INCLUDE_CORE_LOOP_COMMON_H
#pragma once

#include <mutex>

#include "game/game_data.h"

namespace jactorio
{
    struct MainMenuData
    {
        enum class Window
        {
            main,
            new_game,
            load_game,
            save_game
        };

        static constexpr auto kMaxSaveNameLength = 100;

        char saveName[kMaxSaveNameLength + 1] = ""; // Holds user input for save name

        std::string lastLoadError;
        std::string lastSaveError;

        Window currentMenu = Window::main;
    };


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
        MainMenuData mainMenuData;


        bool logicThreadShouldExit             = false;
        volatile bool prototypeLoadingComplete = false;
    };
} // namespace jactorio


#endif // JACTORIO_INCLUDE_CORE_LOOP_COMMON_H
