#include "renderer/rendering/spritemap_generator.h"

#include <map>
#include <algorithm>
#include <stb/stb_image.h>

#include "core/logger.h"
#include "data/data_manager.h"

std::map<unsigned int, jactorio::renderer::Texture*> textures;
std::map<unsigned int, jactorio::renderer::renderer_sprites::Spritemap_data> spritemap_datas;

void jactorio::renderer::renderer_sprites::clear_spritemaps() {
	for (auto& map : textures) {
		delete map.second;
	}
	textures.clear();
	// The pointer which this contains is already cleared by spritemaps
	spritemap_datas.clear();

	LOG_MESSAGE(debug, "Spritemap data cleared");
}

void jactorio::renderer::renderer_sprites::create_spritemap(data::Sprite::sprite_group group, 
                                                            const bool invert_sprites) {
	std::vector<data::Sprite*> sprites =
		data::data_manager::data_raw_get_all<data::Sprite>(data::data_category::sprite);

	// Filter to group only
	sprites.erase(
		std::remove_if(sprites.begin(), sprites.end(), [group](data::Sprite* ptr) {
			const auto sprite_group = ptr->group;

			// Return false to NOT remove
			// Category of none is never removed
			if (sprite_group == data::Sprite::sprite_group::none) {
				LOG_MESSAGE_f(warning, 
				              "Sprite prototype '%s' has category none, and will be added to all sprite groups." 
				              " Consider giving it a category",
				              ptr->name.c_str())
				return false;
			}

			return sprite_group != group;
		}),
		sprites.end()
	);

	const Spritemap_data spritemap_data = gen_spritemap(sprites.data(), sprites.size(), invert_sprites);

	// Texture will delete the sprite* when deleted
	textures[static_cast<int>(group)] = new Texture(spritemap_data.spritemap);
	spritemap_datas[static_cast<int>(group)] = (spritemap_data);
}


const jactorio::renderer::renderer_sprites::Spritemap_data& jactorio::renderer::renderer_sprites::get_spritemap(
	data::Sprite::sprite_group group) {
	return spritemap_datas[static_cast<int>(group)];
}

const jactorio::renderer::Texture* jactorio::renderer::renderer_sprites::get_texture(
	data::Sprite::sprite_group group) {
	return textures[static_cast<int>(group)];
}


jactorio::renderer::renderer_sprites::Spritemap_data jactorio::renderer::
renderer_sprites::gen_spritemap(data::Sprite** sprites, const unsigned short count, const bool invert_sprites) {
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
					unsigned int sprite_index;

					if (invert_sprites) {
						sprite_index = 
							(sprite_width * (sprite_height - 1 - y) + x) * 4 + color_offset;
					}
					else {
						sprite_index = (sprite_width * y + x) * 4 + color_offset;
					}
					
					spritemap_buffer[(pixels_x * y + x + x_offset) * 4 + color_offset]
						= sprite_data[sprite_index];
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


	Spritemap_data spritemap_data;
	spritemap_data.spritemap = spritemap;
	spritemap_data.sprite_positions = image_positions;

	return spritemap_data;
}
