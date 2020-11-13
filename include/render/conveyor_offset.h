// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDER_CONVEYOR_OFFSET_H
#define JACTORIO_INCLUDE_RENDER_CONVEYOR_OFFSET_H
#pragma once

#include "game/logic/conveyor_prop.h"

namespace jactorio::render
{
    struct ConveyorOffset
    {
        using ValueT = double;

        ConveyorOffset() = delete;

    private:
        // Used to line up on the L / R sides of the belt for all 4 directions
        // Direction is direction of item movement for the conveyor

        static constexpr ValueT kLBaseOffset = 0.3;
        static constexpr ValueT kRBaseOffset = 0.7;

        static constexpr ValueT kHalfItemWidth = game::ConveyorProp::kItemWidth / 2;

    public:
        struct Up
        {
            Up() = delete;

            // k <Type> <Left or right lane> <X / Y Coordinate which should be affected>


            // base offset for direction
            static constexpr ValueT kLX = kLBaseOffset - kHalfItemWidth;
            static constexpr ValueT kRX = kRBaseOffset - kHalfItemWidth;


            // Additional offset applied to the base offset for direction

            // Bend left
            static constexpr ValueT kBlLY = kRBaseOffset - kHalfItemWidth;
            static constexpr ValueT kBlRY = kLBaseOffset - kHalfItemWidth;

            // Bend right
            static constexpr ValueT kBrLY = kLBaseOffset - kHalfItemWidth;
            static constexpr ValueT kBrRY = kRBaseOffset - kHalfItemWidth;

            // Feed side (left and right lanes are the same)
            static constexpr ValueT kSfY = kRBaseOffset - kHalfItemWidth;

            // Straight
            static constexpr ValueT kSY = 0.25;
        };

        struct Right
        {
            Right() = delete;

            static constexpr ValueT kLY = kLBaseOffset - kHalfItemWidth;
            static constexpr ValueT kRY = kRBaseOffset - kHalfItemWidth;


            static constexpr ValueT kBlLX = kLBaseOffset - kHalfItemWidth;
            static constexpr ValueT kBlRX = kRBaseOffset - kHalfItemWidth;

            static constexpr ValueT kBrLX = kRBaseOffset - kHalfItemWidth;
            static constexpr ValueT kBrRX = kLBaseOffset - kHalfItemWidth;

            static constexpr ValueT kSfX = kLBaseOffset - kHalfItemWidth;


            static constexpr ValueT kSX = 0.75;
        };

        struct Down
        {
            Down() = delete;

            static constexpr ValueT kLX = kRBaseOffset - kHalfItemWidth;
            static constexpr ValueT kRX = kLBaseOffset - kHalfItemWidth;


            static constexpr ValueT kBlLY = kLBaseOffset - kHalfItemWidth;
            static constexpr ValueT kBlRY = kRBaseOffset - kHalfItemWidth;

            static constexpr ValueT kBrLY = kRBaseOffset - kHalfItemWidth;
            static constexpr ValueT kBrRY = kLBaseOffset - kHalfItemWidth;

            static constexpr ValueT kSfY = kLBaseOffset - kHalfItemWidth;


            static constexpr ValueT kSY = 0.75;
        };

        struct Left
        {
            Left() = delete;

            static constexpr ValueT kLY = kRBaseOffset - kHalfItemWidth;
            static constexpr ValueT kRY = kLBaseOffset - kHalfItemWidth;


            static constexpr ValueT kBlLX = kRBaseOffset - kHalfItemWidth;
            static constexpr ValueT kBlRX = kLBaseOffset - kHalfItemWidth;

            static constexpr ValueT kBrLX = kLBaseOffset - kHalfItemWidth;
            static constexpr ValueT kBrRX = kRBaseOffset - kHalfItemWidth;

            static constexpr ValueT kSfX = kRBaseOffset - kHalfItemWidth;


            static constexpr ValueT kSX = 0.25;
        };
    };

} // namespace jactorio::render

#endif // JACTORIO_INCLUDE_RENDER_CONVEYOR_OFFSET_H
