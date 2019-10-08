#include <SFML/Graphics/Image.hpp>
#include "renderer/rendering/renderer_sprites.h"

jactorio::renderer::Renderer_sprites::Spritemap_data
	jactorio::renderer::Renderer_sprites::gen_spritemap(Sprite* sprites, unsigned short count) {

	sf::Image image;
	image.create(20, 20, sf::Color::Black);
	
	return Spritemap_data{};
}
