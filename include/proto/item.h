// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_PROTO_ITEM_H
#define JACTORIO_INCLUDE_PROTO_ITEM_H
#pragma once

#include "data/cereal/serialization_type.h"
#include "proto/abstract/item_base.h"

#include <cereal/types/vector.hpp>

namespace jactorio::proto
{
    class Entity;
}

namespace jactorio::proto
{
    struct ItemStack;

    ///
    /// Creating a item prototype may omit sprite group, it is automatically defaulted to both gui and terrain
    class Item final : public ItemBase
    {
    public:
        using StackCount = uint16_t;
        using Inventory  = std::vector<ItemStack>;

    private:
        static constexpr StackCount kDefaultStackSize = 50;

    public:
        // Hard coded item inames
        static constexpr char kInventorySelectedCursor[] = "__core__/inventory-selected-cursor";
        static constexpr char kResetIname[]              = "__core__/reset";


        PROTOTYPE_CATEGORY(item);

        Item() : stackSize(kDefaultStackSize) {}

        explicit Item(Sprite* sprite) : ItemBase(sprite), stackSize(kDefaultStackSize) {}

        /// If this item belongs to an entity - otherwise nullptr
        Entity* entityPrototype = nullptr;


        ///
        /// Number of items which can be together
        PYTHON_PROP_REF(StackCount, stackSize);


        void ValidatedPostLoad() override;
    };

    struct ItemStack
    {
        data::SerialProtoPtr<const Item> item = nullptr;
        Item::StackCount count                = 0;

        /// proto::Item which this->item is restricted to
        data::SerialProtoPtr<const Item> filter = nullptr;

        CEREAL_SERIALIZE(archive) {
            archive(item, count, filter);
        }
    };

} // namespace jactorio::proto

#endif // JACTORIO_INCLUDE_PROTO_ITEM_H
