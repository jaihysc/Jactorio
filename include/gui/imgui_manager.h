// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GUI_IMGUI_MANAGER_H
#define JACTORIO_INCLUDE_GUI_IMGUI_MANAGER_H
#pragma once

#include "core/data_type.h"

namespace jactorio::data
{
    class PrototypeManager;
} // namespace jactorio::data

namespace jactorio::game
{
    class EventData;
    class Player;
    class Logic;
} // namespace jactorio::game

namespace jactorio::render
{
    class DisplayWindow;
    class RendererSprites;
} // namespace jactorio::render


namespace jactorio::gui
{
    struct MenuData;


    // If true, ImGui has handled the a input event and thus should not be carried to down the layer
    inline bool input_mouse_captured    = false;
    inline bool input_keyboard_captured = false;


    ///
    /// Initializes the spritemap for rendering the character menus
    /// \remark Requires Sprite::sprite_group::gui to be initialized
    void SetupCharacterData(render::RendererSprites& renderer_sprites);

    void Setup(const render::DisplayWindow& display_window);

    void ImguiBeginFrame(const render::DisplayWindow& display_window);
    void ImguiRenderFrame();

    void ImguiDraw(const render::DisplayWindow& display_window,
                   GameWorlds& worlds,
                   game::Logic& logic,
                   game::Player& player,
                   const data::PrototypeManager& proto_manager,
                   game::EventData& event);

    void ImguiTerminate();
} // namespace jactorio::gui

#endif // JACTORIO_INCLUDE_GUI_IMGUI_MANAGER_H
