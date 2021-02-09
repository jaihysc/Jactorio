// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "core/orientation.h"

#include "core/coordinate_tuple.h"

namespace jactorio
{
    TEST(Orientation, Construct) {
        const Orientation orien = Orientation::up;
        EXPECT_EQ(orien, Orientation::up);
    }

    TEST(Orientation, Enumerate) {
        const Orientation o = Orientation::left;

        switch (o) {
        case Orientation::up:
            FAIL();
        case Orientation::right:
            FAIL();
        case Orientation::down:
            FAIL();
        case Orientation::left:
            break;

        default:
            FAIL();
        }
    }

    TEST(Orientation, Invert) {
        EXPECT_EQ(Orientation(Orientation::up).Invert(), Orientation::down);
        EXPECT_EQ(Orientation(Orientation::right).Invert(), Orientation::left);
        EXPECT_EQ(Orientation(Orientation::down).Invert(), Orientation::up);
        EXPECT_EQ(Orientation(Orientation::left).Invert(), Orientation::right);

        const Orientation o = Orientation::up;
        EXPECT_EQ(o.Inverted(), Orientation::down);
    }

    TEST(Orientation, Increment) {
        Position2<int> i;
        Position2Increment(Orientation::up, i, 1);
        EXPECT_EQ(i.x, 0);
        EXPECT_EQ(i.y, -1);

        Position2<int> j;
        Position2Increment(Orientation::right, j, -1);
        EXPECT_EQ(j.x, -1);
        EXPECT_EQ(j.y, 0);

        Position2<int> k;
        Position2Increment(Orientation::down, k, 1);
        EXPECT_EQ(k.x, 0);
        EXPECT_EQ(k.y, 1);

        Position2<int> l;
        Position2Increment(Orientation::left, l, 2);
        EXPECT_EQ(l.x, -2);
        EXPECT_EQ(l.y, 0);
    }
} // namespace jactorio