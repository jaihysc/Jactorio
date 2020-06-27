// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/22/2019

#ifndef JACTORIO_INCLUDE_RENDERER_WINDOW_WINDOW_MANAGER_H
#define JACTORIO_INCLUDE_RENDERER_WINDOW_WINDOW_MANAGER_H
#pragma once

#include <SDL.h>

#include "jactorio.h"

namespace jactorio::renderer
{
	///
	/// \brief Manages the window in which the user sees

	///
	/// \brief Creates openGL context and window
	/// \exception Data_exception if logo cannot be found
	/// \return Non 0 if error occurred
	int InitWindow(int width, int height);

	///
	/// \brief Terminates openGL context and window
	/// \return Non 0 if error occurred
	int TerminateWindow();


	J_NODISCARD bool IsFullscreen();
	void SetFullscreen(bool desired_fullscreen);


	SDL_Window* GetWindow();
	SDL_GLContext GetContext();
	bool WindowContextActive();

	void HandleSdlEvent(const SDL_Event& sdl_event);
}

#endif //JACTORIO_INCLUDE_RENDERER_WINDOW_WINDOW_MANAGER_H
