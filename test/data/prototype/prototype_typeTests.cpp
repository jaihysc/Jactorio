// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 04/12/2020

#include <gtest/gtest.h>

#include "data/prototype/prototype_type.h"
#include "data/prototype/interface/rotatable.h"

namespace data
{
	TEST(PrototypeType, Tile4WayConstruct) {
		const jactorio::data::Tile_4_way tile{
			{1, 2},
			{3, 4},
			{5, 6},
			{7, 8}
		};

		EXPECT_EQ(tile.up.first, 1);
		EXPECT_EQ(tile.up.second, 2);

		EXPECT_EQ(tile.right.first, 3);
		EXPECT_EQ(tile.right.second, 4);

		EXPECT_EQ(tile.down.first, 5);
		EXPECT_EQ(tile.down.second, 6);

		EXPECT_EQ(tile.left.first, 7);
		EXPECT_EQ(tile.left.second, 8);
	}

	TEST(PrototypeType, Tile4WayGet) {
		const jactorio::data::Tile_4_way tile{
			{1, 2},
			{3, 4},
			{5, 6},
			{7, 8}
		};

		const auto up = tile.get(jactorio::data::Orientation::up);
		EXPECT_EQ(up.first, 1);
		EXPECT_EQ(up.second, 2);

		const auto right = tile.get(jactorio::data::Orientation::right);
		EXPECT_EQ(right.first, 3);
		EXPECT_EQ(right.second, 4);

		const auto down = tile.get(jactorio::data::Orientation::down);
		EXPECT_EQ(down.first, 5);
		EXPECT_EQ(down.second, 6);

		const auto left = tile.get(jactorio::data::Orientation::left);
		EXPECT_EQ(left.first, 7);
		EXPECT_EQ(left.second, 8);
	}
}
