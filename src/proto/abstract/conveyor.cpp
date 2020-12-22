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
    return static_cast<uint16_t>(ConveyorCalcLineOrien(world_data, world_coords, orientation));
}

SpriteFrameT proto::Conveyor::OnRGetSpriteFrame(const UniqueDataBase& /*unique_data*/,
                                                const GameTickT game_tick) const {
    return AllOfSet(*sprite, game_tick);
}


// ======================================================================
// Build / Remove / Neighbor update

///
///	\brief Calculates the line orientation of neighboring conveyors
void CalculateNeighborLineOrientation(game::WorldData& world, const WorldCoord& coord) {

    auto calculate_neighbor = [&world](const WorldCoord& neighbor_coord) {
        auto* con_data = GetConData(world, neighbor_coord);

        if (con_data != nullptr) {
            const auto orien = proto::ConveyorData::ToOrientation(con_data->lOrien);
            con_data->SetOrientation(ConveyorCalcLineOrien(world, neighbor_coord, orien));
        }
    };

    calculate_neighbor({coord.x, coord.y - 1});
    calculate_neighbor({coord.x + 1, coord.y});
    calculate_neighbor({coord.x, coord.y + 1});
    calculate_neighbor({coord.x - 1, coord.y});
}


///
/// Shifts origin segment forwards if neighbor line orientation matches template arguments
template <proto::LineOrientation BendLeft,
          proto::LineOrientation BendRight,
          proto::LineOrientation LeftOnly,
          proto::LineOrientation RightOnly>
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
void TryShiftSegment(game::WorldData& world,
                     const WorldCoord& coord,
                     const std::shared_ptr<game::ConveyorStruct>& origin_segment) {

    switch (origin_segment->direction) {

        // Neighbor's line orientation should have been changed to "dir_dir"
        // because this was called after CalculateNeighborLineOrientation
        // Their actual orientation is the second direction

    case proto::Orientation::up:
        TryShiftSegment<proto::LineOrientation::up_left,
                        proto::LineOrientation::up_right,
                        proto::LineOrientation::right,
                        proto::LineOrientation::left>(
            world, coord, origin_segment, GetConData(world, {coord.x, coord.y - 1}));
        break;
    case proto::Orientation::right:
        TryShiftSegment<proto::LineOrientation::right_up,
                        proto::LineOrientation::right_down,
                        proto::LineOrientation::down,
                        proto::LineOrientation::up>(
            world, coord, origin_segment, GetConData(world, {coord.x + 1, coord.y}));
        break;
    case proto::Orientation::down:
        TryShiftSegment<proto::LineOrientation::down_right,
                        proto::LineOrientation::down_left,
                        proto::LineOrientation::left,
                        proto::LineOrientation::right>(
            world, coord, origin_segment, GetConData(world, {coord.x, coord.y + 1}));
        break;
    case proto::Orientation::left:
        TryShiftSegment<proto::LineOrientation::left_down,
                        proto::LineOrientation::left_up,
                        proto::LineOrientation::up,
                        proto::LineOrientation::down>(
            world, coord, origin_segment, GetConData(world, {coord.x - 1, coord.y}));
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
                                  const proto::LineOrientation origin_line_orientation) {

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
    case proto::LineOrientation::up_left:
        change_ttype(origin_coord.x, origin_coord.y + 1, game::ConveyorStruct::TerminationType::bend_left);
        break;
    case proto::LineOrientation::up_right:
        change_ttype(origin_coord.x, origin_coord.y + 1, game::ConveyorStruct::TerminationType::bend_right);
        break;

        // Right
    case proto::LineOrientation::right_up:
        change_ttype(origin_coord.x - 1, origin_coord.y, game::ConveyorStruct::TerminationType::bend_left);
        break;
    case proto::LineOrientation::right_down:
        change_ttype(origin_coord.x - 1, origin_coord.y, game::ConveyorStruct::TerminationType::bend_right);
        break;

        // Down
    case proto::LineOrientation::down_right:
        change_ttype(origin_coord.x, origin_coord.y - 1, game::ConveyorStruct::TerminationType::bend_left);
        break;
    case proto::LineOrientation::down_left:
        change_ttype(origin_coord.x, origin_coord.y - 1, game::ConveyorStruct::TerminationType::bend_right);
        break;

        // Left
    case proto::LineOrientation::left_down:
        change_ttype(origin_coord.x + 1, origin_coord.y, game::ConveyorStruct::TerminationType::bend_left);
        break;
    case proto::LineOrientation::left_up:
        change_ttype(origin_coord.x + 1, origin_coord.y, game::ConveyorStruct::TerminationType::bend_right);
        break;


        // Straight (Check for conveyors on both sides to make side only)
    case proto::LineOrientation::up:
        try_change_ttype(origin_coord.x - 1,
                         origin_coord.y,
                         proto::Orientation::right,
                         game::ConveyorStruct::TerminationType::left_only);
        try_change_ttype(origin_coord.x + 1,
                         origin_coord.y,
                         proto::Orientation::left,
                         game::ConveyorStruct::TerminationType::right_only);
        break;
    case proto::LineOrientation::right:
        try_change_ttype(origin_coord.x,
                         origin_coord.y - 1,
                         proto::Orientation::down,
                         game::ConveyorStruct::TerminationType::left_only);
        try_change_ttype(origin_coord.x,
                         origin_coord.y + 1,
                         proto::Orientation::up,
                         game::ConveyorStruct::TerminationType::right_only);
        break;
    case proto::LineOrientation::down:
        try_change_ttype(origin_coord.x - 1,
                         origin_coord.y,
                         proto::Orientation::right,
                         game::ConveyorStruct::TerminationType::right_only);
        try_change_ttype(origin_coord.x + 1,
                         origin_coord.y,
                         proto::Orientation::left,
                         game::ConveyorStruct::TerminationType::left_only);
        break;
    case proto::LineOrientation::left:
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

void proto::Conveyor::OnBuild(game::WorldData& world,
                              game::LogicData& /*logic*/,
                              const WorldCoord& coord,
                              game::ChunkTileLayer& tile_layer,
                              const Orientation orientation) const {

    auto& con_data = tile_layer.MakeUniqueData<ConveyorData>();
    ConveyorCreate(world, coord, con_data, orientation);

    const auto& con_structure_p = con_data.structure;

    const auto line_orientation = ConveyorCalcLineOrien(world, coord, orientation);

    con_data.set    = static_cast<uint16_t>(line_orientation);
    con_data.lOrien = line_orientation;


    // Update neighbor orientation has to be done PRIOR to applying segment shift

    CalculateNeighborLineOrientation(world, coord);

    TryShiftSegment(world, coord, con_structure_p);
    CalculateNeighborTermination<false>(world, coord, line_orientation);

    ConveyorConnect(world, coord);
}

void proto::Conveyor::OnNeighborUpdate(game::WorldData& world,
                                       game::LogicData& /*logic*/,
                                       const WorldCoord& /*emit_world_coords*/,
                                       const WorldCoord& receive_world_coords,
                                       Orientation /*emit_orientation*/) const {
    // Run stuff here that on_build and on_remove both calls

    auto* line_data = GetConData(world, {receive_world_coords.x, receive_world_coords.y});
    if (line_data == nullptr) // Conveyor does not exist here
        return;

    // Reset segment lane item index to 0, since the head items MAY now have somewhere to go
    line_data->structure->left.index  = 0;
    line_data->structure->right.index = 0;

    CalculateNeighborTermination<true>(world, receive_world_coords, line_data->lOrien);
}

void proto::Conveyor::OnRemove(game::WorldData& world,
                               game::LogicData& /*logic*/,
                               const WorldCoord& coord,
                               game::ChunkTileLayer& /*tile_layer*/) const {
    ConveyorDisconnect(world, coord);
    ConveyorRemove(world, coord);
    CalculateNeighborLineOrientation(world, coord);
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
