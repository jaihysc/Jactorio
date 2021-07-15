// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDER_RENDER_CONTROLLER_H
#define JACTORIO_INCLUDE_RENDER_RENDER_CONTROLLER_H
#pragma once

#include "gui/imgui_manager.h"
#include "render/display_window.h"
#include "render/renderer_common.h"
#include "render/spritemap_generator.h"
#include "render/tile_renderer.h"

namespace jactorio
{
    class ThreadedLoopCommon;
}

namespace jactorio::render
{
    /// Top level class for controlling rendering
    class RenderController
    {
    public:
        /// Initializes components for rendering which does NOT require game data
        /// \exception If error
        void Init();

        /// Initializes components for rendering which DOES require game data
        /// \exception If error
        void LoadedInit(ThreadedLoopCommon& common);


        /// Prepares a frame in main menu, does NOT draw to screen
        void RenderMainMenu(ThreadedLoopCommon& common) const;

        /// Prepares a frame in world, does NOT draw to screen
        void RenderWorld(ThreadedLoopCommon& common);

        // The ordering of members is deliberate to control destruction order
        DisplayWindow displayWindow; // Holds gl context(all members below require)
        RendererCommon rCommon;
        gui::ImGuiManager imManager{rCommon};
        TileRenderer renderer{rCommon};
        RendererSprites rendererSprites;

    private:
        /// Sets up properties shared by both the tile and imgui renderer
        void InitRendererCommon() const noexcept;
        void InitGuiFont(ThreadedLoopCommon& common);
        void InitTextures(ThreadedLoopCommon& common);

        bool setupGui_ = false;
    };
} // namespace jactorio::render

#endif // JACTORIO_INCLUDE_RENDER_RENDER_CONTROLLER_H
