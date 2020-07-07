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
	///
	/// Creating a item prototype may omit sprite group, it is automatically defaulted to both gui and terrain
	class Item final : public ItemBase
	{
	public:
		using StackCount = uint16_t;
		using Stack = std::pair<const Item*, StackCount>;
		using Inventory = std::vector<Stack>;

	private:
		static constexpr StackCount kDefaultStackSize = 50;

	public:
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
		PYTHON_PROP_REF(Item, StackCount, stackSize)


		void ValidatedPostLoad() override {
			sprite->DefaultSpriteGroup({Sprite::SpriteGroup::terrain, Sprite::SpriteGroup::gui});
		}
	};
}

#endif //JACTORIO_INCLUDE_DATA_PROTOTYPE_ITEM_ITEM_H
