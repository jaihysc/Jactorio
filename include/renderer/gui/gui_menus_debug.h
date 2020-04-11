// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 01/01/2020

#ifndef JACTORIO_INCLUDE_RENDERER_GUI_GUI_MENUS_DEBUG_H
#define JACTORIO_INCLUDE_RENDERER_GUI_GUI_MENUS_DEBUG_H
#pragma once

#include <imgui/imgui.h>

#include "game/player/player_data.h"

namespace jactorio::renderer::gui
{
	// For performing logic when certain debug options are checked
	void debug_menu_logic(game::Player_data& player_data);

	void debug_menu_main(ImGuiWindowFlags window_flags, game::Player_data& player_data);


	void debug_timings();

	void debug_item_spawner(game::Player_data& player_data);

	void debug_transport_line_info(game::Player_data& player_data);
}

#endif //JACTORIO_INCLUDE_RENDERER_GUI_GUI_MENUS_DEBUG_H
