// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "data/prototype/abstract_proto/transport_line.h"

#include <array>
#include <cmath>

#include "game/logic/transport_segment.h"
#include "renderer/rendering/data_renderer.h"

using namespace jactorio;
using LineData4Way = data::TransportLine::LineData4Way;


data::Orientation data::TransportLineData::ToOrientation(const LineOrientation line_orientation) {
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

data::TransportLineData* data::TransportLine::GetLineData(game::WorldData& world_data,
                                                          const WorldCoordAxis world_x,
                                                          const WorldCoordAxis world_y) {
    auto* tile = world_data.GetTile(world_x, world_y);
    if (!tile) // No tile exists
        return nullptr;

    auto& layer = tile->GetLayer(game::TileLayer::entity);

    if (!dynamic_cast<const TransportLine*>( // Not an instance of transport line
            layer.prototypeData.Get()))
        return nullptr;

    return layer.GetUniqueData<TransportLineData>();
}

const data::TransportLineData* data::TransportLine::GetLineData(const game::WorldData& world_data,
                                                                const WorldCoordAxis world_x,
                                                                const WorldCoordAxis world_y) {
    return const_cast<TransportLineData*>(GetLineData(const_cast<game::WorldData&>(world_data), world_x, world_y));
}

LineData4Way data::TransportLine::GetLineData4(game::WorldData& world_data, const WorldCoord& origin_coord) {
    auto* up    = GetLineData(world_data, origin_coord.x, origin_coord.y - 1);
    auto* right = GetLineData(world_data, origin_coord.x + 1, origin_coord.y);
    auto* down  = GetLineData(world_data, origin_coord.x, origin_coord.y + 1);
    auto* left  = GetLineData(world_data, origin_coord.x - 1, origin_coord.y);

    return {up, right, down, left};
}

std::shared_ptr<game::TransportSegment>* data::TransportLine::GetTransportSegment(game::WorldData& world_data,
                                                                                  const WorldCoordAxis world_x,
                                                                                  const WorldCoordAxis world_y) {
    auto* tile = world_data.GetTile(world_x, world_y);
    if (tile) {
        auto& layer = tile->GetLayer(game::TileLayer::entity);
        if (!layer.prototypeData.Get() || layer.prototypeData->Category() != DataCategory::transport_belt)
            return nullptr;

        auto* unique_data = layer.GetUniqueData<TransportLineData>();
        return &unique_data->lineSegment;
    }

    return nullptr;
}


data::TransportLineData::LineOrientation data::TransportLine::GetLineOrientation(const Orientation orientation,
                                                                                 const LineData4Way& line_data4) {
    auto* up    = line_data4[0];
    auto* right = line_data4[1];
    auto* down  = line_data4[2];
    auto* left  = line_data4[3];

    /// true if has neighbor line and its orientation matches provided
    auto neighbor_valid = [](const TransportLineData* transport_line, const Orientation orient) {
        return transport_line != nullptr && TransportLineData::ToOrientation(transport_line->orientation) == orient;
    };

    switch (orientation) {
    case Orientation::up:
        if (!neighbor_valid(down, Orientation::up) &&
            neighbor_valid(left, Orientation::right) != neighbor_valid(right, Orientation::left)) {

            if (neighbor_valid(left, Orientation::right))
                return TransportLineData::LineOrientation::right_up;
            return TransportLineData::LineOrientation::left_up;
        }
        return TransportLineData::LineOrientation::up;

    case Orientation::right:
        if (!neighbor_valid(left, Orientation::right) &&
            neighbor_valid(up, Orientation::down) != neighbor_valid(down, Orientation::up)) {

            if (neighbor_valid(up, Orientation::down))
                return TransportLineData::LineOrientation::down_right;
            return TransportLineData::LineOrientation::up_right;
        }
        return TransportLineData::LineOrientation::right;

    case Orientation::down:
        if (!neighbor_valid(up, Orientation::down) &&
            neighbor_valid(left, Orientation::right) != neighbor_valid(right, Orientation::left)) {

            if (neighbor_valid(left, Orientation::right))
                return TransportLineData::LineOrientation::right_down;
            return TransportLineData::LineOrientation::left_down;
        }
        return TransportLineData::LineOrientation::down;

    case Orientation::left:
        if (!neighbor_valid(right, Orientation::left) &&
            neighbor_valid(up, Orientation::down) != neighbor_valid(down, Orientation::up)) {

            if (neighbor_valid(up, Orientation::down))
                return TransportLineData::LineOrientation::down_left;
            return TransportLineData::LineOrientation::up_left;
        }
        return TransportLineData::LineOrientation::left;
    }

    assert(false); // Missing switch case
    return TransportLineData::LineOrientation::up;
}


// ======================================================================
// Game events


void data::TransportLine::OnRDrawUniqueData(renderer::RendererLayer& layer,
                                            const SpriteUvCoordsT& uv_coords,
                                            const core::Position2<float>& pixel_offset,
                                            const UniqueDataBase* unique_data) const {
    const auto& line_data = *static_cast<const TransportLineData*>(unique_data);

    // Only draw for the head of segments
    if (line_data.lineSegment->terminationType == game::TransportSegment::TerminationType::straight &&
        line_data.lineSegmentIndex != 0)
        return;

    if (line_data.lineSegment->terminationType != game::TransportSegment::TerminationType::straight &&
        line_data.lineSegmentIndex != 1)
        return;

    DrawTransportSegmentItems(layer, uv_coords, pixel_offset, *line_data.lineSegment);
}

data::Sprite::SetT data::TransportLine::OnRGetSpriteSet(const Orientation orientation,
                                                        game::WorldData& world_data,
                                                        const WorldCoord& world_coords) const {
    return static_cast<uint16_t>(GetLineOrientation(orientation, GetLineData4(world_data, world_coords)));
}

data::Sprite::FrameT data::TransportLine::OnRGetSpriteFrame(const UniqueDataBase&, const GameTickT game_tick) const {
    return AllOfSet(*sprite, game_tick);
}


void RemoveFromLogic(game::WorldData& world_data,
                     const WorldCoord& world_coords,
                     game::TransportSegment& line_segment) {
    world_data.LogicRemove(game::Chunk::LogicGroup::transport_line, world_coords, [&](auto* t_layer) {
        auto* line_data = t_layer->template GetUniqueData<data::TransportLineData>();
        return line_data->lineSegment.get() == &line_segment;
    });
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
//
//
//
//
//

// ======================================================================
// Build / Remove / Neighbor update

static void ShiftSegmentHeadForward(game::TransportSegment& line_segment) {
    line_segment.length++;
    line_segment.itemOffset++;
}

static void ShiftSegmentHeadBackward(game::TransportSegment& line_segment) {
    line_segment.length--;
    line_segment.itemOffset--;
}

///
/// \brief Updates the world tiles which references a transport segment, props: line_segment_index, line_segment
/// \param world_coords Beginning tile to update
/// \param line_segment Beginning segment, traveling inverse Orientation line_segment.length tiles, <br>
/// all tiles set to reference this
/// \param offset Offsets segment id numbering, world_coords must be also adjusted to the appropriate offset when
/// calling
static void UpdateSegmentTiles(game::WorldData& world_data,
                               const WorldCoord& world_coords,
                               const std::shared_ptr<game::TransportSegment>& line_segment,
                               const int offset = 0) {
    using OffsetT = ChunkCoordAxis;

    OffsetT x_offset = 0;
    OffsetT y_offset = 0;

    // Should be -1, 0, 1 depending on orientation
    OffsetT x_change = 0;
    OffsetT y_change = 0;
    OrientationIncrement(line_segment->direction, x_change, y_change, -1);

    // Adjust the segment index number of all following segments
    for (auto i = offset; i < line_segment->length; ++i) {
        auto* i_line_data =
            data::TransportLine::GetLineData(world_data, world_coords.x + x_offset, world_coords.y + y_offset);
        if (!i_line_data)
            continue;

        core::SafeCastAssign(i_line_data->lineSegmentIndex, i);
        i_line_data->lineSegment = line_segment;

        x_offset += x_change;
        y_offset += y_change;
    }
}

///
///	\brief Calculates the line orientation of  neighboring transport lines
void CalculateNeighborLineOrientation(game::WorldData& world_data,
                                      const WorldCoord& world_coords,
                                      const LineData4Way& line_data_4,
                                      data::TransportLineData* center) {
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
    auto* top    = data::TransportLine::GetLineData(world_data, world_coords.x, world_coords.y - 2);
    auto* right  = data::TransportLine::GetLineData(world_data, world_coords.x + 2, world_coords.y);
    auto* bottom = data::TransportLine::GetLineData(world_data, world_coords.x, world_coords.y + 2);
    auto* left   = data::TransportLine::GetLineData(world_data, world_coords.x - 2, world_coords.y);

    auto* t_left  = data::TransportLine::GetLineData(world_data, world_coords.x - 1, world_coords.y - 1);
    auto* t_right = data::TransportLine::GetLineData(world_data, world_coords.x + 1, world_coords.y - 1);

    auto* b_left  = data::TransportLine::GetLineData(world_data, world_coords.x - 1, world_coords.y + 1);
    auto* b_right = data::TransportLine::GetLineData(world_data, world_coords.x + 1, world_coords.y + 1);

    // Top neighbor
    if (line_data_4[0])
        line_data_4[0]->SetOrientation(data::TransportLine::GetLineOrientation(
            data::TransportLineData::ToOrientation(line_data_4[0]->orientation), {top, t_right, center, t_left}));
    // Right
    if (line_data_4[1])
        line_data_4[1]->SetOrientation(data::TransportLine::GetLineOrientation(
            data::TransportLineData::ToOrientation(line_data_4[1]->orientation), {t_right, right, b_right, center}));
    // Bottom
    if (line_data_4[2])
        line_data_4[2]->SetOrientation(data::TransportLine::GetLineOrientation(
            data::TransportLineData::ToOrientation(line_data_4[2]->orientation), {center, b_right, bottom, b_left}));
    // Left
    if (line_data_4[3])
        line_data_4[3]->SetOrientation(data::TransportLine::GetLineOrientation(
            data::TransportLineData::ToOrientation(line_data_4[3]->orientation), {t_left, center, b_left, left}));
}


// ======================================================================
// Updating termination type

///
/// \brief Shifts origin segment forwards if neighbor line orientation matches template arguments
template <data::TransportLineData::LineOrientation BendLeft,
          data::TransportLineData::LineOrientation BendRight,
          data::TransportLineData::LineOrientation LeftOnly,
          data::TransportLineData::LineOrientation RightOnly>
void TryShiftSegment(game::WorldData& world_data,
                     const WorldCoord& origin_coords,
                     const std::shared_ptr<game::TransportSegment>& origin_segment,
                     data::TransportLineData* neighbor_data) {
    if (!neighbor_data)
        return;

    switch (neighbor_data->orientation) {
    case BendLeft:
    case BendRight:
    case LeftOnly:
    case RightOnly:
        ShiftSegmentHeadForward(*origin_segment);
        UpdateSegmentTiles(world_data, origin_coords, origin_segment, 1);
        break;

    default:
        break;
    }
}

///
/// \brief Shifts origin segment forwards depending on its orientation with the appropriate neighbor
void TryShiftSegment(game::WorldData& world_data,
                     const WorldCoord& origin_coords,
                     const std::shared_ptr<game::TransportSegment>& origin_segment,
                     LineData4Way& neighbor_data4) {
    switch (origin_segment->direction) {

        // Neighbor's line orientation should have been changed to "dir_dir"
        // because this was called after CalculateNeighborLineOrientation
        // Their actual orientation is the second direction

    case data::Orientation::up:
        TryShiftSegment<data::TransportLineData::LineOrientation::up_left,
                        data::TransportLineData::LineOrientation::up_right,
                        data::TransportLineData::LineOrientation::right,
                        data::TransportLineData::LineOrientation::left>(
            world_data, origin_coords, origin_segment, neighbor_data4[0]);
        break;
    case data::Orientation::right:
        TryShiftSegment<data::TransportLineData::LineOrientation::right_up,
                        data::TransportLineData::LineOrientation::right_down,
                        data::TransportLineData::LineOrientation::down,
                        data::TransportLineData::LineOrientation::up>(
            world_data, origin_coords, origin_segment, neighbor_data4[1]);
        break;
    case data::Orientation::down:
        TryShiftSegment<data::TransportLineData::LineOrientation::down_right,
                        data::TransportLineData::LineOrientation::down_left,
                        data::TransportLineData::LineOrientation::left,
                        data::TransportLineData::LineOrientation::right>(
            world_data, origin_coords, origin_segment, neighbor_data4[2]);
        break;
    case data::Orientation::left:
        TryShiftSegment<data::TransportLineData::LineOrientation::left_down,
                        data::TransportLineData::LineOrientation::left_up,
                        data::TransportLineData::LineOrientation::up,
                        data::TransportLineData::LineOrientation::down>(
            world_data, origin_coords, origin_segment, neighbor_data4[3]);
        break;
    }
}

///
/// \brief Changes terminate type of neighbors depending on origin
/// \tparam IsNeighborUpdate If true, length and itemOffset will also be updated
/// \remark This does not move across logic chunks and may make the position negative
template <bool IsNeighborUpdate>
void CalculateNeighborTermination(game::WorldData& world_data,
                                  const WorldCoord& origin_coord,
                                  const data::TransportLineData::LineOrientation origin_line_orientation) {

    auto bend_update = [&world_data](const WorldCoordAxis w_x,
                                     const WorldCoordAxis w_y,
                                     const game::TransportSegment::TerminationType new_ttype) {
        auto* line_segment = data::TransportLine::GetTransportSegment(world_data, w_x, w_y);
        if (line_segment) {
            if constexpr (!IsNeighborUpdate) {
                ShiftSegmentHeadForward(**line_segment);
            }
            line_segment->get()->terminationType = new_ttype;

            UpdateSegmentTiles(world_data, {w_x, w_y}, *line_segment, 1);
        }
    };

    auto side_update = [&world_data](const WorldCoordAxis w_x,
                                     const WorldCoordAxis w_y,
                                     const data::Orientation required_direction,
                                     const game::TransportSegment::TerminationType new_ttype) {
        auto* line_segment = data::TransportLine::GetTransportSegment(world_data, w_x, w_y);
        if (line_segment) {
            if (line_segment->get()->direction != required_direction)
                return;

            if constexpr (!IsNeighborUpdate) {
                ShiftSegmentHeadForward(**line_segment);
            }
            line_segment->get()->terminationType = new_ttype;

            UpdateSegmentTiles(world_data, {w_x, w_y}, *line_segment, 1);
        }
    };

    switch (origin_line_orientation) {
        // Up
    case data::TransportLineData::LineOrientation::up_left:
        bend_update(origin_coord.x, origin_coord.y + 1, game::TransportSegment::TerminationType::bend_left);
        break;
    case data::TransportLineData::LineOrientation::up_right:
        bend_update(origin_coord.x, origin_coord.y + 1, game::TransportSegment::TerminationType::bend_right);
        break;

        // Right
    case data::TransportLineData::LineOrientation::right_up:
        bend_update(origin_coord.x - 1, origin_coord.y, game::TransportSegment::TerminationType::bend_left);
        break;
    case data::TransportLineData::LineOrientation::right_down:
        bend_update(origin_coord.x - 1, origin_coord.y, game::TransportSegment::TerminationType::bend_right);
        break;

        // Down
    case data::TransportLineData::LineOrientation::down_right:
        bend_update(origin_coord.x, origin_coord.y - 1, game::TransportSegment::TerminationType::bend_left);
        break;
    case data::TransportLineData::LineOrientation::down_left:
        bend_update(origin_coord.x, origin_coord.y - 1, game::TransportSegment::TerminationType::bend_right);
        break;

        // Left
    case data::TransportLineData::LineOrientation::left_down:
        bend_update(origin_coord.x + 1, origin_coord.y, game::TransportSegment::TerminationType::bend_left);
        break;
    case data::TransportLineData::LineOrientation::left_up:
        bend_update(origin_coord.x + 1, origin_coord.y, game::TransportSegment::TerminationType::bend_right);
        break;


        // Straight (Check for transport lines on both sides to make side only)
    case data::TransportLineData::LineOrientation::up:
        side_update(origin_coord.x - 1,
                    origin_coord.y,
                    data::Orientation::right,
                    game::TransportSegment::TerminationType::left_only);
        side_update(origin_coord.x + 1,
                    origin_coord.y,
                    data::Orientation::left,
                    game::TransportSegment::TerminationType::right_only);
        break;
    case data::TransportLineData::LineOrientation::right:
        side_update(origin_coord.x,
                    origin_coord.y - 1,
                    data::Orientation::down,
                    game::TransportSegment::TerminationType::left_only);
        side_update(origin_coord.x,
                    origin_coord.y + 1,
                    data::Orientation::up,
                    game::TransportSegment::TerminationType::right_only);
        break;
    case data::TransportLineData::LineOrientation::down:
        side_update(origin_coord.x - 1,
                    origin_coord.y,
                    data::Orientation::right,
                    game::TransportSegment::TerminationType::right_only);
        side_update(origin_coord.x + 1,
                    origin_coord.y,
                    data::Orientation::left,
                    game::TransportSegment::TerminationType::left_only);
        break;
    case data::TransportLineData::LineOrientation::left:
        side_update(origin_coord.x,
                    origin_coord.y - 1,
                    data::Orientation::down,
                    game::TransportSegment::TerminationType::right_only);
        side_update(origin_coord.x,
                    origin_coord.y + 1,
                    data::Orientation::up,
                    game::TransportSegment::TerminationType::left_only);
        break;
    }
}


// ======================================================================
// Build

///
/// \brief Initializes line data and groups transport segments
/// Sets the transport segment grouped / newly created with in tile_layer and returns it
/// \return Created data for at tile_layer, was a new transport segment created
data::TransportLineData& InitTransportSegment(game::WorldData& world_data,
                                              const WorldCoord& world_coords,
                                              const data::Orientation orientation,
                                              game::ChunkTileLayer& tile_layer,
                                              LineData4Way& line_data) {
    /*
     * Transport line grouping rules:
     *
     * < < < [1, 2, 3] - Direction [order];
     * Line ahead:
     *		- Extends length of transport line segment
     *
     * < < < [3, 2, 1]
     * Line behind:
     *		- Moves head of transport segment, shift leading item 1 tile back
     *
     * < < < [1, 3, 2]
     * Line ahead and behind:
     *		- Behaves as line ahead
     */

    static_assert(static_cast<int>(data::Orientation::left) == 3); // Indexing line_data will be out of range

    auto& origin_chunk = *world_data.GetChunkW(world_coords);

    std::shared_ptr<game::TransportSegment> line_segment;
    int line_segment_index = 0;

    enum class InitSegmentStatus
    {
        new_segment,
        group_ahead, // Segment ahead of current location
        group_behind // Segment behind current location
    } status;
    const auto index  = static_cast<int>(orientation);
    const int i_index = data::InvertOrientation(index);

    if (!line_data[index] || line_data[index]->lineSegment->direction != orientation) {

        status = InitSegmentStatus::new_segment; // If failed to group with ahead, this is chosen

        // Failed to group with ahead, try to group with segment behind
        if (line_data[i_index] && line_data[i_index]->lineSegment->direction == orientation) {
            // Group behind

            WorldCoord behind_coords = world_coords;
            OrientationIncrement(orientation, behind_coords.x, behind_coords.y, -1);

            // Within the same logic chunk = Can group behind
            status = InitSegmentStatus::group_behind;
            if (&origin_chunk != world_data.GetChunkW(behind_coords)) {
                // Different memory addresses = different logic chunks
                status = InitSegmentStatus::new_segment;
            }
        }
    }
    else {
        // Group ahead

        WorldCoord ahead_coords = world_coords;
        OrientationIncrement(orientation, ahead_coords.x, ahead_coords.y, 1);

        status = InitSegmentStatus::group_ahead;
        if (&origin_chunk != world_data.GetChunkW(ahead_coords)) {
            // Different memory addresses = different logic chunks
            status = InitSegmentStatus::new_segment;
        }
    }

    // ======================================================================

    switch (status) {
    case InitSegmentStatus::new_segment:
        line_segment =
            std::make_shared<game::TransportSegment>(orientation, game::TransportSegment::TerminationType::straight, 1);

        world_data.LogicRegister(game::Chunk::LogicGroup::transport_line, world_coords, game::TileLayer::entity);
        break;

    case InitSegmentStatus::group_behind:
        // The transport segment's position is adjusted by init_transport_segment
        // Move the segment head behind forwards to current position
        line_segment = line_data[i_index]->lineSegment;
        ShiftSegmentHeadForward(*line_segment);
        break;

    case InitSegmentStatus::group_ahead:
        line_segment = line_data[index]->lineSegment;

        line_data[index]->lineSegment->length++; // Lengthening its tail, not head
        line_segment_index = line_data[index]->lineSegmentIndex + 1;
        break;

    default:
        assert(false);
        break;
    }

    // Create unique data at tile
    auto* unique_data = tile_layer.MakeUniqueData<data::TransportLineData>(line_segment);
    assert(unique_data != nullptr);

    core::SafeCastAssign(unique_data->lineSegmentIndex, line_segment_index);

    // Line data is not initialized yet inside switch
    if (status == InitSegmentStatus::group_behind) {
        // Remove old head from logic group, add new head which is now 1 tile ahead
        RemoveFromLogic(world_data, world_coords, *line_segment);
        world_data.LogicRegister(game::Chunk::LogicGroup::transport_line, world_coords, game::TileLayer::entity);

        // Renumber
        UpdateSegmentTiles(world_data, world_coords, line_segment);
    }

    return *unique_data;
}

///
/// \brief Determines a line + neighbor segment's target segment
/// \tparam OriginConnect Orientation required for origin_segment to connect to neighbor segment
/// \tparam TargetConnect Orientation required for neighbor segment to connect to origin segment
template <data::Orientation OriginConnect, data::Orientation TargetConnect>
void CalculateLineTargets(game::WorldData& world_data,
                          data::TransportLineData& origin_data,
                          const data::Orientation origin_orientation,
                          const WorldCoordAxis neighbor_world_x,
                          const WorldCoordAxis neighbor_world_y) {
    auto& origin_segment = *origin_data.lineSegment;

    auto* neighbor_data = data::TransportLine::GetLineData(world_data, neighbor_world_x, neighbor_world_y);
    if (!neighbor_data)
        return;


    auto& neighbor_segment = *neighbor_data->lineSegment;

    // Do not attempt to connect to itself
    if (&origin_segment == &neighbor_segment)
        return;

    const bool origin_can_connect   = origin_orientation == OriginConnect;
    const bool neighbor_can_connect = neighbor_segment.direction == TargetConnect;

    // Only 1 can be valid at a time (does not both point to each other)
    // Either origin feeds into neighbor, or neighbor feeds into origin depending on which one is valid
    if (origin_can_connect == neighbor_can_connect)
        return;


    auto connect_segment = [](game::TransportSegment& from, data::TransportLineData& to_data) {
        auto& to = *to_data.lineSegment;

        from.targetSegment = &to;

        from.targetInsertOffset = to_data.lineSegmentIndex;
        to.GetOffsetAbs(from.targetInsertOffset);
    };

    if (origin_can_connect)
        connect_segment(origin_segment, *neighbor_data);
    else
        connect_segment(neighbor_segment, origin_data);
}

///
/// \brief Set the target segment to the neighbor origin is pointing to,
/// and the neighbor's target segment which is pointing to origin
void CalculateLineTargets4(game::WorldData& world_data,
                           const WorldCoord& origin_coord,
                           const data::Orientation origin_orient,
                           data::TransportLineData& origin_data,
                           const LineData4Way& line_data_4) {
    if (line_data_4[0])
        CalculateLineTargets<data::Orientation::up, data::Orientation::down>(
            world_data, origin_data, origin_orient, origin_coord.x, origin_coord.y - 1);

    if (line_data_4[1])
        CalculateLineTargets<data::Orientation::right, data::Orientation::left>(
            world_data, origin_data, origin_orient, origin_coord.x + 1, origin_coord.y);

    if (line_data_4[2])
        CalculateLineTargets<data::Orientation::down, data::Orientation::up>(
            world_data, origin_data, origin_orient, origin_coord.x, origin_coord.y + 1);

    if (line_data_4[3])
        CalculateLineTargets<data::Orientation::left, data::Orientation::right>(
            world_data, origin_data, origin_orient, origin_coord.x - 1, origin_coord.y);
}

void data::TransportLine::OnBuild(game::WorldData& world_data,
                                  game::LogicData& /*logic_data*/,
                                  const WorldCoord& world_coords,
                                  game::ChunkTileLayer& tile_layer,
                                  const Orientation orientation) const {
    auto line_data_4 = GetLineData4(world_data, world_coords);

    auto& line_data            = InitTransportSegment(world_data, world_coords, orientation, tile_layer, line_data_4);
    const auto& line_segment_p = line_data.lineSegment;

    const auto line_orientation = GetLineOrientation(orientation, line_data_4);

    line_data.set         = static_cast<uint16_t>(line_orientation);
    line_data.orientation = line_orientation;


    // Update neighbor orientation has to be done PRIOR to applying segment shift

    CalculateNeighborLineOrientation(world_data, world_coords, line_data_4, &line_data);

    TryShiftSegment(world_data, world_coords, line_segment_p, line_data_4);
    CalculateNeighborTermination<false>(world_data, world_coords, line_orientation);

    CalculateLineTargets4(world_data, world_coords, orientation, line_data, line_data_4);
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
//
//
//
//
//

// ======================================================================
// Neighbor update
void data::TransportLine::OnNeighborUpdate(game::WorldData& world_data,
                                           game::LogicData& /*logic_data*/,
                                           const WorldCoord& /*emit_world_coords*/,
                                           const WorldCoord& receive_world_coords,
                                           Orientation /*emit_orientation*/) const {
    // Run stuff here that on_build and on_remove both calls

    auto* line_data = GetLineData(world_data, receive_world_coords.x, receive_world_coords.y);
    if (!line_data) // Transport line does not exist here
        return;

    // Reset segment lane item index to 0, since the head items MAY now have somewhere to go
    line_data->lineSegment->left.index  = 0;
    line_data->lineSegment->right.index = 0;

    CalculateNeighborTermination<true>(world_data, receive_world_coords, line_data->orientation);
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
//
//
//
//
//

// ======================================================================
// Remove

///
/// \brief Updates neighboring segments after transport line is removed
/// \param origin_coord Coords of origin_segment
/// \param origin_data Removed line segment data
/// \param neighbor_data Neighboring line segment data
void DisconnectSegment(game::WorldData& world_data,
                       WorldCoord origin_coord,
                       data::TransportLineData* origin_data,
                       data::TransportLineData* neighbor_data) {


    // Neighbor must target origin segment
    if (!neighbor_data || neighbor_data->lineSegment->targetSegment != origin_data->lineSegment.get())
        return;

    auto& neighbor_segment_p = neighbor_data->lineSegment;
    auto& neighbor_segment   = *neighbor_segment_p;

    neighbor_segment.targetSegment = nullptr;

    switch (neighbor_segment.terminationType) {

        // Convert bend to straight
    case game::TransportSegment::TerminationType::bend_left:
    case game::TransportSegment::TerminationType::bend_right:
    case game::TransportSegment::TerminationType::right_only:
    case game::TransportSegment::TerminationType::left_only:

        ShiftSegmentHeadBackward(neighbor_segment);
        neighbor_segment.terminationType = game::TransportSegment::TerminationType::straight;

        // Renumber segments following origin from index 0, formerly 1
        OrientationIncrement(neighbor_segment.direction, origin_coord.x, origin_coord.y, -1.f);

        UpdateSegmentTiles(world_data, origin_coord, neighbor_segment_p);
        break;

    default:
        // Does not bend
        break;
    }
}

void DisconnectNeighborSegments(game::WorldData& world_data,
                                const WorldCoord& origin_coord,
                                data::TransportLineData* origin_data,
                                const LineData4Way& neighbor_data) {
    for (int i = 0; i < 4; ++i) {
        DisconnectSegment(world_data, origin_coord, origin_data, neighbor_data[i]);
    }
}


double ToChunkOffset(const WorldCoordAxis world_coord) {
    return fabs(game::WorldData::WorldCToChunkC(world_coord) * game::Chunk::kChunkWidth - world_coord);
}

void data::TransportLine::OnRemove(game::WorldData& world_data,
                                   game::LogicData&,
                                   const WorldCoord& world_coords,
                                   game::ChunkTileLayer& tile_layer) const {
    auto* origin_data      = tile_layer.GetUniqueData<TransportLineData>();
    const auto line_data_4 = GetLineData4(world_data, world_coords);


    CalculateNeighborLineOrientation(world_data, world_coords, line_data_4, nullptr);
    DisconnectNeighborSegments(world_data, world_coords, origin_data, line_data_4);

    auto& chunk = *world_data.GetChunkW(world_coords);

    // o_ = old
    // n_ = new

    auto& o_line_data          = *tile_layer.GetUniqueData<TransportLineData>();
    const auto& o_line_segment = o_line_data.lineSegment;

    auto n_seg_coords = world_coords;

    OrientationIncrement(o_line_segment->direction, n_seg_coords.x, n_seg_coords.y, -1);

    // Create new segment at behind cords if not the end of a segment
    const auto n_seg_length = o_line_segment->length - o_line_data.lineSegmentIndex - 1;
    if (n_seg_length > 0) {
        // Create new segment
        const auto n_segment = std::make_shared<game::TransportSegment>(
            o_line_segment->direction, game::TransportSegment::TerminationType::straight, n_seg_length);
        n_segment->itemOffset = o_line_segment->itemOffset - o_line_data.lineSegmentIndex - 1;


        // Add to be considered for logic updates
        chunk.GetLogicGroup(game::Chunk::LogicGroup::transport_line)
            .emplace_back(&world_data.GetTile(n_seg_coords)->GetLayer(game::TileLayer::entity));

        // ======================================================================

        // Update trailing segments to use new segment and renumber
        UpdateSegmentTiles(world_data, n_seg_coords, n_segment);

        // Update other segments leading into old segment
        // TODO improve this algorithm for updating target segments
        for (int i = 0; i < game::Chunk::kChunkArea; ++i) {
            auto& layer = chunk.Tiles()[i].GetLayer(game::TileLayer::entity);
            if (!layer.prototypeData.Get() || layer.prototypeData->Category() != DataCategory::transport_belt)
                continue;

            const auto& line_segment = layer.GetUniqueData<TransportLineData>()->lineSegment.get();


            const auto position_x = i % game::Chunk::kChunkWidth;
            const auto position_y = i / game::Chunk::kChunkWidth;
            bool valid_neighbor   = false; // Neighbor must be BEHIND the segment which was removed
            switch (o_line_segment->direction) {
            case Orientation::up:
                valid_neighbor = position_y > ToChunkOffset(world_coords.y);
                break;
            case Orientation::right:
                valid_neighbor = position_x < ToChunkOffset(world_coords.x);
                break;
            case Orientation::down:
                valid_neighbor = position_y < ToChunkOffset(world_coords.y);
                break;
            case Orientation::left:
                valid_neighbor = position_x > ToChunkOffset(world_coords.x);
                break;
            }

            if (valid_neighbor && line_segment->targetSegment == o_line_segment.get()) {
                line_segment->targetSegment = n_segment.get();
            }
        }


        // Update segment in neighboring logic chunk leading into old_segment
        ChunkCoord neighbor_chunk_coords = chunk.GetPosition();
        OrientationIncrement(o_line_segment->direction, neighbor_chunk_coords.x, neighbor_chunk_coords.y, -1);

        auto* neighbor_l_chunk = world_data.GetChunkC(neighbor_chunk_coords);
        if (neighbor_l_chunk) {
            for (auto& t_line : neighbor_l_chunk->GetLogicGroup(game::Chunk::LogicGroup::transport_line)) {
                const auto& i_segment = t_line->GetUniqueData<TransportLineData>()->lineSegment;

                if (i_segment->targetSegment == o_line_segment.get()) {
                    i_segment->targetSegment = n_segment.get();
                }
            }
        }
    }

    // Remove original transport line segment referenced in Transport_line_data if is head of segment
    // If not head, reduce the length of original segment to index + 1
    if (o_line_data.lineSegmentIndex == 0 ||
        (o_line_data.lineSegmentIndex == 1 &&
         o_line_segment->terminationType != // Head of bending segments start at 1
             game::TransportSegment::TerminationType::straight)) {

        RemoveFromLogic(world_data, world_coords, *o_line_segment);
    }
    else {
        o_line_segment->length = o_line_data.lineSegmentIndex;
    }
}

void data::TransportLine::OnDeserialize(game::WorldData& world_data,
                                        const WorldCoord& world_coord,
                                        game::ChunkTileLayer& tile_layer) const {
    auto* origin_data = tile_layer.GetUniqueData<TransportLineData>();
    assert(origin_data != nullptr);

    CalculateLineTargets4(world_data,
                          world_coord,
                          TransportLineData::ToOrientation(origin_data->orientation),
                          *origin_data,
                          GetLineData4(world_data, world_coord));
}
