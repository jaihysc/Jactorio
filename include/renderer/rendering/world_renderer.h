#ifndef RENDERER_RENDERING_WORLD_RENDERER_H
#define RENDERER_RENDERING_WORLD_RENDERER_H

#include "renderer/rendering/renderer.h"

namespace jactorio::renderer::world_renderer
{
	inline long long player_position_x = 0;
	inline long long player_position_y = 0;

	/**
	 * Moves the world to match player_position_ <br>
	 * This is achieved by offsetting the rendered chunks, for decimal numbers, the view matrix is used
	 * @param renderer The renderer on which the world is drawn
	 */
	void render_player_position(Renderer* renderer);
}

#endif // RENDERER_RENDERING_WORLD_RENDERER_H
