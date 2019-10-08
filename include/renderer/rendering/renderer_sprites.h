#ifndef RENDERER_RENDERING_RENDERER_SPRITES_H
#define RENDERER_RENDERING_RENDERER_SPRITES_H

#include <unordered_map>
#include <string>

// Generates spritemaps on initialization with tile sprites
// - Concatenate sprite into spritemap
// - Location of a sprite within spritemap retrieved with a getter
namespace jactorio
{
	namespace renderer
	{
		class Renderer_sprites
		{
		public:
			struct Sprite
			{
				char* data;
				unsigned short pixels_x;
				unsigned short pixels_y;
			};

			struct Sprite_position
			{
				float top_left;
				float top_right;
				float bottom_left;
				float bottom_right;
			};
		
			struct Spritemap_data
			{
				Sprite spritemap;
				// 0 - 1 positions of the sprite within the spritemap
				// Upper left is 0, 0 - bottom right is 1, 1
				std::unordered_map<std::string, Sprite_position> sprite_positions;
			};
			
			/*!
			 * Generated spritemap will be purely horizontal, all sprites concatenated side by side
			 */
			static Spritemap_data gen_spritemap(Sprite* sprites, unsigned short count);
		};

	}
}

#endif // RENDERER_RENDERING_RENDERER_SPRITES_H
