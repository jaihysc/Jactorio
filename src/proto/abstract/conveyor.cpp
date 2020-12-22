// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "proto/abstract/conveyor.h"

#include <array>
#include <cmath>

#include "game/logic/conveyor_struct.h"
#include "game/logic/conveyor_utility.h"
#include "game/world/world_data.h"
#include "proto/sprite.h"
#include "render/proto_renderer.h"

using namespace jactorio;
using LineData4Way = proto::Conveyor::LineData4Way;


proto::Orientation proto::ConveyorData::ToOrientation(const LineOrientation line_orientation) {
    switch (line_orientation) {
    case LineOrientation::up:
    case LineOrientation::right_up:
    case LineOrientation::left_up:
        return Orientation::up;

    case LineOrientation::right:
    case LineOrientation::up_right:
    case LineOrientation::down_right:
        return Orientation::right;

    case LineOrientation::down:
    case LineOrientation::right_down:
    case LineOrientation::left_down:
        return Orientation::down;

    case LineOrientation::left:
    case LineOrientation::up_left:
    case LineOrientation::down_left:
        return Orientation::left;
    }

    assert(false); // Missing switch case
    return Orientation::up;
}

//
//
//
//
//
//
//
//
//
//
// ======================================================================
// Data access

LineData4Way proto::Conveyor::GetLineData4(game::WorldData& world_data, const WorldCoord& origin_coord) {
    auto* up    = GetConData(world_data, {origin_coord.x, origin_coord.y - 1});
    auto* right = GetConData(world_data, {origin_coord.x + 1, origin_coord.y});
    auto* down  = GetConData(world_data, {origin_coord.x, origin_coord.y + 1});
    auto* left  = GetConData(world_data, {origin_coord.x - 1, origin_coord.y});

    return {up, right, down, left};
}

proto::ConveyorData::LineOrientation proto::Conveyor::GetLineOrientation(const Orientation orientation,
                                                                         const LineData4Way& line_data4) {
    auto* up    = line_data4[0];
    auto* right = line_data4[1];
    auto* down  = line_data4[2];
    auto* left  = line_data4[3];

    /// true if has neighbor line and its orientation matches provided
    auto neighbor_valid = [](const ConveyorData* conveyor, const Orientation orient) {
        return conveyor != nullptr && ConveyorData::ToOrientation(conveyor->lOrien) == orient;
    };

    switch (orientation) {
    case Orientation::up:
        if (!neighbor_valid(down, Orientation::up) &&
            neighbor_valid(left, Orientation::right) != neighbor_valid(right, Orientation::left)) {

            if (neighbor_valid(left, Orientation::right))
                return ConveyorData::LineOrientation::right_up;
            return ConveyorData::LineOrientation::left_up;
        }
        return ConveyorData::LineOrientation::up;

    case Orientation::right:
        if (!neighbor_valid(left, Orientation::right) &&
            neighbor_valid(up, Orientation::down) != neighbor_valid(down, Orientation::up)) {

            if (neighbor_valid(up, Orientation::down))
                return ConveyorData::LineOrientation::down_right;
            return ConveyorData::LineOrientation::up_right;
        }
        return ConveyorData::LineOrientation::right;

    case Orientation::down:
        if (!neighbor_valid(up, Orientation::down) &&
            neighbor_valid(left, Orientation::right) != neighbor_valid(right, Orientation::left)) {

            if (neighbor_valid(left, Orientation::right))
                return ConveyorData::LineOrientation::right_down;
            return ConveyorData::LineOrientation::left_down;
        }
        return ConveyorData::LineOrientation::down;

    case Orientation::left:
        if (!neighbor_valid(right, Orientation::left) &&
            neighbor_valid(up, Orientation::down) != neighbor_valid(down, Orientation::up)) {

            if (neighbor_valid(up, Orientation::down))
                return ConveyorData::LineOrientation::down_left;
            return ConveyorData::LineOrientation::up_left;
        }
        return ConveyorData::LineOrientation::left;
    }

    assert(false); // Missing switch case
    return ConveyorData::LineOrientation::up;
}


