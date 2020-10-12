// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GUI_GUI_MENUS_DEBUG_H
#define JACTORIO_INCLUDE_GUI_GUI_MENUS_DEBUG_H
#pragma once

#include "core/data_type.h"

namespace jactorio
{
    namespace data
    {
        class PrototypeManager;
    }

    namespace game
    {
        class LogicData;
        class PlayerData;
    } // namespace game
} // namespace jactorio

namespace jactorio::gui
{
    // Debug menu main is in gui_menus.h

    // For performing logic when certain debug options are checked
    void DebugMenuLogic(GameWorlds& worlds,
                        game::LogicData& logic,
                        game::PlayerData& player,
                        const data::PrototypeManager& data_manager);

    void DebugTimings();

    void DebugItemSpawner(game::PlayerData& player_data, const data::PrototypeManager& data_manager);

    void DebugConveyorInfo(GameWorlds& worlds, game::PlayerData& player, const data::PrototypeManager& proto_manager);

    void DebugInserterInfo(GameWorlds& worlds, game::PlayerData& player);

    void DebugWorldInfo(GameWorlds& worlds, const game::PlayerData& player);

    void DebugLogicInfo(const game::LogicData& logic_data);
} // namespace jactorio::gui

#endif // JACTORIO_INCLUDE_GUI_GUI_MENUS_DEBUG_H
