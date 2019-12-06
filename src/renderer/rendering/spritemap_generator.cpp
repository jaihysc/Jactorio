#include "renderer/rendering/spritemap_generator.h"

#include "core/logger.h"
#include "data/data_manager.h"

jactorio::renderer::Renderer_sprites::Spritemap_data jactorio::renderer::
Renderer_sprites::gen_spritemap(data::Sprite** sprites, const unsigned short count) const {
	LOG_MESSAGE_f(info, "Generating spritemap with %d tiles...", count);
	
	// Calculate spritemap dimensions
	unsigned int pixels_x = 0;
	unsigned int pixels_y = 0;

	for (int i = 0; i < count; ++i) {
		pixels_x += sprites[i]->get_width();
		if (sprites[i]->get_height() > pixels_y)
			pixels_y = sprites[i]->get_height();
	}

	auto* spritemap_buffer = new unsigned char[
		static_cast<unsigned long long>(pixels_x) * pixels_y * 4];
	
	std::unordered_map<unsigned int, Image_position> image_positions;

	
	// Offset the x pixels of each new sprite so they don't overlap each other
	unsigned int x_offset = 0;

	for (int i = 0; i < count; ++i) {
		const unsigned char* sprite_data = sprites[i]->get_sprite_data_ptr();

		const unsigned int sprite_width  = sprites[i]->get_width();
		const unsigned int sprite_height = sprites[i]->get_height();
		
		// Copy data onto spritemap
		for (unsigned int y = 0; y < sprite_height; ++y) {
			for (unsigned int x = 0; x < sprite_width; ++x) {
				for (unsigned int color_offset = 0; color_offset < 4; ++color_offset) {
					spritemap_buffer[(pixels_x * y + x + x_offset) * 4 + color_offset]
						= sprite_data[(sprite_width * y + x) * 4 + color_offset];
				}
			}
		}

		// Keep track of image positions within the spritemap
		{
			auto& image_position = image_positions[sprites[i]->internal_id];

			// Shrinks the dimensions of the images by this amount on all sides
			// Used to avoid texture leaking
			constexpr float coordinate_shrink_amount = 1;

			image_position.top_left =
				Position_pair{
					static_cast<float>(x_offset) + coordinate_shrink_amount,
					coordinate_shrink_amount
				};
			image_position.top_right =
				Position_pair{
					static_cast<float>(x_offset + sprite_width) - coordinate_shrink_amount,
					coordinate_shrink_amount
				};

			image_position.bottom_left =
				Position_pair{
					static_cast<float>(x_offset) + coordinate_shrink_amount,
					static_cast<float>(sprite_height) - coordinate_shrink_amount
				};
			image_position.bottom_right =
				Position_pair{
					static_cast<float>(x_offset + sprite_width) - coordinate_shrink_amount,
					static_cast<float>(sprite_height) - coordinate_shrink_amount
				};
		}

		x_offset += sprite_width;
	}


	// Normalize positions based on image size to value between 0 - 1
	for (auto& image : image_positions) {
		auto& position = image.second;

		position.top_left.x /= static_cast<float>(pixels_x);
		position.top_left.y /= static_cast<float>(pixels_y);

		position.top_right.x /= static_cast<float>(pixels_x);
		position.top_right.y /= static_cast<float>(pixels_y);

		position.bottom_left.x /= static_cast<float>(pixels_x);
		position.bottom_left.y /= static_cast<float>(pixels_y);

		position.bottom_right.x /= static_cast<float>(pixels_x);
		position.bottom_right.y /= static_cast<float>(pixels_y);
	}

	auto spritemap = new data::Sprite;
	spritemap->set_sprite_data_ptr(spritemap_buffer, pixels_x, pixels_y);

	
	data::data_manager::data_raw_add(
		data::data_category::sprite, "game-renderer-spritemap", spritemap);

	
	Spritemap_data spritemap_data;
	spritemap_data.spritemap = spritemap;
	spritemap_data.sprite_positions = image_positions;

	return spritemap_data;
}
