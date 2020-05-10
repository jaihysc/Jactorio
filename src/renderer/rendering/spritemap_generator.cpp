// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/22/2019

#include "renderer/rendering/spritemap_generator.h"

#include <algorithm>
#include <map>
#include <stb/stb_image.h>

#include "core/logger.h"
#include "data/data_manager.h"

void jactorio::renderer::RendererSprites::ClearSpritemaps() {
	for (auto& map : textures_) {
		delete map.second;
	}
	textures_.clear();
	// The pointer which this contains is already cleared by spritemaps
	spritemapDatas_.clear();
}

void jactorio::renderer::RendererSprites::CreateSpritemap(data::Sprite::SpriteGroup group,
                                                          const bool invert_sprites) {
	std::vector<data::Sprite*> sprites =
		data::DataRawGetAll<data::Sprite>(data::DataCategory::sprite);

	// Filter to group only
	sprites.erase(
		std::remove_if(sprites.begin(), sprites.end(), [group](data::Sprite* ptr) {
			// Return false to NOT remove
			// Category of none is never removed
			if (ptr->group.empty()) {
				LOG_MESSAGE_f(warning,
				              "Sprite prototype '%s' does not have a category, and will be added to all sprite groups."
				              " Consider giving it a category",
				              ptr->name.c_str())
				return false;
			}

			return !ptr->IsInGroup(group);
		}),
		sprites.end()
	);

	const SpritemapData spritemap_data = GenSpritemap(sprites.data(), sprites.size(), invert_sprites);

	// Texture will delete the sprite* when deleted
	textures_[static_cast<int>(group)] = new Texture(spritemap_data.spriteBuffer, spritemap_data.width, spritemap_data.height);
	spritemapDatas_[static_cast<int>(group)] = (spritemap_data);
}


const jactorio::renderer::RendererSprites::SpritemapData& jactorio::renderer::RendererSprites::GetSpritemap(
	data::Sprite::SpriteGroup group) {
	return spritemapDatas_[static_cast<int>(group)];
}

const jactorio::renderer::Texture* jactorio::renderer::RendererSprites::GetTexture(
	data::Sprite::SpriteGroup group) {
	return textures_[static_cast<int>(group)];
}


jactorio::renderer::RendererSprites::SpritemapData jactorio::renderer::RendererSprites::GenSpritemap(
	data::Sprite** sprites, const uint64_t count, const bool invert_sprites) const {
	LOG_MESSAGE_f(info, "Generating spritemap with %lld tiles...", count);

	// Calculate spritemap dimensions
	unsigned int pixels_x = 0;
	unsigned int pixels_y = 0;

	for (uint64_t i = 0; i < count; ++i) {
		pixels_x += sprites[i]->GetWidth();
		if (sprites[i]->GetHeight() > pixels_y)
			pixels_y = sprites[i]->GetHeight();
	}

	auto* spritemap_buffer = new unsigned char[
		static_cast<unsigned long long>(pixels_x) * pixels_y * 4];

	std::unordered_map<unsigned int, core::QuadPosition> image_positions;


	// Offset the x pixels of each new sprite so they don't overlap each other
	unsigned int x_offset = 0;

	for (uint64_t i = 0; i < count; ++i) {
		const unsigned char* sprite_data = sprites[i]->GetSpritePtr();

		const unsigned int sprite_width  = sprites[i]->GetWidth();
		const unsigned int sprite_height = sprites[i]->GetHeight();

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
			auto& image_position = image_positions[sprites[i]->internalId];

			// Shrinks the dimensions of the images by this amount on all sides
			// Used to avoid texture leaking
			constexpr float coordinate_shrink_amount = 1;

			image_position.topLeft =
				core::Position2{
					static_cast<float>(x_offset) + coordinate_shrink_amount,
					coordinate_shrink_amount
				};

			image_position.bottomRight =
				core::Position2{
					static_cast<float>(x_offset + sprite_width) - coordinate_shrink_amount,
					static_cast<float>(sprite_height) - coordinate_shrink_amount
				};
		}

		x_offset += sprite_width;
	}


	// Normalize positions based on image size to value between 0 - 1
	for (auto& image : image_positions) {
		auto& position = image.second;

		position.topLeft.x /= static_cast<float>(pixels_x);
		position.topLeft.y /= static_cast<float>(pixels_y);

		position.bottomRight.x /= static_cast<float>(pixels_x);
		position.bottomRight.y /= static_cast<float>(pixels_y);
	}


	SpritemapData spritemap_data;
	spritemap_data.spriteBuffer = spritemap_buffer;
	spritemap_data.width        = pixels_x;
	spritemap_data.height       = pixels_y;

	spritemap_data.spritePositions = image_positions;

	return spritemap_data;
}
