// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/logic/conveyor_utility.h"

#include "game/logic/conveyor_struct.h"
#include "game/world/world.h"
#include "proto/abstract/conveyor.h"
#include "proto/splitter.h"
#include "proto/sprite.h"

using namespace jactorio;

void game::BuildConveyor(World& world,
                         const WorldCoord& coord,
                         proto::ConveyorData& conveyor,
                         const Orientation direction,
                         const LogicGroup logic_group) {

    ConveyorCreate(world, coord, conveyor, direction, logic_group);

    ConveyorNeighborConnect(world, coord);
    ConveyorUpdateNeighborTermination(world, coord);
    ConveyorUpdateNeighborLineOrien(world, coord);
}

void game::RemoveConveyor(World& world, const WorldCoord& coord, const LogicGroup logic_group) {
    ConveyorNeighborDisconnect(world, coord);
    ConveyorDestroy(world, coord, logic_group);
    ConveyorUpdateNeighborLineOrien(world, coord);
}


std::pair<const proto::Conveyor*, proto::ConveyorData*> game::GetConveyorInfo(World& world, const WorldCoord& coord) {
    auto [proto, data] = GetConveyorInfo(static_cast<const World&>(world), coord);
    return {proto, const_cast<proto::ConveyorData*>(data)};
}

std::pair<const proto::Conveyor*, const proto::ConveyorData*> game::GetConveyorInfo(const World& world,
                                                                                    const WorldCoord& coord) {
    const auto* tile = world.GetTile(coord, TileLayer::entity);
    if (tile == nullptr)
        return {nullptr, nullptr};

    return GetConveyorInfo(*tile);
}

std::pair<const proto::Conveyor*, proto::ConveyorData*> game::GetConveyorInfo(ChunkTile& tile) {
    auto [proto, data] = GetConveyorInfo(static_cast<const ChunkTile&>(tile));
    return {proto, const_cast<proto::ConveyorData*>(data)};
}

std::pair<const proto::Conveyor*, const proto::ConveyorData*> game::GetConveyorInfo(const ChunkTile& tile) {
    const auto* proto = tile.GetPrototype();
    if (proto == nullptr)
        return {nullptr, nullptr};

    switch (proto->GetCategory()) {
    case proto::Category::transport_belt:
        return {SafeCast<const proto::Conveyor*>(proto), tile.GetUniqueData<proto::ConveyorData>()};

    case proto::Category::splitter:
    {
        const auto* splitter_data = tile.GetUniqueData<proto::SplitterData>();
        assert(splitter_data != nullptr);

        if (splitter_data->orientation == Orientation::up || splitter_data->orientation == Orientation::right) {
            if (tile.IsTopLeft()) {
                return {SafeCast<const proto::Conveyor*>(proto), &splitter_data->left};
            }
            return {SafeCast<const proto::Conveyor*>(proto), &splitter_data->right};
        }

        // Down or left
        if (tile.IsTopLeft()) {
            return {SafeCast<const proto::Conveyor*>(proto), &splitter_data->right};
        }
        return {SafeCast<const proto::Conveyor*>(proto), &splitter_data->left};
    }

    default:
        return {nullptr, nullptr};
    }
}

/// Determines origin and neighbor's targets
/// \tparam OriginConnect Origin orientation required for origin to connect to neighbor
/// \tparam NeighborConnect Neighbor orientation required for neighbor to connect to origin
template <Direction OriginConnect, Direction NeighborConnect>
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
        from.sideInsertIndex = to.structIndex;
        to.structure->GetOffsetAbs(from.sideInsertIndex);
    };


    if (origin_can_connect)
        connect_segment(origin_struct, neighbor);
    else
        connect_segment(neighbor_struct, origin);
}

/// Connection logic for origin and neighbor
/// \tparam OriginConnect Origin orientation required for origin to connect to neighbor
/// \tparam XOffset Offset applied to origin to get neighbor
/// \tparam YOffset Offset applied to origin to get neighbor
template <Direction OriginConnect, int XOffset, int YOffset>
static void DoConnect(game::World& world, const WorldCoord& coord) {
    auto [current_proto, current_data] = GetConveyorInfo(world, {coord.x, coord.y});
    auto [neigh_proto, neigh_data]     = GetConveyorInfo(world, {coord.x + XOffset, coord.y + YOffset});

    // Multi-tile neighbors may not have structures while processing removes for all its tiles
    if (current_data == nullptr || neigh_data == nullptr || neigh_data->structure == nullptr)
        return;

    CalculateTargets<OriginConnect, Orientation::Invert(OriginConnect)>(*current_data, *neigh_data);
}


