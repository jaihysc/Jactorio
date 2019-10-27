#ifndef RENDERER_RENDERING_RENDERER_MANAGER_H
#define RENDERER_RENDERING_RENDERER_MANAGER_H

#include <string>
#include <vector>

namespace jactorio::renderer
{
	struct Render_data
	{
		// Top left
		unsigned short tl_x;
		// Top left
		unsigned short tl_y;


		// Bottom right
		unsigned short br_x;
		// Bottom right
		unsigned short br_y;

		std::string* internal_name;
	};

	namespace renderer_manager
	{
		/**
		 * Adds an item which will be drawn by the renderer <br>
		 * Positions in pixels <br>
		 * 0, 0 is top left, bottom right is size of the display window
		 * @param tl_x Sprite position top left
		 * @param tl_y Sprite position top left
		 * @param br_x Sprite position bottom right
		 * @param br_y Sprite position bottom right
		 * @param sprite_internal_name Internal name of sprite
		 */
		void add_draw_item(unsigned int tl_x, unsigned int tl_y,
		                   unsigned int br_x, unsigned int br_y,
		                   const std::string& sprite_internal_name);

		std::vector<Render_data>* get_render_data();
	};
}

#endif // RENDERER_RENDERING_RENDERER_MANAGER_H
