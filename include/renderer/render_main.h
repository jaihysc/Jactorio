#ifndef JACTORIO_INCLUDE_RENDERER_RENDER_MAIN_H
#define JACTORIO_INCLUDE_RENDERER_RENDER_MAIN_H

#include <mutex>

#include "renderer/rendering/renderer.h"

namespace jactorio::renderer
{
	void set_recalculate_renderer(unsigned short window_size_x, unsigned short window_size_y);

	/**
	 * Gives pointer to base renderer rendering the map for changing the zoom level or any operation
	 */
	Renderer* get_base_renderer();

	/**
	 * Initiates rendering and starts the rendering thread
	 * @param mutex Will lock when preparing render data and rendering, unlock while sleeping until the next frame
	 * @return non-zero if error occurred
	 */
	int render_init(std::mutex* mutex);
}

#endif //JACTORIO_INCLUDE_RENDERER_RENDER_MAIN_H
