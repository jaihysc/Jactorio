// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/22/2019

#include <gtest/gtest.h>

#include "core/resource_guard.h"
#include "data/data_manager.h"
#include "renderer/rendering/spritemap_generator.h"

namespace renderer
{
	class SpritemapCreationTest : public testing::Test
	{
	protected:
		jactorio::renderer::RendererSprites rendererSprites_{};

		void TearDown() override {
			jactorio::data::ClearData();
		}
	};

	TEST_F(SpritemapCreationTest, CreateSpritemap) {
		// Sprite data delete by guard
		DataRawAdd("sprite1",
		           new jactorio::data::Sprite("test/graphics/test/test_tile.png",
		                                      {jactorio::data::Sprite::SpriteGroup::terrain}));
		DataRawAdd("sprite2",
		           new jactorio::data::Sprite("test/graphics/test/test_tile1.png",
		                                      {jactorio::data::Sprite::SpriteGroup::terrain}));

		DataRawAdd("sprite3",
		           new jactorio::data::Sprite("test/graphics/test/test_tile2.png",
		                                      {jactorio::data::Sprite::SpriteGroup::gui}));
		DataRawAdd("sprite4",
		           new jactorio::data::Sprite("test/graphics/test/test_tile3.png",
		                                      {jactorio::data::Sprite::SpriteGroup::gui}));

		// Should filter out to only 2 entries
		const auto data = rendererSprites_.CreateSpritemap(jactorio::data::Sprite::SpriteGroup::terrain, false);

		EXPECT_EQ(data.width, 64);
		EXPECT_EQ(data.height, 32);

	}

	TEST_F(SpritemapCreationTest, CreateSpritemapCategoryNone) {
		// If a sprite does not have a group specified (sprite_group::none):

		// Sprite data delete by guard
		// Terrain
		DataRawAdd("sprite1",
		           new jactorio::data::Sprite("test/graphics/test/test_tile.png",
		                                      {jactorio::data::Sprite::SpriteGroup::terrain}));
		DataRawAdd("sprite2",
		           new jactorio::data::Sprite("test/graphics/test/test_tile1.png",
		                                      {jactorio::data::Sprite::SpriteGroup::terrain}));

		// Gui
		DataRawAdd("sprite3",
		           new jactorio::data::Sprite("test/graphics/test/test_tile2.png",
		                                      {jactorio::data::Sprite::SpriteGroup::gui}));
		DataRawAdd("sprite4",
		           new jactorio::data::Sprite("test/graphics/test/test_tile3.png",
		                                      {jactorio::data::Sprite::SpriteGroup::gui}));

		// None
		DataRawAdd("spriteNone",
		           new jactorio::data::Sprite("test/graphics/test/test_tile.png",
		                                      {}));

		// Should filter out to 3 entries, total width of 32 * 3
		const auto data = rendererSprites_.CreateSpritemap(jactorio::data::Sprite::SpriteGroup::terrain, false);

		EXPECT_EQ(data.width, 96);
		EXPECT_EQ(data.height, 32);

	}


	// ======================================================================

	/// \brief For testing the actual generation algorithm
	class SpritemapGeneratorTest : public testing::Test
	{
	protected:
		jactorio::renderer::RendererSprites rendererSprites_{};

		/// \return true if pixel contains specified color
		static bool GetPixelColor(const unsigned char* img_ptr,
		                          const unsigned int image_width,
		                          const unsigned int x,
		                          const unsigned int y,
		                          const unsigned short r,
		                          const unsigned short g,
		                          const unsigned short b,
		                          const unsigned short a) {
			const unsigned int offset = (image_width * y + x) * 4;

			bool valid = true;

			if (img_ptr[offset + 0] != r ||
				img_ptr[offset + 1] != g ||
				img_ptr[offset + 2] != b ||
				img_ptr[offset + 3] != a)
				valid = false;

			return valid;
		}
	};

