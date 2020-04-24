// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/15/2019

#ifndef JACTORIO_INCLUDE_RENDERER_RENDER_MAIN_H
#define JACTORIO_INCLUDE_RENDERER_RENDER_MAIN_H
#pragma once

#include "renderer/rendering/renderer.h"

namespace jactorio::renderer
{
	void set_recalculate_renderer(unsigned short window_size_x, unsigned short window_size_y);

	/**
	 * Gives pointer to base renderer rendering the map for changing the zoom level or any operation
	 */
	Renderer* get_base_renderer();

	inline bool render_thread_should_exit = false;

	///
	/// \brief Initiates rendering and starts the rendering thread
	/// \param mutex Will lock when preparing render data and rendering, unlock while sleeping until the next frame
	void render_init();
}

#endif //JACTORIO_INCLUDE_RENDERER_RENDER_MAIN_H
