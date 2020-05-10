// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#ifndef JACTORIO_INCLUDE_GAME_LOGIC_TRANSPORT_LINE_CONTROLLER_H
#define JACTORIO_INCLUDE_GAME_LOGIC_TRANSPORT_LINE_CONTROLLER_H
#pragma once

#include <decimal.h>

#include "game/world/logic_chunk.h"
#include "game/world/world_data.h"

///
/// \brief Transport line logic for anything moving items
namespace jactorio::game
{
	constexpr int kTransportLineDecimalPlace = 3;
	using TransportLineOffset = dec::decimal<kTransportLineDecimalPlace>;

	/* Placement of items on transport line (Expressed as decimal percentages of a tile)
	 * | R Padding 0.0
	 * |
	 * ------------------------------------------------- 0.1
	 *
	 * <<<<<< center of R item <<<<<<<<<<<<<<<<<<<<<<<< 0.3
	 *
	 * ====== CENTER OF BELT ========================== 0.5
	 *
	 * <<<<<< center of L item <<<<<<<<<<<<<<<<<<<<<<<< 0.7
	 * 
	 * ------------------------------------------------- 0.9
	 * |
	 * | L Padding 1.0
	 *
	 * With an item_width of 0.4f:
	 * A right item will occupy the entire space from 0.1 to 0.5
	 * A left item will occupy the entire space from 0.5 to 0.9
	 */

	/*
	 * Item wakeup:
	 *
	 * After a transport line lane stops, it cannot wake up by itself, another transport line or lane must call the member update_wakeup
	 * in transport_line_structure
	 */

	/// Width of one item on a belt (in tiles)
	constexpr double kItemWidth = 0.4f;

	/// Distance left between each item when transport line is fully compressed (in tiles)
	constexpr double kItemSpacing = 0.25f;

	// Number of tiles to offset items in order to line up on the L / R sides of the belt for all 4 directions
	// Direction is direction of item movement for the transport line

	constexpr double kLineBaseOffsetLeft  = 0.3;
	constexpr double kLineBaseOffsetRight = 0.7;

	constexpr double kLineLeftUpStraightItemOffset    = 0.25;
	constexpr double kLineRightDownStraightItemOffset = 0.75;

	// up, right, down, left
	constexpr double kLineUpLItemOffsetX = kLineBaseOffsetLeft - kItemWidth / 2;
	constexpr double kLineUpRItemOffsetX = kLineBaseOffsetRight - kItemWidth / 2;

	constexpr double kLineRightLItemOffsetY = kLineBaseOffsetLeft - kItemWidth / 2;
	constexpr double kLineRightRItemOffsetY = kLineBaseOffsetRight - kItemWidth / 2;

	constexpr double kLineDownLItemOffsetX = kLineBaseOffsetRight - kItemWidth / 2;
	constexpr double kLineDownRItemOffsetX = kLineBaseOffsetLeft - kItemWidth / 2;

	constexpr double kLineLeftLItemOffsetY = kLineBaseOffsetRight - kItemWidth / 2;
	constexpr double kLineLeftRItemOffsetY = kLineBaseOffsetLeft - kItemWidth / 2;

	// Bend left
	constexpr double kLineUpBlLItemOffsetY = kLineBaseOffsetRight - kItemWidth / 2;
	constexpr double kLineUpBlRItemOffsetY = kLineBaseOffsetLeft - kItemWidth / 2;

	constexpr double kLineRightBlLItemOffsetX = kLineBaseOffsetLeft - kItemWidth / 2;
	constexpr double kLineRightBlRItemOffsetX = kLineBaseOffsetRight - kItemWidth / 2;

	constexpr double kLineDownBlLItemOffsetY = kLineBaseOffsetLeft - kItemWidth / 2;
	constexpr double kLineDownBlRItemOffsetY = kLineBaseOffsetRight - kItemWidth / 2;

	constexpr double kLineLeftBlLItemOffsetX = kLineBaseOffsetRight - kItemWidth / 2;
	constexpr double kLineLeftBlRItemOffsetX = kLineBaseOffsetLeft - kItemWidth / 2;

	// Bend right
	constexpr double kLineUpBrLItemOffsetY = kLineBaseOffsetLeft - kItemWidth / 2;
	constexpr double kLineUpBrRItemOffsetY = kLineBaseOffsetRight - kItemWidth / 2;

	constexpr double kLineRightBrLItemOffsetX = kLineBaseOffsetRight - kItemWidth / 2;
	constexpr double kLineRightBrRItemOffsetX = kLineBaseOffsetLeft - kItemWidth / 2;

	constexpr double kLineDownBrLItemOffsetY = kLineBaseOffsetRight - kItemWidth / 2;
	constexpr double kLineDownBrRItemOffsetY = kLineBaseOffsetLeft - kItemWidth / 2;

	constexpr double kLineLeftBrLItemOffsetX = kLineBaseOffsetLeft - kItemWidth / 2;
	constexpr double kLineLeftBrRItemOffsetX = kLineBaseOffsetRight - kItemWidth / 2;

	// Feed side (left and right lanes are the same)
	constexpr double kLineUpSingleSideItemOffsetY    = kLineBaseOffsetRight - kItemWidth / 2;
	constexpr double kLineRightSingleSideItemOffsetX = kLineBaseOffsetLeft - kItemWidth / 2;
	constexpr double kLineDownSingleSideItemOffsetY  = kLineBaseOffsetLeft - kItemWidth / 2;
	constexpr double kLineLeftSingleSideItemOffsetX  = kLineBaseOffsetRight - kItemWidth / 2;

	///
	/// \brief Moves items for transport lines
	/// \param l_chunk Chunk to update
	void LogicUpdateMoveItems(LogicChunk* l_chunk);

	///
	/// \brief Transitions items on transport lines to other lines and modifies whether of not the line is active
	/// \param l_chunk Chunk to update
	void LogicUpdateTransitionItems(LogicChunk* l_chunk);


	///
	/// \brief Updates belt logic for a logic chunk
	void TransportLineLogicUpdate(WorldData& world_data);
}

#endif //JACTORIO_INCLUDE_GAME_LOGIC_TRANSPORT_LINE_CONTROLLER_H
