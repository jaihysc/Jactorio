#ifndef DATA_PROTOTYPE_TILE_H
#define DATA_PROTOTYPE_TILE_H

#include "data/prototype/prototype_base.h"
#include "data/prototype/sprite.h"

namespace jactorio::data
{
	class Tile : public Prototype_base
	{
	public:
		Tile() = default;

		/**
		 * @param sprite_ptr Points to a sprite prototype
		 */
		explicit Tile(Sprite* sprite_ptr)
			: sprite_ptr(sprite_ptr) {}

		Sprite* sprite_ptr = nullptr;
	};
}

#endif // DATA_PROTOTYPE_TILE_H
