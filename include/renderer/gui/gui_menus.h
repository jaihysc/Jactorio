#ifndef RENDERER_GUI_GUI_MENUS_H
#define RENDERER_GUI_GUI_MENUS_H

#include <imgui/imgui.h>

#include "data/prototype/item/item.h"

namespace jactorio::renderer::gui
{
	// ==========================================================================================
	// Player menus
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

	// ==========================================================================================
	// Entity menus
	void container_entity(data::item_stack* inv, uint16_t inv_size);
}

#endif // RENDERER_GUI_GUI_MENUS_H
