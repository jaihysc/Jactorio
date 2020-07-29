// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDERER_RENDER_MAIN_H
#define JACTORIO_INCLUDE_RENDERER_RENDER_MAIN_H
#pragma once

#include "renderer/rendering/renderer.h"

namespace jactorio::renderer
{
	void ChangeWindowSize(unsigned int window_size_x, unsigned int window_size_y);

	///
	/// \brief Gives pointer to base renderer rendering the map for changing the zoom level or any operation
	Renderer* GetBaseRenderer();

	inline bool render_thread_should_exit = false;

	///
	/// \brief Initiates rendering and starts the rendering thread
	void RenderInit();
}

#endif //JACTORIO_INCLUDE_RENDERER_RENDER_MAIN_H
