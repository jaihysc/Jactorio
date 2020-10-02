// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDER_DISPLAY_WINDOW_H
#define JACTORIO_INCLUDE_RENDER_DISPLAY_WINDOW_H
#pragma once

#include <SDL.h>
#include <array>

#include "jactorio.h"

namespace jactorio
{
    class ThreadedLoopCommon;
}

namespace jactorio::render
{
    ///
    /// Manages a window in which the user sees
    class DisplayWindow
    {
    public:
        ///
        /// Creates openGL context and window
        /// \exception Data_exception if logo cannot be found
        /// \return Non 0 if error occurred
        int Init(int width, int height);

        ///
        /// Terminates openGL context and window
        /// \return Non 0 if error occurred
        int Terminate();


        J_NODISCARD bool IsFullscreen() const;
        void SetFullscreen(bool desired_fullscreen);


        J_NODISCARD SDL_Window* GetWindow() const;
        J_NODISCARD SDL_GLContext GetContext() const;
        J_NODISCARD bool WindowContextActive() const;

        void HandleSdlEvent(ThreadedLoopCommon& common, const SDL_Event& sdl_event) const;

    private:
        std::array<int, 2> windowPos_{0, 0};
        std::array<int, 2> windowSize_{0, 0};

        SDL_Window* sdlWindow_      = nullptr;
        SDL_GLContext sdlGlContext_ = nullptr;

        bool glContextActive_ = false;
    };
} // namespace jactorio::render

#endif // JACTORIO_INCLUDE_RENDER_DISPLAY_WINDOW_H
