#include <gtest/gtest.h>

#include "data/prototype/sprite.h"

namespace data::prototype
{
	TEST(sprite, load_sprite) {
		{
			jactorio::data::Sprite sprite{};

			EXPECT_EQ(sprite.sprite_image.getSize().x, 0);
			EXPECT_EQ(sprite.sprite_image.getSize().y, 0);
			
			sprite.load_sprite("test/graphics/test/test_tile.png");

			EXPECT_EQ(sprite.sprite_image.getSize().x, 32);
			EXPECT_EQ(sprite.sprite_image.getSize().y, 32);
		}
		{
			const jactorio::data::Sprite sprite("test/graphics/test/test_tile.png");

			EXPECT_EQ(sprite.sprite_image.getSize().x, 32);
			EXPECT_EQ(sprite.sprite_image.getSize().y, 32);
		}
	}
}