// ======================================================================
// Game events


void proto::Conveyor::OnRDrawUniqueData(render::RendererLayer& layer,
                                        const SpriteUvCoordsT& uv_coords,
                                        const core::Position2<float>& pixel_offset,
                                        const UniqueDataBase* unique_data) const {
    const auto& line_data = *static_cast<const ConveyorData*>(unique_data);

    // Only draw for the head of segments
    if (line_data.structure->terminationType == game::ConveyorStruct::TerminationType::straight &&
        line_data.structIndex != 0)
        return;

    if (line_data.structure->terminationType != game::ConveyorStruct::TerminationType::straight &&
        line_data.structIndex != 1)
        return;

    DrawConveyorSegmentItems(layer, uv_coords, pixel_offset, *line_data.structure);
}

SpriteSetT proto::Conveyor::OnRGetSpriteSet(const Orientation orientation,
                                            game::WorldData& world_data,
                                            const WorldCoord& world_coords) const {
    return static_cast<uint16_t>(GetLineOrientation(orientation, GetLineData4(world_data, world_coords)));
}

SpriteFrameT proto::Conveyor::OnRGetSpriteFrame(const UniqueDataBase& /*unique_data*/,
                                                const GameTickT game_tick) const {
    return AllOfSet(*sprite, game_tick);
}


// ======================================================================
// Build / Remove / Neighbor update

///	\brief Calculates the line orientation of  neighboring conveyors
void CalculateNeighborLineOrientation(game::WorldData& world_data,
                                      const WorldCoord& world_coords,
                                      const LineData4Way& line_data_4,
                                      proto::ConveyorData* center) {
    // Building a belt will update all neighboring belts (X), which thus requires tiles (*)
    /*
     *     ---
     *     |*|        Top
     *   -------
     *   |*|x|*|      Top left, top center, top right
     * -----------
     * |*|x|x|x|*|    Left, Center left, Center right, Right
     * -----------
     *   |*|x|*|      Bottom left, bottom center, bottom right
     *   -------
     *     |*|        Bottom
     *     ---
     */
    auto* top    = GetConData(world_data, {world_coords.x, world_coords.y - 2});
    auto* right  = GetConData(world_data, {world_coords.x + 2, world_coords.y});
    auto* bottom = GetConData(world_data, {world_coords.x, world_coords.y + 2});
    auto* left   = GetConData(world_data, {world_coords.x - 2, world_coords.y});

    auto* t_left  = GetConData(world_data, {world_coords.x - 1, world_coords.y - 1});
    auto* t_right = GetConData(world_data, {world_coords.x + 1, world_coords.y - 1});

    auto* b_left  = GetConData(world_data, {world_coords.x - 1, world_coords.y + 1});
    auto* b_right = GetConData(world_data, {world_coords.x + 1, world_coords.y + 1});

    // Top neighbor
    if (line_data_4[0] != nullptr)
        line_data_4[0]->SetOrientation(proto::Conveyor::GetLineOrientation(
            proto::ConveyorData::ToOrientation(line_data_4[0]->lOrien), {top, t_right, center, t_left}));
    // Right
    if (line_data_4[1] != nullptr)
        line_data_4[1]->SetOrientation(proto::Conveyor::GetLineOrientation(
            proto::ConveyorData::ToOrientation(line_data_4[1]->lOrien), {t_right, right, b_right, center}));
    // Bottom
    if (line_data_4[2] != nullptr)
        line_data_4[2]->SetOrientation(proto::Conveyor::GetLineOrientation(
            proto::ConveyorData::ToOrientation(line_data_4[2]->lOrien), {center, b_right, bottom, b_left}));
    // Left
    if (line_data_4[3] != nullptr)
        line_data_4[3]->SetOrientation(proto::Conveyor::GetLineOrientation(
            proto::ConveyorData::ToOrientation(line_data_4[3]->lOrien), {t_left, center, b_left, left}));
}


