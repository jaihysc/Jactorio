// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "render/proto_renderer.h"

#include "game/logic/conveyor_struct.h"
#include "proto/inserter.h"
#include "proto/sprite.h"
#include "render/conveyor_offset.h"
#include "render/imgui_renderer.h"
#include "render/tile_renderer.h"

using namespace jactorio;

/// \param tile_offset Tile offset (for distance after each item)
static void PrepareConveyorSegmentData(render::IRenderBuffer& buf,
                                       const SpriteTexCoords& tex_coords,
                                       const game::ConveyorStruct& conveyor,
                                       std::deque<game::ConveyorItem>& conveyor_lane,
                                       Position2<double> tile_offset,
                                       const Position2<OverlayOffsetAxis>& pixel_offset) {
    using namespace game;

    // Either offset_x or offset_y which will be INCREASED or DECREASED
    double* target_offset;
    double multiplier = 1; // Either 1 or -1 to add or subtract

    switch (conveyor.direction) {
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
    if (conveyor.terminationType != ConveyorStruct::TerminationType::straight) {
        Position2Increment(conveyor.direction, tile_offset, 1);
    }

    for (const auto& [dist, item] : conveyor_lane) {
        // Move the target offset (up or down depending on multiplier)
        *target_offset += dist.getAsDouble() * multiplier;

        // tl = Top left; br = Bottom right
        constexpr auto f_tile_width = static_cast<float>(render::TileRenderer::tileWidth);
        const auto tl               = Position2{
            pixel_offset.x + LossyCast<float>(tile_offset.x) * f_tile_width,
            pixel_offset.y + LossyCast<float>(tile_offset.y) * f_tile_width,
        };
        const auto br = Position2{
            pixel_offset.x + LossyCast<float>(tile_offset.x + ConveyorProp::kItemWidth) * f_tile_width,
            pixel_offset.y + LossyCast<float>(tile_offset.y + ConveyorProp::kItemWidth) * f_tile_width,
        };

        const auto& uv = tex_coords[item->sprite->texCoordId];


        const auto index = buf.vert.size();

        buf.vert.push_back({{tl.x, tl.y}, //
                            {uv.topLeft.x, uv.topLeft.y},
                            IM_COL32(255, 255, 255, 255)});
        buf.vert.push_back({{tl.x, br.y}, //
                            {uv.topLeft.x, uv.bottomRight.y},
                            IM_COL32(255, 255, 255, 255)});
        buf.vert.push_back({{br.x, br.y}, //
                            {uv.bottomRight.x, uv.bottomRight.y},
                            IM_COL32(255, 255, 255, 255)});
        buf.vert.push_back({{br.x, tl.y}, //
                            {uv.bottomRight.x, uv.topLeft.y},
                            IM_COL32(255, 255, 255, 255)});

        buf.idx.push_back(index);
        buf.idx.push_back(index + 1);
        buf.idx.push_back(index + 2);
        buf.idx.push_back(index + 2);
        buf.idx.push_back(index + 3);
        buf.idx.push_back(index);
    }
}

static void PrepareConveyorSegmentItemsLeft(render::IRenderBuffer& buf,
                                            const SpriteTexCoords& tex_coords,
                                            const Position2<OverlayOffsetAxis>& pixel_offset,
                                            game::ConveyorStruct& conveyor) {
    using namespace render;

    Position2<double> tile_offset;

    // Don't render if items are not marked visible! Wow!
    if (!conveyor.left.visible) {
        return;
    }

    // Left
    // The offsets for straight are always applied to bend left and right
    switch (conveyor.direction) {
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
    switch (conveyor.terminationType) {
    case game::ConveyorStruct::TerminationType::straight:
        switch (conveyor.direction) {
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
        switch (conveyor.direction) {
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
        switch (conveyor.direction) {
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
        switch (conveyor.direction) {
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
    PrepareConveyorSegmentData(buf, tex_coords, conveyor, conveyor.left.lane, tile_offset, pixel_offset);
}

static void PrepareConveyorSegmentItemsRight(render::IRenderBuffer& buf,
                                             const SpriteTexCoords& tex_coords,
                                             const Position2<OverlayOffsetAxis>& pixel_offset,
                                             game::ConveyorStruct& conveyor) {
    using namespace render;
    Position2<double> tile_offset;

    if (!conveyor.right.visible) {
        return;
    }

    // The offsets for straight are always applied to bend left and right
    switch (conveyor.direction) {
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
    switch (conveyor.terminationType) {
    case game::ConveyorStruct::TerminationType::straight:
        switch (conveyor.direction) {
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
        switch (conveyor.direction) {
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
        switch (conveyor.direction) {
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
        switch (conveyor.direction) {
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
    PrepareConveyorSegmentData(buf, tex_coords, conveyor, conveyor.right.lane, tile_offset, pixel_offset);
}

void render::PrepareConveyorSegmentItems(IRenderBuffer& buf,
                                         const SpriteTexCoords& tex_coords,
                                         const Position2<OverlayOffsetAxis>& pixel_offset,
                                         game::ConveyorStruct& conveyor) {
    PrepareConveyorSegmentItemsLeft(buf, tex_coords, pixel_offset, conveyor);
    PrepareConveyorSegmentItemsRight(buf, tex_coords, pixel_offset, conveyor);
}

// ======================================================================

void render::DrawInserterArm(IRenderBuffer& buf,
                             const SpriteTexCoords& uv_coords,
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
        buf.PushBack({{// Cover tile
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

        buf.PushBack({{{pixel_offset.x + held_item_pixel_offset, pixel_offset.y + held_item_pixel_offset},
                         {pixel_offset.x + Renderer::tileWidth - held_item_pixel_offset,
                          pixel_offset.y + Renderer::tileWidth - held_item_pixel_offset}},
                        {uv.topLeft, uv.bottomRight}},
                       kPixelZ);
    }
    */
}
