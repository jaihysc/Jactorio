// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_CORE_LOOP_COMMON_H
#define JACTORIO_INCLUDE_CORE_LOOP_COMMON_H
#pragma once

#include <mutex>
#include <optional>

#include "game/game_data.h"
#include "game/player/keybind_manager.h"
#include "gui/main_menu_data.h"

namespace jactorio
{
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
        gui::MainMenuData mainMenuData;


        volatile bool prototypeLoadingComplete = false;


        ///
        /// Clears and reconstructs GameDataGlobal
        void ResetGlobalData();

    private:
        std::optional<game::GameDataGlobal> gameDataGlobal_{std::in_place};

    public:
        // Requires game data global to be constructed first
        game::KeybindManager keybindManager{gameDataLocal.input.key, GetDataGlobal()};
    };

    inline void ThreadedLoopCommon::ResetGlobalData() {
        GetDataGlobal().ClearRefsToWorld(gameDataLocal);

        gameDataGlobal_.reset();
        gameDataGlobal_.emplace();
    }
} // namespace jactorio


#endif // JACTORIO_INCLUDE_CORE_LOOP_COMMON_H
