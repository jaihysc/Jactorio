// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_LOGIC_TRANSPORT_LINE_CONTROLLER_H
#define JACTORIO_INCLUDE_GAME_LOGIC_TRANSPORT_LINE_CONTROLLER_H
#pragma once

namespace jactorio::game
{
    class WorldData;

    /// Transport line logic for anything moving items

    // For storing line offsets during transitions, items are treated as having no width

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

    /// Width of one item on a belt (in tiles)
    constexpr double kItemWidth = 0.4;

    /// Distance left between each item when transport line is fully compressed (in tiles)
    constexpr double kItemSpacing = 0.25;

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


    // When bending, the amounts below are reduced from the distance to the end of the next segment (see diagram below)
    //
    // === 0.7 ===
    // =0.3=
    //     ------------------------->
    //     ^         *
    //     |    -------------------->
    //     |    ^    *
    //     |    |    *
    //     |    |    *
    //
    constexpr double kBendLeftLReduction = 0.7;
    constexpr double kBendLeftRReduction = 0.3;

    constexpr double kBendRightLReduction = 0.3;
    constexpr double kBendRightRReduction = 0.7;

    constexpr double kTargetSideOnlyReduction = 0.7;


    // ======================================================================


    ///
    /// Updates belt logic for a logic chunk
    void TransportLineLogicUpdate(WorldData& world_data);
} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_LOGIC_TRANSPORT_LINE_CONTROLLER_H
