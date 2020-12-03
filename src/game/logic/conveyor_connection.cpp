// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/logic/conveyor_connection.h"

#include "game/logic/conveyor_struct.h"
#include "game/world/world_data.h"
#include "proto/abstract/conveyor.h"

using namespace jactorio;

///
/// Fetches struct at coord, nullptr if non existent
static game::ConveyorStruct* GetStruct(game::WorldData& world, const WorldCoord& coord) {
    // TODO incomplete
    auto* tile = world.GetTile(coord);

    auto& layer = tile->GetLayer(game::TileLayer::entity);

    const auto* proto = layer.GetPrototypeData();

    switch (proto->category) {
    case proto::Category::transport_belt:
        break;

    default:
        break;
    }

    return layer.GetUniqueData<proto::ConveyorData>()->structure.get();
}

///
/// Determines origin and neighbor's targets
/// \tparam OriginConnect Origin orientation required for origin to connect to neighbor
/// \tparam NeighborConnect Neighbor orientation required for neighbor to connect to origin
template <proto::Orientation OriginConnect, proto::Orientation NeighborConnect>
static void CalculateTargets(game::ConveyorStruct& origin, game::ConveyorStruct& neighbor) {
    // Do not attempt to connect to itself
    // if (&origin == &neighbor)
    //     return;

    const bool origin_can_connect   = origin.direction == OriginConnect;
    const bool neighbor_can_connect = neighbor.direction == NeighborConnect;

    // Only 1 struct can be a target (does not both target each other)
    // Either origin feeds into neighbor, or neighbor feeds into origin
    if (origin_can_connect == neighbor_can_connect)
        return;


    auto connect_segment = [](game::ConveyorStruct& from, game::ConveyorStruct& to) {
        from.target = &to;

        // from.targetInsertOffset = to.lineSegmentIndex; // TODO I need an index!
        to.GetOffsetAbs(from.targetInsertOffset);
    };

    if (origin_can_connect)
        connect_segment(origin, neighbor);
    else
        connect_segment(neighbor, origin);
}

void game::ConnectUp(WorldData& world, const WorldCoord& coord) {
    auto* current_struct = GetStruct(world, {coord.x, coord.y});
    auto* neigh_struct   = GetStruct(world, {coord.x, coord.y - 1});

    // TODO incomplete

    CalculateTargets<proto::Orientation::up, proto::Orientation::down>(*current_struct, *neigh_struct);
}