void game::ConveyorNeighborConnect(World& world, const WorldCoord& coord) {
    ConveyorConnectUp(world, coord);
    ConveyorConnectRight(world, coord);
    ConveyorConnectDown(world, coord);
    ConveyorConnectLeft(world, coord);
}

void game::ConveyorConnectUp(World& world, const WorldCoord& coord) {
    DoConnect<Orientation::up, 0, -1>(world, coord);
}

void game::ConveyorConnectRight(World& world, const WorldCoord& coord) {
    DoConnect<Orientation::right, 1, 0>(world, coord);
}

void game::ConveyorConnectDown(World& world, const WorldCoord& coord) {
    DoConnect<Orientation::down, 0, 1>(world, coord);
}

void game::ConveyorConnectLeft(World& world, const WorldCoord& coord) {
    DoConnect<Orientation::left, -1, 0>(world, coord);
}


/// Disconnects segments and adjusts segment termination type accordingly
template <int XOffset, int YOffset>
void DisconnectSegment(game::World& world, const WorldCoord& coord) {
    const auto neighbor_coord = WorldCoord(coord.x + XOffset, coord.y + YOffset);

    auto [origin_proto, origin_data]     = GetConveyorInfo(world, coord);
    auto [neighbor_proto, neighbor_data] = GetConveyorInfo(world, neighbor_coord);

    assert(origin_data != nullptr);

    // Multi-tile neighbors may not have structures while processing removes for all its tiles
    if (neighbor_data == nullptr || neighbor_data->structure == nullptr)
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

void game::ConveyorNeighborDisconnect(World& world, const WorldCoord& coord) {
    ConveyorDisconnectUp(world, coord);
    ConveyorDisconnectRight(world, coord);
    ConveyorDisconnectDown(world, coord);
    ConveyorDisconnectLeft(world, coord);
}

void game::ConveyorDisconnectUp(World& world, const WorldCoord& coord) {
    DisconnectSegment<0, -1>(world, coord);
}

void game::ConveyorDisconnectRight(World& world, const WorldCoord& coord) {
    DisconnectSegment<1, 0>(world, coord);
}

void game::ConveyorDisconnectDown(World& world, const WorldCoord& coord) {
    DisconnectSegment<0, 1>(world, coord);
}

void game::ConveyorDisconnectLeft(World& world, const WorldCoord& coord) {
    DisconnectSegment<-1, 0>(world, coord);
}


void game::ConveyorCreate(World& world,
                          const WorldCoord& coord,
                          proto::ConveyorData& conveyor,
                          const Orientation direction,
                          const LogicGroup logic_group) {

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
        current_coord.Increment(direction);

        // Grouping only allowed within the same chunk to guarantee a conveyor will be rendered
        if (&origin_chunk != world.GetChunkW(current_coord)) {
            return nullptr;
        }

        return GetConveyorInfo(world, current_coord).second;
    };

    auto get_behind = [&world, direction, &origin_chunk](WorldCoord current_coord) -> proto::ConveyorData* {
        current_coord.Increment(direction, -1);

        if (&origin_chunk != world.GetChunkW(current_coord)) {
            return nullptr;
        }

        return GetConveyorInfo(world, current_coord).second;
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
            world.LogicRemove(logic_group, coord.Incremented(con_behind_struct.direction, -1), TileLayer::entity);
            world.LogicRegister(logic_group, coord, TileLayer::entity);

            ConveyorRenumber(world, coord);
            return;
        }
    }

    // Create new conveyor
    conveyor.structure = std::make_shared<ConveyorStruct>(direction, ConveyorStruct::TerminationType::straight, 1);
    world.LogicRegister(logic_group, coord, TileLayer::entity);
}

