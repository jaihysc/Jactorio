// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_LOGIC_CONVEYOR_CONTROLLER_H
#define JACTORIO_INCLUDE_GAME_LOGIC_CONVEYOR_CONTROLLER_H
#pragma once

namespace jactorio::game
{
    class World;

    /// Updates belt logic for a world
    void ConveyorLogicUpdate(World& world);

    /// Updates splitter logic for a world
    void SplitterLogicUpdate(World& world);
} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_LOGIC_CONVEYOR_CONTROLLER_H
