// 
// spriteTests.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 12/06/2019
// Last modified: 03/15/2020
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
}
