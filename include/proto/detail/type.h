// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_PROTO_DETAIL_TYPE_H
#define JACTORIO_INCLUDE_PROTO_DETAIL_TYPE_H
#pragma once

#include <type_traits>

#include "jactorio.h"

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

    enum class Orientation
    {
        up = 0,
        right,
        down,
        left
    };

    inline int InvertOrientation(const int orientation) {
        assert(0 <= orientation && orientation <= 3);
        return (orientation + 2) % 4;
    }

    inline Orientation InvertOrientation(Orientation orientation) {
        return static_cast<Orientation>(InvertOrientation(static_cast<int>(orientation)));
    }

    ///
    /// Increments or decrements Ty var depending on orientation.
    /// up--, right++, down++, left--
    /// \remark Type must implement operator+= and operator-=
    /// \remark if incrementer type not provided, Typeof x will bw used
    template <typename TyX, typename TyY, typename TyInc = TyX>
    void OrientationIncrement(const Orientation orientation, TyX& x, TyY& y, TyInc increment = 1) {
        static_assert(std::is_same<TyX, TyY>::value);
        static_assert(std::is_signed<TyX>::value);

        switch (orientation) {
        case Orientation::up:
            y -= static_cast<TyX>(increment);
            break;
        case Orientation::right:
            x += static_cast<TyX>(increment);
            break;
        case Orientation::down:
            y += static_cast<TyX>(increment);
            break;
        case Orientation::left:
            x -= static_cast<TyX>(increment);
            break;

        default:
            assert(false);
            break;
        }
    }

    inline const char* OrientationToStr(const Orientation orientation) {
        switch (orientation) {
        case Orientation::up:
            return "Up";
        case Orientation::right:
            return "Right";
        case Orientation::down:
            return "Down";
        case Orientation::left:
            return "Left";

        default:
            assert(false); // Missing switch case
            break;
        }

        return "";
    }
} // namespace jactorio::proto

#endif // JACTORIO_INCLUDE_PROTO_DETAIL_TYPE_H
