#ifndef RENDERER_GUI_GUI_MENUS_DEBUG_H
#define RENDERER_GUI_GUI_MENUS_DEBUG_H

#include <imgui/imgui.h>

namespace jactorio::renderer::gui
{
	// For performing logic when certain debug options are checked
	void debug_menu_logic();

	void debug_menu_main(ImGuiWindowFlags window_flags);

	void debug_timings(ImGuiWindowFlags window_flags);

	void debug_item_spawner(ImGuiWindowFlags window_flags);

}

#endif // RENDERER_GUI_GUI_MENUS_DEBUG_H
