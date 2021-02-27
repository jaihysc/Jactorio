// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_LOGISTIC_INVENTORY_H
#define JACTORIO_INCLUDE_GAME_LOGISTIC_INVENTORY_H
#pragma once

#include "proto/item.h"

#include "data/cereal/serialization_type.h"

#include <cereal/types/vector.hpp>

namespace jactorio::game
{
    class Inventory;

    class ItemStack
    {
        friend Inventory;

    public:
        friend bool operator==(const ItemStack& lhs, const ItemStack& rhs) {
            return std::tie(lhs.item, lhs.count, lhs.filter) == std::tie(rhs.item, rhs.count, rhs.filter);
        }

        friend bool operator!=(const ItemStack& lhs, const ItemStack& rhs) {
            return !(lhs == rhs);
        }


        ///
        /// \remark Comparison between two stacks is reversible
        /// \return true if provided stack matches filter of current stack and can thus be inserted into it
        J_NODISCARD bool MatchesFilter(const ItemStack& other) const;

        ///
        /// Clears item and count, NOT filter
        void Clear() noexcept;


        ///
        /// \return true if stack holds no items
        J_NODISCARD bool Empty() const noexcept;

        ///
        /// \return true if stack >= stack size
        J_NODISCARD bool Full() const noexcept;

        ///
        /// \return true if this stack contains more than its stack size
        J_NODISCARD bool Overloaded() const noexcept;


        ///
        /// \return Available space left in this stack
        J_NODISCARD proto::Item::StackCount FreeCount() const noexcept;


        ///
        /// Deletes provided number of items from current stack
        /// \remark Ensure stack has enough to delete
        void Delete(proto::Item::StackCount amount) noexcept;

        ///
        /// Attempts to drop one item from provided stack to current item stack
        /// \return true if stack is empty
        bool DropOne(ItemStack& stack) noexcept;

        ///
        /// \return true if provided ItemStack holds the selection cursor from the player
        J_NODISCARD bool IsCursor() const noexcept;


        data::SerialProtoPtr<const proto::Item> item = nullptr;
        proto::Item::StackCount count                = 0;

        /// proto::Item which this->item is restricted to
        data::SerialProtoPtr<const proto::Item> filter = nullptr;

        CEREAL_SERIALIZE(archive) {
            archive(item, count, filter);
        }

    private:
        ///
        /// Validates that the contents of this itemstack is valid
        void Verify() const noexcept;
    };

    class Inventory
    {
    public:
        Inventory() = default;
        explicit Inventory(const std::size_t size) : inv_(size) {}

        // stl forwarded

        J_NODISCARD ItemStack& operator[](std::size_t index);
        J_NODISCARD const ItemStack& operator[](std::size_t index) const;

        J_NODISCARD auto begin() {
            return inv_.begin();
        }

        J_NODISCARD auto end() {
            return inv_.end();
        }

        void Resize(std::size_t size);

        J_NODISCARD std::size_t Size() const;


        //

        ///
        /// Sorts by localized name, grouping multiples of the same item into one stack, obeying stack size
        void Sort();

        ///
        /// Returns whether or not item stack can be added to the current inventory
        /// \return true if succeeded, first index which can be added at
        J_NODISCARD std::pair<bool, size_t> CanAdd(const ItemStack& stack) const;

        ///
        /// Adds stack to current inventory
        /// \return Number of items which were NOT added
        proto::Item::StackCount Add(const ItemStack& stack);

        ///
        /// Adds stack to the current inventory, subtracting added amount from stack
        ///
        /// Adds to the inventory left to right, top to bottom | Fill slots of same type if it encounters them
        /// \return false if failed, amount left in stack is remaining amount
        bool AddSub(ItemStack& stack);

        ///
        /// Gets count of item in inventory
        J_NODISCARD uint32_t Count(const proto::Item& item) const;

        ///
        /// Gets the first item within the inventory
        /// \return nullptr if no items were found
        J_NODISCARD const proto::Item* First() const;


        ///
        /// Removes amount of specified item from inventory
        /// \return false if there is insufficient items to remove, inventory unchanged
        bool Remove(const proto::Item& item, uint32_t amount);

        ///
        /// Removes amount of specified item from inventory
        /// \remark DOES NOT check that there are sufficient items to remove, use Remove() if checking is needed
        void Delete(const proto::Item& item, uint32_t amount);


        CEREAL_SERIALIZE(archive) {
            archive(inv_);
        }

    private:
        ///
        /// Validates that the contents of the inventory are valid
        void Verify() const;

        std::vector<ItemStack> inv_;
    };


    ///
    /// Moves itemstack origin to target
    /// When stacking items, the stack prioritizes the target inventory
    /// \remarks This should be only used by the player, machines should use Inventory::Add instead
    /// \return true if origin_stack is empty after the move
    bool MoveItemstackToIndex(ItemStack& origin_stack, ItemStack& target_stack, unsigned short mouse_button);

} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_LOGISTIC_INVENTORY_H
