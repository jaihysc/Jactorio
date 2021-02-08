// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "core/orientation.h"

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
        int i = 0;
        OrientationIncrement<int>(Orientation::up, i, i);
        EXPECT_EQ(i, -1);

        int j   = 0;
        int j_y = 0;
        OrientationIncrement<int>(Orientation::right, j, j_y);
        EXPECT_EQ(j, 1);

        int k   = 0;
        int k_y = 0;
        OrientationIncrement<int>(Orientation::down, k, k_y);
        EXPECT_EQ(k_y, 1);

        int l = 0;
        OrientationIncrement<int>(Orientation::left, l, l, 2);
        EXPECT_EQ(l, -2);
    }
} // namespace jactorio