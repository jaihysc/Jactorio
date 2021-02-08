// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_CORE_ORIENTATION_H
#define JACTORIO_INCLUDE_CORE_ORIENTATION_H
#pragma once

#include <type_traits>

#include "jactorio.h"

#include "data/cereal/serialize.h"

namespace jactorio
{
    class Orientation
    {
    public:
        enum Direction : uint8_t
        {
            up = 0,
            right,
            down,
            left
        };

        // Intentionally non explicit to serve as replacement for enum class Orientation

        constexpr Orientation() = default;
        constexpr Orientation(const Direction value) : dir_(value) {}

        constexpr operator Direction() const {
            return dir_;
        }


        ///
        /// Inverts current direction
        constexpr Orientation& Invert() {
            dir_ = Invert(dir_);
            return *this;
        }

        ///
        /// \return Inverted direction of current direction
        J_NODISCARD constexpr Orientation Inverted() const {
            return Invert(dir_);
        }

        ///
        /// \return Inverted direction of provided direction
        static constexpr Direction Invert(const Direction value) {
            assert(0 <= value && value <= 3);
            return static_cast<Direction>((value + 2) % 4);
        }

        J_NODISCARD constexpr const char* ToCstr() const {
            switch (dir_) {
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
                return "";
            }
        }


        CEREAL_SERIALIZE(archive) {
            archive(dir_);
        }

    private:
        Direction dir_ = Orientation::up;
    };

    // Supports templates which formerly used enum class Orientation
    using Direction = Orientation::Direction;


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
} // namespace jactorio

#endif // JACTORIO_INCLUDE_CORE_ORIENTATION_H
