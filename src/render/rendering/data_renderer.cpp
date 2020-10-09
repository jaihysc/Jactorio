// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "render/rendering/data_renderer.h"

#include "jactorio.h"

#include "proto/inserter.h"
#include "proto/sprite.h"
#include "game/logic/transport_segment.h"
#include "render/rendering/renderer.h"

using namespace jactorio;

constexpr float kPixelZ = 0.1f;

///
/// \param tile_x Tile offset (for distance after each item)
/// \param tile_y Tile offset
void PrepareTransportSegmentData(render::RendererLayer& layer,
                                 const SpriteUvCoordsT& uv_coords,
                                 const game::TransportSegment& line_segment,
                                 std::deque<game::TransportLineItem>& line_segment_side,
                                 double tile_x,
                                 double tile_y,
                                 const core::Position2<OverlayOffsetAxis>& pixel_offset) {
    using namespace game;

    // Either offset_x or offset_y which will be INCREASED or DECREASED
    double* target_offset;
    double multiplier = 1; // Either 1 or -1 to add or subtract

    switch (line_segment.direction) {
    case data::Orientation::up:
        target_offset = &tile_y;
        break;
    case data::Orientation::right:
        target_offset = &tile_x;
        multiplier    = -1;
        break;
    case data::Orientation::down:
        target_offset = &tile_y;
        multiplier    = -1;
        break;
    case data::Orientation::left:
        target_offset = &tile_x;
        break;

    default:
        assert(false); // Missing switch case
        target_offset = &tile_y;
        break;
    }

    // Shift items 1 tile forwards if segment bends
    if (line_segment.terminationType != TransportSegment::TerminationType::straight) {
        OrientationIncrement(line_segment.direction, tile_x, tile_y);
    }

    for (const auto& line_item : line_segment_side) {
        // Move the target offset (up or down depending on multiplier)
        *target_offset += line_item.dist.getAsDouble() * multiplier;

        const auto& uv_pos = render::Renderer::GetSpriteUvCoords(uv_coords, line_item.item->sprite->internalId);

        constexpr float pixel_z = kPixelZ;
        // In pixels
        layer.PushBack({{
                            {
                                pixel_offset.x +
                                    core::LossyCast<float>(tile_x) * core::SafeCast<float>(render::Renderer::tileWidth),
                                pixel_offset.y +
                                    core::LossyCast<float>(tile_y) * core::SafeCast<float>(render::Renderer::tileWidth),
                            },
                            {
                                pixel_offset.x +
                                    core::LossyCast<float>(tile_x + kItemWidth) *
                                        core::SafeCast<float>(render::Renderer::tileWidth),
                                pixel_offset.y +
                                    core::LossyCast<float>(tile_y + kItemWidth) *
                                        core::SafeCast<float>(render::Renderer::tileWidth),
                            },
                        },
                        {uv_pos.topLeft, uv_pos.bottomRight}},
                       pixel_z);
    }
}

