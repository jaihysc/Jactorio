// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_LOGIC_CONVEYOR_UTILITY_H
#define JACTORIO_INCLUDE_GAME_LOGIC_CONVEYOR_UTILITY_H
#pragma once

#include <memory>

#include "core/data_type.h"

namespace jactorio
{
    namespace proto
    {
        enum class Orientation;
        struct ConveyorData;
    } // namespace proto
} // namespace jactorio

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
    /// Conveyor grows 1 tile longer in front of the current head
    void ConveyorLengthenFront(ConveyorStruct& con_struct);

    ///
    /// Conveyor shrinks 1 tile shorter, the tile after the current head becomes the head
    void ConveyorShortenFront(ConveyorStruct& con_struct);

    ///
    /// Removes conveyor to be considered for logic updates
    void ConveyorLogicRemove(WorldData& world_data, const WorldCoord& world_coords, ConveyorStruct& con_struct);

    // TODO split this into smaller functions
    ///
    /// Updates the tiles along a conveyor segment, props: line_segment_index, line_segment
    /// \param world_coords Beginning tile to update
    /// \param con_struct_p Beginning segment, traveling inverse Orientation line_segment.length tiles,
    /// all tiles set to reference this
    /// \param offset Offsets segment id numbering, world_coords must be also adjusted to the appropriate offset when
    /// calling
    void UpdateSegmentTiles(WorldData& world_data,
                            const WorldCoord& world_coords,
                            const std::shared_ptr<ConveyorStruct>& con_struct_p,
                            int offset = 0);
} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_LOGIC_CONVEYOR_UTILITY_H
