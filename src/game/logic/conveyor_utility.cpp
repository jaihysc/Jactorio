// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/logic/conveyor_utility.h"

#include "game/logic/conveyor_struct.h"
#include "game/world/world_data.h"
#include "proto/abstract/conveyor.h"

using namespace jactorio;

///
/// Fetches conveyor data at coord, nullptr if non existent
static proto::ConveyorData* GetConData(game::WorldData& world, const WorldCoord& coord) {
    auto* tile = world.GetTile(coord);
    if (tile == nullptr)
        return nullptr;

    auto& layer = tile->GetLayer(game::TileLayer::entity);

    const auto* proto = layer.GetPrototypeData();
    if (proto == nullptr)
        return nullptr;

    switch (proto->GetCategory()) {
    case proto::Category::transport_belt:
        return layer.GetUniqueData<proto::ConveyorData>();

    default:
        return nullptr;
    }
}

///
/// Determines origin and neighbor's targets
/// \tparam OriginConnect Origin orientation required for origin to connect to neighbor
/// \tparam NeighborConnect Neighbor orientation required for neighbor to connect to origin
template <proto::Orientation OriginConnect, proto::Orientation NeighborConnect>
static void CalculateTargets(proto::ConveyorData& origin, proto::ConveyorData& neighbor) {
    assert(origin.structure != nullptr);
    assert(neighbor.structure != nullptr);

    auto& origin_struct   = *origin.structure;
    auto& neighbor_struct = *neighbor.structure;

    // Do not connect to itself
    if (&origin_struct == &neighbor_struct)
        return;

    const bool origin_can_connect   = origin_struct.direction == OriginConnect;
    const bool neighbor_can_connect = neighbor_struct.direction == NeighborConnect;

    // Only 1 struct can be a target (does not both target each other)
    // Either origin feeds into neighbor, or neighbor feeds into origin
    if (origin_can_connect == neighbor_can_connect)
        return;


    auto connect_segment = [](game::ConveyorStruct& from, proto::ConveyorData& to) {
        from.target = to.structure.get();

        // Insert at the correct offset for targets spanning > 1 tiles
        from.targetInsertOffset = to.structIndex;
        to.structure->GetOffsetAbs(from.targetInsertOffset);
    };


    if (origin_can_connect)
        connect_segment(origin_struct, neighbor);
    else
        connect_segment(neighbor_struct, origin);
}

///
/// Connection logic for origin and neighbor
/// \tparam OriginConnect Origin orientation required for origin to connect to neighbor
/// \tparam XOffset Offset applied to origin to get neighbor
/// \tparam YOffset Offset applied to origin to get neighbor
template <proto::Orientation OriginConnect, int XOffset, int YOffset>
static void DoConnect(game::WorldData& world, const WorldCoord& coord) {
    auto* current_struct = GetConData(world, {coord.x, coord.y});
    auto* neigh_struct   = GetConData(world, {coord.x + XOffset, coord.y + YOffset});

    if (current_struct == nullptr || neigh_struct == nullptr)
        return;

    CalculateTargets<OriginConnect, InvertOrientation(OriginConnect)>(*current_struct, *neigh_struct);
}


void game::ConveyorConnect(WorldData& world, const WorldCoord& coord) {
    ConveyorConnectUp(world, coord);
    ConveyorConnectRight(world, coord);
    ConveyorConnectDown(world, coord);
    ConveyorConnectLeft(world, coord);
}

void game::ConveyorConnectUp(WorldData& world, const WorldCoord& coord) {
    DoConnect<proto::Orientation::up, 0, -1>(world, coord);
}

void game::ConveyorConnectRight(WorldData& world, const WorldCoord& coord) {
    DoConnect<proto::Orientation::right, 1, 0>(world, coord);
}

void game::ConveyorConnectDown(WorldData& world, const WorldCoord& coord) {
    DoConnect<proto::Orientation::down, 0, 1>(world, coord);
}

void game::ConveyorConnectLeft(WorldData& world, const WorldCoord& coord) {
    DoConnect<proto::Orientation::left, -1, 0>(world, coord);
}


