// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_CORE_ORIENTATION_H
#define JACTORIO_INCLUDE_CORE_ORIENTATION_H
#pragma once

#include <cassert>
#include <type_traits>

namespace jactorio
{
    enum class Orientation
    {
        up = 0,
        right,
        down,
        left
    };

    constexpr int InvertOrientation(const int orientation) {
        assert(0 <= orientation && orientation <= 3);
        return (orientation + 2) % 4;
    }

    constexpr Orientation InvertOrientation(Orientation orientation) {
        return static_cast<Orientation>(InvertOrientation(static_cast<int>(orientation)));
    }

    ///
    /// Increments or decrements Ty var depending on orientation.
    /// up--, right++, down++, left--
    /// \remark Type must implement operator+= and operator-=
    /// \remark if incrementer type not provided, Typeof x will bw used
    template <typename TyX, typename TyY, typename TyInc = TyX>
    constexpr void OrientationIncrement(const Orientation orientation, TyX& x, TyY& y, TyInc increment = 1) {
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

    constexpr const char* OrientationToStr(const Orientation orientation) {
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

} // namespace jactorio

#endif // JACTORIO_INCLUDE_CORE_ORIENTATION_H
