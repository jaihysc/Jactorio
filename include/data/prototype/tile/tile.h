#ifndef DATA_PROTOTYPE_TILE_TILE_H
#define DATA_PROTOTYPE_TILE_TILE_H

#include "data/prototype/prototype_base.h"
#include "data/prototype/sprite.h"

namespace jactorio::data
{
	/**
	 * Items which are auto-placed by the world generator <br>
	 * Configure how the world generator places these tiles with a Noise_layer
	 */
	class Tile : public Prototype_base
	{
	public:
		/**
		 * If true, resources will not spawn on this and player cannot navigate onto it
		 */
		bool is_water = false;
		
		Tile() = default;

		/**
		 * @param sprite_ptr Points to a sprite prototype
		 */
		explicit Tile(Sprite* sprite_ptr)
			: sprite_ptr(sprite_ptr) {}

		Sprite* sprite_ptr = nullptr;
	};
}

#endif // DATA_PROTOTYPE_TILE_TILE_H