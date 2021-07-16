// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "render/proto_renderer.h"

#include <glm/gtx/rotate_vector.hpp>

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

void render::PrepareInserterParts(IRenderBuffer& buf,
                                  const SpriteTexCoords& tex_coords,
                                  const Position2<OverlayOffsetAxis>& pixel_offset,
                                  const proto::Inserter& inserter,
                                  const proto::InserterData& inserter_data) {
    // These constants are of a tile
    constexpr auto arm_width  = 0.5f;
    constexpr auto arm_length = 0.9f; // Since arm is centered, arm sticks out of the tile by 0.4

    // Since center of inserter on sprite is NOT center of tile, offset a little so arm lines up with inserter
    constexpr auto arm_offset = glm::vec2{-0.05, -0.05f};


    constexpr auto arm_pixel_offset = arm_offset * glm::vec2{static_cast<float>(TileRenderer::tileWidth)};

    constexpr auto arm_pixel_width = arm_width * TileRenderer::tileWidth;
    const auto arm_pixel_length    = arm_length * TileRenderer::tileWidth +
        SafeCast<float>(TileRenderer::tileWidth) * SafeCast<float>(inserter.tileReach - 1);
    // ^^^ Accounts for arm lengths > 1

    // Rotation from 12:00 position
    const auto rotation_rad = glm::radians(LossyCast<float>(inserter_data.rotationDegree.getAsDouble()) +
                                           static_cast<float>(inserter_data.orientation) * 90);

    // To world space
    const auto transform = glm::vec2{pixel_offset.x + static_cast<float>(TileRenderer::tileWidth) / 2,
                                     pixel_offset.y + static_cast<float>(TileRenderer::tileWidth) / 2} +
        arm_pixel_offset;

    // Arm
    {
        const auto& uv = tex_coords[inserter.handSprite->texCoordId];

        // Center of rotation is the end of the arm
        // Thus center end of arm at 0,0
        auto tl = glm::vec2{-arm_pixel_width / 2, -arm_pixel_length};
        auto br = glm::vec2{arm_pixel_width / 2, 0.f};
        auto tr = glm::vec2{br.x, tl.y};
        auto bl = glm::vec2{tl.x, br.y};

        tl = glm::rotate(tl, rotation_rad);
        tr = glm::rotate(tr, rotation_rad);
        bl = glm::rotate(bl, rotation_rad);
        br = glm::rotate(br, rotation_rad);

        tl += transform;
        tr += transform;
        bl += transform;
        br += transform;

        const auto index = buf.vert.size();

        buf.vert.push_back({{tl.x, tl.y}, //
                            {uv.topLeft.x, uv.topLeft.y},
                            IM_COL32(255, 255, 255, 255)});
        buf.vert.push_back({{bl.x, bl.y}, //
                            {uv.topLeft.x, uv.bottomRight.y},
                            IM_COL32(255, 255, 255, 255)});
        buf.vert.push_back({{br.x, br.y}, //
                            {uv.bottomRight.x, uv.bottomRight.y},
                            IM_COL32(255, 255, 255, 255)});
        buf.vert.push_back({{tr.x, tr.y}, //
                            {uv.bottomRight.x, uv.topLeft.y},
                            IM_COL32(255, 255, 255, 255)});

        buf.idx.push_back(index);
        buf.idx.push_back(index + 1);
        buf.idx.push_back(index + 2);
        buf.idx.push_back(index + 2);
        buf.idx.push_back(index + 3);
        buf.idx.push_back(index);
    }

    // Held item
    if (inserter_data.status == proto::InserterData::Status::dropoff) {
        // Render the top of item to align with the top of inserter arm
        constexpr auto item_pixel_width = static_cast<float>(TileRenderer::tileWidth) * game::ConveyorProp::kItemWidth;

        /*
        // To always have the item upright when picking up:
        // 1. Rotate item in opposite direction, so when rotated to arm item is upright
        // 2. Transform to arm top
        // 3. Rotate item to arm position
        // 4. Transform to world

        // Removed since it is not very noticeable
        // Also, when it drop off onto a conveyor, it is upside down :(

        auto tl = glm::vec2{-item_pixel_width / 2, -item_pixel_width / 2};
        auto br = glm::vec2{item_pixel_width / 2, item_pixel_width / 2};
        auto tr = glm::vec2{br.x, tl.y};
        auto bl = glm::vec2{tl.x, br.y};

        // When inserter is up, must rotate 180 deg since pickup is facing down
        const auto item_rotation_rad = glm::radians(180 - static_cast<float>(inserter_data.orientation) * 90);

        tl = glm::rotate(tl, item_rotation_rad);
        tr = glm::rotate(tr, item_rotation_rad);
        bl = glm::rotate(bl, item_rotation_rad);
        br = glm::rotate(br, item_rotation_rad);

        // To 12:00 position of arm
        const auto transform_to_arm = glm::vec2{0, -arm_pixel_length + item_pixel_width / 2};
        tl += transform_to_arm;
        tr += transform_to_arm;
        bl += transform_to_arm;
        br += transform_to_arm;
        */

        auto tl = glm::vec2{-item_pixel_width / 2, -arm_pixel_length};
        auto br = glm::vec2{item_pixel_width / 2, -arm_pixel_length + item_pixel_width};
        auto tr = glm::vec2{br.x, tl.y};
        auto bl = glm::vec2{tl.x, br.y};

        tl = glm::rotate(tl, rotation_rad);
        tr = glm::rotate(tr, rotation_rad);
        bl = glm::rotate(bl, rotation_rad);
        br = glm::rotate(br, rotation_rad);

        tl += transform;
        tr += transform;
        bl += transform;
        br += transform;

        const auto& uv   = tex_coords[inserter_data.heldItem.item->sprite->texCoordId];
        const auto index = buf.vert.size();

        buf.vert.push_back({{tl.x, tl.y}, //
                            {uv.topLeft.x, uv.topLeft.y},
                            IM_COL32(255, 255, 255, 255)});
        buf.vert.push_back({{bl.x, bl.y}, //
                            {uv.topLeft.x, uv.bottomRight.y},
                            IM_COL32(255, 255, 255, 255)});
        buf.vert.push_back({{br.x, br.y}, //
                            {uv.bottomRight.x, uv.bottomRight.y},
                            IM_COL32(255, 255, 255, 255)});
        buf.vert.push_back({{tr.x, tr.y}, //
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
