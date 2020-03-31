// 
// item.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 01/20/2020
// Last modified: 03/14/2020
// 

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_ITEM_ITEM_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_ITEM_ITEM_H
#pragma once

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

#endif //JACTORIO_INCLUDE_DATA_PROTOTYPE_ITEM_ITEM_H