///
/// Shifts origin segment forwards if neighbor line orientation matches template arguments
template <proto::ConveyorData::LineOrientation BendLeft,
          proto::ConveyorData::LineOrientation BendRight,
          proto::ConveyorData::LineOrientation LeftOnly,
          proto::ConveyorData::LineOrientation RightOnly>
void TryShiftSegment(game::WorldData& world_data,
                     const WorldCoord& origin_coords,
                     const std::shared_ptr<game::ConveyorStruct>& origin_segment,
                     proto::ConveyorData* neighbor_data) {
    if (!neighbor_data)
        return;

    switch (neighbor_data->lOrien) {
    case BendLeft:
    case BendRight:
    case LeftOnly:
    case RightOnly:
        ConveyorLengthenFront(*origin_segment);

        // Id 0 belongs to the other conveyor segment, thus id at this coordinate is 1
        ConveyorRenumber(world_data, origin_coords, 1);
        break;

    default:
        break;
    }
}

///
/// Shifts origin segment forwards depending on its orientation with the appropriate neighbor
void TryShiftSegment(game::WorldData& world_data,
                     const WorldCoord& origin_coords,
                     const std::shared_ptr<game::ConveyorStruct>& origin_segment,
                     LineData4Way& neighbor_data4) {
    switch (origin_segment->direction) {

        // Neighbor's line orientation should have been changed to "dir_dir"
        // because this was called after CalculateNeighborLineOrientation
        // Their actual orientation is the second direction

    case proto::Orientation::up:
        TryShiftSegment<proto::ConveyorData::LineOrientation::up_left,
                        proto::ConveyorData::LineOrientation::up_right,
                        proto::ConveyorData::LineOrientation::right,
                        proto::ConveyorData::LineOrientation::left>(
            world_data, origin_coords, origin_segment, neighbor_data4[0]);
        break;
    case proto::Orientation::right:
        TryShiftSegment<proto::ConveyorData::LineOrientation::right_up,
                        proto::ConveyorData::LineOrientation::right_down,
                        proto::ConveyorData::LineOrientation::down,
                        proto::ConveyorData::LineOrientation::up>(
            world_data, origin_coords, origin_segment, neighbor_data4[1]);
        break;
    case proto::Orientation::down:
        TryShiftSegment<proto::ConveyorData::LineOrientation::down_right,
                        proto::ConveyorData::LineOrientation::down_left,
                        proto::ConveyorData::LineOrientation::left,
                        proto::ConveyorData::LineOrientation::right>(
            world_data, origin_coords, origin_segment, neighbor_data4[2]);
        break;
    case proto::Orientation::left:
        TryShiftSegment<proto::ConveyorData::LineOrientation::left_down,
                        proto::ConveyorData::LineOrientation::left_up,
                        proto::ConveyorData::LineOrientation::up,
                        proto::ConveyorData::LineOrientation::down>(
            world_data, origin_coords, origin_segment, neighbor_data4[3]);
        break;
    }
}

