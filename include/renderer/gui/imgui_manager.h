// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/22/2019

#ifndef JACTORIO_INCLUDE_RENDERER_GUI_IMGUI_MANAGER_H
#define JACTORIO_INCLUDE_RENDERER_GUI_IMGUI_MANAGER_H
#pragma once

#include "game/event/event.h"
#include "game/player/player_data.h"
#include "renderer/rendering/spritemap_generator.h"

struct GLFWwindow;

namespace jactorio::renderer
{
	// If true, ImGui has handled the a input event and thus should not be carried to down the layer
	inline bool input_captured = false;


	struct MenuData
	{
		MenuData(const std::unordered_map<unsigned, core::QuadPosition>& sprite_positions, const unsigned tex_id)
			: spritePositions(sprite_positions),
			  texId(tex_id) {
		}

		const std::unordered_map<unsigned, core::QuadPosition>& spritePositions;
		unsigned int texId = 0;  // Assigned by openGL
	};

	///
	/// \brief Initializes the spritemap for rendering the character menus <br>
	/// \remark Requires Sprite::sprite_group::gui to be initialized
	void SetupCharacterData(RendererSprites& renderer_sprites);
	J_NODISCARD MenuData GetMenuData();

	///
	/// \brief Begins a self contained drawing loop which displays the specified error message
	void ShowErrorPrompt(const std::string& err_title, const std::string& err_message);

	void Setup(GLFWwindow* window);

	void ImguiDraw(game::PlayerData& player_data, game::EventData& event);

	void ImguiTerminate();
}

#endif //JACTORIO_INCLUDE_RENDERER_GUI_IMGUI_MANAGER_H
