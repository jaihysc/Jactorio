// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDER_RENDER_CONTROLLER_H
#define JACTORIO_INCLUDE_RENDER_RENDER_CONTROLLER_H
#pragma once

#include "render/display_window.h"
#include "render/opengl/shader.h"
#include "render/renderer.h"
#include "render/spritemap_generator.h"

namespace jactorio
{
    class ThreadedLoopCommon;
}

namespace jactorio::render
{
    class Renderer;

    /// Top level class for controlling rendering
    class RenderController
    {
    public:
        RenderController() = default;
        ~RenderController();

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

        DisplayWindow displayWindow;
        Renderer renderer;
        RendererSprites rendererSprites;
        Shader shader;

    private:
        void InitGui();
        void InitGuiFont(ThreadedLoopCommon& common) const;
        void InitTextures(ThreadedLoopCommon& common);
        /// \exception std::runtime_error Too many tex coords for shader
        void InitShader(Renderer& renderer);

        /// Sends the tex coords for next animation frame to GPU
        void UpdateAnimationTexCoords() const noexcept;

        bool setupGui_ = false;
    };
} // namespace jactorio::render

#endif // JACTORIO_INCLUDE_RENDER_RENDER_CONTROLLER_H
