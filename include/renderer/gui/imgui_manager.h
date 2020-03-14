// 
// imgui_manager.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 10/22/2019
// Last modified: 03/12/2020
// 

#ifndef RENDERER_GUI_IMGUI_MANAGER_H
#define RENDERER_GUI_IMGUI_MANAGER_H
#pragma once

#include <GLFW/glfw3.h>

#include "game/player/player_data.h"

namespace jactorio::renderer::imgui_manager
{
	// If true, ImGui has handled the a input event and thus should not be carried to down the layer
	inline bool input_captured = false;

	/**
	 * Specifies gui window to draw
	 */
	enum class guiWindow
	{
		none = -1,

		character = 0,
		debug
	};

	struct Menu_data
	{
		std::unordered_map<unsigned, core::Quad_position> sprite_positions;
		unsigned int tex_id = 0;  // Assigned by openGL
	};

	/**
	 * Initializes the spritemap for rendering the character menus <br>
	 * Requires Sprite::sprite_group::gui to be initialized
	 */
	void setup_character_data();
	J_NODISCARD Menu_data& get_menu_data();

	/**
	 * Begins a self contained drawing loop which displays the specified error message
	 */
	void show_error_prompt(const std::string& err_title, const std::string& err_message);

	void setup(GLFWwindow* window);

	void set_window_visibility(guiWindow window, bool visibility);
	bool get_window_visibility(guiWindow window);

	void imgui_draw(game::Player_data& player_data);

	void imgui_terminate();
}

#endif // RENDERER_GUI_IMGUI_MANAGER_H
