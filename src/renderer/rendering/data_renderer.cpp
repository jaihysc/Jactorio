// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "renderer/rendering/data_renderer.h"

#include "jactorio.h"
#include "data/prototype/type.h"
#include "data/prototype/entity/transport_line.h"
#include "game/logic/transport_line_controller.h"
#include "renderer/opengl/shader_manager.h"
#include "renderer/rendering/renderer.h"

///
/// \param tile_x Tile offset (for distance after each item)
/// \param tile_y Tile offset
/// \param x_offset Pixel offset (for aligning the sprite on screen)
/// \param y_offset Pixel offset
void PrepareTransportSegmentData(jactorio::renderer::RendererLayer& layer,
                                 const jactorio::game::TransportSegment& line_segment,
                                 std::deque<jactorio::game::TransportLineItem>& line_segment_side,
                                 float tile_x, float tile_y,
                                 const float x_offset, const float y_offset) {
	using namespace jactorio::game;

	// Either offset_x or offset_y which will be INCREASED or DECREASED
	float* target_offset;
	double multiplier = 1;  // Either 1 or -1 to add or subtract

	switch (line_segment.direction) {
	case jactorio::data::Orientation::up:
		target_offset = &tile_y;
		break;
	case jactorio::data::Orientation::right:
		target_offset = &tile_x;
		multiplier = -1;
		break;
	case jactorio::data::Orientation::down:
		target_offset = &tile_y;
		multiplier = -1;
		break;
	case jactorio::data::Orientation::left:
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

	// for (const auto& line_item : line_segment_side) {
	// 	// Move the target offset (up or down depending on multiplier)
	// 	*target_offset += line_item.first.getAsDouble() * multiplier;
	//
	// 	// TODO pass spritemap coords
	// 	const auto& uv_pos = jactorio::renderer::Renderer::GetSpritemapCoords(line_item.second->sprite->internalId);
	//
	// 	// In pixels
	// 	layer.PushBack(jactorio::renderer::RendererLayer::Element(
	// 		{
	// 			{
	// 				{
	// 					x_offset + tile_x * static_cast<float>(jactorio::renderer::Renderer::tileWidth),
	// 					y_offset + tile_y * static_cast<float>(jactorio::renderer::Renderer::tileWidth)
	// 				},
	// 				{
	// 					x_offset +
	// 					static_cast<float>(tile_x + kItemWidth) * static_cast<float>(jactorio::renderer::Renderer::tileWidth),
	// 					y_offset +
	// 					static_cast<float>(tile_y + kItemWidth) * static_cast<float>(jactorio::renderer::Renderer::tileWidth)
	// 				},
	// 			},
	// 			{uv_pos.topLeft, uv_pos.bottomRight}
	// 		}
	// 	));
	// }
}

void jactorio::renderer::DrawTransportSegmentItems(RendererLayer& layer,
                                                   const float x_offset, const float y_offset,
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
	PrepareTransportSegmentData(layer,
	                            line_segment, line_segment.left.lane,
	                            tile_x_offset, tile_y_offset,
	                            x_offset, y_offset);

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
	PrepareTransportSegmentData(layer,
	                            line_segment, line_segment.right.lane,
	                            tile_x_offset, tile_y_offset,
	                            x_offset, y_offset);
}
