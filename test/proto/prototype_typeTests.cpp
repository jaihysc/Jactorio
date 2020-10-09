// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "proto/prototype_type.h"
#include "proto/sprite.h"

namespace jactorio::data
{
    TEST(PrototypeType, Tile4WayConstruct) {
        const Tile4Way tile{{1, 2}, {3, 4}, {5, 6}, {7, 8}};

        EXPECT_EQ(tile.up.x, 1);
        EXPECT_EQ(tile.up.y, 2);

        EXPECT_EQ(tile.right.x, 3);
        EXPECT_EQ(tile.right.y, 4);

        EXPECT_EQ(tile.down.x, 5);
        EXPECT_EQ(tile.down.y, 6);

        EXPECT_EQ(tile.left.x, 7);
        EXPECT_EQ(tile.left.y, 8);
    }

    TEST(PrototypeType, Tile4WayGet) {
        const Tile4Way tile{{1, 2}, {3, 4}, {5, 6}, {7, 8}};

        const auto up = tile.Get(Orientation::up);
        EXPECT_EQ(up.x, 1);
        EXPECT_EQ(up.y, 2);

        const auto right = tile.Get(Orientation::right);
        EXPECT_EQ(right.x, 3);
        EXPECT_EQ(right.y, 4);

        const auto down = tile.Get(Orientation::down);
        EXPECT_EQ(down.x, 5);
        EXPECT_EQ(down.y, 6);

        const auto left = tile.Get(Orientation::left);
        EXPECT_EQ(left.x, 7);
        EXPECT_EQ(left.y, 8);
    }

    TEST(PrototypeType, InvertOrientation) {
        using namespace data;

        EXPECT_EQ(InvertOrientation(Orientation::up), Orientation::down);
        EXPECT_EQ(InvertOrientation(Orientation::right), Orientation::left);
        EXPECT_EQ(InvertOrientation(Orientation::down), Orientation::up);
        EXPECT_EQ(InvertOrientation(Orientation::left), Orientation::right);

        EXPECT_EQ(InvertOrientation(0), 2);
        EXPECT_EQ(InvertOrientation(1), 3);
        EXPECT_EQ(InvertOrientation(2), 0);
        EXPECT_EQ(InvertOrientation(3), 1);
    }

    TEST(PrototypeType, OrientationIncrement) {
        using namespace data;

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
} // namespace jactorio::data
