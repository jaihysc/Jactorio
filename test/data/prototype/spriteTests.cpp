// 
// spriteTests.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 12/06/2019
// Last modified: 03/17/2020
// 

#include <gtest/gtest.h>

#include "data/prototype/sprite.h"

namespace data::prototype
{
	TEST(sprite, load_sprite) {
		{
			jactorio::data::Sprite sprite{};

			EXPECT_EQ(sprite.get_width(), 0);
			EXPECT_EQ(sprite.get_height(), 0);

			sprite.load_image("test/graphics/test/test_tile.png");

			EXPECT_EQ(sprite.get_width(), 32);
			EXPECT_EQ(sprite.get_height(), 32);
		}
		{
			const jactorio::data::Sprite sprite("test/graphics/test/test_tile.png");

			EXPECT_EQ(sprite.get_width(), 32);
			EXPECT_EQ(sprite.get_height(), 32);
		}
	}

	TEST(sprite, get_coords) {
		{
			jactorio::data::Sprite sprite{};
			sprite.sets = 4;
			sprite.frames = 10;

			auto coords = sprite.get_coords(0, 0);
			EXPECT_FLOAT_EQ(coords.top_left.x, 0.f);
			EXPECT_FLOAT_EQ(coords.top_left.y, 0.f);

			EXPECT_FLOAT_EQ(coords.bottom_right.x, 0.1f);
			EXPECT_FLOAT_EQ(coords.bottom_right.y, 0.25f);
		}
		{
			jactorio::data::Sprite sprite{};
			sprite.sets = 49;
			sprite.frames = 2;

			auto coords = sprite.get_coords(6, 1);
			EXPECT_FLOAT_EQ(coords.top_left.x, 0.5f);
			EXPECT_FLOAT_EQ(coords.top_left.y, 0.1224489795918f);

			EXPECT_FLOAT_EQ(coords.bottom_right.x, 1.f);
			EXPECT_FLOAT_EQ(coords.bottom_right.y, 0.14285714285714f);
		}
	}

	TEST(sprite, get_coords_trimmed) {
		// This requires width_ and height_ to be initialized
		{
			jactorio::data::Sprite sprite{};
			sprite.sets = 2;
			sprite.frames = 3;

			sprite.set_width(620);
			sprite.set_height(190);
			sprite.trim = 12;

			const auto coords = sprite.get_coords_trimmed(1, 2);
			EXPECT_FLOAT_EQ(coords.top_left.x, 0.686021505f);
			EXPECT_FLOAT_EQ(coords.top_left.y, 0.563157894f);

			EXPECT_FLOAT_EQ(coords.bottom_right.x, 0.980645161f);
			EXPECT_FLOAT_EQ(coords.bottom_right.y, 0.936842105f);
		}
	}
}
