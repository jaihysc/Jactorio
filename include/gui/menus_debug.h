// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GUI_MENUS_DEBUG_H
#define JACTORIO_INCLUDE_GUI_MENUS_DEBUG_H
#pragma once

#include "core/data_type.h"

namespace jactorio::data
{
    class PrototypeManager;
} // namespace jactorio::data

namespace jactorio::game
{
    class Logic;
    class Player;
} // namespace jactorio::game

namespace jactorio::render
{
    class Renderer;
} // namespace jactorio::render

namespace jactorio::gui
{
    // Debug menu main is in menus.h

    // For performing logic when certain debug options are checked
    void DebugMenuLogic(GameWorlds& worlds,
                        game::Logic& logic,
                        game::Player& player,
                        const data::PrototypeManager& proto,
                        render::Renderer& renderer);

    void DebugTimings();

    void DebugItemSpawner(game::Player& player, const data::PrototypeManager& proto);

    /// Info on tile currently hovered over
    void DebugTileInfo(GameWorlds& worlds, game::Player& player);

    void DebugConveyorInfo(GameWorlds& worlds,
                           game::Player& player,
                           const data::PrototypeManager& proto,
                           render::Renderer& renderer);

    void DebugInserterInfo(GameWorlds& worlds, game::Player& player);

    void DebugWorldInfo(GameWorlds& worlds, const game::Player& player);

    void DebugLogicInfo(const game::Logic& logic);
} // namespace jactorio::gui

#endif // JACTORIO_INCLUDE_GUI_MENUS_DEBUG_H
