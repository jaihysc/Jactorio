// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_LOGIC_CONVEYOR_CONNECTION_H
#define JACTORIO_INCLUDE_GAME_LOGIC_CONVEYOR_CONNECTION_H
#pragma once

#include <functional>

#include "core/data_type.h"

namespace jactorio::game
{
    class WorldData;
    class ConveyorStruct;

    using OnConnectCallback = std::function<void(ConveyorStruct& from, ConveyorStruct& to)>;

    ///
    /// Calls Connect for up, right, down, left
    /// \param coord Current struct's coordinate
    void ConveyorConnect(WorldData& world, const WorldCoord& coord);

    ///
    /// Calls Disconnect for up, right, down, left
    /// \param coord Current struct's coordinate
    void ConveyorDisconnect(WorldData& world, const WorldCoord& coord);


    ///
    /// Attempts to connect to a connectable conveyor struct above or connect struct above to current
    /// \param coord Current struct's coordinate
    void ConveyorConnectUp(
        WorldData& world,
        const WorldCoord& coord,
        const OnConnectCallback& callback = [](auto& /*from*/, auto& /*to*/) {});

    ///
    /// Attempts to disconnect from connected conveyor struct above
    /// \param coord Current struct's coordinate
    void ConveyorDisconnectUp(WorldData& world, const WorldCoord& coord);

    void ConveyorConnectRight(WorldData& world, const WorldCoord& coord);
    void ConveyorDisconnectRight(WorldData& world, const WorldCoord& coord);

    void ConveyorConnectDown(WorldData& world, const WorldCoord& coord);
    void ConveyorDisconnectDown(WorldData& world, const WorldCoord& coord);

    void ConveyorConnectLeft(WorldData& world, const WorldCoord& coord);
    void ConveyorDisconnectLeft(WorldData& world, const WorldCoord& coord);
} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_LOGIC_CONVEYOR_CONNECTION_H
