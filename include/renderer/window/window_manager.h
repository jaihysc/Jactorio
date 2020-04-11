// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/22/2019

#ifndef JACTORIO_INCLUDE_RENDERER_WINDOW_WINDOW_MANAGER_H
#define JACTORIO_INCLUDE_RENDERER_WINDOW_WINDOW_MANAGER_H
#pragma once

#include <GLFW/glfw3.h>

#include "jactorio.h"

// https://stackoverflow.com/questions/47402766/switching-between-windowed-and-full-screen-in-opengl-glfw-3-2

/**
 * Manages the window in which the user sees
 */
namespace jactorio::renderer::window_manager
{
	///
	/// \brief Creates openGL context and window
	/// \exception Data_exception if logo cannot be found
	/// \return Non 0 if error occurred
	int init(int width, int height);

	/**
	 * Terminates openGL context and window
	 * @return Non 0 if error occurred
	 */
	int terminate();


	J_NODISCARD bool is_fullscreen();
	void set_fullscreen(bool fullscreen);


	GLFWwindow* get_window();
	bool context_active();
}

#endif //JACTORIO_INCLUDE_RENDERER_WINDOW_WINDOW_MANAGER_H
