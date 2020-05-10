// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 01/20/2020

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_ITEM_ITEM_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_ITEM_ITEM_H
#pragma once

#include <utility>

#include "data/prototype/prototype_base.h"
#include "data/prototype/sprite.h"
#include "data/prototype/item/item_base.h"

namespace jactorio::data
{
	class Entity;
}

namespace jactorio::data
{
	class Item final : public ItemBase
	{
	public:
		using ItemStack = uint16_t;
		static constexpr ItemStack kDefaultStackSize = 50;

		PROTOTYPE_CATEGORY(item);

		Item()
			: stackSize(kDefaultStackSize) {
		}

		explicit Item(Sprite* sprite)
			: ItemBase(sprite), stackSize(kDefaultStackSize) {
		}

		/// If this item belongs to an entity - otherwise nullptr
		Entity* entityPrototype              = nullptr;
		DataCategory entityPrototypeCategory = DataCategory::none;


		///
		/// \brief Number of items which can be together
		PYTHON_PROP_REF(Item, ItemStack, stackSize)
	};


	// Makes arrays holding items more clear than just unsigned int
	// Item* and amount in current stack
	// Left: Item prototype
	// Right: Item count
	using ItemStack = std::pair<Item*, Item::ItemStack>;
}

#endif //JACTORIO_INCLUDE_DATA_PROTOTYPE_ITEM_ITEM_H
