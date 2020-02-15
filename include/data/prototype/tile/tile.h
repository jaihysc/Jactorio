#ifndef DATA_PROTOTYPE_TILE_TILE_H
#define DATA_PROTOTYPE_TILE_TILE_H

#include "data/prototype/prototype_base.h"
#include "data/prototype/sprite.h"

namespace jactorio::data
{
	/**
	 * Tiles which are auto-placed by the world generator <br>
	 * Configure how the world generator places these tiles with a Noise_layer
	 */
	class Tile : public Prototype_base
	{
	public:		
		Tile()
			: is_water(false), sprite(nullptr) {
		}

		/**
		 * @param sprite_ptr Points to a sprite prototype
		 */
		explicit Tile(Sprite* sprite_ptr)
			: is_water(false), sprite(sprite_ptr) {
		}

		~Tile() override = default;

		/**
		 * If true, resources will not spawn on this and player cannot navigate onto it
		 */
		PYTHON_PROP_REF(Tile, bool, is_water)

		PYTHON_PROP(Tile, Sprite*, sprite)

		
		void post_load_validate() const override;
	};

	inline void Tile::post_load_validate() const {
		J_DATA_ASSERT(sprite != nullptr, "Sprite was not provided");
	}
}

#endif // DATA_PROTOTYPE_TILE_TILE_H
