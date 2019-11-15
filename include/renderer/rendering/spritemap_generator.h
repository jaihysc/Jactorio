#ifndef RENDERER_RENDERING_SPRITEMAP_GENERATOR_H
#define RENDERER_RENDERING_SPRITEMAP_GENERATOR_H

#include <unordered_map>
#include <string>
#include <SFML/Graphics/Image.hpp>

#include "data/prototype/prototype_base.h"

// Generates spritemaps on initialization with tile sprites
// - Concatenate sprite into spritemap
// - Location of a sprite within spritemap
namespace jactorio::renderer
{
	class Renderer_sprites
	{
	public:

		struct Position_pair
		{
			float x;
			float y;
		};

		struct Image_position
		{
			Position_pair top_left;
			Position_pair top_right;
			Position_pair bottom_left;
			Position_pair bottom_right;
		};

		struct Spritemap_data
		{
			sf::Image spritemap;

			// Image positions retrieved via the path originally given to create the spritemap
			// 0 - 1 positions of the sprite within the spritemap
			// Upper left is 0, 0 - bottom right is 1, 1
			// std::string is internal name of prototype
			std::unordered_map<std::string, Image_position> sprite_positions;
		};

		/*!
		 * Generated spritemap will be purely horizontal, all images concatenated side by side
		 */
		Spritemap_data gen_spritemap(data::Prototype_base* images, unsigned short count) const;


	private:
		static void set_image_positions(Image_position& image_position,
		                                sf::Vector2u image_dimensions,
		                                unsigned int& offset_x);
	};
}

#endif // RENDERER_RENDERING_SPRITEMAP_GENERATOR_H
