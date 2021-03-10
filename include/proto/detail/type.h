// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_PROTO_DETAIL_TYPE_H
#define JACTORIO_INCLUDE_PROTO_DETAIL_TYPE_H
#pragma once

#include "core/data_type.h"

namespace jactorio::proto
{
    using ProtoFloatT = double;
    using ProtoIntT   = int32_t;
    using ProtoUintT  = uint32_t;

    // Defines types for prototype classes

    /// Conveyor item distance
    using LineDistT       = Decimal3T;
    using RotationDegreeT = Decimal3T;

    /// <Entry direction>_<Exit direction>
    enum class LineOrientation
    {
        // Following the sets of the sprite
        up_left  = 10,
        up       = 2,
        up_right = 8,

        right_up   = 6,
        right      = 0,
        right_down = 11,

        down_right = 5,
        down       = 3,
        down_left  = 7,

        left_down = 9,
        left      = 1,
        left_up   = 4,
    };
} // namespace jactorio::proto

#endif // JACTORIO_INCLUDE_PROTO_DETAIL_TYPE_H
