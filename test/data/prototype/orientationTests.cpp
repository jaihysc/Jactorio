#include <gtest/gtest.h>

#include "data/prototype/orientation.h"

namespace data
{
	TEST(Orientation, InvertOrientation) {
		using namespace jactorio::data;
		
		EXPECT_EQ(invert_orientation(Orientation::up)   , Orientation::down );
		EXPECT_EQ(invert_orientation(Orientation::right), Orientation::left );
		EXPECT_EQ(invert_orientation(Orientation::down) , Orientation::up   );
		EXPECT_EQ(invert_orientation(Orientation::left) , Orientation::right);
		
		EXPECT_EQ(invert_orientation(0), 2);
		EXPECT_EQ(invert_orientation(1), 3);
		EXPECT_EQ(invert_orientation(2), 0);
		EXPECT_EQ(invert_orientation(3), 1);
	}

	TEST(Orientation, OrientationIncrement) {
		using namespace jactorio::data;

		int i = 0;
		orientation_increment<int>(Orientation::up, i, i);
		EXPECT_EQ(i, -1);
		
		int j = 0;
		int j_y = 0;
		orientation_increment<int>(Orientation::right, j, j_y);
		EXPECT_EQ(j, 1);

		int k = 0;
		int k_y = 0;
		orientation_increment<int>(Orientation::down, k, k_y);
		EXPECT_EQ(k_y, 1);

		int l = 0;
		orientation_increment<int>(Orientation::left, l, l, 2);
		EXPECT_EQ(l, -2);
	}
}