#ifndef DATA_PROTOTYPE_SPRITE_H
#define DATA_PROTOTYPE_SPRITE_H

#include <SFML/Graphics/Image.hpp>
#include <string>

#include "prototype_base.h"
#include "core/filesystem.h"

namespace jactorio::data
{
	/**
	 * Wrapper for sf::sprite
	 */
	class Sprite : public Prototype_base
	{
	public:
		Sprite() = default;

		explicit Sprite(const std::string& sprite_path) {
			load_sprite(sprite_path);
		}

		/**
		 * Named sprite_image to avoid confusion with Sprite class
		 */
		sf::Image sprite_image;
		
		/**
		 * Loads a sprite from sprite_path into member sprite <br>
		 * Do not include ~/data/
		 */
		void load_sprite(const std::string& sprite_path) {
			sprite_image = sf::Image();
		
			const std::string path = core::filesystem::resolve_path("~/data/" + sprite_path);
			sprite_image.loadFromFile(path);
		}
	};
}

#endif // DATA_PROTOTYPE_SPRITE_H
