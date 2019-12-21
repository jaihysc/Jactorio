#ifndef RENDERER_RENDERING_SPRITEMAP_GENERATOR_H
#define RENDERER_RENDERING_SPRITEMAP_GENERATOR_H

#include <unordered_map>

#include "data/prototype/sprite.h"

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
			data::Sprite* spritemap;

			// Image positions retrieved via the path originally given to create the spritemap
			// 0 - 1 positions of the sprite within the spritemap
			// Upper left is 0, 0 - bottom right is 1, 1
			// std::string is internal name of prototype
			std::unordered_map<unsigned int, Image_position> sprite_positions;
		};

		/*!
		 * Generated spritemap will be purely horizontal, all images concatenated side by side <br>
		 * Color in non specified areas of the spritemap are undefined <br>
		 * !!Spritemap pointer must be manually deleted
		 * @param sprites Pointer array to pointers towards sprite prototypes
		 * @param count Count of pointer array
		 */
		Spritemap_data gen_spritemap(data::Sprite** sprites, unsigned short count) const;
	};
}

#endif // RENDERER_RENDERING_SPRITEMAP_GENERATOR_H
