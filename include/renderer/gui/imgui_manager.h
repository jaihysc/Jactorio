#ifndef RENDERER_GUI_IMGUI_MANAGER_H
#define RENDERER_GUI_IMGUI_MANAGER_H

#include <GLFW/glfw3.h>

#include "renderer/rendering/spritemap_generator.h"

namespace jactorio::renderer::imgui_manager
{
	/**
	 * Specifies gui window to draw
	 */
	enum gui_window_type
	{
		none = -1,
		character = 0,
	};

	struct Character_menu_data
	{
		std::unordered_map<unsigned, renderer_sprites::Image_position> sprite_positions;
		unsigned int tex_id = 0;  // Assigned by openGL
	};
	
	/**
	 * Initializes the spritemap for rendering the character menus <br>
	 * Requires Sprite::sprite_group::gui to be initialized
	 */
	void setup_character_data();
	
	inline bool show_debug_menu = false;
	inline bool show_inventory_menu = false;

	/**
	 * Begins a self contained drawing loop which displays the specified error message
	 */
	void show_error_prompt(const std::string& err_title, const std::string& err_message);
	
	void setup(GLFWwindow* window);
	void imgui_draw();
	void imgui_terminate();
}

#endif // RENDERER_GUI_IMGUI_MANAGER_H
