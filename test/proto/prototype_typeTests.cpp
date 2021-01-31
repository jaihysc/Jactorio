// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "proto/detail/prototype_type.h"
#include "proto/sprite.h"

namespace jactorio::proto
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
} // namespace jactorio::proto
