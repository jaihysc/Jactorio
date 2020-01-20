#include <gtest/gtest.h>

#include "renderer/rendering/spritemap_generator.h"
#include "data/data_manager.h"
#include "core/resource_guard.h"

namespace renderer
{
	TEST(spritemap_generator, create_spritemap) {
		using namespace jactorio::renderer::renderer_sprites;
		namespace data_manager = jactorio::data::data_manager;

		auto guard = jactorio::core::Resource_guard(data_manager::clear_data);
		auto guard2 = jactorio::core::Resource_guard(clear_spritemaps);

		// Sprite data delete by guard
		data_manager::data_raw_add(jactorio::data::data_category::sprite, "sprite1", 
		                           new jactorio::data::Sprite("test/graphics/test/test_tile.png", 
		                                                      jactorio::data::Sprite::sprite_group::terrain));
		data_manager::data_raw_add(jactorio::data::data_category::sprite, "sprite2",
		                           new jactorio::data::Sprite("test/graphics/test/test_tile1.png",
		                                                      jactorio::data::Sprite::sprite_group::terrain));
		
		data_manager::data_raw_add(jactorio::data::data_category::sprite, "sprite3",
		                           new jactorio::data::Sprite("test/graphics/test/test_tile2.png",
		                                                      jactorio::data::Sprite::sprite_group::gui));
		data_manager::data_raw_add(jactorio::data::data_category::sprite, "sprite4",
		                           new jactorio::data::Sprite("test/graphics/test/test_tile3.png",
		                                                      jactorio::data::Sprite::sprite_group::gui));
		
		// Should filter out to only 2 entries
		create_spritemap(jactorio::data::Sprite::sprite_group::terrain, false);

		const Spritemap_data& data = get_spritemap(jactorio::data::Sprite::sprite_group::terrain);
		
		EXPECT_EQ(data.spritemap->get_width(), 64);
		EXPECT_EQ(data.spritemap->get_height(), 32);

	}

	TEST(spritemap_generator, create_spritemap_category_none) {
		// If a sprite does not have a group specified (sprite_group::none):
		// it will be added with every spritemap generated
		
		using namespace jactorio::renderer::renderer_sprites;
		namespace data_manager = jactorio::data::data_manager;

		auto guard = jactorio::core::Resource_guard(data_manager::clear_data);
		auto guard2 = jactorio::core::Resource_guard(clear_spritemaps);

		// Sprite data delete by guard
		// Terrain
		data_manager::data_raw_add(jactorio::data::data_category::sprite, "sprite1",
		                           new jactorio::data::Sprite("test/graphics/test/test_tile.png",
		                                                      jactorio::data::Sprite::sprite_group::terrain));
		data_manager::data_raw_add(jactorio::data::data_category::sprite, "sprite2",
		                           new jactorio::data::Sprite("test/graphics/test/test_tile1.png",
		                                                      jactorio::data::Sprite::sprite_group::terrain));

		// Gui
		data_manager::data_raw_add(jactorio::data::data_category::sprite, "sprite3",
		                           new jactorio::data::Sprite("test/graphics/test/test_tile2.png",
		                                                      jactorio::data::Sprite::sprite_group::gui));
		data_manager::data_raw_add(jactorio::data::data_category::sprite, "sprite4",
		                           new jactorio::data::Sprite("test/graphics/test/test_tile3.png",
		                                                      jactorio::data::Sprite::sprite_group::gui));

		// None
		data_manager::data_raw_add(jactorio::data::data_category::sprite, "spriteNone",
		                           new jactorio::data::Sprite("test/graphics/test/test_tile.png",
		                                                      jactorio::data::Sprite::sprite_group::none));
		
		// Should filter out to 3 entries, total width of 32 * 3
		create_spritemap(jactorio::data::Sprite::sprite_group::terrain, false);

		const Spritemap_data& data = get_spritemap(jactorio::data::Sprite::sprite_group::terrain);

		EXPECT_EQ(data.spritemap->get_width(), 96);
		EXPECT_EQ(data.spritemap->get_height(), 32);

	}
	
