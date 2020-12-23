// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_LOGIC_CONVEYOR_UTILITY_H
#define JACTORIO_INCLUDE_GAME_LOGIC_CONVEYOR_UTILITY_H
#pragma once

#include <memory>

#include "jactorio.h"

#include "core/data_type.h"
#include "proto/detail/type.h"

namespace jactorio
{
    namespace proto
    {
        struct ConveyorData;
    } // namespace proto
} // namespace jactorio

namespace jactorio::game
{
    class WorldData;
    class ConveyorStruct;

    ///
    /// Fetches conveyor data at coord, nullptr if non existent
    J_NODISCARD proto::ConveyorData* GetConData(WorldData& world, const WorldCoord& coord);
    J_NODISCARD const proto::ConveyorData* GetConData(const WorldData& world, const WorldCoord& coord);

    ///
    /// Calls ConveyorConnect up, right, down, left
    /// \param coord Current struct's coordinate
    void ConveyorConnect(WorldData& world, const WorldCoord& coord);

    ///
    /// Attempts to connect between a connectable conveyor segment above
    /// \param coord Current conveyor struct's coordinate
    void ConveyorConnectUp(WorldData& world, const WorldCoord& coord);

    ///
    /// Attempts to connect between a connectable conveyor segment to right
    /// \param coord Current conveyor struct's coordinate
    void ConveyorConnectRight(WorldData& world, const WorldCoord& coord);

    ///
    /// Attempts to connect between a connectable conveyor segment below
    /// \param coord Current conveyor struct's coordinate
    void ConveyorConnectDown(WorldData& world, const WorldCoord& coord);

    ///
    /// Attempts to connect between a connectable conveyor segment to left
    /// \param coord Current conveyor struct's coordinate
    void ConveyorConnectLeft(WorldData& world, const WorldCoord& coord);


    ///
    /// Calls ConveyorDisconnect up, right, down, left
    /// \param coord Current struct's coordinate
    void ConveyorDisconnect(WorldData& world, const WorldCoord& coord);

    ///
    /// Attempts to disconnect conveyor segment above
    /// \param coord Current struct's coordinate
    void ConveyorDisconnectUp(WorldData& world, const WorldCoord& coord);

    ///
    /// Attempts to disconnect conveyor segment to right
    /// \param coord Current struct's coordinate
    void ConveyorDisconnectRight(WorldData& world, const WorldCoord& coord);

    ///
    /// Attempts to disconnect conveyor segment below
    /// \param coord Current struct's coordinate
    void ConveyorDisconnectDown(WorldData& world, const WorldCoord& coord);

    ///
    /// Attempts to disconnect conveyor segment to left
    /// \param coord Current struct's coordinate
    void ConveyorDisconnectLeft(WorldData& world, const WorldCoord& coord);


    ///
    /// Creates conveyor structure for provided conveyor
    ///
    /// The conveyor structure can either be grouped with the conveyor structure ahead, behind,
    /// or a new conveyor structure created
    /// \param direction Direction of conveyor
    void ConveyorCreate(WorldData& world,
                        const WorldCoord& coord,
                        proto::ConveyorData& conveyor,
                        proto::Orientation direction);

    ///
    /// Removes conveyor structure at provided coordinates
    ///
    /// Will ungroup the segment as necessary
    void ConveyorRemove(WorldData& world, const WorldCoord& coord);


    ///
    /// Conveyor grows 1 tile longer in front of the current head
    void ConveyorLengthenFront(ConveyorStruct& con_struct);

    ///
    /// Conveyor shrinks 1 tile shorter, the tile after the current head becomes the head
    void ConveyorShortenFront(ConveyorStruct& con_struct);

    ///
    /// Removes conveyor to be considered for logic updates
    void ConveyorLogicRemove(WorldData& world_data, const WorldCoord& world_coords, ConveyorStruct& con_struct);

    ///
    /// Renumbers structIndex for tiles along a conveyor segment at provided coords
    /// \param start_id Id for current coordinate, renumber stops when start_id + 1 >= con_struct length
    void ConveyorRenumber(WorldData& world, WorldCoord coord, int start_id = 0);

    ///
    /// Changes the conveyor structure for a conveyor segment, updates neighboring structure's targets
    /// if they used the old conveyor structure.
    /// Updates tiles equal to provided con struct's length
    /// \param con_struct_p Structure to change to
    void ConveyorChangeStructure(WorldData& world,
                                 WorldCoord coord,
                                 const std::shared_ptr<ConveyorStruct>& con_struct_p);

    ///
    /// Calculates line orientation for conveyor data at coord with provided direction
    ///
    /// Direction is provided separately to allow use when there is no conveyor data at coord
    /// A tile is deemed to not have a conveyor if its structure is nullptr
    J_NODISCARD proto::LineOrientation ConveyorCalcLineOrien(const WorldData& world,
                                                             const WorldCoord& coord,
                                                             proto::Orientation direction);
    ///
    /// Updates line orientation for 4 neighbors of coord
    /// See ConveyorCalcLineOrien
    void ConveyorUpdateNeighborLineOrien(WorldData& world, const WorldCoord& coord);
} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_LOGIC_CONVEYOR_UTILITY_H
