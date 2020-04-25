// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 01/01/2020

#ifndef JACTORIO_INCLUDE_RENDERER_GUI_GUI_MENUS_H
#define JACTORIO_INCLUDE_RENDERER_GUI_GUI_MENUS_H
#pragma once

#include "jactorio.h"

namespace jactorio
{
	namespace game
	{
		struct Player_data;
	}

	namespace data
	{
		struct Unique_data_base;
	}
}


namespace jactorio::renderer::gui
{
	/// Function to draw the menu
	using menu_function = void (*)(game::Player_data& player_data, const data::Unique_data_base* unique_data);

	// ======================================================================
	// Substitutes name_ below at macro definitions to create an array of guis

	// crafting_queue : Draws the crafting queue in the bottom left of the screen
	// pickup_progressbar : Draws progressbar indicating entity pickup status

#define J_GUI_WINDOW\
	J_GUI_WINDOW_SUB(debug_menu)\
	\
	J_GUI_WINDOW_SUB(character_menu)\
	J_GUI_WINDOW_SUB(cursor_window)\
	J_GUI_WINDOW_SUB(crafting_queue)\
	J_GUI_WINDOW_SUB(pickup_progressbar)\
	\
	J_GUI_WINDOW_SUB(container_entity)\
	J_GUI_WINDOW_SUB(mining_drill)

	// ======================================================================
	// Macro definitions - 3

	// Function
#define J_GUI_WINDOW_SUB(name_)\
	void name_(game::Player_data& player_data, const data::Unique_data_base* unique_data = nullptr);

	J_GUI_WINDOW

	
	// Enum
#define J_GUI_WINDOW_SUB(name_)\
	name_,

	enum class menu
	{
		J_GUI_WINDOW
	};


	// Menu array
#define J_GUI_WINDOW_SUB(name_)\
	{name_},

	///
	/// \remark Index with menu
	struct Menu
	{
		Menu(const menu_function draw_ptr) noexcept
			: draw_ptr(draw_ptr) {
		}

		menu_function draw_ptr = nullptr;
		bool visible = false;
	};

	inline Menu menus[]
	{
		J_GUI_WINDOW
	};

#undef J_GUI_WINDOW

	// ======================================================================
	// Window visibility handling

	J_NODISCARD inline bool is_visible(menu gui_menu) {
		return menus[static_cast<int>(gui_menu)].visible;
	}

	inline void set_visible(menu gui_menu, const bool visibility) {
		menus[static_cast<int>(gui_menu)].visible = visibility;
	}
}

#endif //JACTORIO_INCLUDE_RENDERER_GUI_GUI_MENUS_H
