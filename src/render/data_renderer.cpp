// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "render/data_renderer.h"

#include "jactorio.h"

#include "game/logic/conveyor_segment.h"
#include "proto/inserter.h"
#include "proto/sprite.h"
#include "render/renderer.h"

using namespace jactorio;

constexpr float kPixelZ = 0.1f;

///
/// \param tile_x Tile offset (for distance after each item)
/// \param tile_y Tile offset
void PrepareConveyorSegmentData(render::RendererLayer& layer,
                                const SpriteUvCoordsT& uv_coords,
                                const game::ConveyorSegment& line_segment,
                                std::deque<game::ConveyorItem>& line_segment_side,
                                double tile_x,
                                double tile_y,
                                const core::Position2<OverlayOffsetAxis>& pixel_offset) {
    using namespace game;

    // Either offset_x or offset_y which will be INCREASED or DECREASED
    double* target_offset;
    double multiplier = 1; // Either 1 or -1 to add or subtract

    switch (line_segment.direction) {
    case proto::Orientation::up:
        target_offset = &tile_y;
        break;
    case proto::Orientation::right:
        target_offset = &tile_x;
        multiplier    = -1;
        break;
    case proto::Orientation::down:
        target_offset = &tile_y;
        multiplier    = -1;
        break;
    case proto::Orientation::left:
        target_offset = &tile_x;
        break;

    default:
        assert(false); // Missing switch case
        target_offset = &tile_y;
        break;
    }

    // Shift items 1 tile forwards if segment bends
    if (line_segment.terminationType != ConveyorSegment::TerminationType::straight) {
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
                                    core::LossyCast<float>(tile_x + ConveyorProp::kItemWidth) *
                                        core::SafeCast<float>(render::Renderer::tileWidth),
                                pixel_offset.y +
                                    core::LossyCast<float>(tile_y + ConveyorProp::kItemWidth) *
                                        core::SafeCast<float>(render::Renderer::tileWidth),
                            },
                        },
                        {uv_pos.topLeft, uv_pos.bottomRight}},
                       pixel_z);
    }
}

