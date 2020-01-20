#ifndef RENDER_MAIN_H
#define RENDER_MAIN_H

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
	 * @return non-zero if error occurred
	 */
	int render_init();
}

#endif // RENDER_MAIN_H
