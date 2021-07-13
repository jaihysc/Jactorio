// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GUI_IMGUI_MANAGER_H
#define JACTORIO_INCLUDE_GUI_IMGUI_MANAGER_H
#pragma once

#include "core/data_type.h"
#include "render/imgui_renderer.h"

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

namespace jactorio::proto
{
    class Localization;
}

namespace jactorio::render
{
    class DisplayWindow;
    class RendererSprites;
    class Spritemap;
    class Texture;
} // namespace jactorio::render


namespace jactorio::gui
{
    struct MenuData;


    // If true, ImGui has handled the a input event and thus should not be carried to down the layer
    inline bool input_mouse_captured    = false;
    inline bool input_keyboard_captured = false;

    /// Manages imgui context
    class ImGuiManager
    {
    public:
        ImGuiManager() = default;
        ~ImGuiManager();

        /// Sets up ImGui context
        void Init(const render::DisplayWindow& display_window);

        /// Initializes the spritemap for rendering the character menus
        /// \remark Requires Sprite::sprite_group::gui to be initialized
        void InitCharacterData(const render::Spritemap& spritemap, const render::Texture& texture);

        /// Loads glyphs from provided localization's font
        /// \exception std::runtime_error if load failed
        void LoadFont(const proto::Localization& localization) const;

        // TODO move this elsewhere
        /// Draws all the menus when in world
        void Draw(GameWorlds& worlds,
                  game::Logic& logic,
                  game::Player& player,
                  const data::PrototypeManager& proto,
                  game::EventData& event) const;

        void BeginFrame(const render::DisplayWindow& display_window) const;
        void RenderFrame() const;

        render::ImGuiRenderer imRenderer;

    private:
        bool hasImGuiContext_ = false;
        bool hasInitRenderer_ = false;

        const SpriteTexCoords* spritePositions_ = nullptr;
        unsigned int texId_                     = 0; // Assigned by openGL
    };

} // namespace jactorio::gui

#endif // JACTORIO_INCLUDE_GUI_IMGUI_MANAGER_H
