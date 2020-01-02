#ifndef DATA_PROTOTYPE_ITEM_ITEM_H
#define DATA_PROTOTYPE_ITEM_ITEM_H

#include <utility>

#include "data/prototype/prototype_base.h"
#include "data/prototype/sprite.h"

namespace jactorio::data
{
	class Item : public Prototype_base
	{
	public:
		Item()
			:sprite(nullptr), stack_size(50) {
		}

		explicit Item(Sprite* sprite)
			: sprite(sprite) {
			
		}

		~Item() override = default;

		
		PYTHON_PROP(Item, Sprite*, sprite)

		/**
		 * Number of items which can be together
		 */
		PYTHON_PROP_REF(Item, unsigned short, stack_size)
	};

	// Makes arrays holding items more clear than just unsigned int
	// Item* and amount in current stack
	// Left: Item prototype
	// Right: Item count
	using item_stack = std::pair<Item*, unsigned short>;
}

#endif // DATA_PROTOTYPE_ITEM_ITEM_H
