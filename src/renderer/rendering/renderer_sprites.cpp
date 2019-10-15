#include "renderer/rendering/renderer_sprites.h"

#include "core/filesystem.h"

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
Renderer_sprites::gen_spritemap(
	std::string* image_paths, const unsigned short count) const {

	unsigned int image_index = 0;

	// Keep track of the positions of each image
	std::unordered_map<std::string, Image_position> image_positions;
	unsigned int image_position_offset_x = 0;


	auto* concat_image_old = new sf::Image{};
	// This is a pointer to avoid copying when moving working_image into this
	concat_image_old->loadFromFile(
		core::filesystem::resolve_path(image_paths[image_index++])
	);
	set_image_positions(image_positions[image_paths[image_index - 1]],
	                    concat_image_old->getSize(), image_position_offset_x);


	// Repeatedly concat 1 image into another, gradually getting larger
	while (image_index < count) {
		// concat_image_old is the previous concat results
		// concat_image_new is the new image

		sf::Image concat_image_new{};
		concat_image_new.loadFromFile(
			core::filesystem::resolve_path(image_paths[image_index++])
		);

		const sf::Vector2u image_old_prop = concat_image_old->getSize();
		const sf::Vector2u image_new_prop = concat_image_new.getSize();


		// -----------------------------------------------
		// Store image positions, 0, 0 is top left
		// n, n bottom right

		// Initialize the unordered_map with path names of each image as key
		set_image_positions(image_positions[image_paths[image_index - 1]],
		                    image_new_prop, image_position_offset_x);


		// -----------------------------------------------
		// Add the image pixels X together
		// image pixels Y will be the largest value of both images

		const unsigned int pixels_y = (image_old_prop.y > image_new_prop.y)
			                              ? image_old_prop.y
			                              : image_new_prop.y;

		auto* working_image = new sf::Image{};
		working_image->create(image_old_prop.x + image_new_prop.x, pixels_y);


		// Concat the images
		for (unsigned int y = 0; y < image_old_prop.y; ++y)
			for (unsigned int x = 0; x < image_old_prop.x; ++x)
				working_image->setPixel(x, y,
				                        concat_image_old->getPixel(x, y));
		delete concat_image_old;


		const unsigned int offset = image_old_prop.x;
		for (unsigned int y = 0; y < image_new_prop.y; ++y)
			for (unsigned int x = 0; x < image_new_prop.x; ++x)
				working_image->setPixel(offset + x, y,
				                        concat_image_new.getPixel(x, y));

		// Move working image to be concat again
		concat_image_old = working_image;
	}


	// Normalize positions based on image size to value between 0 - 1
	const sf::Vector2u spritemap_prop = concat_image_old->getSize();
	for (auto& image : image_positions) {
		auto& position = image.second;

		position.top_left.x /= spritemap_prop.x;
		position.top_left.y /= spritemap_prop.y;

		position.top_right.x /= spritemap_prop.x;
		position.top_right.y /= spritemap_prop.y;

		position.bottom_left.x /= spritemap_prop.x;
		position.bottom_left.y /= spritemap_prop.y;

		position.bottom_right.x /= spritemap_prop.x;
		position.bottom_right.y /= spritemap_prop.y;
	}


	Spritemap_data spritemap_data{};
	spritemap_data.spritemap = *concat_image_old;
	spritemap_data.sprite_positions = image_positions;

	return spritemap_data;
}
