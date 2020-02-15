#ifndef DATA_PROTOTYPE_ITEM_ITEM_H
#define DATA_PROTOTYPE_ITEM_ITEM_H

#include <utility>

#include "data/prototype/prototype_base.h"
#include "data/prototype/sprite.h"
#include "data/prototype/item/item_base.h"

namespace jactorio::data
{
	class Item final : public Item_base
	{
	public:
		Item()
			: stack_size(50) {
		}

		explicit Item(Sprite* sprite)
			: Item_base(sprite), stack_size(50) {
		}

		/**
		 * If this item belongs to an entity - otherwise nullptr
		 */
		void* entity_prototype = nullptr;
		data_category entity_prototype_category = data_category::none;

		
		/**
		 * Number of items which can be together <br>
		 * Default to 50
		 */
		PYTHON_PROP_REF(Item, unsigned short, stack_size)
	};

	// Makes arrays holding items more clear than just unsigned int
	// Item* and amount in current stack
	// Left: Item prototype
	// Right: Item count
	using item_stack = std::pair<Item*, uint16_t>;
}

#endif // DATA_PROTOTYPE_ITEM_ITEM_H
