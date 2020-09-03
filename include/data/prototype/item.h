// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_ITEM_ITEM_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_ITEM_ITEM_H
#pragma once

#include "data/cereal/serialization_type.h"
#include "data/prototype/sprite.h"
#include "data/prototype/abstract_proto/item_base.h"

#include <cereal/types/vector.hpp>

namespace jactorio::data
{
	class Entity;
}

namespace jactorio::data
{
	struct ItemStack;
	
	///
	/// Creating a item prototype may omit sprite group, it is automatically defaulted to both gui and terrain
	class Item final : public ItemBase
	{
	public:
		using StackCount = uint16_t;
		using Inventory = std::vector<ItemStack>;

    private:
        static constexpr StackCount kDefaultStackSize_ = 50;

    public:
		// Hard coded item inames
		static constexpr char kInventorySelectedCursor[] = "__core__/inventory-selected-cursor";
		static constexpr char kResetIname[]              = "__core__/reset";


		PROTOTYPE_CATEGORY(item);

		Item()
			: stackSize(kDefaultStackSize_) {
		}

		explicit Item(Sprite* sprite)
			: ItemBase(sprite), stackSize(kDefaultStackSize_) {
		}

		/// If this item belongs to an entity - otherwise nullptr
		Entity* entityPrototype              = nullptr;


        ///
		/// \brief Number of items which can be together
		PYTHON_PROP_REF(StackCount, stackSize);


		void ValidatedPostLoad() override {
			sprite->DefaultSpriteGroup({Sprite::SpriteGroup::terrain, Sprite::SpriteGroup::gui});
		}
	};

	struct ItemStack
	{
		SerialProtoPtr<const Item> item = nullptr;
		Item::StackCount count = 0;

		/// data::Item which this->item is restricted to
		SerialProtoPtr<const Item> filter = nullptr;

		CEREAL_SERIALIZE(archive) {
			archive(item, count, filter);
		}
	};

}

#endif //JACTORIO_INCLUDE_DATA_PROTOTYPE_ITEM_ITEM_H
