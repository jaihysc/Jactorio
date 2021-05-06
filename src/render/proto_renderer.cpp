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

/// \param tile_offset Tile offset (for distance after each item)
void PrepareConveyorSegmentData(render::RendererLayer& layer,
                                const SpriteUvCoordsT& uv_coords,
                                const game::ConveyorStruct& line_segment,
                                std::deque<game::ConveyorItem>& line_segment_side,
                                Position2<double> tile_offset,
                                const Position2<OverlayOffsetAxis>& pixel_offset) {
    using namespace game;

    // Either offset_x or offset_y which will be INCREASED or DECREASED
    double* target_offset;
    double multiplier = 1; // Either 1 or -1 to add or subtract

    switch (line_segment.direction) {
    case Orientation::up:
        target_offset = &tile_offset.y;
        break;
    case Orientation::right:
        target_offset = &tile_offset.x;
        multiplier    = -1;
        break;
    case Orientation::down:
        target_offset = &tile_offset.y;
        multiplier    = -1;
        break;
    case Orientation::left:
        target_offset = &tile_offset.x;
        break;

    default:
        assert(false); // Missing switch case
        target_offset = &tile_offset.y;
        break;
    }

    // Shift items 1 tile forwards if segment bends
    if (line_segment.terminationType != ConveyorStruct::TerminationType::straight) {
        Position2Increment(line_segment.direction, tile_offset, 1);
    }

    for (const auto& line_item : line_segment_side) {
        // Move the target offset (up or down depending on multiplier)
        *target_offset += line_item.dist.getAsDouble() * multiplier;

        constexpr float pixel_z = kPixelZ;
        // In pixels

        /* // TODO removed
        layer.PushBack(
            {{
                 {
                     pixel_offset.x + LossyCast<float>(tile_offset.x) * SafeCast<float>(render::Renderer::tileWidth),
                     pixel_offset.y + LossyCast<float>(tile_offset.y) * SafeCast<float>(render::Renderer::tileWidth),
                 },
                 {
                     pixel_offset.x +
                         LossyCast<float>(tile_offset.x + ConveyorProp::kItemWidth) *
                             SafeCast<float>(render::Renderer::tileWidth),
                     pixel_offset.y +
                         LossyCast<float>(tile_offset.y + ConveyorProp::kItemWidth) *
                             SafeCast<float>(render::Renderer::tileWidth),
                 },
             },
             {uv_pos.topLeft, uv_pos.bottomRight}},
            pixel_z);
            */
    }
}

