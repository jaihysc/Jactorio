#include "renderer/rendering/spritemap_generator.h"

#include "core/logger.h"

void jactorio::renderer::Renderer_sprites::set_image_positions(
	Image_position& image_position,
	const sf::Vector2u image_dimensions,
	unsigned int& offset_x) {

	image_position.top_left = Position_pair{static_cast<float>(offset_x), 0};
	image_position.top_right = Position_pair{
		static_cast<float>(offset_x + image_dimensions.x), 0
	};

	image_position.bottom_left =
		Position_pair{
			static_cast<float>(offset_x), static_cast<float>(image_dimensions.y)
		};
	image_position.bottom_right =
		Position_pair{
			static_cast<float>(offset_x + image_dimensions.x),
			static_cast<float>(image_dimensions.y)
		};

	offset_x += image_dimensions.x;
}


jactorio::renderer::Renderer_sprites::Spritemap_data jactorio::renderer::
Renderer_sprites::gen_spritemap(data::Sprite** images, const unsigned short count) const {
	LOG_MESSAGE_f(info, "Generating spritemap with %d tiles...", count);
	
	std::unordered_map<std::string, Image_position> image_positions;
	unsigned int image_position_offset_x = 0;

	// Calculate spritemap dimensions
	unsigned int pixels_x = 0;
	unsigned int pixels_y = 0;
	
	for (int i = 0; i < count; ++i) {
		sf::Vector2u size = images[i]->sprite_image.getSize();
		
		pixels_x += size.x;
		if (size.y > pixels_y)
			pixels_y = size.y;
	}
	
	sf::Image spritemap{};
	spritemap.create(pixels_x, pixels_y, sf::Color{0, 0, 0});

	// Offset the x pixels of each new image so they don't overwrite each other
	unsigned int x_offset = 0;
	for (int i = 0; i < count; ++i) {
		sf::Image image = images[i]->sprite_image;
		image.flipVertically();

		const auto image_size = image.getSize();
		// Concat the images
		for (unsigned int y = 0; y < image_size.y; ++y)
			for (unsigned int x = 0; x < image_size.x; ++x)
				spritemap.setPixel(x_offset + x, y, image.getPixel(x, y));

		x_offset += image_size.x;

		// Keep track of image positions within the spritemap
		set_image_positions(image_positions[images[i]->name],
		                    image_size, image_position_offset_x);
	}


	// Normalize positions based on image size to value between 0 - 1
	for (auto& image : image_positions) {
		auto& position = image.second;

		position.top_left.x /= pixels_x;
		position.top_left.y /= pixels_y;

		position.top_right.x /= pixels_x;
		position.top_right.y /= pixels_y;

		position.bottom_left.x /= pixels_x;
		position.bottom_left.y /= pixels_y;

		position.bottom_right.x /= pixels_x;
		position.bottom_right.y /= pixels_y;
	}


	Spritemap_data spritemap_data;
	spritemap_data.spritemap = spritemap;
	spritemap_data.sprite_positions = image_positions;

	return spritemap_data;
}
