// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "data/prototype/sprite.h"
#include "game/world/overlay_element.h"

namespace jactorio::game
{
    TEST(OverlayElement, Construct) {
        const data::Sprite sprite;

        /*/
        OverlayElement o1 = {sprite, {0.5, 1}, {2, 3}};

        EXPECT_FLOAT_EQ(o1.position.x, 0.5);
        EXPECT_FLOAT_EQ(o1.position.y, 1);
        EXPECT_FLOAT_EQ(o1.position.z, 0);

        EXPECT_FLOAT_EQ(o1.size.x, 2);
        EXPECT_FLOAT_EQ(o1.size.y, 3);
        */


        const OverlayElement o2 = {sprite, {0.5, 1}, {2, 3}, OverlayLayer::cursor};

        EXPECT_FLOAT_EQ(o2.position.x, 0.5f);
        EXPECT_FLOAT_EQ(o2.position.y, 1.f);
        EXPECT_FLOAT_EQ(o2.position.z, 0.4f);

        EXPECT_FLOAT_EQ(o2.size.x, 2.f);
        EXPECT_FLOAT_EQ(o2.size.y, 3.f);


        const OverlayElement o3 = {sprite, {0.5f, 1.f, 5000.123f}, {2.f, 3.f}};

        EXPECT_FLOAT_EQ(o3.position.x, 0.5f);
        EXPECT_FLOAT_EQ(o3.position.y, 1.f);
        EXPECT_FLOAT_EQ(o3.position.z, 5000.123f);

        EXPECT_FLOAT_EQ(o3.size.x, 2.f);
        EXPECT_FLOAT_EQ(o3.size.y, 3.f);
    }

    TEST(OverlayElement, GetSetZPosition) {
        const data::Sprite sprite;
        OverlayElement oe = {sprite, {0.5, 1, 0.f}, {2, 3}};

        EXPECT_FLOAT_EQ(oe.position.z, 0.f);

        oe.SetZPosition(12);
        EXPECT_FLOAT_EQ(oe.position.z, 12.f);

        oe.SetZPosition(OverlayLayer::cursor);
        EXPECT_FLOAT_EQ(oe.position.z, 0.4f);

        EXPECT_FLOAT_EQ(oe.ToZPosition(OverlayLayer::cursor), 0.4f);
    }
} // namespace jactorio::game