///
/// Changes terminate type of neighbors depending on origin
/// \tparam IsNeighborUpdate If true, length and itemOffset will also be updated
/// \remark This does not move across logic chunks and may make the position negative
template <bool IsNeighborUpdate>
void CalculateNeighborTermination(game::WorldData& world_data,
                                  const WorldCoord& origin_coord,
                                  const proto::ConveyorData::LineOrientation origin_line_orientation) {

    ///
    /// Updates neighbor at coordinates
    auto change_ttype = [&world_data](const WorldCoordAxis w_x,
                                      const WorldCoordAxis w_y,
                                      const game::ConveyorStruct::TerminationType new_ttype) {
        auto* con_data = GetConData(world_data, {w_x, w_y});
        if (con_data != nullptr) {
            // If termination type is no longer straight, it its length is now +1 and must renumber all its tiles
            // excluding id 0, since that belongs to its target

            if constexpr (!IsNeighborUpdate) {
                ConveyorLengthenFront(*con_data->structure);
            }
            con_data->structure->terminationType = new_ttype;
            ConveyorRenumber(world_data, {w_x, w_y}, 1);
        }
    };

    ///
    /// Updates neighbor at coordinates if it matches required_direction
    auto try_change_ttype = [&world_data, &change_ttype](const WorldCoordAxis w_x,
                                                         const WorldCoordAxis w_y,
                                                         const proto::Orientation required_direction,
                                                         const game::ConveyorStruct::TerminationType new_ttype) {
        auto* con_data = GetConData(world_data, {w_x, w_y});

        if (con_data != nullptr) {
            if (con_data->structure->direction != required_direction)
                return;
        }

        change_ttype(w_x, w_y, new_ttype);
    };

    switch (origin_line_orientation) {
        // Up
    case proto::ConveyorData::LineOrientation::up_left:
        change_ttype(origin_coord.x, origin_coord.y + 1, game::ConveyorStruct::TerminationType::bend_left);
        break;
    case proto::ConveyorData::LineOrientation::up_right:
        change_ttype(origin_coord.x, origin_coord.y + 1, game::ConveyorStruct::TerminationType::bend_right);
        break;

        // Right
    case proto::ConveyorData::LineOrientation::right_up:
        change_ttype(origin_coord.x - 1, origin_coord.y, game::ConveyorStruct::TerminationType::bend_left);
        break;
    case proto::ConveyorData::LineOrientation::right_down:
        change_ttype(origin_coord.x - 1, origin_coord.y, game::ConveyorStruct::TerminationType::bend_right);
        break;

        // Down
    case proto::ConveyorData::LineOrientation::down_right:
        change_ttype(origin_coord.x, origin_coord.y - 1, game::ConveyorStruct::TerminationType::bend_left);
        break;
    case proto::ConveyorData::LineOrientation::down_left:
        change_ttype(origin_coord.x, origin_coord.y - 1, game::ConveyorStruct::TerminationType::bend_right);
        break;

        // Left
    case proto::ConveyorData::LineOrientation::left_down:
        change_ttype(origin_coord.x + 1, origin_coord.y, game::ConveyorStruct::TerminationType::bend_left);
        break;
    case proto::ConveyorData::LineOrientation::left_up:
        change_ttype(origin_coord.x + 1, origin_coord.y, game::ConveyorStruct::TerminationType::bend_right);
        break;


        // Straight (Check for conveyors on both sides to make side only)
    case proto::ConveyorData::LineOrientation::up:
        try_change_ttype(origin_coord.x - 1,
                         origin_coord.y,
                         proto::Orientation::right,
                         game::ConveyorStruct::TerminationType::left_only);
        try_change_ttype(origin_coord.x + 1,
                         origin_coord.y,
                         proto::Orientation::left,
                         game::ConveyorStruct::TerminationType::right_only);
        break;
    case proto::ConveyorData::LineOrientation::right:
        try_change_ttype(origin_coord.x,
                         origin_coord.y - 1,
                         proto::Orientation::down,
                         game::ConveyorStruct::TerminationType::left_only);
        try_change_ttype(origin_coord.x,
                         origin_coord.y + 1,
                         proto::Orientation::up,
                         game::ConveyorStruct::TerminationType::right_only);
        break;
    case proto::ConveyorData::LineOrientation::down:
        try_change_ttype(origin_coord.x - 1,
                         origin_coord.y,
                         proto::Orientation::right,
                         game::ConveyorStruct::TerminationType::right_only);
        try_change_ttype(origin_coord.x + 1,
                         origin_coord.y,
                         proto::Orientation::left,
                         game::ConveyorStruct::TerminationType::left_only);
        break;
    case proto::ConveyorData::LineOrientation::left:
        try_change_ttype(origin_coord.x,
                         origin_coord.y - 1,
                         proto::Orientation::down,
                         game::ConveyorStruct::TerminationType::right_only);
        try_change_ttype(origin_coord.x,
                         origin_coord.y + 1,
                         proto::Orientation::up,
                         game::ConveyorStruct::TerminationType::left_only);
        break;
    }
}

