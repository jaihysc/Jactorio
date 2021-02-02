// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GUI_MENUS_DEBUG_H
#define JACTORIO_INCLUDE_GUI_MENUS_DEBUG_H
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
        class Logic;
        class Player;
    } // namespace game
} // namespace jactorio

namespace jactorio::gui
{
    // Debug menu main is in menus.h

    // For performing logic when certain debug options are checked
    void DebugMenuLogic(GameWorlds& worlds,
                        game::Logic& logic,
                        game::Player& player,
                        const data::PrototypeManager& data_manager);

    void DebugTimings();

    void DebugItemSpawner(game::Player& player, const data::PrototypeManager& data_manager);

    ///
    /// Info on tile currently hovered over
    void DebugTileInfo(GameWorlds& worlds, game::Player& player);

    void DebugConveyorInfo(GameWorlds& worlds, game::Player& player, const data::PrototypeManager& proto_manager);

    void DebugInserterInfo(GameWorlds& worlds, game::Player& player);

    void DebugWorldInfo(GameWorlds& worlds, const game::Player& player);

    void DebugLogicInfo(const game::Logic& logic);
} // namespace jactorio::gui

#endif // JACTORIO_INCLUDE_GUI_MENUS_DEBUG_H
