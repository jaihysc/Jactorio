#ifndef RENDERER_GUI_IMGUI_MANAGER_H
#define RENDERER_GUI_IMGUI_MANAGER_H

#include <GLFW/glfw3.h>

#include "renderer/rendering/spritemap_generator.h"

namespace jactorio::renderer::imgui_manager
{
	// If true, ImGui has handled the a input event and thus should not be carried to down the layer
	inline bool input_captured = false;
	
	/**
	 * Specifies gui window to draw
	 */
	enum class gui_window
	{
		none = -1,
		
		character = 0,
		debug
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

	/**
	 * Begins a self contained drawing loop which displays the specified error message
	 */
	void show_error_prompt(const std::string& err_title, const std::string& err_message);
	
	void setup(GLFWwindow* window);

	void set_window_visibility(gui_window window, bool visibility);
	bool get_window_visibility(gui_window window);
	
	void imgui_draw();
	
	void imgui_terminate();
}

#endif // RENDERER_GUI_IMGUI_MANAGER_H
