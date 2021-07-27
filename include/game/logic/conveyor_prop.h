// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_LOGIC_CONVEYOR_PROP_H
#define JACTORIO_INCLUDE_GAME_LOGIC_CONVEYOR_PROP_H
#pragma once

namespace jactorio::game
{
    struct ConveyorProp
    {
        using ValueT = double;

        ConveyorProp() = delete;

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
        static constexpr ValueT kItemWidth = 0.4;

        /// Distance left between each item when conveyor is fully compressed (in tiles)
        /// front of item to front of item
        static constexpr ValueT kItemSpacing = 0.25;

        /// Distance after entering splitter when items can be swapped
        /// When rendering items on splitters, prevents swapping while items still in view
        static constexpr ValueT kSplitterThreshold = 0.25;


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
        static constexpr ValueT kBendLeftLReduction = 0.7;
        static constexpr ValueT kBendLeftRReduction = 0.3;

        static constexpr ValueT kBendRightLReduction = 0.3;
        static constexpr ValueT kBendRightRReduction = 0.7;

        static constexpr ValueT kTargetSideOnlyReduction = 0.7;
    };

} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_LOGIC_CONVEYOR_PROP_H
