// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/logic/conveyor_connection.h"

#include "game/logic/conveyor_struct.h"
#include "game/world/world_data.h"
#include "proto/abstract/conveyor.h"

using namespace jactorio;

///
/// Fetches struct at coord, nullptr if non existent
static proto::ConveyorData* GetStruct(game::WorldData& world, const WorldCoord& coord) {
    auto* tile = world.GetTile(coord);
    assert(tile != nullptr);

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
    auto* current_struct = GetStruct(world, {coord.x, coord.y});
    auto* neigh_struct   = GetStruct(world, {coord.x + XOffset, coord.y + YOffset});

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
