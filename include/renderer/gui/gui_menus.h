#ifndef RENDERER_GUI_GUI_MENUS_H
#define RENDERER_GUI_GUI_MENUS_H

#include <imgui/imgui.h>

namespace jactorio::renderer::gui
{
	void character_menu(ImGuiWindowFlags window_flags);
	
	void cursor_window();

	/**
	 * Draws the crafting queue in the bottom left of the screen
	 */
	void crafting_queue();

	/**
	 * Draws progressbar indicating entity pickup status
	 */
	void pickup_progressbar();
}

#endif // RENDERER_GUI_GUI_MENUS_H
