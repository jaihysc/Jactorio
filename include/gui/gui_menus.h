// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GUI_GUI_MENUS_H
#define JACTORIO_INCLUDE_GUI_GUI_MENUS_H
#pragma once

#include "jactorio.h"

#include "render/gui_renderer.h"

namespace jactorio::gui
{
    /// Function to draw the menu
    using MenuFunction = void (*)(const render::GuiRenderer& g_rendr);

    // ======================================================================
    // Substitutes name_ below at macro definitions to create an array of guis

    // crafting_queue : Draws the crafting queue in the bottom left of the screen
    // pickup_progressbar : Draws progressbar indicating entity pickup status

#define J_GUI_WINDOW                    \
    J_GUI_WINDOW_SUB(MainMenu)          \
                                        \
    J_GUI_WINDOW_SUB(DebugMenu)         \
                                        \
    J_GUI_WINDOW_SUB(CharacterMenu)     \
    J_GUI_WINDOW_SUB(CursorWindow)      \
    J_GUI_WINDOW_SUB(CraftingQueue)     \
    J_GUI_WINDOW_SUB(PickupProgressbar) \
                                        \
    J_GUI_WINDOW_SUB(ContainerEntity)   \
    J_GUI_WINDOW_SUB(MiningDrill)       \
    J_GUI_WINDOW_SUB(AssemblyMachine)

    // ======================================================================
    // Macro definitions - 3

    // Function
#define J_GUI_WINDOW_SUB(name__) void name__(const render::GuiRenderer& g_rendr);

    J_GUI_WINDOW


    // Enum
#undef J_GUI_WINDOW_SUB
#define J_GUI_WINDOW_SUB(name__) name__,

    enum class Menu
    {
        J_GUI_WINDOW
    };


    // Menu array
#undef J_GUI_WINDOW_SUB
#define J_GUI_WINDOW_SUB(name__) {name__},

    ///
    /// \remark Index with menu
    struct GuiWindow
    {
        GuiWindow(const MenuFunction draw_ptr) noexcept : drawPtr(draw_ptr) {}

        MenuFunction drawPtr = nullptr;
        bool visible         = false;
    };

    inline GuiWindow menus[]{J_GUI_WINDOW};

#undef J_GUI_WINDOW
#undef J_GUI_WINDOW_SUB

    // ======================================================================
    // Window visibility handling

    J_NODISCARD inline bool IsVisible(Menu gui_menu) {
        return menus[static_cast<int>(gui_menu)].visible;
    }

    inline void SetVisible(Menu gui_menu, const bool visibility) {
        menus[static_cast<int>(gui_menu)].visible = visibility;
    }
} // namespace jactorio::gui

#endif // JACTORIO_INCLUDE_GUI_GUI_MENUS_H
