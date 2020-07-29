// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "renderer/rendering/data_renderer.h"

#include "jactorio.h"
#include "data/prototype/type.h"
#include "data/prototype/entity/transport_line.h"
#include "game/logic/transport_line_controller.h"
#include "renderer/rendering/renderer.h"

using namespace jactorio;

constexpr float kPixelZ = 0.1f;

///
/// \param tile_x Tile offset (for distance after each item)
/// \param tile_y Tile offset
void PrepareTransportSegmentData(renderer::RendererLayer& layer, const SpriteUvCoordsT& uv_coords,
                                 const game::TransportSegment& line_segment,
                                 std::deque<game::TransportLineItem>& line_segment_side,
                                 float tile_x, float tile_y,
                                 const core::Position2<OverlayOffsetAxis>& pixel_offset) {
	using namespace game;

	// Either offset_x or offset_y which will be INCREASED or DECREASED
	float* target_offset;
	double multiplier = 1;  // Either 1 or -1 to add or subtract

	switch (line_segment.direction) {
	case data::Orientation::up:
		target_offset = &tile_y;
		break;
	case data::Orientation::right:
		target_offset = &tile_x;
		multiplier = -1;
		break;
	case data::Orientation::down:
		target_offset = &tile_y;
		multiplier = -1;
		break;
	case data::Orientation::left:
		target_offset = &tile_x;
		break;

	default:
		assert(false);  // Missing switch case
		break;
	}

	// Shift items 1 tile forwards if segment bends
	if (line_segment.terminationType != TransportSegment::TerminationType::straight) {
		OrientationIncrement(line_segment.direction, tile_x, tile_y);
	}

	for (const auto& line_item : line_segment_side) {
		// Move the target offset (up or down depending on multiplier)
		*target_offset += line_item.first.getAsDouble() * multiplier;

		const auto& uv_pos = renderer::Renderer::GetSpriteUvCoords(uv_coords, line_item.second->sprite->internalId);

		constexpr float pixel_z = kPixelZ;
		// In pixels
		layer.PushBack(
			{
				{
					{
						pixel_offset.x + tile_x * static_cast<float>(renderer::Renderer::tileWidth),
						pixel_offset.y + tile_y * static_cast<float>(renderer::Renderer::tileWidth),
					},
					{
						pixel_offset.x +
						static_cast<float>(tile_x + kItemWidth) * static_cast<float>(renderer::Renderer::tileWidth),
						pixel_offset.y +
						static_cast<float>(tile_y + kItemWidth) * static_cast<float>(renderer::Renderer::tileWidth),
					},
				},
				{uv_pos.topLeft, uv_pos.bottomRight}
			}
			,
			pixel_z);
	}
}

void renderer::DrawTransportSegmentItems(RendererLayer& layer, const SpriteUvCoordsT& uv_coords,
                                         const core::Position2<OverlayOffsetAxis>& pixel_offset,
                                         game::TransportSegment& line_segment) {
	float tile_x_offset = 0;
	float tile_y_offset = 0;

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
	PrepareTransportSegmentData(layer, uv_coords,
	                            line_segment, line_segment.left.lane,
	                            tile_x_offset, tile_y_offset,
	                            pixel_offset);

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
	PrepareTransportSegmentData(layer, uv_coords,
	                            line_segment, line_segment.right.lane,
	                            tile_x_offset, tile_y_offset,
	                            pixel_offset);
}

// ======================================================================

void renderer::DrawInserterArm(RendererLayer& layer, const SpriteUvCoordsT& uv_coords,
                               const core::Position2<OverlayOffsetAxis>& pixel_offset, const data::Inserter& inserter_proto,
                               const data::InserterData& inserter_data) {
	const auto& uv = Renderer::GetSpriteUvCoords(uv_coords, inserter_proto.handSprite->internalId);

	constexpr int arm_width        = 2;
	constexpr int arm_pixel_offset = (Renderer::tileWidth - arm_width) / 2;

	// Ensures arm is always facing pickup / dropoff
	const float rotation_offset = static_cast<float>(inserter_data.orientation) * 90;

	layer.PushBack(
		{
			{  // Cover tile
				{
					pixel_offset.x + arm_pixel_offset,
					pixel_offset.y + arm_pixel_offset
				},
				{
					pixel_offset.x + Renderer::tileWidth - arm_pixel_offset,
					pixel_offset.y + Renderer::tileWidth - arm_pixel_offset
				}
			},
			{
				uv.topLeft,
				uv.bottomRight
			}
		}, kPixelZ, static_cast<float>(inserter_data.rotationDegree.getAsDouble() + rotation_offset)
	);
}
