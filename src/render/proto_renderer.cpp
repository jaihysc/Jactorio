// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "render/proto_renderer.h"

#include "jactorio.h"

#include "game/logic/conveyor_struct.h"
#include "proto/inserter.h"
#include "proto/sprite.h"
#include "render/conveyor_offset.h"
#include "render/renderer.h"

using namespace jactorio;

constexpr float kPixelZ = 0.1f;

///
/// \param tile_x Tile offset (for distance after each item)
/// \param tile_y Tile offset
void PrepareConveyorSegmentData(render::RendererLayer& layer,
                                const SpriteUvCoordsT& uv_coords,
                                const game::ConveyorStruct& line_segment,
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
    if (line_segment.terminationType != ConveyorStruct::TerminationType::straight) {
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
                                      game::ConveyorStruct& line_segment) {
    double tile_x_offset = 0;
    double tile_y_offset = 0;

    // Don't render if items are not marked visible! Wow!
    if (!line_segment.left.visible)
        goto prepare_right;

    // Left
    // The offsets for straight are always applied to bend left and right
    switch (line_segment.direction) {
    case proto::Orientation::up:
        tile_x_offset += ConveyorOffset::Up::kLX;
        break;
    case proto::Orientation::right:
        tile_y_offset += ConveyorOffset::Right::kLY;
        break;
    case proto::Orientation::down:
        tile_x_offset += ConveyorOffset::Down::kLX;
        break;
    case proto::Orientation::left:
        tile_y_offset += ConveyorOffset::Left::kLY;
        break;
    }

    // Left side
    switch (line_segment.terminationType) {
    case game::ConveyorStruct::TerminationType::straight:
        switch (line_segment.direction) {
        case proto::Orientation::up:
            tile_y_offset -= ConveyorOffset::Up::kSY;
            break;
        case proto::Orientation::right:
            tile_x_offset += ConveyorOffset::Right::kSX;
            break;
        case proto::Orientation::down:
            tile_y_offset += ConveyorOffset::Down::kSY;
            break;
        case proto::Orientation::left:
            tile_x_offset -= ConveyorOffset::Left::kSX;
            break;
        }
        break;

    case game::ConveyorStruct::TerminationType::bend_left:
        switch (line_segment.direction) {
        case proto::Orientation::up:
            tile_y_offset += ConveyorOffset::Up::kBlLY;
            break;
        case proto::Orientation::right:
            tile_x_offset += ConveyorOffset::Right::kBlLX;
            break;
        case proto::Orientation::down:
            tile_y_offset += ConveyorOffset::Down::kBlLY;
            break;
        case proto::Orientation::left:
            tile_x_offset += ConveyorOffset::Left::kBlLX;
            break;
        }
        break;

    case game::ConveyorStruct::TerminationType::bend_right:
        switch (line_segment.direction) {
        case proto::Orientation::up:
            tile_y_offset += ConveyorOffset::Up::kBrLY;
            break;
        case proto::Orientation::right:
            tile_x_offset += ConveyorOffset::Right::kBrLX;
            break;
        case proto::Orientation::down:
            tile_y_offset += ConveyorOffset::Down::kBrLY;
            break;
        case proto::Orientation::left:
            tile_x_offset += ConveyorOffset::Left::kBrLX;
            break;
        }
        break;

        // Side insertion
    case game::ConveyorStruct::TerminationType::right_only:
    case game::ConveyorStruct::TerminationType::left_only:
        switch (line_segment.direction) {
        case proto::Orientation::up:
            tile_y_offset += ConveyorOffset::Up::kSfY;
            break;
        case proto::Orientation::right:
            tile_x_offset += ConveyorOffset::Right::kSfX;
            break;
        case proto::Orientation::down:
            tile_y_offset += ConveyorOffset::Down::kSfY;
            break;
        case proto::Orientation::left:
            tile_x_offset += ConveyorOffset::Left::kSfX;
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
        tile_x_offset += ConveyorOffset::Up::kRX;
        break;
    case proto::Orientation::right:
        tile_y_offset += ConveyorOffset::Right::kRY;
        break;
    case proto::Orientation::down:
        tile_x_offset += ConveyorOffset::Down::kRX;
        break;
    case proto::Orientation::left:
        tile_y_offset += ConveyorOffset::Left::kRY;
        break;
    }


    // Right side
    switch (line_segment.terminationType) {
    case game::ConveyorStruct::TerminationType::straight:
        switch (line_segment.direction) {
        case proto::Orientation::up:
            tile_y_offset -= ConveyorOffset::Up::kSY;
            break;
        case proto::Orientation::right:
            tile_x_offset += ConveyorOffset::Right::kSX;
            break;
        case proto::Orientation::down:
            tile_y_offset += ConveyorOffset::Down::kSY;
            break;
        case proto::Orientation::left:
            tile_x_offset -= ConveyorOffset::Left::kSX;
            break;
        }
        break;

    case game::ConveyorStruct::TerminationType::bend_left:
        switch (line_segment.direction) {
        case proto::Orientation::up:
            tile_y_offset += ConveyorOffset::Up::kBlRY;
            break;
        case proto::Orientation::right:
            tile_x_offset += ConveyorOffset::Right::kBlRX;
            break;
        case proto::Orientation::down:
            tile_y_offset += ConveyorOffset::Down::kBlRY;
            break;
        case proto::Orientation::left:
            tile_x_offset += ConveyorOffset::Left::kBlRX;
            break;
        }
        break;

    case game::ConveyorStruct::TerminationType::bend_right:
        switch (line_segment.direction) {
        case proto::Orientation::up:
            tile_y_offset += ConveyorOffset::Up::kBrRY;
            break;
        case proto::Orientation::right:
            tile_x_offset += ConveyorOffset::Right::kBrRX;
            break;
        case proto::Orientation::down:
            tile_y_offset += ConveyorOffset::Down::kBrRY;
            break;
        case proto::Orientation::left:
            tile_x_offset += ConveyorOffset::Left::kBrRX;
            break;
        }
        break;

        // Side insertion
    case game::ConveyorStruct::TerminationType::right_only:
    case game::ConveyorStruct::TerminationType::left_only:
        switch (line_segment.direction) {
        case proto::Orientation::up:
            tile_y_offset += ConveyorOffset::Up::kSfY;
            break;
        case proto::Orientation::right:
            tile_x_offset += ConveyorOffset::Right::kSfX;
            break;
        case proto::Orientation::down:
            tile_y_offset += ConveyorOffset::Down::kSfY;
            break;
        case proto::Orientation::left:
            tile_x_offset += ConveyorOffset::Left::kSfX;
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
