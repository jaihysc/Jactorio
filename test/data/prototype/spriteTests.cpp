// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 12/06/2019

#include <gtest/gtest.h>

#include "data/prototype/sprite.h"

namespace data::prototype
{
	TEST(Sprite, SpriteCopy) {
		jactorio::data::Sprite first{};
		auto second = first;

		EXPECT_NE(first.GetSpritePtr(), second.GetSpritePtr());
	}

	TEST(Sprite, SpriteMove) {
		jactorio::data::Sprite first{};
		first.LoadImage("test/graphics/test/test_tile.png");

		auto second = std::move(first);

		EXPECT_EQ(first.GetSpritePtr(), nullptr);
		EXPECT_NE(second.GetSpritePtr(), nullptr);
	}

	TEST(Sprite, LoadSprite) {
		{
			jactorio::data::Sprite sprite{};

			EXPECT_EQ(sprite.GetWidth(), 0);
			EXPECT_EQ(sprite.GetHeight(), 0);

			sprite.LoadImage("test/graphics/test/test_tile.png");

			EXPECT_EQ(sprite.GetWidth(), 32);
			EXPECT_EQ(sprite.GetHeight(), 32);
		}
		{
			const jactorio::data::Sprite sprite("test/graphics/test/test_tile.png");

			EXPECT_EQ(sprite.GetWidth(), 32);
			EXPECT_EQ(sprite.GetHeight(), 32);
		}
	}

	TEST(Sprite, GetCoords) {
		{
			jactorio::data::Sprite sprite{};
			sprite.sets   = 4;
			sprite.frames = 10;

			auto coords = sprite.GetCoords(0, 0);
			EXPECT_FLOAT_EQ(coords.topLeft.x, 0.f);
			EXPECT_FLOAT_EQ(coords.topLeft.y, 0.f);

			EXPECT_FLOAT_EQ(coords.bottomRight.x, 0.1f);
			EXPECT_FLOAT_EQ(coords.bottomRight.y, 0.25f);
		}
		{
			// 4 % 4 = 0
			jactorio::data::Sprite sprite{};
			sprite.sets   = 4;
			sprite.frames = 10;

			auto coords = sprite.GetCoords(4, 0);
			EXPECT_FLOAT_EQ(coords.topLeft.x, 0.f);
			EXPECT_FLOAT_EQ(coords.topLeft.y, 0.f);

			EXPECT_FLOAT_EQ(coords.bottomRight.x, 0.1f);
			EXPECT_FLOAT_EQ(coords.bottomRight.y, 0.25f);
		}

		{
			jactorio::data::Sprite sprite{};
			sprite.sets   = 49;
			sprite.frames = 2;

			auto coords = sprite.GetCoords(6, 1);
			EXPECT_FLOAT_EQ(coords.topLeft.x, 0.5f);
			EXPECT_FLOAT_EQ(coords.topLeft.y, 0.1224489795918f);

			EXPECT_FLOAT_EQ(coords.bottomRight.x, 1.f);
			EXPECT_FLOAT_EQ(coords.bottomRight.y, 0.14285714285714f);
		}

		// Frames will wrap around
		{
			jactorio::data::Sprite sprite{};
			sprite.sets   = 2;
			sprite.frames = 2;

			auto coords = sprite.GetCoords(0, 3);
			EXPECT_FLOAT_EQ(coords.topLeft.x, 0.5f);
			EXPECT_FLOAT_EQ(coords.topLeft.y, 0.5f);

			EXPECT_FLOAT_EQ(coords.bottomRight.x, 1.f);
			EXPECT_FLOAT_EQ(coords.bottomRight.y, 1.f);
		}
	}

	TEST(Sprite, GetCoordsTrimmed) {
		// This requires width_ and height_ to be initialized
		{
			jactorio::data::Sprite sprite{};
			sprite.sets   = 2;
			sprite.frames = 3;

			sprite.SetWidth(620);
			sprite.SetHeight(190);
			sprite.trim = 12;

			const auto coords = sprite.GetCoordsTrimmed(1, 2);
			EXPECT_FLOAT_EQ(coords.topLeft.x, 0.686021505f);
			EXPECT_FLOAT_EQ(coords.topLeft.y, 0.563157894f);

			EXPECT_FLOAT_EQ(coords.bottomRight.x, 0.980645161f);
			EXPECT_FLOAT_EQ(coords.bottomRight.y, 0.936842105f);
		}

		{
			// 3 % 2 = 1 
			jactorio::data::Sprite sprite{};
			sprite.sets   = 2;
			sprite.frames = 3;

			sprite.SetWidth(620);
			sprite.SetHeight(190);
			sprite.trim = 12;

			const auto coords = sprite.GetCoordsTrimmed(3, 2);
			EXPECT_FLOAT_EQ(coords.topLeft.x, 0.686021505f);
			EXPECT_FLOAT_EQ(coords.topLeft.y, 0.563157894f);

			EXPECT_FLOAT_EQ(coords.bottomRight.x, 0.980645161f);
			EXPECT_FLOAT_EQ(coords.bottomRight.y, 0.936842105f);
		}
	}
}
