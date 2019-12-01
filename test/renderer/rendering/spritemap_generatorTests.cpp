#include <gtest/gtest.h>

#include "renderer/rendering/spritemap_generator.h"

namespace renderer
{
	// void debug_print_color(const sf::Color color) {
	// printf("%d %d %d %d\n", color.r, color.g, color.b, color.a);
    // }

	TEST(spritemap_generator, gen_spritemap) {
		// Provide series of sprites in array
		// Expect concatenated image and its properties

		// Spritemap is purely horizontal, with smaller images aligned to the top
		// Image positions are retrieved from the spritemap via the path originally given to create the spritemap

		// Images 0 - 2 are 32 x 32 px
		// Image 3 is 64 x 64
		const auto prototypes = new jactorio::data::Sprite * [4];
		for (int i = 0; i < 4; ++i) {
			prototypes[i] = new jactorio::data::Sprite;
		}

		prototypes[0]->name = "1";
		prototypes[0]->load_sprite("test/graphics/test/test_tile.png");

		prototypes[1]->name = "2";
		prototypes[1]->load_sprite("test/graphics/test/test_tile1.png");

		prototypes[2]->name = "3";
		prototypes[2]->load_sprite("test/graphics/test/test_tile2.png");

		prototypes[3]->name = "4";
		prototypes[3]->load_sprite("test/graphics/test/test_tile3.png");

		const auto r_sprites = jactorio::renderer::Renderer_sprites{};

		const auto spritemap = r_sprites.gen_spritemap(prototypes, 4);

		EXPECT_EQ(spritemap.spritemap.getSize().x, 160);
		EXPECT_EQ(spritemap.spritemap.getSize().y, 64);

		// Sample spots on the concatenated image
		// Image 0
		EXPECT_EQ(spritemap.spritemap.getPixel(26, 6), sf::Color(0, 0, 0, 255));
		EXPECT_EQ(spritemap.spritemap.getPixel(5, 26), sf::Color(0, 105, 162, 255));

		// Image 1
		EXPECT_EQ(spritemap.spritemap.getPixel(47, 26), sf::Color(83, 83, 83, 255));
		EXPECT_EQ(spritemap.spritemap.getPixel(50, 9), sf::Color(255, 255, 255, 255));

		// Image 2
		EXPECT_EQ(spritemap.spritemap.getPixel(83, 5), sf::Color(255, 0, 0, 255));
		EXPECT_EQ(spritemap.spritemap.getPixel(71, 18), sf::Color(255, 255, 255, 255));

		// Image 3
		EXPECT_EQ(spritemap.spritemap.getPixel(125, 53), sf::Color(77, 57, 76, 255));
		EXPECT_EQ(spritemap.spritemap.getPixel(142, 22), sf::Color(42, 15, 136, 255));

		// Empty area
		EXPECT_EQ(spritemap.spritemap.getPixel(95, 32), sf::Color(0, 0, 0, 255));
		EXPECT_EQ(spritemap.spritemap.getPixel(52, 59), sf::Color(0, 0, 0, 255));
		EXPECT_EQ(spritemap.spritemap.getPixel(95, 63), sf::Color(0, 0, 0, 255));


		// Positions
		// 0.f; 0.f; // upper left
		// 1.f; 0.f;  // upper right
		// 0.f; 1.f,  // bottom left
		// 1.f; 1.f;  // bottom right

		// Validate that the image positions are calculated correctly
		const auto img1 = spritemap.sprite_positions.at("1");
		EXPECT_EQ(img1.top_left.x, 0.00625f);
		EXPECT_EQ(img1.top_left.y, 0.015625f);

		EXPECT_EQ(img1.top_right.x, 0.19375f);
		EXPECT_EQ(img1.top_right.y, 0.015625f);

		EXPECT_EQ(img1.bottom_left.x, 0.00625f);
		EXPECT_EQ(img1.bottom_left.y, 0.484375f);

		EXPECT_EQ(img1.bottom_right.x, 0.19375f);
		EXPECT_EQ(img1.bottom_right.y, 0.484375f);

		const auto img2 = spritemap.sprite_positions.at("2");
		EXPECT_EQ(img2.top_left.x, 0.20625f);
		EXPECT_EQ(img2.top_left.y, 0.015625f);

		EXPECT_EQ(img2.top_right.x, 0.39375f);
		EXPECT_EQ(img2.top_right.y, 0.015625f);

		EXPECT_EQ(img2.bottom_left.x, 0.20625f);
		EXPECT_EQ(img2.bottom_left.y, 0.484375f);

		EXPECT_EQ(img2.bottom_right.x, 0.39375f);
		EXPECT_EQ(img2.bottom_right.y, 0.484375f);

		const auto img3 = spritemap.sprite_positions.at("3");
		EXPECT_EQ(img3.top_left.x, 0.40625f);
		EXPECT_EQ(img3.top_left.y, 0.015625f);

		EXPECT_EQ(img3.top_right.x, 0.59375f);
		EXPECT_EQ(img3.top_right.y, 0.015625f);

		EXPECT_EQ(img3.bottom_left.x, 0.40625f);
		EXPECT_EQ(img3.bottom_left.y, 0.484375f);

		EXPECT_EQ(img3.bottom_right.x, 0.59375f);
		EXPECT_EQ(img3.bottom_right.y, 0.484375f);

		const auto img4 = spritemap.sprite_positions.at("4");
		EXPECT_EQ(img4.top_left.x, 0.60625f);
		EXPECT_EQ(img4.top_left.y, 0.015625f);

		EXPECT_EQ(img4.top_right.x, 0.99375f);
		EXPECT_EQ(img4.top_right.y, 0.015625f);

		EXPECT_EQ(img4.bottom_left.x, 0.60625f);
		EXPECT_EQ(img4.bottom_left.y, 0.984375f);

		EXPECT_EQ(img4.bottom_right.x, 0.99375f);
		EXPECT_EQ(img4.bottom_right.y, 0.984375f);
	}
}
