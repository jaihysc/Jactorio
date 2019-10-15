#include <gtest/gtest.h>

#include "renderer/rendering/renderer_sprites.h"
#include "renderer/rendering/renderer.h"

void debug_print_color(const sf::Color color) {
	printf("%d %d %d %d\n", color.r, color.g, color.b, color.a);
}

TEST(renderer_sprites, gen_spritemap) {
	// Provide series of sprites in array
	// Expect concatenated image and its properties

	// Spritemap is purely horizontal, with smaller images aligned to the top
	// Image positions are retrieved from the spritemap via the path originally given to create the spritemap
	
	// Images are 32 x 32 px
	const auto img_paths = new std::string[4] {
		"data/base/graphics/terrain/test/test_tile.png",
		"data/base/graphics/terrain/test/test_tile1.png",
		"data/base/graphics/terrain/test/test_tile2.png",
		"data/base/graphics/terrain/test/test_tile3.png",
	};

	const auto r_sprites = jactorio::renderer::Renderer_sprites{};
	
	const auto spritemap = r_sprites.gen_spritemap(img_paths, 4);

	EXPECT_EQ(spritemap.spritemap.getSize().x, 128);
	EXPECT_EQ(spritemap.spritemap.getSize().y, 32);

	// Concatenated image
	EXPECT_EQ(spritemap.spritemap.getPixel(0, 0), sf::Color(1, 84, 255, 255));
	EXPECT_EQ(spritemap.spritemap.getPixel(31, 30), sf::Color(251, 224, 18, 255));
	EXPECT_EQ(spritemap.spritemap.getPixel(31, 31), sf::Color(30, 239, 62, 255));

	// Image 2
	EXPECT_EQ(spritemap.spritemap.getPixel(32 + 6, 5), sf::Color(0, 105, 162, 255));

	// Image 3
	// debug_print_color(spritemap.spritemap.getPixel(31 + 31 + 26, 8));
	EXPECT_EQ(spritemap.spritemap.getPixel(32 + 32 + 26, 8), sf::Color(8, 252, 199, 255));


	// Positions
	// 0.f; 0.f; // upper left
	// 1.f; 0.f;  // upper right
	// 0.f; 1.f,  // bottom left
	// 1.f; 1.f;  // bottom right
	
	const auto img1 = spritemap.sprite_positions.at("data/base/graphics/terrain/test/test_tile.png");
	EXPECT_EQ(img1.top_left.x, 0);
	EXPECT_EQ(img1.top_left.y, 0);

	EXPECT_EQ(img1.top_right.x, 0.25);
	EXPECT_EQ(img1.top_right.y, 0);

	EXPECT_EQ(img1.bottom_left.x, 0);
	EXPECT_EQ(img1.bottom_left.y, 1);

	EXPECT_EQ(img1.bottom_right.x, 0.25);
	EXPECT_EQ(img1.bottom_right.y, 1);

	const auto img2 = spritemap.sprite_positions.at("data/base/graphics/terrain/test/test_tile1.png");
	EXPECT_EQ(img2.top_left.x, 0.25);
	EXPECT_EQ(img2.top_left.y, 0);

	EXPECT_EQ(img2.top_right.x, 0.5);
	EXPECT_EQ(img2.top_right.y, 0);

	EXPECT_EQ(img2.bottom_left.x, 0.25);
	EXPECT_EQ(img2.bottom_left.y, 1);

	EXPECT_EQ(img2.bottom_right.x, 0.5);
	EXPECT_EQ(img2.bottom_right.y, 1);
}