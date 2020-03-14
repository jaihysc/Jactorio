#ifndef RENDERER_GUI_GUI_MENUS_DEBUG_H
#define RENDERER_GUI_GUI_MENUS_DEBUG_H

#include <imgui/imgui.h>

#include "game/player/player_data.h"

namespace jactorio::renderer::gui
{
	// For performing logic when certain debug options are checked
	void debug_menu_logic(game::Player_data& player_data);

	void debug_menu_main(ImGuiWindowFlags window_flags, game::Player_data& player_data);

	void debug_timings(ImGuiWindowFlags window_flags);

	void debug_item_spawner(ImGuiWindowFlags window_flags, game::Player_data& player_data);

}

#endif // RENDERER_GUI_GUI_MENUS_DEBUG_H