void render::DrawConveyorSegmentItems(RendererLayer& layer,
                                      const SpriteUvCoordsT& uv_coords,
                                      const core::Position2<OverlayOffsetAxis>& pixel_offset,
                                      game::ConveyorSegment& line_segment) {
    double tile_x_offset = 0;
    double tile_y_offset = 0;

    // Don't render if items are not marked visible! Wow!
    if (!line_segment.left.visible)
        goto prepare_right;

    // Left
    // The offsets for straight are always applied to bend left and right
    switch (line_segment.direction) {
    case proto::Orientation::up:
        tile_x_offset += game::ConveyorProp::kLineUpLItemOffsetX;
        break;
    case proto::Orientation::right:
        tile_y_offset += game::ConveyorProp::kLineRightLItemOffsetY;
        break;
    case proto::Orientation::down:
        tile_x_offset += game::ConveyorProp::kLineDownLItemOffsetX;
        break;
    case proto::Orientation::left:
        tile_y_offset += game::ConveyorProp::kLineLeftLItemOffsetY;
        break;
    }

    // Left side
    switch (line_segment.terminationType) {
    case game::ConveyorSegment::TerminationType::straight:
        switch (line_segment.direction) {
        case proto::Orientation::up:
            tile_y_offset -= game::ConveyorProp::kLineLeftUpStraightItemOffset;
            break;
        case proto::Orientation::right:
            tile_x_offset += game::ConveyorProp::kLineRightDownStraightItemOffset;
            break;
        case proto::Orientation::down:
            tile_y_offset += game::ConveyorProp::kLineRightDownStraightItemOffset;
            break;
        case proto::Orientation::left:
            tile_x_offset -= game::ConveyorProp::kLineLeftUpStraightItemOffset;
            break;
        }
        break;

    case game::ConveyorSegment::TerminationType::bend_left:
        switch (line_segment.direction) {
        case proto::Orientation::up:
            tile_y_offset += game::ConveyorProp::kLineUpBlLItemOffsetY;
            break;
        case proto::Orientation::right:
            tile_x_offset += game::ConveyorProp::kLineRightBlLItemOffsetX;
            break;
        case proto::Orientation::down:
            tile_y_offset += game::ConveyorProp::kLineDownBlLItemOffsetY;
            break;
        case proto::Orientation::left:
            tile_x_offset += game::ConveyorProp::kLineLeftBlLItemOffsetX;
            break;
        }
        break;

    case game::ConveyorSegment::TerminationType::bend_right:
        switch (line_segment.direction) {
        case proto::Orientation::up:
            tile_y_offset += game::ConveyorProp::kLineUpBrLItemOffsetY;
            break;
        case proto::Orientation::right:
            tile_x_offset += game::ConveyorProp::kLineRightBrLItemOffsetX;
            break;
        case proto::Orientation::down:
            tile_y_offset += game::ConveyorProp::kLineDownBrLItemOffsetY;
            break;
        case proto::Orientation::left:
            tile_x_offset += game::ConveyorProp::kLineLeftBrLItemOffsetX;
            break;
        }
        break;

        // Side insertion
    case game::ConveyorSegment::TerminationType::right_only:
    case game::ConveyorSegment::TerminationType::left_only:
        switch (line_segment.direction) {
        case proto::Orientation::up:
            tile_y_offset += game::ConveyorProp::kLineUpSingleSideItemOffsetY;
            break;
        case proto::Orientation::right:
            tile_x_offset += game::ConveyorProp::kLineRightSingleSideItemOffsetX;
            break;
        case proto::Orientation::down:
            tile_y_offset += game::ConveyorProp::kLineDownSingleSideItemOffsetY;
            break;
        case proto::Orientation::left:
            tile_x_offset += game::ConveyorProp::kLineLeftSingleSideItemOffsetX;
            break;
        }
        break;
    }
    PrepareConveyorSegmentData(
        layer, uv_coords, line_segment, line_segment.left.lane, tile_x_offset, tile_y_offset, pixel_offset);

prepare_right:
    if (!line_segment.right.visible)
        return;

    // Right
    tile_x_offset = 0;
    tile_y_offset = 0;

    // The offsets for straight are always applied to bend left and right
    switch (line_segment.direction) {
    case proto::Orientation::up:
        tile_x_offset += game::ConveyorProp::kLineUpRItemOffsetX;
        break;
    case proto::Orientation::right:
        tile_y_offset += game::ConveyorProp::kLineRightRItemOffsetY;
        break;
    case proto::Orientation::down:
        tile_x_offset += game::ConveyorProp::kLineDownRItemOffsetX;
        break;
    case proto::Orientation::left:
        tile_y_offset += game::ConveyorProp::kLineLeftRItemOffsetY;
        break;
    }


    // Right side
    switch (line_segment.terminationType) {
    case game::ConveyorSegment::TerminationType::straight:
        switch (line_segment.direction) {
        case proto::Orientation::up:
            tile_y_offset -= game::ConveyorProp::kLineLeftUpStraightItemOffset;
            break;
        case proto::Orientation::right:
            tile_x_offset += game::ConveyorProp::kLineRightDownStraightItemOffset;
            break;
        case proto::Orientation::down:
            tile_y_offset += game::ConveyorProp::kLineRightDownStraightItemOffset;
            break;
        case proto::Orientation::left:
            tile_x_offset -= game::ConveyorProp::kLineLeftUpStraightItemOffset;
            break;
        }
        break;

    case game::ConveyorSegment::TerminationType::bend_left:
        switch (line_segment.direction) {
        case proto::Orientation::up:
            tile_y_offset += game::ConveyorProp::kLineUpBlRItemOffsetY;
            break;
        case proto::Orientation::right:
            tile_x_offset += game::ConveyorProp::kLineRightBlRItemOffsetX;
            break;
        case proto::Orientation::down:
            tile_y_offset += game::ConveyorProp::kLineDownBlRItemOffsetY;
            break;
        case proto::Orientation::left:
            tile_x_offset += game::ConveyorProp::kLineLeftBlRItemOffsetX;
            break;
        }
        break;

    case game::ConveyorSegment::TerminationType::bend_right:
        switch (line_segment.direction) {
        case proto::Orientation::up:
            tile_y_offset += game::ConveyorProp::kLineUpBrRItemOffsetY;
            break;
        case proto::Orientation::right:
            tile_x_offset += game::ConveyorProp::kLineRightBrRItemOffsetX;
            break;
        case proto::Orientation::down:
            tile_y_offset += game::ConveyorProp::kLineDownBrRItemOffsetY;
            break;
        case proto::Orientation::left:
            tile_x_offset += game::ConveyorProp::kLineLeftBrRItemOffsetX;
            break;
        }
        break;

        // Side insertion
    case game::ConveyorSegment::TerminationType::right_only:
    case game::ConveyorSegment::TerminationType::left_only:
        switch (line_segment.direction) {
        case proto::Orientation::up:
            tile_y_offset += game::ConveyorProp::kLineUpSingleSideItemOffsetY;
            break;
        case proto::Orientation::right:
            tile_x_offset += game::ConveyorProp::kLineRightSingleSideItemOffsetX;
            break;
        case proto::Orientation::down:
            tile_y_offset += game::ConveyorProp::kLineDownSingleSideItemOffsetY;
            break;
        case proto::Orientation::left:
            tile_x_offset += game::ConveyorProp::kLineLeftSingleSideItemOffsetX;
            break;
        }
        break;
    }
    PrepareConveyorSegmentData(
        layer, uv_coords, line_segment, line_segment.right.lane, tile_x_offset, tile_y_offset, pixel_offset);
}

// ======================================================================

void render::DrawInserterArm(RendererLayer& layer,
                             const SpriteUvCoordsT& uv_coords,
                             const core::Position2<OverlayOffsetAxis>& pixel_offset,
                             const proto::Inserter& inserter_proto,
                             const proto::InserterData& inserter_data) {
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
    if (inserter_data.status == proto::InserterData::Status::dropoff) {
        constexpr auto held_item_pixel_offset =
            core::LossyCast<float>((Renderer::tileWidth - Renderer::tileWidth * game::ConveyorProp::kItemWidth) / 2);

        const auto& uv = Renderer::GetSpriteUvCoords(uv_coords, inserter_data.heldItem.item->sprite->internalId);

        layer.PushBack({{{pixel_offset.x + held_item_pixel_offset, pixel_offset.y + held_item_pixel_offset},
                         {pixel_offset.x + Renderer::tileWidth - held_item_pixel_offset,
                          pixel_offset.y + Renderer::tileWidth - held_item_pixel_offset}},
                        {uv.topLeft, uv.bottomRight}},
                       kPixelZ);
    }
}
