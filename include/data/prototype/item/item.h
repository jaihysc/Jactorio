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
		Sprite* sprite = nullptr;
		unsigned short stack_size = 50;

		Item() = default;
		explicit Item(Sprite* sprite) : sprite(sprite) {}
	};

	// Makes arrays holding items more clear than just unsigned int
	// Item* and amount in current stack
	// Left: Item prototype
	// Right: Item count
	using item_stack = std::pair<Item*, unsigned short>;
}

#endif // DATA_PROTOTYPE_ITEM_ITEM_H
