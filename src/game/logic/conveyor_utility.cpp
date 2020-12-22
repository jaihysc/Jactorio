// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/logic/conveyor_utility.h"

#include "game/logic/conveyor_struct.h"
#include "game/world/world_data.h"
#include "proto/abstract/conveyor.h"

using namespace jactorio;

J_NODISCARD proto::ConveyorData* game::GetConData(WorldData& world, const WorldCoord& coord) {
    auto* tile = world.GetTile(coord);
    if (tile == nullptr)
        return nullptr;

    auto& layer = tile->GetLayer(TileLayer::entity);

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


///
/// Disconnects segments and adjusts segment termination type accordingly
void DisconnectSegment(game::WorldData& world, const WorldCoord& origin_coord, const WorldCoord& neighbor_coord) {
    auto* origin_data   = GetConData(world, origin_coord);
    auto* neighbor_data = GetConData(world, neighbor_coord);

    assert(origin_data != nullptr);

    if (neighbor_data == nullptr)
        return;

    // Neighbor target current, must adjust termination type of neighbor
    if (neighbor_data->structure->target == origin_data->structure.get()) {
        auto& neighbor_struct = *neighbor_data->structure;

        neighbor_struct.target = nullptr;


        switch (neighbor_struct.terminationType) {

            // Convert bend to straight
        case game::ConveyorStruct::TerminationType::bend_left:
        case game::ConveyorStruct::TerminationType::bend_right:
        case game::ConveyorStruct::TerminationType::right_only:
        case game::ConveyorStruct::TerminationType::left_only:

            ConveyorShortenFront(neighbor_struct);
            neighbor_struct.terminationType = game::ConveyorStruct::TerminationType::straight;

            // Renumber tiles following head of neighboring segment from index 0, formerly 1
            ConveyorRenumber(world, neighbor_coord);
            break;

        default:
            // Does not bend
            break;
        }
    }
}

void game::ConveyorDisconnect(WorldData& world, const WorldCoord& coord) {
    ConveyorDisconnectUp(world, coord);
    ConveyorDisconnectRight(world, coord);
    ConveyorDisconnectDown(world, coord);
    ConveyorDisconnectLeft(world, coord);
}

void game::ConveyorDisconnectUp(WorldData& world, const WorldCoord& coord) {
    DisconnectSegment(world, coord, {coord.x, coord.y - 1});
}

void game::ConveyorDisconnectRight(WorldData& world, const WorldCoord& coord) {
    DisconnectSegment(world, coord, {coord.x + 1, coord.y});
}

void game::ConveyorDisconnectDown(WorldData& world, const WorldCoord& coord) {
    DisconnectSegment(world, coord, {coord.x, coord.y + 1});
}

void game::ConveyorDisconnectLeft(WorldData& world, const WorldCoord& coord) {
    DisconnectSegment(world, coord, {coord.x - 1, coord.y});
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

            ConveyorRenumber(world, coord);
            return;
        }
    }

    // Create new conveyor
    conveyor.structure = std::make_shared<ConveyorStruct>(direction, ConveyorStruct::TerminationType::straight, 1);
    world.LogicRegister(Chunk::LogicGroup::conveyor, coord, TileLayer::entity);
}