void game::ConveyorDestroy(World& world, const WorldCoord& coord, const LogicGroup logic_group) {
    // o_ = old
    // n_ = new

    auto [o_line_proto, o_line_data] = GetConveyorInfo(world, coord);
    assert(o_line_data != nullptr);

    const auto& o_line_segment = o_line_data->structure;

    // ======================================================================

    // Create new segment at behind cords if not the end of a segment

    const auto n_seg_coord  = coord.Incremented(o_line_segment->direction, -1);
    const auto n_seg_length = o_line_segment->length - o_line_data->structIndex - 1;

    if (n_seg_length > 0) {
        const auto n_segment = std::make_shared<ConveyorStruct>(
            o_line_segment->direction, ConveyorStruct::TerminationType::straight, n_seg_length);

        // -1 to skip tile which was removed
        n_segment->headOffset = o_line_segment->headOffset - o_line_data->structIndex - 1;

        world.LogicRegister(logic_group, n_seg_coord, TileLayer::entity);

        // Update trailing segments to use new segment and renumber
        ConveyorChangeStructure(world, n_seg_coord, n_segment);
        ConveyorRenumber(world, n_seg_coord);
    }


    // ======================================================================

    // Remove original conveyor segment from logic updates if removed tile was head of segment
    // If not head, reduce the length of original segment to match new length
    if (o_line_data->structIndex == 0 ||
        // Head of bending segments start at 1
        (o_line_data->structIndex == 1 &&
         o_line_segment->terminationType != ConveyorStruct::TerminationType::straight)) {

        world.LogicRemove(logic_group, coord, TileLayer::entity);
    }
    else {
        o_line_segment->length = o_line_data->structIndex;
    }

    // Finished ungrouping, now remove the structure
    o_line_data->structure = nullptr;
}


void game::ConveyorLengthenFront(ConveyorStruct& con_struct) {
    con_struct.length++;
    con_struct.headOffset++;
}

void game::ConveyorShortenFront(ConveyorStruct& con_struct) {
    con_struct.length--;
    con_struct.headOffset--;
}

void game::ConveyorRenumber(World& world, WorldCoord coord, const int start_index) {
    auto [proto, con_data] = GetConveyorInfo(world, coord);
    assert(con_data != nullptr);
    assert(con_data->structure.get() != nullptr);
    for (auto i = start_index; i < con_data->structure->length; ++i) {
        auto [i_proto, i_data] = GetConveyorInfo(world, coord);
        assert(i_data != nullptr);

        SafeCastAssign(i_data->structIndex, i);

        coord.Increment(con_data->structure->direction, -1);
    }
}


/// If current tile has structure which has old_con_struct as a target, change to new_con_struct
void ChangeTargetSingle(game::World& world,
                        const WorldCoord& coord,
                        const game::ConveyorStruct& old_con_struct,
                        game::ConveyorStruct& new_con_struct) {
    auto [proto, con_data] = GetConveyorInfo(world, coord);

    if (con_data != nullptr && con_data->structure->target == &old_con_struct) {
        con_data->structure->target = &new_con_struct;
    }
}

/// Change structures which has old_con_struct as a target to new_con_struct
/// \param coord Coordinate to begin, incrementing in opposite direction of old_con_struct, old_con_struct.length times
void ChangeTarget(game::World& world,
                  WorldCoord coord,
                  const game::ConveyorStruct& old_con_struct,
                  game::ConveyorStruct& new_con_struct) {

    for (unsigned i = 0; i < old_con_struct.length; ++i) {
        ChangeTargetSingle(world, coord, old_con_struct, new_con_struct);

        coord.Increment(old_con_struct.direction, -1);
    }
}

void game::ConveyorChangeStructure(World& world,
                                   WorldCoord coord,
                                   const std::shared_ptr<ConveyorStruct>& con_struct_p) {

    // Update targets of neighbor structures which has old structure as a target

    auto [head_proto, head_con_data] = GetConveyorInfo(world, coord);
    assert(head_con_data != nullptr);

    switch (con_struct_p->direction) {

    case Orientation::up:
    case Orientation::down:
        ChangeTarget(world, {coord.x - 1, coord.y}, *head_con_data->structure, *con_struct_p);
        ChangeTarget(world, {coord.x + 1, coord.y}, *head_con_data->structure, *con_struct_p);
        break;

    case Orientation::right:
    case Orientation::left:
        ChangeTarget(world, {coord.x, coord.y - 1}, *head_con_data->structure, *con_struct_p);
        ChangeTarget(world, {coord.x, coord.y + 1}, *head_con_data->structure, *con_struct_p);
        break;

    default:
        assert(false);
    }

    // Update the tile after the end of the NEW segment as they may be another con struct there
    {
        const auto past_end_coord = coord.Incremented(con_struct_p->direction, con_struct_p->length * -1);

        ChangeTargetSingle(world, past_end_coord, *head_con_data->structure, *con_struct_p);
    }


    // Change structure
    for (unsigned i = 0; i < con_struct_p->length; ++i) {
        auto [i_con_proto, i_con_data] = GetConveyorInfo(world, coord);
        assert(i_con_data != nullptr);


        // New segment should be in the same direction as the old segments
        // It does not make sense to have a conveyor segment pointed up with
        // its internal structure believing it moves right

        assert(i_con_data->structure->direction == con_struct_p->direction);

        i_con_data->structure = con_struct_p;

        coord.Increment(con_struct_p->direction, -1);
    }
}

