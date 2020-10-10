// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_LOGIC_CONVEYOR_PROP_H
#define JACTORIO_INCLUDE_GAME_LOGIC_CONVEYOR_PROP_H
#pragma once

namespace jactorio::game
{
    struct ConveyorProp
    {
        // For storing line offsets during transitions, items are treated as having no width

        /* Placement of items on conveyor (Expressed as decimal percentages of a tile)
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
        static constexpr double kItemWidth = 0.4;

        /// Distance left between each item when conveyor is fully compressed (in tiles)
        static constexpr double kItemSpacing = 0.25;

        // Number of tiles to offset items in order to line up on the L / R sides of the belt for all 4 directions
        // Direction is direction of item movement for the conveyor

        static constexpr double kLineBaseOffsetLeft  = 0.3;
        static constexpr double kLineBaseOffsetRight = 0.7;

        static constexpr double kLineLeftUpStraightItemOffset    = 0.25;
        static constexpr double kLineRightDownStraightItemOffset = 0.75;

        // up, right, down, left
        static constexpr double kLineUpLItemOffsetX = kLineBaseOffsetLeft - kItemWidth / 2;
        static constexpr double kLineUpRItemOffsetX = kLineBaseOffsetRight - kItemWidth / 2;

        static constexpr double kLineRightLItemOffsetY = kLineBaseOffsetLeft - kItemWidth / 2;
        static constexpr double kLineRightRItemOffsetY = kLineBaseOffsetRight - kItemWidth / 2;

        static constexpr double kLineDownLItemOffsetX = kLineBaseOffsetRight - kItemWidth / 2;
        static constexpr double kLineDownRItemOffsetX = kLineBaseOffsetLeft - kItemWidth / 2;

        static constexpr double kLineLeftLItemOffsetY = kLineBaseOffsetRight - kItemWidth / 2;
        static constexpr double kLineLeftRItemOffsetY = kLineBaseOffsetLeft - kItemWidth / 2;

        // Bend left
        static constexpr double kLineUpBlLItemOffsetY = kLineBaseOffsetRight - kItemWidth / 2;
        static constexpr double kLineUpBlRItemOffsetY = kLineBaseOffsetLeft - kItemWidth / 2;

        static constexpr double kLineRightBlLItemOffsetX = kLineBaseOffsetLeft - kItemWidth / 2;
        static constexpr double kLineRightBlRItemOffsetX = kLineBaseOffsetRight - kItemWidth / 2;

        static constexpr double kLineDownBlLItemOffsetY = kLineBaseOffsetLeft - kItemWidth / 2;
        static constexpr double kLineDownBlRItemOffsetY = kLineBaseOffsetRight - kItemWidth / 2;

        static constexpr double kLineLeftBlLItemOffsetX = kLineBaseOffsetRight - kItemWidth / 2;
        static constexpr double kLineLeftBlRItemOffsetX = kLineBaseOffsetLeft - kItemWidth / 2;

        // Bend right
        static constexpr double kLineUpBrLItemOffsetY = kLineBaseOffsetLeft - kItemWidth / 2;
        static constexpr double kLineUpBrRItemOffsetY = kLineBaseOffsetRight - kItemWidth / 2;

        static constexpr double kLineRightBrLItemOffsetX = kLineBaseOffsetRight - kItemWidth / 2;
        static constexpr double kLineRightBrRItemOffsetX = kLineBaseOffsetLeft - kItemWidth / 2;

        static constexpr double kLineDownBrLItemOffsetY = kLineBaseOffsetRight - kItemWidth / 2;
        static constexpr double kLineDownBrRItemOffsetY = kLineBaseOffsetLeft - kItemWidth / 2;

        static constexpr double kLineLeftBrLItemOffsetX = kLineBaseOffsetLeft - kItemWidth / 2;
        static constexpr double kLineLeftBrRItemOffsetX = kLineBaseOffsetRight - kItemWidth / 2;

        // Feed side (left and right lanes are the same)
        static constexpr double kLineUpSingleSideItemOffsetY    = kLineBaseOffsetRight - kItemWidth / 2;
        static constexpr double kLineRightSingleSideItemOffsetX = kLineBaseOffsetLeft - kItemWidth / 2;
        static constexpr double kLineDownSingleSideItemOffsetY  = kLineBaseOffsetLeft - kItemWidth / 2;
        static constexpr double kLineLeftSingleSideItemOffsetX  = kLineBaseOffsetRight - kItemWidth / 2;


        // When bending, the amounts below are reduced from the distance to the end of the next segment (see diagram
        // below)
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
        static constexpr double kBendLeftLReduction = 0.7;
        static constexpr double kBendLeftRReduction = 0.3;

        static constexpr double kBendRightLReduction = 0.3;
        static constexpr double kBendRightRReduction = 0.7;

        static constexpr double kTargetSideOnlyReduction = 0.7;
    };

} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_LOGIC_CONVEYOR_PROP_H
