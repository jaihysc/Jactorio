// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 05/01/2020

#include <gtest/gtest.h>

#include "data/prototype/orientation.h"

namespace data
{
	TEST(Orientation, InvertOrientation) {
		using namespace jactorio::data;

		EXPECT_EQ(InvertOrientation(Orientation::up), Orientation::down);
		EXPECT_EQ(InvertOrientation(Orientation::right), Orientation::left);
		EXPECT_EQ(InvertOrientation(Orientation::down), Orientation::up);
		EXPECT_EQ(InvertOrientation(Orientation::left), Orientation::right);

		EXPECT_EQ(InvertOrientation(0), 2);
		EXPECT_EQ(InvertOrientation(1), 3);
		EXPECT_EQ(InvertOrientation(2), 0);
		EXPECT_EQ(InvertOrientation(3), 1);
	}

	TEST(Orientation, OrientationIncrement) {
		using namespace jactorio::data;

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
}
