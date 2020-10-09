// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDER_GUI_IMGUI_MANAGER_H
#define JACTORIO_INCLUDE_RENDER_GUI_IMGUI_MANAGER_H
#pragma once

#include "core/data_type.h"

namespace jactorio
{
    namespace data
    {
        class PrototypeManager;
    } // namespace data

    namespace game
    {
        class EventData;
        class PlayerData;
        class LogicData;
    } // namespace game

} // namespace jactorio

namespace jactorio::render
{
    class DisplayWindow;
    class RendererSprites;

    struct MenuData;


    // If true, ImGui has handled the a input event and thus should not be carried to down the layer
    inline bool input_mouse_captured    = false;
    inline bool input_keyboard_captured = false;


    ///
    /// Initializes the spritemap for rendering the character menus <br>
    /// \remark Requires Sprite::sprite_group::gui to be initialized
    void SetupCharacterData(RendererSprites& renderer_sprites);

    void Setup(const DisplayWindow& display_window);

    void ImguiBeginFrame(const DisplayWindow& display_window);
    void ImguiRenderFrame();

    void ImguiDraw(const DisplayWindow& display_window,
                   GameWorlds& worlds,
                   game::LogicData& logic,
                   game::PlayerData& player,
                   const data::PrototypeManager& proto_manager,
                   game::EventData& event);

    void ImguiTerminate();
} // namespace jactorio::render

#endif // JACTORIO_INCLUDE_RENDER_GUI_IMGUI_MANAGER_H