void game::ConveyorCreate(WorldData& world,
                          const WorldCoord& coord,
                          proto::ConveyorData& conveyor,
                          const proto::Orientation direction) {

    /*
     * Conveyor grouping rules:
     *
     * < < < [1, 2, 3] - Direction [order];
     * Line ahead:
     *		- Extends length of conveyor segment
     *
     * < < < [3, 2, 1]
     * Line behind:
     *		- Moves head of conveyor segment, shift leading item 1 tile back
     *
     * < < < [1, 3, 2]
     * Line ahead and behind:
     *		- Behaves as line ahead
     */

    auto& origin_chunk = *world.GetChunkW(coord);

    auto get_ahead = [&world, direction, &origin_chunk](WorldCoord current_coord) -> proto::ConveyorData* {
        OrientationIncrement(direction, current_coord.x, current_coord.y, 1);

        // Grouping only allowed within the same chunk to guarantee a conveyor will be rendered
        if (&origin_chunk != world.GetChunkW(current_coord)) {
            return nullptr;
        }

        return GetConData(world, current_coord);
    };

    auto get_behind = [&world, direction, &origin_chunk](WorldCoord current_coord) -> proto::ConveyorData* {
        OrientationIncrement(direction, current_coord.x, current_coord.y, -1);

        if (&origin_chunk != world.GetChunkW(current_coord)) {
            return nullptr;
        }

        return GetConData(world, current_coord);
    };

    // Group ahead

    auto* con_ahead = get_ahead(coord);

    if (con_ahead != nullptr) {
        assert(con_ahead->structure != nullptr);

        auto& con_ahead_struct = *con_ahead->structure;
        if (con_ahead_struct.direction == direction) {
            conveyor.structure = con_ahead->structure;

            con_ahead_struct.length++;
            conveyor.structIndex = con_ahead->structIndex + 1;
            return;
        }
    }


    // Group behind

    auto* con_behind = get_behind(coord);

    if (con_behind != nullptr) {
        assert(con_behind->structure != nullptr);

        auto& con_behind_struct = *con_behind->structure;
        if (con_behind_struct.direction == direction) {
            conveyor.structure = con_behind->structure;

            // Move the conveyor behind's head forwards to current position
            ConveyorLengthenFront(con_behind_struct);

            // Remove old head from logic group, add new head which is now 1 tile ahead
            ConveyorLogicRemove(world, coord, con_behind_struct);
            world.LogicRegister(Chunk::LogicGroup::conveyor, coord, TileLayer::entity);

            UpdateSegmentTiles(world, coord, conveyor.structure);
            return;
        }
    }

    // Create new conveyor
    conveyor.structure = std::make_shared<ConveyorStruct>(direction, ConveyorStruct::TerminationType::straight, 1);
    world.LogicRegister(Chunk::LogicGroup::conveyor, coord, TileLayer::entity);
}

void game::ConveyorLengthenFront(ConveyorStruct& con_struct) {
    con_struct.length++;
    con_struct.itemOffset++;
}

void game::ConveyorShortenFront(ConveyorStruct& con_struct) {
    con_struct.length--;
    con_struct.itemOffset--;
}

void game::ConveyorLogicRemove(WorldData& world_data, const WorldCoord& world_coords, ConveyorStruct& con_struct) {
    world_data.LogicRemove(Chunk::LogicGroup::conveyor, world_coords, [&](auto* t_layer) {
        auto* line_data = t_layer->template GetUniqueData<proto::ConveyorData>();
        return line_data->structure.get() == &con_struct;
    });
}

void game::UpdateSegmentTiles(WorldData& world_data,
                              const WorldCoord& world_coords,
                              const std::shared_ptr<ConveyorStruct>& con_struct_p,
                              const int offset) {
    using OffsetT = ChunkCoordAxis;

    OffsetT x_offset = 0;
    OffsetT y_offset = 0;

    // Should be -1, 0, 1 depending on orientation
    OffsetT x_change = 0;
    OffsetT y_change = 0;
    OrientationIncrement(con_struct_p->direction, x_change, y_change, -1);

    // Adjust the segment index number of all following segments
    for (auto i = offset; i < con_struct_p->length; ++i) {
        auto* i_line_data =
            proto::Conveyor::GetLineData(world_data, world_coords.x + x_offset, world_coords.y + y_offset);

        core::SafeCastAssign(i_line_data->structIndex, i);
        i_line_data->structure = con_struct_p;

        x_offset += x_change;
        y_offset += y_change;
    }
}
