// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 01/01/2020

#ifndef JACTORIO_INCLUDE_RENDERER_GUI_GUI_MENUS_H
#define JACTORIO_INCLUDE_RENDERER_GUI_GUI_MENUS_H
#pragma once

#include "jactorio.h"

#include "data/data_manager.h"

namespace jactorio
{
	namespace game
	{
		struct PlayerData;
	}

	namespace data
	{
		struct UniqueDataBase;
	}
}


namespace jactorio::renderer
{
	/// Function to draw the menu
	using MenuFunction = void (*)(game::PlayerData& player_data, const data::DataManager& data_manager,
								  const data::PrototypeBase* prototype, data::UniqueDataBase* unique_data);

	// ======================================================================
	// Substitutes name_ below at macro definitions to create an array of guis

	// crafting_queue : Draws the crafting queue in the bottom left of the screen
	// pickup_progressbar : Draws progressbar indicating entity pickup status

#define J_GUI_WINDOW\
	J_GUI_WINDOW_SUB(DebugMenu)\
	\
	J_GUI_WINDOW_SUB(CharacterMenu)\
	J_GUI_WINDOW_SUB(CursorWindow)\
	J_GUI_WINDOW_SUB(CraftingQueue)\
	J_GUI_WINDOW_SUB(PickupProgressbar)\
	\
	J_GUI_WINDOW_SUB(ContainerEntity)\
	J_GUI_WINDOW_SUB(MiningDrill)\
	J_GUI_WINDOW_SUB(AssemblyMachine)

	// ======================================================================
	// Macro definitions - 3

	// Function
#define J_GUI_WINDOW_SUB(name_)\
	void name_(game::PlayerData& player_data, const data::DataManager& data_manager,\
			   const data::PrototypeBase* prototype = nullptr, data::UniqueDataBase* unique_data = nullptr);

	J_GUI_WINDOW


	// Enum
#define J_GUI_WINDOW_SUB(name_)\
	name_,

	enum class Menu
	{
		J_GUI_WINDOW
	};


	// Menu array
#define J_GUI_WINDOW_SUB(name_)\
	{name_},

	///
	/// \remark Index with menu
	struct GuiMenu
	{
		GuiMenu(const MenuFunction draw_ptr) noexcept
			: drawPtr(draw_ptr) {
		}

		MenuFunction drawPtr = nullptr;
		bool visible         = false;
	};

	inline GuiMenu menus[]
	{
		J_GUI_WINDOW
	};

#undef J_GUI_WINDOW

	// ======================================================================
	// Window visibility handling

	J_NODISCARD inline bool IsVisible(Menu gui_menu) {
		return menus[static_cast<int>(gui_menu)].visible;
	}

	inline void SetVisible(Menu gui_menu, const bool visibility) {
		menus[static_cast<int>(gui_menu)].visible = visibility;
	}
}

#endif //JACTORIO_INCLUDE_RENDERER_GUI_GUI_MENUS_H