void game::ConveyorRemove(WorldData& world, const WorldCoord& coord) {
    // o_ = old
    // n_ = new

    auto* o_line_data = GetConData(world, coord);
    assert(o_line_data != nullptr);

    const auto& o_line_segment = o_line_data->structure;

    // ======================================================================

    // Create new segment at behind cords if not the end of a segment

    auto n_seg_coords = coord;
    OrientationIncrement(o_line_segment->direction, n_seg_coords.x, n_seg_coords.y, -1);

    const auto n_seg_length = o_line_segment->length - o_line_data->structIndex - 1;

    if (n_seg_length > 0) {
        const auto n_segment = std::make_shared<ConveyorStruct>(
            o_line_segment->direction, ConveyorStruct::TerminationType::straight, n_seg_length);

        // -1 to skip tile which was removed
        n_segment->itemOffset = o_line_segment->itemOffset - o_line_data->structIndex - 1;

        world.LogicRegister(Chunk::LogicGroup::conveyor, n_seg_coords, TileLayer::entity);

        // Update trailing segments to use new segment and renumber
        ConveyorChangeStructure(world, n_seg_coords, n_segment);
        ConveyorRenumber(world, n_seg_coords);
    }


    // ======================================================================

    // Remove original conveyor segment from logic updates if removed tile was head of segment
    // If not head, reduce the length of original segment to match new length
    if (o_line_data->structIndex == 0 ||
        // Head of bending segments start at 1
        (o_line_data->structIndex == 1 &&
         o_line_segment->terminationType != ConveyorStruct::TerminationType::straight)) {

        ConveyorLogicRemove(world, coord, *o_line_segment);
    }
    else {
        o_line_segment->length = o_line_data->structIndex;
    }
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

void game::ConveyorRenumber(WorldData& world, WorldCoord coord, const int start_id) {
    auto* con_data = GetConData(world, coord);
    assert(con_data != nullptr);

    for (auto i = start_id; i < con_data->structure->length; ++i) {
        auto* i_line_data = GetConData(world, coord);
        assert(i_line_data != nullptr);

        core::SafeCastAssign(i_line_data->structIndex, i);

        OrientationIncrement(con_data->structure->direction, coord.x, coord.y, -1);
    }
}


///
/// If current tile has structure which has old_con_struct as a target, change to new_con_struct
void ChangeTargetSingle(game::WorldData& world,
                        const WorldCoord& coord,
                        const game::ConveyorStruct& old_con_struct,
                        game::ConveyorStruct& new_con_struct) {
    auto* con_data = GetConData(world, coord);

    if (con_data != nullptr && con_data->structure->target == &old_con_struct) {
        con_data->structure->target = &new_con_struct;
    }
}

///
/// Change structures which has old_con_struct as a target to new_con_struct
/// \param coord Coordinate to begin, incrementing in opposite direction of old_con_struct, old_con_struct.length times
void ChangeTarget(game::WorldData& world,
                  WorldCoord coord,
                  const game::ConveyorStruct& old_con_struct,
                  game::ConveyorStruct& new_con_struct) {

    for (unsigned i = 0; i < old_con_struct.length; ++i) {
        ChangeTargetSingle(world, coord, old_con_struct, new_con_struct);

        OrientationIncrement(old_con_struct.direction, coord.x, coord.y, -1);
    }
}

void game::ConveyorChangeStructure(WorldData& world,
                                   WorldCoord coord,
                                   const std::shared_ptr<ConveyorStruct>& con_struct_p) {

    // Update targets of neighbor structures which has old structure as a target

    auto* head_con_data = GetConData(world, coord);
    assert(head_con_data != nullptr);

    switch (con_struct_p->direction) {

    case proto::Orientation::up:
    case proto::Orientation::down:
        ChangeTarget(world, {coord.x - 1, coord.y}, *head_con_data->structure, *con_struct_p);
        ChangeTarget(world, {coord.x + 1, coord.y}, *head_con_data->structure, *con_struct_p);
        break;

    case proto::Orientation::right:
    case proto::Orientation::left:
        ChangeTarget(world, {coord.x, coord.y - 1}, *head_con_data->structure, *con_struct_p);
        ChangeTarget(world, {coord.x, coord.y + 1}, *head_con_data->structure, *con_struct_p);
        break;

    default:
        assert(false);
    }

    // Update the tile after the end of the NEW segment as they may be another con struct there
    {
        WorldCoord past_end_coord = coord;
        OrientationIncrement(con_struct_p->direction, past_end_coord.x, past_end_coord.y, con_struct_p->length * -1);

        ChangeTargetSingle(world, past_end_coord, *head_con_data->structure, *con_struct_p);
    }


    // Change structure
    for (unsigned i = 0; i < con_struct_p->length; ++i) {
        auto* i_con_data = GetConData(world, coord);
        assert(i_con_data != nullptr);


        // New segment should be in the same direction as the old segments
        // It does not make sense to have a conveyor segment pointed up with
        // its internal structure believing it moves right

        assert(i_con_data->structure->direction == con_struct_p->direction);

        i_con_data->structure = con_struct_p;

        OrientationIncrement(con_struct_p->direction, coord.x, coord.y, -1);
    }
}
