// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDER_RENDER_LOOP_H
#define JACTORIO_INCLUDE_RENDER_RENDER_LOOP_H
#pragma once

namespace jactorio
{
    struct ThreadedLoopCommon;

    namespace game
    {
        class EventData;
    }
} // namespace jactorio

namespace jactorio::render
{
    class Renderer;

    void ChangeWindowSize(game::EventData& event, unsigned int window_size_x, unsigned int window_size_y);

    ///
    /// Gives pointer to base render rendering the map for changing the zoom level or any operation
    Renderer* GetBaseRenderer();

    ///
    /// Initiates rendering and starts the rendering thread
    void RenderInit(ThreadedLoopCommon& common);
} // namespace jactorio::render

#endif // JACTORIO_INCLUDE_RENDER_RENDER_LOOP_H
