// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "data/prototype/sprite.h"
#include "game/world/overlay_element.h"

namespace jactorio::game
{
	TEST(OverlayElement, Construct) {
		data::Sprite sprite{};

		/*/
		OverlayElement o1 = {sprite, {0.5, 1}, {2, 3}};

		EXPECT_FLOAT_EQ(o1.position.x, 0.5);
		EXPECT_FLOAT_EQ(o1.position.y, 1);
		EXPECT_FLOAT_EQ(o1.position.z, 0);

		EXPECT_FLOAT_EQ(o1.size.x, 2);
		EXPECT_FLOAT_EQ(o1.size.y, 3);
		*/


		OverlayElement o2 = {sprite, {0.5, 1}, {2, 3}, OverlayLayer::general};

		EXPECT_FLOAT_EQ(o2.position.x, 0.5);
		EXPECT_FLOAT_EQ(o2.position.y, 1);
		EXPECT_FLOAT_EQ(o2.position.z, 0.4);

		EXPECT_FLOAT_EQ(o2.size.x, 2);
		EXPECT_FLOAT_EQ(o2.size.y, 3);


		OverlayElement o3 = {sprite, {0.5, 1, 5000.123}, {2, 3}};

		EXPECT_FLOAT_EQ(o3.position.x, 0.5);
		EXPECT_FLOAT_EQ(o3.position.y, 1);
		EXPECT_FLOAT_EQ(o3.position.z, 5000.123);

		EXPECT_FLOAT_EQ(o3.size.x, 2);
		EXPECT_FLOAT_EQ(o3.size.y, 3);
	}
}