void render::DrawTransportSegmentItems(RendererLayer& layer,
                                       const SpriteUvCoordsT& uv_coords,
                                       const core::Position2<OverlayOffsetAxis>& pixel_offset,
                                       game::TransportSegment& line_segment) {
    double tile_x_offset = 0;
    double tile_y_offset = 0;

    // Don't render if items are not marked visible! Wow!
    if (!line_segment.left.visible)
        goto prepare_right;

    // Left
    // The offsets for straight are always applied to bend left and right
    switch (line_segment.direction) {
    case data::Orientation::up:
        tile_x_offset += game::kLineUpLItemOffsetX;
        break;
    case data::Orientation::right:
        tile_y_offset += game::kLineRightLItemOffsetY;
        break;
    case data::Orientation::down:
        tile_x_offset += game::kLineDownLItemOffsetX;
        break;
    case data::Orientation::left:
        tile_y_offset += game::kLineLeftLItemOffsetY;
        break;
    }

    // Left side
    switch (line_segment.terminationType) {
    case game::TransportSegment::TerminationType::straight:
        switch (line_segment.direction) {
        case data::Orientation::up:
            tile_y_offset -= game::kLineLeftUpStraightItemOffset;
            break;
        case data::Orientation::right:
            tile_x_offset += game::kLineRightDownStraightItemOffset;
            break;
        case data::Orientation::down:
            tile_y_offset += game::kLineRightDownStraightItemOffset;
            break;
        case data::Orientation::left:
            tile_x_offset -= game::kLineLeftUpStraightItemOffset;
            break;
        }
        break;

    case game::TransportSegment::TerminationType::bend_left:
        switch (line_segment.direction) {
        case data::Orientation::up:
            tile_y_offset += game::kLineUpBlLItemOffsetY;
            break;
        case data::Orientation::right:
            tile_x_offset += game::kLineRightBlLItemOffsetX;
            break;
        case data::Orientation::down:
            tile_y_offset += game::kLineDownBlLItemOffsetY;
            break;
        case data::Orientation::left:
            tile_x_offset += game::kLineLeftBlLItemOffsetX;
            break;
        }
        break;

    case game::TransportSegment::TerminationType::bend_right:
        switch (line_segment.direction) {
        case data::Orientation::up:
            tile_y_offset += game::kLineUpBrLItemOffsetY;
            break;
        case data::Orientation::right:
            tile_x_offset += game::kLineRightBrLItemOffsetX;
            break;
        case data::Orientation::down:
            tile_y_offset += game::kLineDownBrLItemOffsetY;
            break;
        case data::Orientation::left:
            tile_x_offset += game::kLineLeftBrLItemOffsetX;
            break;
        }
        break;

        // Side insertion
    case game::TransportSegment::TerminationType::right_only:
    case game::TransportSegment::TerminationType::left_only:
        switch (line_segment.direction) {
        case data::Orientation::up:
            tile_y_offset += game::kLineUpSingleSideItemOffsetY;
            break;
        case data::Orientation::right:
            tile_x_offset += game::kLineRightSingleSideItemOffsetX;
            break;
        case data::Orientation::down:
            tile_y_offset += game::kLineDownSingleSideItemOffsetY;
            break;
        case data::Orientation::left:
            tile_x_offset += game::kLineLeftSingleSideItemOffsetX;
            break;
        }
        break;
    }
    PrepareTransportSegmentData(
        layer, uv_coords, line_segment, line_segment.left.lane, tile_x_offset, tile_y_offset, pixel_offset);

prepare_right:
    if (!line_segment.right.visible)
        return;

    // Right
    tile_x_offset = 0;
    tile_y_offset = 0;

    // The offsets for straight are always applied to bend left and right
    switch (line_segment.direction) {
    case data::Orientation::up:
        tile_x_offset += game::kLineUpRItemOffsetX;
        break;
    case data::Orientation::right:
        tile_y_offset += game::kLineRightRItemOffsetY;
        break;
    case data::Orientation::down:
        tile_x_offset += game::kLineDownRItemOffsetX;
        break;
    case data::Orientation::left:
        tile_y_offset += game::kLineLeftRItemOffsetY;
        break;
    }


    // Right side
    switch (line_segment.terminationType) {
    case game::TransportSegment::TerminationType::straight:
        switch (line_segment.direction) {
        case data::Orientation::up:
            tile_y_offset -= game::kLineLeftUpStraightItemOffset;
            break;
        case data::Orientation::right:
            tile_x_offset += game::kLineRightDownStraightItemOffset;
            break;
        case data::Orientation::down:
            tile_y_offset += game::kLineRightDownStraightItemOffset;
            break;
        case data::Orientation::left:
            tile_x_offset -= game::kLineLeftUpStraightItemOffset;
            break;
        }
        break;

    case game::TransportSegment::TerminationType::bend_left:
        switch (line_segment.direction) {
        case data::Orientation::up:
            tile_y_offset += game::kLineUpBlRItemOffsetY;
            break;
        case data::Orientation::right:
            tile_x_offset += game::kLineRightBlRItemOffsetX;
            break;
        case data::Orientation::down:
            tile_y_offset += game::kLineDownBlRItemOffsetY;
            break;
        case data::Orientation::left:
            tile_x_offset += game::kLineLeftBlRItemOffsetX;
            break;
        }
        break;

    case game::TransportSegment::TerminationType::bend_right:
        switch (line_segment.direction) {
        case data::Orientation::up:
            tile_y_offset += game::kLineUpBrRItemOffsetY;
            break;
        case data::Orientation::right:
            tile_x_offset += game::kLineRightBrRItemOffsetX;
            break;
        case data::Orientation::down:
            tile_y_offset += game::kLineDownBrRItemOffsetY;
            break;
        case data::Orientation::left:
            tile_x_offset += game::kLineLeftBrRItemOffsetX;
            break;
        }
        break;

        // Side insertion
    case game::TransportSegment::TerminationType::right_only:
    case game::TransportSegment::TerminationType::left_only:
        switch (line_segment.direction) {
        case data::Orientation::up:
            tile_y_offset += game::kLineUpSingleSideItemOffsetY;
            break;
        case data::Orientation::right:
            tile_x_offset += game::kLineRightSingleSideItemOffsetX;
            break;
        case data::Orientation::down:
            tile_y_offset += game::kLineDownSingleSideItemOffsetY;
            break;
        case data::Orientation::left:
            tile_x_offset += game::kLineLeftSingleSideItemOffsetX;
            break;
        }
        break;
    }
    PrepareTransportSegmentData(
        layer, uv_coords, line_segment, line_segment.right.lane, tile_x_offset, tile_y_offset, pixel_offset);
}

