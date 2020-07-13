// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 01/01/2020

#ifndef JACTORIO_INCLUDE_RENDERER_GUI_GUI_MENUS_DEBUG_H
#define JACTORIO_INCLUDE_RENDERER_GUI_GUI_MENUS_DEBUG_H
#pragma once

#include "game/player/player_data.h"

namespace jactorio::renderer
{
	// Debug menu main is in gui_menus.h

	// For performing logic when certain debug options are checked
	void DebugMenuLogic(game::PlayerData& player_data, const data::PrototypeManager& data_manager);

	void DebugTimings();

	void DebugItemSpawner(game::PlayerData& player_data, const data::PrototypeManager& data_manager);

	void DebugTransportLineInfo(game::PlayerData& player_data, const data::PrototypeManager& data_manager);

	void DebugInserterInfo(game::PlayerData& player_data);
}

#endif //JACTORIO_INCLUDE_RENDERER_GUI_GUI_MENUS_DEBUG_H
