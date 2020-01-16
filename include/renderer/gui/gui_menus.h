#ifndef RENDERER_GUI_GUI_MENUS_H
#define RENDERER_GUI_GUI_MENUS_H

#include <imgui/imgui.h>
#include "renderer/gui/imgui_manager.h"

namespace jactorio::renderer::gui
{
	void character_menu(ImGuiWindowFlags window_flags, const imgui_manager::Character_menu_data& menu_data);
	
	void cursor_window(ImGuiWindowFlags window_flags, imgui_manager::Character_menu_data menu_data);
}

#endif // RENDERER_GUI_GUI_MENUS_H
