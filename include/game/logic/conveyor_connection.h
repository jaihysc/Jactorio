// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_LOGIC_CONVEYOR_CONNECTION_H
#define JACTORIO_INCLUDE_GAME_LOGIC_CONVEYOR_CONNECTION_H
#pragma once

#include "core/data_type.h"

namespace jactorio::game
{
    class WorldData;
    class ConveyorStruct;

    ///
    /// Calls Connect for up, right, down, left
    /// \param coord Current struct's coordinate
    void ConveyorConnect(WorldData& world, const WorldCoord& coord);

    ///
    /// Calls Disconnect for up, right, down, left
    /// \param coord Current struct's coordinate
    void ConveyorDisconnect(WorldData& world, const WorldCoord& coord);


    ///
    /// Attempts to connect to a connectable conveyor struct above or connect connectable conveyor struct above to
    /// current
    /// \param coord Current conveyor struct's coordinate
    void ConveyorConnectUp(WorldData& world, const WorldCoord& coord);

    ///
    /// Attempts to connect to a connectable conveyor struct at right or connect connectable conveyor struct at right to
    /// current
    /// \param coord Current conveyor struct's coordinate
    void ConveyorConnectRight(WorldData& world, const WorldCoord& coord);

    ///
    /// Attempts to connect to a connectable conveyor struct below or connect connectable conveyor struct below to
    /// current
    /// \param coord Current conveyor struct's coordinate
    void ConveyorConnectDown(WorldData& world, const WorldCoord& coord);

    ///
    /// Attempts to connect to a connectable conveyor struct to left or connect connectable conveyor struct to left to
    /// current
    /// \param coord Current conveyor struct's coordinate
    void ConveyorConnectLeft(WorldData& world, const WorldCoord& coord);


    ///
    /// Attempts to disconnect from connected conveyor struct above
    /// \param coord Current struct's coordinate
    void ConveyorDisconnectUp(WorldData& world, const WorldCoord& coord);

    void ConveyorDisconnectRight(WorldData& world, const WorldCoord& coord);

    void ConveyorDisconnectDown(WorldData& world, const WorldCoord& coord);

    void ConveyorDisconnectLeft(WorldData& world, const WorldCoord& coord);
} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_LOGIC_CONVEYOR_CONNECTION_H
