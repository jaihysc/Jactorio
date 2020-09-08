// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDERER_RENDER_MAIN_H
#define JACTORIO_INCLUDE_RENDERER_RENDER_MAIN_H
#pragma once

#include "game/logic_loop.h"
#include "render//rendering/renderer.h"

namespace jactorio::render
{
    void ChangeWindowSize(game::EventData& event, unsigned int window_size_x, unsigned int window_size_y);

    ///
    /// Gives pointer to base render rendering the map for changing the zoom level or any operation
    Renderer* GetBaseRenderer();

    ///
    /// Initiates rendering and starts the rendering thread
    void RenderInit(LogicRenderLoopCommon& common);
} // namespace jactorio::render

#endif // JACTORIO_INCLUDE_RENDERER_RENDER_MAIN_H
