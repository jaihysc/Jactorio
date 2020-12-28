// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "proto/abstract/entity.h"

#include "jactorioTests.h"

namespace jactorio::proto
{
    ///
    /// Entity non rotatable, PostLoad() should also set rotateDimensions to false
    TEST(Entity, NonRotatable) {
        TestMockEntity e;
        e.rotatable = false;

        e.PostLoad();

        EXPECT_FALSE(e.rotateDimensions);

        e.SetWidth(1);
        e.SetHeight(2);

        EXPECT_EQ(e.GetWidth(Orientation::up), 1);
        EXPECT_EQ(e.GetHeight(Orientation::up), 2);

        EXPECT_EQ(e.GetWidth(Orientation::right), 1);
        EXPECT_EQ(e.GetHeight(Orientation::right), 2);

        EXPECT_EQ(e.GetWidth(Orientation::down), 1);
        EXPECT_EQ(e.GetHeight(Orientation::down), 2);

        EXPECT_EQ(e.GetWidth(Orientation::left), 1);
        EXPECT_EQ(e.GetHeight(Orientation::left), 2);
    }

    ///
    /// Entity rotatable, PostLoad() should keep rotateDimensions true
    TEST(Entity, Rotatable) {
        TestMockEntity e;
        e.rotatable = true;

        e.PostLoad();

        EXPECT_TRUE(e.rotateDimensions);

        e.SetWidth(1);
        e.SetHeight(2);

        EXPECT_EQ(e.GetWidth(Orientation::up), 1);
        EXPECT_EQ(e.GetHeight(Orientation::up), 2);

        EXPECT_EQ(e.GetWidth(Orientation::right), 2);
        EXPECT_EQ(e.GetHeight(Orientation::right), 1);

        EXPECT_EQ(e.GetWidth(Orientation::down), 1);
        EXPECT_EQ(e.GetHeight(Orientation::down), 2);

        EXPECT_EQ(e.GetWidth(Orientation::left), 2);
        EXPECT_EQ(e.GetHeight(Orientation::left), 1);
    }
} // namespace jactorio::proto