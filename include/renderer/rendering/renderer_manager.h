#ifndef RENDERER_RENDERING_RENDERER_MANAGER_H
#define RENDERER_RENDERING_RENDERER_MANAGER_H

#include <string>

namespace jactorio::renderer
{
	struct Render_data
	{
		// Renderer will buffer and draw the same items until the timestamp changes
		float timestamp;

		// Number of images on each axis
		unsigned int sprites_x;
		unsigned int sprites_y;

		// Follows dimensions specified by sprites_x and sprites_y
		// Each entry is a sprite id
		std::string* sprite_grid;
	};

	class Renderer_manager
	{
		static Render_data render_data_;

	public:
		/**
		 * Prepares data for the renderer to draw on next render_loop call
		 */
		static void gen_render_data(
			unsigned int sprites_x, unsigned int sprites_y,
			const std::string* sprite_grid);

		static Render_data get_render_data() {
			return render_data_;
		}
	};
}

#endif // RENDERER_RENDERING_RENDERER_MANAGER_H