// ======================================================================

void render::DrawInserterArm(RendererLayer& layer,
                             const SpriteUvCoordsT& uv_coords,
                             const core::Position2<OverlayOffsetAxis>& pixel_offset,
                             const data::Inserter& inserter_proto,
                             const data::InserterData& inserter_data) {
    {
        const auto& uv = Renderer::GetSpriteUvCoords(uv_coords, inserter_proto.handSprite->internalId);

        constexpr float arm_width        = 0.5f;
        constexpr float arm_pixel_offset = (Renderer::tileWidth - arm_width * Renderer::tileWidth) / 2;

        // Ensures arm is always facing pickup / dropoff
        const float rotation_offset = static_cast<float>(inserter_data.orientation) * 90;

        // Hand
        layer.PushBack({{// Cover tile
                         {pixel_offset.x + arm_pixel_offset, pixel_offset.y + arm_pixel_offset},
                         {pixel_offset.x + Renderer::tileWidth - arm_pixel_offset,
                          pixel_offset.y + Renderer::tileWidth - arm_pixel_offset}},
                        {uv.topLeft, uv.bottomRight}},
                       kPixelZ,
                       core::LossyCast<float>(inserter_data.rotationDegree.getAsDouble() + rotation_offset));
    }


    // Held item
    if (inserter_data.status == data::InserterData::Status::dropoff) {
        constexpr auto held_item_pixel_offset =
            core::LossyCast<float>((Renderer::tileWidth - Renderer::tileWidth * game::kItemWidth) / 2);

        const auto& uv = Renderer::GetSpriteUvCoords(uv_coords, inserter_data.heldItem.item->sprite->internalId);

        layer.PushBack({{{pixel_offset.x + held_item_pixel_offset, pixel_offset.y + held_item_pixel_offset},
                         {pixel_offset.x + Renderer::tileWidth - held_item_pixel_offset,
                          pixel_offset.y + Renderer::tileWidth - held_item_pixel_offset}},
                        {uv.topLeft, uv.bottomRight}},
                       kPixelZ);
    }
}
