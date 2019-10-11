#include <SFML/Graphics/Image.hpp>
#include "renderer/rendering/renderer_sprites.h"

jactorio::renderer::Renderer_sprites::Spritemap_data jactorio::renderer::Renderer_sprites::gen_spritemap(
	Sprite* sprites, const unsigned short count) {

	Spritemap_data spritemap_data{};

	// Get spritemap size
	{
		unsigned int spritemap_pixels_x = 0;  // Adds all the pixels_x together
		unsigned int spritemap_pixels_y = 0;  // This will be the highest pixels_y
		
		for (auto i = 0; i < count; ++i) {
			spritemap_pixels_x += sprites[i].pixels_x;
			
			if (sprites[i].pixels_y > spritemap_pixels_y)
				spritemap_pixels_y = sprites[i].pixels_y;
		}

		spritemap_data.spritemap.pixels_x = spritemap_pixels_x;
		spritemap_data.spritemap.pixels_y = spritemap_pixels_y;
	}
	
	sf::Image image;
	image.create(
		spritemap_data.spritemap.pixels_x, 
		spritemap_data.spritemap.pixels_y, sf::Color::Black);
	
	return spritemap_data;
}
