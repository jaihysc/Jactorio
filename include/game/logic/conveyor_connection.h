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
    void Connect(WorldData& world, const WorldCoord& coord);

    ///
    /// Calls Disconnect for up, right, down, left
    /// \param coord Current struct's coordinate
    void Disconnect(WorldData& world, const WorldCoord& coord);


    ///
    /// Attempts to connect to a connectable struct above or connect struct above to current
    /// \param coord Current struct's coordinate
    void ConnectUp(
        WorldData& world,
        const WorldCoord& coord,
        const OnConnectCallback& callback = [](auto& /*from*/, auto& /*to*/) {});

    ///
    /// Attempts to disconnect from connected struct above
    /// \param coord Current struct's coordinate
    void DisconnectUp(WorldData& world, const WorldCoord& coord);

    void ConnectRight(WorldData& world, const WorldCoord& coord);
    void DisconnectRight(WorldData& world, const WorldCoord& coord);

    void ConnectDown(WorldData& world, const WorldCoord& coord);
    void DisconnectDown(WorldData& world, const WorldCoord& coord);

    void ConnectLeft(WorldData& world, const WorldCoord& coord);
    void DisconnectLeft(WorldData& world, const WorldCoord& coord);
} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_LOGIC_CONVEYOR_CONNECTION_H
