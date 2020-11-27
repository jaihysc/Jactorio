// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GUI_MAIN_MENU_DATA_H
#define JACTORIO_INCLUDE_GUI_MAIN_MENU_DATA_H
#pragma once

#include <string>

namespace jactorio::gui
{
    struct MainMenuData
    {
        enum class Window
        {
            main,
            new_game,
            load_game,
            save_game,

            options,
            option_change_keybind
        };

        static constexpr auto kMaxSaveNameLength = 100;

        char saveName[kMaxSaveNameLength + 1] = ""; // Holds user input for save name

        std::string lastLoadError;
        std::string lastSaveError;

        Window currentMenu = Window::main;
    };
} // namespace jactorio::gui

#endif // JACTORIO_INCLUDE_GUI_MAIN_MENU_DATA_H
