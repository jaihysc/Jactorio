#ifndef RENDERER_RENDER_LOOP_CPP
#define RENDERER_RENDER_LOOP_CPP

#include "renderer/rendering/renderer.h"

namespace jactorio::renderer
{
	/**
	 * Called every render update interval to render to the screen
	 * @param renderer
	 */
	void render_loop(Renderer* renderer);
}

#endif // RENDERER_RENDER_LOOP_CPP