proto::LineOrientation game::ConveyorCalcLineOrien(const World& world,
                                                   const WorldCoord& coord,
                                                   const Orientation direction) {
    const auto [up_proto, up]       = GetConveyorInfo(world, {coord.x, coord.y - 1});
    const auto [right_proto, right] = GetConveyorInfo(world, {coord.x + 1, coord.y});
    const auto [down_proto, down]   = GetConveyorInfo(world, {coord.x, coord.y + 1});
    const auto [left_proto, left]   = GetConveyorInfo(world, {coord.x - 1, coord.y});

    /// \return true if has neighbor segment and its direction matches provided
    auto neighbor_valid = [](const proto::ConveyorData* conveyor, const Orientation orient) {
        return conveyor != nullptr && conveyor->structure != nullptr && conveyor->structure->direction == orient;
    };

    switch (direction) {
    case Orientation::up:
        if (!neighbor_valid(down, Orientation::up) &&
            neighbor_valid(left, Orientation::right) != neighbor_valid(right, Orientation::left)) {

            if (neighbor_valid(left, Orientation::right))
                return proto::LineOrientation::right_up;
            return proto::LineOrientation::left_up;
        }
        return proto::LineOrientation::up;

    case Orientation::right:
        if (!neighbor_valid(left, Orientation::right) &&
            neighbor_valid(up, Orientation::down) != neighbor_valid(down, Orientation::up)) {

            if (neighbor_valid(up, Orientation::down))
                return proto::LineOrientation::down_right;
            return proto::LineOrientation::up_right;
        }
        return proto::LineOrientation::right;

    case Orientation::down:
        if (!neighbor_valid(up, Orientation::down) &&
            neighbor_valid(left, Orientation::right) != neighbor_valid(right, Orientation::left)) {

            if (neighbor_valid(left, Orientation::right))
                return proto::LineOrientation::right_down;
            return proto::LineOrientation::left_down;
        }
        return proto::LineOrientation::down;

    case Orientation::left:
        if (!neighbor_valid(right, Orientation::left) &&
            neighbor_valid(up, Orientation::down) != neighbor_valid(down, Orientation::up)) {

            if (neighbor_valid(up, Orientation::down))
                return proto::LineOrientation::down_left;
            return proto::LineOrientation::up_left;
        }
        return proto::LineOrientation::left;

    default:
        assert(false);
        return proto::LineOrientation::up;
    }
}

void game::ConveyorUpdateLineOrien(World& world, const WorldCoord& coord) {
    auto* tile = world.GetTile(coord, TileLayer::entity);
    if (tile == nullptr) {
        return;
    }

    auto [proto, con_data] = GetConveyorInfo(*tile);
    // Not conveyor or splitter
    if (proto == nullptr) {
        return;
    }
    // Splitters don't use line orientations
    if (proto->GetCategory() == proto::Category::splitter) {
        return;
    }
    // Multi-tile neighbors may not have structures while processing removes for all its tiles
    assert(con_data != nullptr);
    if (con_data->structure == nullptr) {
        return;
    }

    auto* conveyor = tile->GetPrototype<proto::Conveyor>();
    assert(conveyor->sprite != nullptr);

    auto line_orien = ConveyorCalcLineOrien(world, coord, con_data->structure->direction);
    world.SetTexCoordId(coord, TileLayer::entity, conveyor->sprite->texCoordId + static_cast<int>(line_orien));
}

void game::ConveyorUpdateNeighborLineOrien(World& world, const WorldCoord& coord) {
    ConveyorUpdateLineOrien(world, {coord.x, coord.y - 1});
    ConveyorUpdateLineOrien(world, {coord.x + 1, coord.y});
    ConveyorUpdateLineOrien(world, {coord.x, coord.y + 1});
    ConveyorUpdateLineOrien(world, {coord.x - 1, coord.y});
}