	// Returns true if pixel contains specified color
	bool get_pixel_color(const unsigned char* img_ptr,
	                     const unsigned int image_width,
	                     const unsigned int x, const unsigned int y,
	                     const unsigned short r, const unsigned short g, const unsigned short b,
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

	
	TEST(spritemap_generator, gen_spritemap_inverted) {
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

		prototypes[0]->internal_id = 1;
		prototypes[0]->load_image("test/graphics/test/test_tile.png");

		prototypes[1]->internal_id = 2;
		prototypes[1]->load_image("test/graphics/test/test_tile1.png");

		prototypes[2]->internal_id = 3;
		prototypes[2]->load_image("test/graphics/test/test_tile2.png");

		prototypes[3]->internal_id = 4;
		prototypes[3]->load_image("test/graphics/test/test_tile3.png");

		const auto spritemap = jactorio::renderer::renderer_sprites::gen_spritemap(prototypes, 4, true);

		EXPECT_EQ(spritemap.spritemap->get_width(), 160);
		EXPECT_EQ(spritemap.spritemap->get_height(), 64);

		// Sample spots on the concatenated image
		// Image 0
		auto* img_ptr = spritemap.spritemap->get_sprite_data_ptr();

		EXPECT_EQ(get_pixel_color(img_ptr, 160, 26, 6, 0, 0, 0, 255), true);
		EXPECT_EQ(get_pixel_color(img_ptr, 160, 5, 26, 0, 105, 162, 255), true);

		// Image 1
		EXPECT_EQ(get_pixel_color(img_ptr, 160, 47, 26, 83, 83, 83, 255), true);
		EXPECT_EQ(get_pixel_color(img_ptr, 160, 50, 9, 255, 255, 255, 255), true);

		// Image 2
		EXPECT_EQ(get_pixel_color(img_ptr, 160, 83, 5, 255, 0, 0, 255), true);
		EXPECT_EQ(get_pixel_color(img_ptr, 160, 71, 18, 255, 255, 255, 255), true);

		// Image 3
		EXPECT_EQ(get_pixel_color(img_ptr, 160, 125, 53, 77, 57, 76, 255), true);
		EXPECT_EQ(get_pixel_color(img_ptr, 160, 142, 22, 42, 15, 136, 255), true);

		// Empty area is undefined
		
		// Positions
		// 0.f; 0.f; // upper left
		// 1.f; 0.f;  // upper right
		// 0.f; 1.f,  // bottom left
		// 1.f; 1.f;  // bottom right

		// Validate that the image positions are calculated correctly
		const auto img1 = spritemap.sprite_positions.at(1);
		EXPECT_EQ(img1.top_left.x, 0.00625f);
		EXPECT_EQ(img1.top_left.y, 0.015625f);

		EXPECT_EQ(img1.bottom_right.x, 0.19375f);
		EXPECT_EQ(img1.top_left.y, 0.015625f);

		EXPECT_EQ(img1.top_left.x, 0.00625f);
		EXPECT_EQ(img1.bottom_right.y, 0.484375f);

		EXPECT_EQ(img1.bottom_right.x, 0.19375f);
		EXPECT_EQ(img1.bottom_right.y, 0.484375f);

		const auto img2 = spritemap.sprite_positions.at(2);
		EXPECT_EQ(img2.top_left.x, 0.20625f);
		EXPECT_EQ(img2.top_left.y, 0.015625f);

		EXPECT_EQ(img2.bottom_right.x, 0.39375f);
		EXPECT_EQ(img2.top_left.y, 0.015625f);

		EXPECT_EQ(img2.top_left.x, 0.20625f);
		EXPECT_EQ(img2.bottom_right.y, 0.484375f);

		EXPECT_EQ(img2.bottom_right.x, 0.39375f);
		EXPECT_EQ(img2.bottom_right.y, 0.484375f);

		const auto img3 = spritemap.sprite_positions.at(3);
		EXPECT_EQ(img3.top_left.x, 0.40625f);
		EXPECT_EQ(img3.top_left.y, 0.015625f);

		EXPECT_EQ(img3.bottom_right.x, 0.59375f);
		EXPECT_EQ(img3.top_left.y, 0.015625f);

		EXPECT_EQ(img3.top_left.x, 0.40625f);
		EXPECT_EQ(img3.bottom_right.y, 0.484375f);

		EXPECT_EQ(img3.bottom_right.x, 0.59375f);
		EXPECT_EQ(img3.bottom_right.y, 0.484375f);

		const auto img4 = spritemap.sprite_positions.at(4);
		EXPECT_EQ(img4.top_left.x, 0.60625f);
		EXPECT_EQ(img4.top_left.y, 0.015625f);

		EXPECT_EQ(img4.bottom_right.x, 0.99375f);
		EXPECT_EQ(img4.top_left.y, 0.015625f);

		EXPECT_EQ(img4.top_left.x, 0.60625f);
		EXPECT_EQ(img4.bottom_right.y, 0.984375f);

		EXPECT_EQ(img4.bottom_right.x, 0.99375f);
		EXPECT_EQ(img4.bottom_right.y, 0.984375f);
	}

	TEST(spritemap_generator, gen_spritemap) {
		// Images 0 - 2 are 32 x 32 px
		const auto prototypes = new jactorio::data::Sprite* [2];
		for (int i = 0; i < 2; ++i) {
			prototypes[i] = new jactorio::data::Sprite;
		}

		prototypes[0]->internal_id = 1;
		prototypes[0]->load_image("test/graphics/test/test_tile.png");

		prototypes[1]->internal_id = 2;
		prototypes[1]->load_image("test/graphics/test/test_tile1.png");

		const auto spritemap = jactorio::renderer::renderer_sprites::gen_spritemap(prototypes, 2, false);

		EXPECT_EQ(spritemap.spritemap->get_width(), 64);
		EXPECT_EQ(spritemap.spritemap->get_height(), 32);

		// Sample spots on the concatenated image
		// Image 0
		auto* img_ptr = spritemap.spritemap->get_sprite_data_ptr();

		// Image 1
		EXPECT_EQ(get_pixel_color(img_ptr, 64, 19, 25, 255, 0, 42, 255), true);
		EXPECT_EQ(get_pixel_color(img_ptr, 64, 25, 7, 8, 252, 199, 255), true);

		// Image 2
		EXPECT_EQ(get_pixel_color(img_ptr, 64, 32 + 23, 11, 149, 149, 149, 255), true);
		EXPECT_EQ(get_pixel_color(img_ptr, 64, 32 + 26, 16, 255, 255, 255, 255), true);

		// Empty area is undefined
	}
}
