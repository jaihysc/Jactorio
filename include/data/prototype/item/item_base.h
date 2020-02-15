#ifndef DATA_PROTOTYPE_ITEM_ITEM_BASE_H
#define DATA_PROTOTYPE_ITEM_ITEM_BASE_H

#include "data/prototype/prototype_base.h"
#include "data/prototype/sprite.h"

namespace jactorio::data
{
	/**
	 * The basic abstract class for items
	 */
	class Item_base : public Prototype_base
	{
	protected:
		Item_base() = default;
		
		explicit Item_base(Sprite* sprite)
			: sprite(sprite) {
		}
		
	public:
		PYTHON_PROP_I(Item_base, Sprite*, sprite, nullptr);
		
		void post_load_validate() const override;
	};

	inline void Item_base::post_load_validate() const {
		J_DATA_ASSERT(sprite != nullptr, "Sprite was not specified")
	}
}

#endif // DATA_PROTOTYPE_ITEM_ITEM_BASE_H
