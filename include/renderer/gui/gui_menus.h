// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 01/01/2020

#ifndef JACTORIO_INCLUDE_RENDERER_GUI_GUI_MENUS_H
#define JACTORIO_INCLUDE_RENDERER_GUI_GUI_MENUS_H
#pragma once

#include <imgui/imgui.h>

#include "data/prototype/item/item.h"
#include "game/player/player_data.h"

namespace jactorio::renderer::gui
{
	// ==========================================================================================
	// Player menus
	void character_menu(ImGuiWindowFlags window_flags, game::Player_data& player_data);

	void cursor_window(game::Player_data& player_data);

	/**
	 * Draws the crafting queue in the bottom left of the screen
	 */
	void crafting_queue(game::Player_data& player_data);

	/**
	 * Draws progressbar indicating entity pickup status
	 */
	void pickup_progressbar(game::Player_data& player_data);

	// ==========================================================================================
	// Entity menus
	void container_entity(game::Player_data& player_data, data::item_stack* inv, uint16_t inv_size);
}

#endif //JACTORIO_INCLUDE_RENDERER_GUI_GUI_MENUS_H
