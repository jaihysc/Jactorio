// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDER_GUI_MAIN_MENU_H
#define JACTORIO_INCLUDE_RENDER_GUI_MAIN_MENU_H
#pragma once

namespace jactorio
{
    struct ThreadedLoopCommon;
}

namespace jactorio::render
{
    ///
    /// Menu upon start
    void StartMenu(ThreadedLoopCommon& common);

    ///
    /// Menu in game
    void MainMenu(ThreadedLoopCommon& common);
} // namespace jactorio::render

#endif // JACTORIO_INCLUDE_RENDER_GUI_MAIN_MENU_H
