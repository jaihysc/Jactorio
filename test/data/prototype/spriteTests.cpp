// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "data/prototype/sprite.h"

namespace jactorio::data
{
	TEST(Sprite, SpriteCopy) {
		Sprite first{};
		auto second = first;

		EXPECT_NE(first.GetSpritePtr(), second.GetSpritePtr());
	}

	TEST(Sprite, SpriteMove) {
		Sprite first{};
		first.LoadImage("test/graphics/test/test_tile.png");

		auto second = std::move(first);

		EXPECT_EQ(first.GetSpritePtr(), nullptr);
		EXPECT_NE(second.GetSpritePtr(), nullptr);
	}

	TEST(Sprite, TrySetDefaultSpriteGroup) {
		{
			// Item's sprite group should be set to terrain and gui if blank
			Sprite sprite{};

			auto& group = sprite.group;

			sprite.DefaultSpriteGroup({
				Sprite::SpriteGroup::terrain,
				Sprite::SpriteGroup::gui
			});

			EXPECT_NE(std::find(group.begin(), group.end(), jactorio::data::Sprite::SpriteGroup::terrain), group.end());
			EXPECT_NE(std::find(group.begin(), group.end(), jactorio::data::Sprite::SpriteGroup::gui), group.end());
		}
		{
			// If not blank, use initialization provided sprite groups
			Sprite sprite{};

			auto& group = sprite.group;
			group.push_back(Sprite::SpriteGroup::gui);

			EXPECT_EQ(group.size(), 1);
		}
	}

	TEST(Sprite, LoadSprite) {
		{
			Sprite sprite{};

			EXPECT_EQ(sprite.GetWidth(), 0);
			EXPECT_EQ(sprite.GetHeight(), 0);

			sprite.LoadImage("test/graphics/test/test_tile.png");

			EXPECT_EQ(sprite.GetWidth(), 32);
			EXPECT_EQ(sprite.GetHeight(), 32);
		}
		{
			const Sprite sprite("test/graphics/test/test_tile.png");

			EXPECT_EQ(sprite.GetWidth(), 32);
			EXPECT_EQ(sprite.GetHeight(), 32);
		}
	}

	TEST(Sprite, GetCoords) {
		{
			Sprite sprite{};
			sprite.SetWidth(1);
			sprite.SetHeight(1);
			sprite.sets   = 4;
			sprite.frames = 10;

			auto coords = sprite.GetCoords(3, 0);
			EXPECT_FLOAT_EQ(coords.topLeft.x, 0.f);
			EXPECT_FLOAT_EQ(coords.topLeft.y, 0.f);

			EXPECT_FLOAT_EQ(coords.bottomRight.x, 0.1f);
			EXPECT_FLOAT_EQ(coords.bottomRight.y, 0.25f);
		}
		{
			// 7 % 4 = 3
			Sprite sprite{};
			sprite.SetWidth(1);
			sprite.SetHeight(1);
			sprite.sets   = 4;
			sprite.frames = 10;

			auto coords = sprite.GetCoords(7, 0);
			EXPECT_FLOAT_EQ(coords.topLeft.x, 0.f);
			EXPECT_FLOAT_EQ(coords.topLeft.y, 0.f);

			EXPECT_FLOAT_EQ(coords.bottomRight.x, 0.1f);
			EXPECT_FLOAT_EQ(coords.bottomRight.y, 0.25f);
		}

		{
			Sprite sprite{};
			sprite.SetWidth(1);
			sprite.SetHeight(1);
			sprite.sets   = 49;
			sprite.frames = 2;

			auto coords = sprite.GetCoords(42, 1);
			EXPECT_FLOAT_EQ(coords.topLeft.x, 0.5f);
			EXPECT_FLOAT_EQ(coords.topLeft.y, 0.1224489795918f);

			EXPECT_FLOAT_EQ(coords.bottomRight.x, 1.f);
			EXPECT_FLOAT_EQ(coords.bottomRight.y, 0.14285714285714f);
		}

		// Frames will wrap around
		{
			Sprite sprite{};
			sprite.SetWidth(1);
			sprite.SetHeight(1);
			sprite.sets   = 2;
			sprite.frames = 2;

			auto coords = sprite.GetCoords(0, 3);
			EXPECT_FLOAT_EQ(coords.topLeft.x, 0.5f);
			EXPECT_FLOAT_EQ(coords.topLeft.y, 0.f);

			EXPECT_FLOAT_EQ(coords.bottomRight.x, 1.f);
			EXPECT_FLOAT_EQ(coords.bottomRight.y, 0.5f);
		}
	}

	TEST(Sprite, GetCoordsTrimmed) {
		// This requires width_ and height_ to be initialized
		{
			Sprite sprite{};
			sprite.sets   = 2;
			sprite.frames = 3;

			sprite.SetWidth(620);
			sprite.SetHeight(190);
			sprite.trim = 12;

			const auto coords = sprite.GetCoords(0, 2);
			EXPECT_FLOAT_EQ(coords.topLeft.x, 0.686021505f);
			EXPECT_FLOAT_EQ(coords.topLeft.y, 0.563157894f);

			EXPECT_FLOAT_EQ(coords.bottomRight.x, 0.980645161f);
			EXPECT_FLOAT_EQ(coords.bottomRight.y, 0.936842105f);
		}

		{
			// 4 % 2 = 0 
			Sprite sprite{};
			sprite.sets   = 2;
			sprite.frames = 3;

			sprite.SetWidth(620);
			sprite.SetHeight(190);
			sprite.trim = 12;

			const auto coords = sprite.GetCoords(4, 2);
			EXPECT_FLOAT_EQ(coords.topLeft.x, 0.686021505f);
			EXPECT_FLOAT_EQ(coords.topLeft.y, 0.563157894f);

			EXPECT_FLOAT_EQ(coords.bottomRight.x, 0.980645161f);
			EXPECT_FLOAT_EQ(coords.bottomRight.y, 0.936842105f);
		}
	}

	TEST(Sprite, GetCoordsTrimmedInverted) {
		{
			Sprite sprite{};
			sprite.Set_invertSetFrame(true);
			sprite.sets   = 2;
			sprite.frames = 3;

			sprite.SetWidth(100);
			sprite.SetHeight(100);

			const auto coords = sprite.GetCoords(2, 1);
			EXPECT_FLOAT_EQ(coords.topLeft.x, 0.5f);
			EXPECT_FLOAT_EQ(coords.topLeft.y, 0.6666666f);

			EXPECT_FLOAT_EQ(coords.bottomRight.x, 1.f);
			EXPECT_FLOAT_EQ(coords.bottomRight.y, 1.f);
		}

		{
			// 3 % 2 = 1 
			Sprite sprite{};
			sprite.Set_invertSetFrame(true);
			sprite.sets   = 3;
			sprite.frames = 2;

			sprite.SetWidth(620);
			sprite.SetHeight(190);
			sprite.trim = 12;

			const auto coords = sprite.GetCoords(1, 2);
			EXPECT_FLOAT_EQ(coords.topLeft.x, 0.686021505f);
			EXPECT_FLOAT_EQ(coords.topLeft.y, 0.563157894f);

			EXPECT_FLOAT_EQ(coords.bottomRight.x, 0.980645161f);
			EXPECT_FLOAT_EQ(coords.bottomRight.y, 0.936842105f);
		}
	}
}
