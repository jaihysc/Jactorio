// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_CORE_COORDINATE_TUPLE_H
#define JACTORIO_INCLUDE_CORE_COORDINATE_TUPLE_H
#pragma once

#include <tuple>

#include "data/cereal/serialize.h"

#include <cereal/types/base_class.hpp>

namespace jactorio
{
    template <typename TVal>
    struct Position1
    {
        static_assert(std::is_trivial<TVal>::value);

        using ValueT = TVal;

        Position1() : x(0) {}

        explicit Position1(TVal x) : x(x) {}

        TVal x;


        CEREAL_SERIALIZE(archive) {
            archive(x);
        }

        friend bool operator==(const Position1& lhs, const Position1& rhs) {
            return lhs.x == rhs.x;
        }

        friend bool operator!=(const Position1& lhs, const Position1& rhs) {
            return !(lhs == rhs);
        }
    };

    template <typename TVal>
    struct Position2 : Position1<TVal>
    {
        using ValueT = TVal;

        Position2() : Position1<TVal>(), y(0) {}

        Position2(const Position1<TVal>& x, TVal y) : Position1<TVal>(x), y(y) {}

        Position2(TVal x, TVal y) : Position1<TVal>(x), y(y) {}

        TVal y;


        CEREAL_SERIALIZE(archive) {
            archive(cereal::base_class<Position1<TVal>>(this), y);
        }

        friend bool operator==(const Position2& lhs, const Position2& rhs) {
            return std::tie(static_cast<const Position1<TVal>&>(lhs), lhs.y) ==
                std::tie(static_cast<const Position1<TVal>&>(rhs), rhs.y);
        }

        friend bool operator!=(const Position2& lhs, const Position2& rhs) {
            return !(lhs == rhs);
        }
    };

    template <typename TVal>
    struct Position3 : Position2<TVal>
    {
        using ValueT = TVal;

        Position3() : Position2<TVal>(), z(0) {}

        Position3(const Position2<TVal>& xy, TVal z) : Position2<TVal>(xy), z(z) {}

        Position3(TVal x, TVal y, TVal z) : Position2<TVal>(x, y), z(z) {}

        TVal z;


        CEREAL_SERIALIZE(archive) {
            archive(cereal::base_class<Position2<TVal>>(this), z);
        }

        friend bool operator==(const Position3& lhs, const Position3& rhs) {
            return std::tie(static_cast<const Position2<TVal>&>(lhs), lhs.z) ==
                std::tie(static_cast<const Position2<TVal>&>(rhs), rhs.z);
        }

        friend bool operator!=(const Position3& lhs, const Position3& rhs) {
            return !(lhs == rhs);
        }
    };


    template <typename TPosition>
    struct QuadPosition
    {
        static_assert(std::is_base_of<Position1<typename TPosition::ValueT>, TPosition>::value);

        using PositionT = TPosition;

        QuadPosition() = default;

        QuadPosition(const TPosition top_left, const TPosition bottom_right)
            : topLeft(top_left), bottomRight(bottom_right) {}

        TPosition topLeft;
        TPosition bottomRight;
    };
} // namespace jactorio

#endif // JACTORIO_INCLUDE_CORE_COORDINATE_TUPLE_H
