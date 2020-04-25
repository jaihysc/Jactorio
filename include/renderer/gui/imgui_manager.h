// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/22/2019

#ifndef JACTORIO_INCLUDE_RENDERER_GUI_IMGUI_MANAGER_H
#define JACTORIO_INCLUDE_RENDERER_GUI_IMGUI_MANAGER_H
#pragma once

#include "game/event/event.h"
#include "game/player/player_data.h"
#include "renderer/rendering/spritemap_generator.h"

struct GLFWwindow;

namespace jactorio::renderer::imgui_manager
{
	// If true, ImGui has handled the a input event and thus should not be carried to down the layer
	inline bool input_captured = false;


	struct Menu_data
	{
		Menu_data(const std::unordered_map<unsigned, core::Quad_position>& sprite_positions, const unsigned tex_id)
			: sprite_positions(sprite_positions),
			  tex_id(tex_id) {
		}

		const std::unordered_map<unsigned, core::Quad_position>& sprite_positions;
		unsigned int tex_id = 0;  // Assigned by openGL
	};

	/**
	 * Initializes the spritemap for rendering the character menus <br>
	 * Requires Sprite::sprite_group::gui to be initialized
	 */
	void setup_character_data(Renderer_sprites& renderer_sprites);
	J_NODISCARD Menu_data get_menu_data();

	/**
	 * Begins a self contained drawing loop which displays the specified error message
	 */
	void show_error_prompt(const std::string& err_title, const std::string& err_message);

	void setup(GLFWwindow* window);

	void imgui_draw(game::Player_data& player_data, game::Event_data& event);

	void imgui_terminate();
}

#endif //JACTORIO_INCLUDE_RENDERER_GUI_IMGUI_MANAGER_H