void proto::Conveyor::OnBuild(game::WorldData& world_data,
                              game::LogicData& /*logic_data*/,
                              const WorldCoord& world_coords,
                              game::ChunkTileLayer& tile_layer,
                              const Orientation orientation) const {
    auto line_data_4 = GetLineData4(world_data, world_coords);

    auto& con_data = tile_layer.MakeUniqueData<ConveyorData>();
    ConveyorCreate(world_data, world_coords, con_data, orientation);

    const auto& con_structure_p = con_data.structure;
    const auto line_orientation = GetLineOrientation(orientation, line_data_4);

    con_data.set    = static_cast<uint16_t>(line_orientation);
    con_data.lOrien = line_orientation;


    // Update neighbor orientation has to be done PRIOR to applying segment shift

    CalculateNeighborLineOrientation(world_data, world_coords, line_data_4, &con_data);

    TryShiftSegment(world_data, world_coords, con_structure_p, line_data_4);
    CalculateNeighborTermination<false>(world_data, world_coords, line_orientation);

    ConveyorConnect(world_data, world_coords);
}

void proto::Conveyor::OnNeighborUpdate(game::WorldData& world_data,
                                       game::LogicData& /*logic_data*/,
                                       const WorldCoord& /*emit_world_coords*/,
                                       const WorldCoord& receive_world_coords,
                                       Orientation /*emit_orientation*/) const {
    // Run stuff here that on_build and on_remove both calls

    auto* line_data = GetConData(world_data, {receive_world_coords.x, receive_world_coords.y});
    if (line_data == nullptr) // Conveyor does not exist here
        return;

    // Reset segment lane item index to 0, since the head items MAY now have somewhere to go
    line_data->structure->left.index  = 0;
    line_data->structure->right.index = 0;

    CalculateNeighborTermination<true>(world_data, receive_world_coords, line_data->lOrien);
}

void proto::Conveyor::OnRemove(game::WorldData& world_data,
                               game::LogicData& /*logic_data*/,
                               const WorldCoord& world_coords,
                               game::ChunkTileLayer& /*tile_layer*/) const {
    const auto line_data_4 = GetLineData4(world_data, world_coords);


    CalculateNeighborLineOrientation(world_data, world_coords, line_data_4, nullptr);
    ConveyorDisconnect(world_data, world_coords);

    ConveyorRemove(world_data, world_coords);
}

void proto::Conveyor::OnDeserialize(game::WorldData& world_data,
                                    const WorldCoord& world_coord,
                                    game::ChunkTileLayer& tile_layer) const {
    auto* origin_data = tile_layer.GetUniqueData<ConveyorData>();
    assert(origin_data != nullptr);

    ConveyorConnect(world_data, world_coord);
}

void proto::Conveyor::PostLoad() {
    // Convert floating point speed to fixed precision decimal speed
    speed = LineDistT(speedFloat);
}

void proto::Conveyor::PostLoadValidate(const data::PrototypeManager& /*proto_manager*/) const {
    J_DATA_ASSERT(speedFloat >= 0.001, "Conveyor speed below minimum 0.001");
    // Cannot exceed item_width because of limitations in the logic
    J_DATA_ASSERT(speedFloat < 0.25, "Conveyor speed equal or above maximum of 0.25");
}

void proto::Conveyor::ValidatedPostLoad() {
    sprite->DefaultSpriteGroup({Sprite::SpriteGroup::terrain});
}
