// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_LOGIC_CONVEYOR_UTILITY_H
#define JACTORIO_INCLUDE_GAME_LOGIC_CONVEYOR_UTILITY_H
#pragma once

#include <memory>
#include <vector>

#include "jactorio.h"

#include "core/data_type.h"
#include "core/orientation.h"
#include "game/world/logic_group.h"
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
    class World;
    class ChunkTileLayer;
    class ConveyorStruct;

    ///
    /// Processes all steps for cleanly building a conveyor
    ///
    /// 1. Create conveyor struct for provided conveyor
    /// 2. Set conveyor line orientation
    /// 3. Connect neighbor conveyor connections
    /// 4. Update neighbor termination type
    /// 5. Update neighbor line orientation
    /// \remark Additional neighbors must be updated via ConveyorUpdateNeighborTermination in OnNeighborUpdate
    /// \param logic_group Logic group of conveyor
    void BuildConveyor(World& world,
                       const WorldCoord& coord,
                       proto::ConveyorData& conveyor,
                       Orientation direction,
                       LogicGroup logic_group);

    ///
    /// Processes all steps for cleanly removing a conveyor
    ///
    /// 1. Destroy conveyor structure
    /// 2. Disconnect neighbors
    /// 3. Update neighbor line orientation
    /// \remark Additional neighbors must be updated via ConveyorUpdateNeighborTermination in OnNeighborUpdate
    /// \param logic_group Logic group of conveyor
    void RemoveConveyor(World& world, const WorldCoord& coord, LogicGroup logic_group);


    ///
    /// Fetches conveyor data at coord, nullptr if non existent
    J_NODISCARD proto::ConveyorData* GetConData(World& world, const WorldCoord& coord);
    J_NODISCARD const proto::ConveyorData* GetConData(const World& world, const WorldCoord& coord);

    ///
    /// Fetches conveyor data at tile layer, nullptr if non existent
    J_NODISCARD proto::ConveyorData* GetConData(ChunkTileLayer& ctl);
    J_NODISCARD const proto::ConveyorData* GetConData(const ChunkTileLayer& ctl);

    ///
    /// Calls ConveyorConnect up, right, down, left
    /// \param coord Current struct's coordinate
    void ConveyorNeighborConnect(World& world, const WorldCoord& coord);

    ///
    /// Attempts to connect between a connectable conveyor segment above
    /// \param coord Current conveyor struct's coordinate
    void ConveyorConnectUp(World& world, const WorldCoord& coord);

    ///
    /// Attempts to connect between a connectable conveyor segment to right
    /// \param coord Current conveyor struct's coordinate
    void ConveyorConnectRight(World& world, const WorldCoord& coord);

    ///
    /// Attempts to connect between a connectable conveyor segment below
    /// \param coord Current conveyor struct's coordinate
    void ConveyorConnectDown(World& world, const WorldCoord& coord);

    ///
    /// Attempts to connect between a connectable conveyor segment to left
    /// \param coord Current conveyor struct's coordinate
    void ConveyorConnectLeft(World& world, const WorldCoord& coord);


    ///
    /// Calls ConveyorDisconnect up, right, down, left
    /// \param coord Current struct's coordinate
    void ConveyorNeighborDisconnect(World& world, const WorldCoord& coord);

    ///
    /// Attempts to disconnect conveyor segment above
    /// \param coord Current struct's coordinate
    void ConveyorDisconnectUp(World& world, const WorldCoord& coord);

    ///
    /// Attempts to disconnect conveyor segment to right
    /// \param coord Current struct's coordinate
    void ConveyorDisconnectRight(World& world, const WorldCoord& coord);

    ///
    /// Attempts to disconnect conveyor segment below
    /// \param coord Current struct's coordinate
    void ConveyorDisconnectDown(World& world, const WorldCoord& coord);

    ///
    /// Attempts to disconnect conveyor segment to left
    /// \param coord Current struct's coordinate
    void ConveyorDisconnectLeft(World& world, const WorldCoord& coord);


    ///
    /// Creates conveyor structure for provided conveyor
    ///
    /// The conveyor structure can either be grouped with the conveyor structure ahead, behind,
    /// or a new conveyor structure created
    /// \param direction Direction of conveyor
    /// \param logic_group Logic group of conveyor
    void ConveyorCreate(World& world,
                        const WorldCoord& coord,
                        proto::ConveyorData& conveyor,
                        Orientation direction,
                        LogicGroup logic_group);

    ///
    /// Destroys conveyor structure at provided coordinates
    ///
    /// Will ungroup the segment as necessary
    /// \param logic_group Logic group of conveyor
    void ConveyorDestroy(World& world, const WorldCoord& coord, LogicGroup logic_group);


    ///
    /// Conveyor grows 1 tile longer in front of the current head
    void ConveyorLengthenFront(ConveyorStruct& con_struct);

    ///
    /// Conveyor shrinks 1 tile shorter, the tile after the current head becomes the head
    void ConveyorShortenFront(ConveyorStruct& con_struct);

    ///
    /// Removes conveyor at coord to be considered for logic updates
    /// \param logic_group Logic group of conveyor
    void ConveyorLogicRemove(World& world, const WorldCoord& coord, ConveyorStruct& con_struct, LogicGroup logic_group);

    ///
    /// Renumbers structIndex for tiles along a conveyor segment at provided coord
    /// \param start_index Index to start renumbering at, renumber stops when index >= con_struct length
    void ConveyorRenumber(World& world, WorldCoord coord, int start_index = 0);

    ///
    /// Changes the conveyor structure for a conveyor segment, updates neighboring structure's targets
    /// if they used the old conveyor structure.
    /// Updates tiles equal to provided con struct's length
    /// \param con_struct_p Structure to change to
    void ConveyorChangeStructure(World& world, WorldCoord coord, const std::shared_ptr<ConveyorStruct>& con_struct_p);

    ///
    /// Calculates line orientation for conveyor data at coord with provided direction
    ///
    /// Direction is provided separately to allow use when there is no conveyor data at coord
    /// A tile is deemed to not have a conveyor if its structure is nullptr
    J_NODISCARD proto::LineOrientation ConveyorCalcLineOrien(const World& world,
                                                             const WorldCoord& coord,
                                                             Orientation direction);
    ///
    /// Updates line orientation for 4 neighbors of coord
    /// See ConveyorCalcLineOrien
    void ConveyorUpdateNeighborLineOrien(World& world, const WorldCoord& coord);

    ///
    /// Updates the termination type for 4 neighbors of coord
    void ConveyorUpdateNeighborTermination(World& world, const WorldCoord& coord);

} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_LOGIC_CONVEYOR_UTILITY_H
