// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GUI_CONTEXT_H
#define JACTORIO_INCLUDE_GUI_CONTEXT_H
#pragma once

#include "core/data_type.h"

namespace jactorio::data
{
    class PrototypeManager;
} // namespace jactorio::data

namespace jactorio::game
{
    class Player;
    class Logic;
} // namespace jactorio::game

namespace jactorio::gui
{
    struct MenuData;
} // namespace jactorio::gui

namespace jactorio::gui
{
    struct Context
    {
        GameWorlds& worlds;
        game::Logic& logic;
        game::Player& player;

        const data::PrototypeManager& proto;

        MenuData& menuData;
        /// Location player clicked/activated (Not guaranteed top left of entities)
        WorldCoord coord;
    };
} // namespace jactorio::gui

#endif // JACTORIO_INCLUDE_GUI_CONTEXT_H
