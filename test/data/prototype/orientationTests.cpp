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
}