void render::DrawConveyorSegmentItems(RendererLayer& layer,
                                      const SpriteUvCoordsT& uv_coords,
                                      const Position2<OverlayOffsetAxis>& pixel_offset,
                                      game::ConveyorStruct& line_segment) {
    Position2<double> tile_offset;

    // Don't render if items are not marked visible! Wow!
    if (!line_segment.left.visible)
        goto prepare_right;

    // Left
    // The offsets for straight are always applied to bend left and right
    switch (line_segment.direction) {
    case Orientation::up:
        tile_offset.x += ConveyorOffset::Up::kLX;
        break;
    case Orientation::right:
        tile_offset.y += ConveyorOffset::Right::kLY;
        break;
    case Orientation::down:
        tile_offset.x += ConveyorOffset::Down::kLX;
        break;
    case Orientation::left:
        tile_offset.y += ConveyorOffset::Left::kLY;
        break;
    }

    // Left side
    switch (line_segment.terminationType) {
    case game::ConveyorStruct::TerminationType::straight:
        switch (line_segment.direction) {
        case Orientation::up:
            tile_offset.y -= ConveyorOffset::Up::kSY;
            break;
        case Orientation::right:
            tile_offset.x += ConveyorOffset::Right::kSX;
            break;
        case Orientation::down:
            tile_offset.y += ConveyorOffset::Down::kSY;
            break;
        case Orientation::left:
            tile_offset.x -= ConveyorOffset::Left::kSX;
            break;
        }
        break;

    case game::ConveyorStruct::TerminationType::bend_left:
        switch (line_segment.direction) {
        case Orientation::up:
            tile_offset.y += ConveyorOffset::Up::kBlLY;
            break;
        case Orientation::right:
            tile_offset.x += ConveyorOffset::Right::kBlLX;
            break;
        case Orientation::down:
            tile_offset.y += ConveyorOffset::Down::kBlLY;
            break;
        case Orientation::left:
            tile_offset.x += ConveyorOffset::Left::kBlLX;
            break;
        }
        break;

    case game::ConveyorStruct::TerminationType::bend_right:
        switch (line_segment.direction) {
        case Orientation::up:
            tile_offset.y += ConveyorOffset::Up::kBrLY;
            break;
        case Orientation::right:
            tile_offset.x += ConveyorOffset::Right::kBrLX;
            break;
        case Orientation::down:
            tile_offset.y += ConveyorOffset::Down::kBrLY;
            break;
        case Orientation::left:
            tile_offset.x += ConveyorOffset::Left::kBrLX;
            break;
        }
        break;

        // Side insertion
    case game::ConveyorStruct::TerminationType::right_only:
    case game::ConveyorStruct::TerminationType::left_only:
        switch (line_segment.direction) {
        case Orientation::up:
            tile_offset.y += ConveyorOffset::Up::kSfY;
            break;
        case Orientation::right:
            tile_offset.x += ConveyorOffset::Right::kSfX;
            break;
        case Orientation::down:
            tile_offset.y += ConveyorOffset::Down::kSfY;
            break;
        case Orientation::left:
            tile_offset.x += ConveyorOffset::Left::kSfX;
            break;
        }
        break;
    }
    PrepareConveyorSegmentData(layer, uv_coords, line_segment, line_segment.left.lane, tile_offset, pixel_offset);

prepare_right:
    if (!line_segment.right.visible)
        return;

    // Right
    tile_offset.x = 0;
    tile_offset.y = 0;

    // The offsets for straight are always applied to bend left and right
    switch (line_segment.direction) {
    case Orientation::up:
        tile_offset.x += ConveyorOffset::Up::kRX;
        break;
    case Orientation::right:
        tile_offset.y += ConveyorOffset::Right::kRY;
        break;
    case Orientation::down:
        tile_offset.x += ConveyorOffset::Down::kRX;
        break;
    case Orientation::left:
        tile_offset.y += ConveyorOffset::Left::kRY;
        break;
    }


    // Right side
    switch (line_segment.terminationType) {
    case game::ConveyorStruct::TerminationType::straight:
        switch (line_segment.direction) {
        case Orientation::up:
            tile_offset.y -= ConveyorOffset::Up::kSY;
            break;
        case Orientation::right:
            tile_offset.x += ConveyorOffset::Right::kSX;
            break;
        case Orientation::down:
            tile_offset.y += ConveyorOffset::Down::kSY;
            break;
        case Orientation::left:
            tile_offset.x -= ConveyorOffset::Left::kSX;
            break;
        }
        break;

    case game::ConveyorStruct::TerminationType::bend_left:
        switch (line_segment.direction) {
        case Orientation::up:
            tile_offset.y += ConveyorOffset::Up::kBlRY;
            break;
        case Orientation::right:
            tile_offset.x += ConveyorOffset::Right::kBlRX;
            break;
        case Orientation::down:
            tile_offset.y += ConveyorOffset::Down::kBlRY;
            break;
        case Orientation::left:
            tile_offset.x += ConveyorOffset::Left::kBlRX;
            break;
        }
        break;

    case game::ConveyorStruct::TerminationType::bend_right:
        switch (line_segment.direction) {
        case Orientation::up:
            tile_offset.y += ConveyorOffset::Up::kBrRY;
            break;
        case Orientation::right:
            tile_offset.x += ConveyorOffset::Right::kBrRX;
            break;
        case Orientation::down:
            tile_offset.y += ConveyorOffset::Down::kBrRY;
            break;
        case Orientation::left:
            tile_offset.x += ConveyorOffset::Left::kBrRX;
            break;
        }
        break;

        // Side insertion
    case game::ConveyorStruct::TerminationType::right_only:
    case game::ConveyorStruct::TerminationType::left_only:
        switch (line_segment.direction) {
        case Orientation::up:
            tile_offset.y += ConveyorOffset::Up::kSfY;
            break;
        case Orientation::right:
            tile_offset.x += ConveyorOffset::Right::kSfX;
            break;
        case Orientation::down:
            tile_offset.y += ConveyorOffset::Down::kSfY;
            break;
        case Orientation::left:
            tile_offset.x += ConveyorOffset::Left::kSfX;
            break;
        }
        break;
    }
    PrepareConveyorSegmentData(layer, uv_coords, line_segment, line_segment.right.lane, tile_offset, pixel_offset);
}

// ======================================================================

void render::DrawInserterArm(RendererLayer& layer,
                             const SpriteUvCoordsT& uv_coords,
                             const Position2<OverlayOffsetAxis>& pixel_offset,
                             const proto::Inserter& inserter_proto,
                             const proto::InserterData& inserter_data) {

    // TODO removed
    /*
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
                       LossyCast<float>(inserter_data.rotationDegree.getAsDouble() + rotation_offset));
    }


    // Held item
    if (inserter_data.status == proto::InserterData::Status::dropoff) {
        constexpr auto held_item_pixel_offset =
            LossyCast<float>((Renderer::tileWidth - Renderer::tileWidth * game::ConveyorProp::kItemWidth) / 2);

        const auto& uv = Renderer::GetSpriteUvCoords(uv_coords, inserter_data.heldItem.item->sprite->internalId);

        layer.PushBack({{{pixel_offset.x + held_item_pixel_offset, pixel_offset.y + held_item_pixel_offset},
                         {pixel_offset.x + Renderer::tileWidth - held_item_pixel_offset,
                          pixel_offset.y + Renderer::tileWidth - held_item_pixel_offset}},
                        {uv.topLeft, uv.bottomRight}},
                       kPixelZ);
    }
    */
}
