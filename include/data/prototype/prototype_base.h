#ifndef DATA_PROTOTYPE_PROTOTYPE_BASE_H
#define DATA_PROTOTYPE_PROTOTYPE_BASE_H

#include <SFML/Graphics/Image.hpp>
#include <string>
#include "core/filesystem.h"

namespace jactorio::data
{
	class Prototype_base
	{
	public:
		/**
		 * Internal name <br>
		 * MUST BE unique
		 */
		std::string name;
		
		/**
		 * Sprite for the prototype
		 */
		sf::Image sprite;

		/**
		 * Loads a sprite from sprite_path into member sprite <br>
		 * Do not include ~/data/
		 */
		void load_sprite(const std::string& sprite_path) {
			sprite = sf::Image();

			const std::string path = core::filesystem::resolve_path("~/data/" + sprite_path);
			sprite.loadFromFile(path);
		}
	};
}

#endif // DATA_PROTOTYPE_PROTOTYPE_BASE_H