void game::ConveyorUpdateNeighborTermination(World& world, const WorldCoord& coord) {

    /// Changes termination type at coordinates
    auto change_ttype = [&world](const WorldCoord& neighbor_coord, const ConveyorStruct::TerminationType new_ttype) {
        auto [proto, con_data] = GetConveyorInfo(world, neighbor_coord);
        if (con_data != nullptr) {
            assert(con_data->structure != nullptr);

            // If termination type is no longer straight, its length is now +1 and must renumber all its tiles
            // excluding id 0, since that belongs to its target

            if (con_data->structure->terminationType == ConveyorStruct::TerminationType::straight) {
                ConveyorLengthenFront(*con_data->structure);
            }

            con_data->structure->terminationType = new_ttype;
            ConveyorRenumber(world, neighbor_coord, 1);
        }
    };

    /// Changes termination type at coordinates if it matches required_direction
    auto try_change_ttype = [&world, &change_ttype](const WorldCoord& neighbor_coord,
                                                    const Orientation required_direction,
                                                    const ConveyorStruct::TerminationType new_ttype) {
        auto [proto, con_data] = GetConveyorInfo(world, neighbor_coord);

        // Multi-tile neighbors may not have structures while processing removes for all its tiles
        if (con_data == nullptr || con_data->structure == nullptr)
            return;

        if (con_data->structure->direction != required_direction)
            return;

        change_ttype(neighbor_coord, new_ttype);
    };

    auto [proto, con_data] = GetConveyorInfo(world, coord);
    assert(con_data != nullptr);

    switch (ConveyorCalcLineOrien(world, coord, con_data->structure->direction)) {
        // Up
    case proto::LineOrientation::up_left:
        change_ttype({coord.x, coord.y + 1}, ConveyorStruct::TerminationType::bend_left);
        break;
    case proto::LineOrientation::up_right:
        change_ttype({coord.x, coord.y + 1}, ConveyorStruct::TerminationType::bend_right);
        break;

        // Right
    case proto::LineOrientation::right_up:
        change_ttype({coord.x - 1, coord.y}, ConveyorStruct::TerminationType::bend_left);
        break;
    case proto::LineOrientation::right_down:
        change_ttype({coord.x - 1, coord.y}, ConveyorStruct::TerminationType::bend_right);
        break;

        // Down
    case proto::LineOrientation::down_right:
        change_ttype({coord.x, coord.y - 1}, ConveyorStruct::TerminationType::bend_left);
        break;
    case proto::LineOrientation::down_left:
        change_ttype({coord.x, coord.y - 1}, ConveyorStruct::TerminationType::bend_right);
        break;

        // Left
    case proto::LineOrientation::left_down:
        change_ttype({coord.x + 1, coord.y}, ConveyorStruct::TerminationType::bend_left);
        break;
    case proto::LineOrientation::left_up:
        change_ttype({coord.x + 1, coord.y}, ConveyorStruct::TerminationType::bend_right);
        break;


        // Straight (Check for conveyors on both sides to make side only)
    case proto::LineOrientation::up:
        try_change_ttype({coord.x - 1, coord.y}, Orientation::right, ConveyorStruct::TerminationType::left_only);
        try_change_ttype({coord.x + 1, coord.y}, Orientation::left, ConveyorStruct::TerminationType::right_only);
        break;
    case proto::LineOrientation::right:
        try_change_ttype({coord.x, coord.y - 1}, Orientation::down, ConveyorStruct::TerminationType::left_only);
        try_change_ttype({coord.x, coord.y + 1}, Orientation::up, ConveyorStruct::TerminationType::right_only);
        break;
    case proto::LineOrientation::down:
        try_change_ttype({coord.x - 1, coord.y}, Orientation::right, ConveyorStruct::TerminationType::right_only);
        try_change_ttype({coord.x + 1, coord.y}, Orientation::left, ConveyorStruct::TerminationType::left_only);
        break;
    case proto::LineOrientation::left:
        try_change_ttype({coord.x, coord.y - 1}, Orientation::down, ConveyorStruct::TerminationType::right_only);
        try_change_ttype({coord.x, coord.y + 1}, Orientation::up, ConveyorStruct::TerminationType::left_only);
        break;
    }
}