	TEST_F(SpritemapGeneratorTest, GenSpritemapInverted) {
		// Provide series of sprites in array
		// Expect concatenated image and its properties

		// Spritemap is purely horizontal, with smaller images aligned to the top
		// Image positions are retrieved from the spritemap via the path originally given to create the spritemap

		// Images 0 - 2 are 32 x 32 px
		// Image 3 is 64 x 64

		jactorio::data::Sprite** prototypes = nullptr;
		jactorio::core::CapturingGuard<void()> guard([&] {
			for (int i = 0; i < 4; ++i) {
				delete prototypes[i];
			}
			delete[] prototypes;
		});

		prototypes = new jactorio::data::Sprite*[4];
		for (int i = 0; i < 4; ++i) {
			prototypes[i] = new jactorio::data::Sprite;
		}

		prototypes[0]->internalId = 1;
		prototypes[0]->LoadImage("test/graphics/test/test_tile.png");

		prototypes[1]->internalId = 2;
		prototypes[1]->LoadImage("test/graphics/test/test_tile1.png");

		prototypes[2]->internalId = 3;
		prototypes[2]->LoadImage("test/graphics/test/test_tile2.png");

		prototypes[3]->internalId = 4;
		prototypes[3]->LoadImage("test/graphics/test/test_tile3.png");

		const auto spritemap = rendererSprites_.GenSpritemap(prototypes, 4, true);

		EXPECT_EQ(spritemap.width, 160);
		EXPECT_EQ(spritemap.height, 64);

		// Sample spots on the concatenated image
		// Image 0
		auto* img_ptr = spritemap.spriteBuffer.get();

		EXPECT_EQ(GetPixelColor(img_ptr, 160, 26, 6, 0, 0, 0, 255), true);
		EXPECT_EQ(GetPixelColor(img_ptr, 160, 5, 26, 0, 105, 162, 255), true);

		// Image 1
		EXPECT_EQ(GetPixelColor(img_ptr, 160, 47, 26, 83, 83, 83, 255), true);
		EXPECT_EQ(GetPixelColor(img_ptr, 160, 50, 9, 255, 255, 255, 255), true);

		// Image 2
		EXPECT_EQ(GetPixelColor(img_ptr, 160, 83, 5, 255, 0, 0, 255), true);
		EXPECT_EQ(GetPixelColor(img_ptr, 160, 71, 18, 255, 255, 255, 255), true);

		// Image 3
		EXPECT_EQ(GetPixelColor(img_ptr, 160, 125, 53, 77, 57, 76, 255), true);
		EXPECT_EQ(GetPixelColor(img_ptr, 160, 142, 22, 42, 15, 136, 255), true);

		// Empty area is undefined

		// Positions
		// 0.f; 0.f; // upper left
		// 1.f; 0.f;  // upper right
		// 0.f; 1.f,  // bottom left
		// 1.f; 1.f;  // bottom right

		// Validate that the image positions are calculated correctly
		const auto img1 = spritemap.spritePositions.at(1);
		EXPECT_EQ(img1.topLeft.x, 0.00625f);
		EXPECT_EQ(img1.topLeft.y, 0.015625f);

		EXPECT_EQ(img1.bottomRight.x, 0.19375f);
		EXPECT_EQ(img1.topLeft.y, 0.015625f);

		EXPECT_EQ(img1.topLeft.x, 0.00625f);
		EXPECT_EQ(img1.bottomRight.y, 0.484375f);

		EXPECT_EQ(img1.bottomRight.x, 0.19375f);
		EXPECT_EQ(img1.bottomRight.y, 0.484375f);

		const auto img2 = spritemap.spritePositions.at(2);
		EXPECT_EQ(img2.topLeft.x, 0.20625f);
		EXPECT_EQ(img2.topLeft.y, 0.015625f);

		EXPECT_EQ(img2.bottomRight.x, 0.39375f);
		EXPECT_EQ(img2.topLeft.y, 0.015625f);

		EXPECT_EQ(img2.topLeft.x, 0.20625f);
		EXPECT_EQ(img2.bottomRight.y, 0.484375f);

		EXPECT_EQ(img2.bottomRight.x, 0.39375f);
		EXPECT_EQ(img2.bottomRight.y, 0.484375f);

		const auto img3 = spritemap.spritePositions.at(3);
		EXPECT_EQ(img3.topLeft.x, 0.40625f);
		EXPECT_EQ(img3.topLeft.y, 0.015625f);

		EXPECT_EQ(img3.bottomRight.x, 0.59375f);
		EXPECT_EQ(img3.topLeft.y, 0.015625f);

		EXPECT_EQ(img3.topLeft.x, 0.40625f);
		EXPECT_EQ(img3.bottomRight.y, 0.484375f);

		EXPECT_EQ(img3.bottomRight.x, 0.59375f);
		EXPECT_EQ(img3.bottomRight.y, 0.484375f);

		const auto img4 = spritemap.spritePositions.at(4);
		EXPECT_EQ(img4.topLeft.x, 0.60625f);
		EXPECT_EQ(img4.topLeft.y, 0.015625f);

		EXPECT_EQ(img4.bottomRight.x, 0.99375f);
		EXPECT_EQ(img4.topLeft.y, 0.015625f);

		EXPECT_EQ(img4.topLeft.x, 0.60625f);
		EXPECT_EQ(img4.bottomRight.y, 0.984375f);

		EXPECT_EQ(img4.bottomRight.x, 0.99375f);
		EXPECT_EQ(img4.bottomRight.y, 0.984375f);
	}

	TEST_F(SpritemapGeneratorTest, GenSpritemap) {
		// Images 0 - 2 are 32 x 32 px
		jactorio::data::Sprite** prototypes = nullptr;
		jactorio::core::CapturingGuard<void()> guard([&] {
			for (int i = 0; i < 2; ++i) {
				delete prototypes[i];
			}
			delete[] prototypes;
		});

		prototypes = new jactorio::data::Sprite*[2];
		for (int i = 0; i < 2; ++i) {
			prototypes[i] = new jactorio::data::Sprite;
		}

		prototypes[0]->internalId = 1;
		prototypes[0]->LoadImage("test/graphics/test/test_tile.png");

		prototypes[1]->internalId = 2;
		prototypes[1]->LoadImage("test/graphics/test/test_tile1.png");

		const auto spritemap = rendererSprites_.GenSpritemap(prototypes, 2, false);

		EXPECT_EQ(spritemap.width, 64);
		EXPECT_EQ(spritemap.height, 32);

		// Sample spots on the concatenated image
		// Image 0
		auto* img_ptr = spritemap.spriteBuffer.get();

		// Image 1
		EXPECT_EQ(GetPixelColor(img_ptr, 64, 19, 25, 255, 0, 42, 255), true);
		EXPECT_EQ(GetPixelColor(img_ptr, 64, 25, 7, 8, 252, 199, 255), true);

		// Image 2
		EXPECT_EQ(GetPixelColor(img_ptr, 64, 32 + 23, 11, 149, 149, 149, 255), true);
		EXPECT_EQ(GetPixelColor(img_ptr, 64, 32 + 26, 16, 255, 255, 255, 255), true);

		// Empty area is undefined
	}
}
