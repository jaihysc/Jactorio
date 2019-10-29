#include <gtest/gtest.h>

#include "renderer/rendering/renderer_sprites.h"

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
		"data/test/graphics/test/test_tile.png",
		"data/test/graphics/test/test_tile1.png",
		"data/test/graphics/test/test_tile2.png",
		"data/test/graphics/test/test_tile3.png",
	};

	const auto r_sprites = jactorio::renderer::Renderer_sprites{};
	
	const auto spritemap = r_sprites.gen_spritemap(img_paths, 4);

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
	const auto img1 = spritemap.sprite_positions.at("data/test/graphics/test/test_tile.png");
	EXPECT_EQ(img1.top_left.x, 0.f);
	EXPECT_EQ(img1.top_left.y, 0.f);

	EXPECT_EQ(img1.top_right.x, 0.2f);
	EXPECT_EQ(img1.top_right.y, 0.f);

	EXPECT_EQ(img1.bottom_left.x, 0.f);
	EXPECT_EQ(img1.bottom_left.y, 0.5f);

	EXPECT_EQ(img1.bottom_right.x, 0.2f);
	EXPECT_EQ(img1.bottom_right.y, 0.5f);

	const auto img2 = spritemap.sprite_positions.at("data/test/graphics/test/test_tile1.png");
	EXPECT_EQ(img2.top_left.x, 0.2f);
	EXPECT_EQ(img2.top_left.y, 0.f);

	EXPECT_EQ(img2.top_right.x, 0.4f);
	EXPECT_EQ(img2.top_right.y, 0.f);

	EXPECT_EQ(img2.bottom_left.x, 0.2f);
	EXPECT_EQ(img2.bottom_left.y, 0.5f);

	EXPECT_EQ(img2.bottom_right.x, 0.4f);
	EXPECT_EQ(img2.bottom_right.y, 0.5f);

	const auto img3 = spritemap.sprite_positions.at("data/test/graphics/test/test_tile2.png");
	EXPECT_EQ(img3.top_left.x, 0.4f);
	EXPECT_EQ(img3.top_left.y, 0.f);

	EXPECT_EQ(img3.top_right.x, 0.6f);
	EXPECT_EQ(img3.top_right.y, 0.f);

	EXPECT_EQ(img3.bottom_left.x, 0.4f);
	EXPECT_EQ(img3.bottom_left.y, 0.5f);

	EXPECT_EQ(img3.bottom_right.x, 0.6f);
	EXPECT_EQ(img3.bottom_right.y, 0.5f);

	const auto img4 = spritemap.sprite_positions.at("data/test/graphics/test/test_tile3.png");
	EXPECT_EQ(img4.top_left.x, 0.6f);
	EXPECT_EQ(img4.top_left.y, 0.f);

	EXPECT_EQ(img4.top_right.x, 1.f);
	EXPECT_EQ(img4.top_right.y, 0.f);

	EXPECT_EQ(img4.bottom_left.x, 0.6f);
	EXPECT_EQ(img4.bottom_left.y, 1.f);

	EXPECT_EQ(img4.bottom_right.x, 1.f);
	EXPECT_EQ(img4.bottom_right.y, 1.f);
}