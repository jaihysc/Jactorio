// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_CORE_LOOP_COMMON_H
#define JACTORIO_INCLUDE_CORE_LOOP_COMMON_H
#pragma once

#include <mutex>
#include <optional>

#include "game/game_data.h"

namespace jactorio
{
    struct MainMenuData
    {
        // TODO move this to header
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
    class ThreadedLoopCommon
    {
    public:
        enum class GameState
        {
            main_menu,
            in_world,
            quit
        };


        ThreadedLoopCommon() {
            gameDataGlobal_.emplace();
        }


        [[nodiscard]] game::GameDataGlobal& GetDataGlobal() noexcept {
            return gameDataGlobal_.value();
        }

        [[nodiscard]] const game::GameDataGlobal& GetDataGlobal() const noexcept {
            return gameDataGlobal_.value();
        }


        std::mutex playerDataMutex;
        std::mutex worldDataMutex;

        game::GameDataLocal gameDataLocal;

        GameState gameState = GameState::main_menu;
        MainMenuData mainMenuData;


        bool logicThreadShouldExit             = false;
        volatile bool prototypeLoadingComplete = false;


        ///
        /// Reconstructs GameDataGlobal
        void ResetGlobalData() {
            gameDataGlobal_.reset();
            gameDataGlobal_.emplace();
        }

    private:
        std::optional<game::GameDataGlobal> gameDataGlobal_;
    };
} // namespace jactorio


#endif // JACTORIO_INCLUDE_CORE_LOOP_COMMON_H
