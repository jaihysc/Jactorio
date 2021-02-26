// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/logistic/inventory.h"

#include <cassert>
#include <utility>

#include "jactorio.h"

#include "core/resource_guard.h"

using namespace jactorio;

#ifdef JACTORIO_DEBUG_BUILD

#define J_INVENTORY_VERIFY(guard__) \
    Verify();                       \
    CapturingGuard<void()> guard__([&]() { Verify(); })
#else

#define J_INVENTORY_VERIFY(guard__)

#endif


bool game::ItemStack::MatchesFilter(const ItemStack& other) const {
    // This stack must match other stack if filter is set and this stack has an item prototype
    return item == nullptr || other.filter == nullptr || item == other.filter;
}

void game::ItemStack::Clear() noexcept {
    item  = nullptr;
    count = 0;
}

bool game::ItemStack::Empty() const noexcept {
    return item == nullptr;
}

bool game::ItemStack::DropOne(ItemStack& stack) noexcept {
    item = stack.item;
    count++;

    // Empty?
    stack.count--;
    if (stack.count == 0) {
        stack.item = nullptr;
        return true;
    }

    return false;
}

bool game::ItemStack::IsCursor() const noexcept {
    return item != nullptr && item->GetLocalizedName() == proto::Item::kInventorySelectedCursor;
}

void game::ItemStack::Verify() const noexcept {
    if (item == nullptr || count == 0) {
        // Inventory selection cursor exempt from having 0 for count
        if (IsCursor()) {
            return;
        }

        assert(item == nullptr);
        assert(count == 0);
    }
}


// ======================================================================


game::ItemStack& game::Inventory::operator[](const std::size_t index) {
    return inv_[index];
}

const game::ItemStack& game::Inventory::operator[](const std::size_t index) const {
    return inv_[index];
}

void game::Inventory::Resize(const std::size_t size) {
    inv_.resize(size);
}

std::size_t game::Inventory::Size() const {
    return inv_.size();
}

void game::Inventory::Sort() {
    // The inventory must be sorted without moving the selected cursor

    // Copy non-cursor into a new array, sort it, copy it back minding the selection cursor
    std::vector<ItemStack> temp_inv;
    temp_inv.reserve(inv_.size());

    for (const auto& stack : inv_) {
        if (stack.IsCursor() || stack.Empty()) {
            continue;
        }

        temp_inv.push_back(stack);
    }

    // Sort temp inventory (does not contain cursor)
    std::sort(temp_inv.begin(), temp_inv.end(), [](const ItemStack& lhs, const ItemStack& rhs) {
        return lhs.item->GetLocalizedName() < rhs.item->GetLocalizedName();
    });

    // Compress item stacks

    // Behaves as follows:
    //  A A A B  C C
    //  A A A B 2C
    // 2A A   B 2C
    // 3A     B 2C

    // Cannot compress index 0 with something before it
    if (!temp_inv.empty()) {
        for (std::size_t i_selected = temp_inv.size() - 1; i_selected > 0; --i_selected) {

            auto& stack = temp_inv[i_selected];

            if (stack.Empty()) {
                continue;
            }

            // Try and compress selected item
            for (std::size_t i = 0; i < i_selected; ++i) {

                auto& target_stack = temp_inv[i];

                if (target_stack.item != stack.item) {
                    continue;
                }
                // Do not fill into stacks exceeding stack size
                if (target_stack.count > target_stack.item->stackSize) {
                    continue;
                }

                const uint16_t target_free_count = target_stack.item->stackSize - target_stack.count;

                // Fills target stack, has extra remaining
                if (stack.count > target_free_count) {
                    target_stack.count += target_free_count;
                    stack.count -= target_free_count;
                }
                // No remaining after dropping in target stack
                else {
                    target_stack.count += stack.count;

                    stack.Clear();
                    break;
                }
            }
        }
    }


    // Copy sorted inventory back into origin inventory

    assert(temp_inv.size() <= inv_.size());

    std::size_t i_inv = 0;
    for (std::size_t i_temp = 0; i_temp < temp_inv.size(); ++i_temp) {
        if (temp_inv[i_temp].Empty()) {
            continue;
        }

        if (inv_[i_inv].IsCursor()) {
            i_inv++;
        }

        assert(i_inv < inv_.size());
        assert(i_temp < temp_inv.size());

        inv_[i_inv] = temp_inv[i_temp];
        i_inv++;
    }

    // Copy empty spaces into the remainder of the slots
    for (; i_inv < inv_.size(); ++i_inv) {
        if (inv_[i_inv].IsCursor())
            continue;

        inv_[i_inv].Clear();
    }
}

std::pair<bool, size_t> game::Inventory::CanAdd(const ItemStack& stack) const {
    J_INVENTORY_VERIFY(guard);

    // Amount left which needs to be added
    auto remaining_add = stack.count;
    for (size_t i = 0; i < inv_.size(); ++i) {
        const auto& slot = inv_[i];

        if (!stack.MatchesFilter(slot))
            continue;

        // Item of same type
        if (slot.item == stack.item) {
            // Amount that can be added to fill the slot
            const auto max_add_amount = stack.item->stackSize - slot.count;
            if (max_add_amount < 0)
                continue;

            assert(max_add_amount >= 0);

            // Attempting to add more than what is available
            if (max_add_amount >= remaining_add)
                return {true, i};

            remaining_add -= max_add_amount;
        }
        // Empty slot
        else if (slot.item == nullptr)
            return {true, i};
    }

    return {false, 0};
}

proto::Item::StackCount game::Inventory::Add(const ItemStack& stack) {
    J_INVENTORY_VERIFY(guard);

    // Amount left which needs to be added
    auto remaining_add = stack.count;
    for (auto& slot : inv_) {
        if (!stack.MatchesFilter(slot))
            continue;

        // Item of same type
        if (slot.item == stack.item) {
            // Amount that can be added to fill the slot
            const auto max_add_amount = stack.item->stackSize - slot.count;
            if (max_add_amount < 0)
                continue;

            assert(max_add_amount >= 0);

            // Attempting to add more than what is available
            if (max_add_amount >= remaining_add) {
                slot.count += remaining_add;
                return 0;
            }

            slot.count += max_add_amount;
            remaining_add -= max_add_amount;
        }
        // Empty slot
        else if (slot.item == nullptr) {
            slot.item  = stack.item;
            slot.count = remaining_add;

            return 0;
        }
    }

    return remaining_add;
}

bool game::Inventory::AddSub(ItemStack& stack) {
    J_INVENTORY_VERIFY(guard);

    const auto remainder = Add(stack);
    if (remainder == 0) {
        stack.count = 0;
        return true;
    }

    // Subtract difference between what is in stack and what remains, since that is what was added
    stack.count -= stack.count - remainder;
    return false;
}

uint32_t game::Inventory::Count(const proto::Item& item) const {
    J_INVENTORY_VERIFY(guard);

    uint32_t count = 0;
    for (const auto& i : inv_) {
        if (i.item == &item)
            count += i.count;
    }
    return count;
}

const proto::Item* game::Inventory::First() const {
    J_INVENTORY_VERIFY(guard);

    for (const auto& i : inv_) {
        if (i.item != nullptr) {
            assert(i.count != 0);
            return i.item.Get();
        }
    }
    return nullptr;
}


bool game::Inventory::Remove(const proto::Item& item, const uint32_t amount) {
    J_INVENTORY_VERIFY(guard);

    // Not enough to remove
    if (Count(item) < amount)
        return false;

    Delete(item, amount);
    return true;
}

void game::Inventory::Delete(const proto::Item& item, uint32_t amount) {
    J_INVENTORY_VERIFY(guard);

    for (auto& inv_i : inv_) {
        if (inv_i.item == &item) {
            // Enough to remove and move on
            if (amount > inv_i.count) {
                amount -= inv_i.count;
                inv_i = {nullptr, 0};
            }
            // Not enough to remove and move on
            else {
                inv_i.count -= amount;
                if (inv_i.count == 0) {
                    inv_i = {nullptr, 0};
                }

                return;
            }
        }
    }
}


void game::Inventory::Verify() const {
    for (const auto& stack : inv_) {
        stack.Verify();
    }
}


// ======================================================================


bool game::MoveItemstackToIndex(ItemStack& origin_stack, ItemStack& target_stack, const unsigned short mouse_button) {
    assert(mouse_button == 0 || mouse_button == 1); // Only left and right click are currently supported

    // Moving nothing to nothing
    if (origin_stack.item == nullptr && target_stack.item == nullptr)
        return true;

    if (!origin_stack.MatchesFilter(target_stack))
        return false;

    // Items are of the same type
    if (origin_stack.item == target_stack.item) {
        assert(origin_stack.item != nullptr); // Invalid itemstack
        assert(target_stack.item != nullptr); // Invalid itemstack
        assert(origin_stack.count != 0);      // Invalid itemstack
        assert(target_stack.count != 0);      // Invalid itemstack

        assert(origin_stack.item->stackSize > 0); // Invalid itemstack stacksize
        assert(target_stack.item->stackSize > 0); // Invalid itemstack stacksize

        if (mouse_button == 0) {
            // Not exceeding max stack size
            if (origin_stack.count + target_stack.count <= origin_stack.item->stackSize) {
                // Move the item
                target_stack.count += origin_stack.count;

                // Remove the item from the original location
                origin_stack.item  = nullptr;
                origin_stack.count = 0;

                return true;
            }

            // Swap places if same type, and target is full
            if (target_stack.count == target_stack.item->stackSize) {
                std::swap(target_stack, origin_stack);
                return false;
            }

            // Addition of both stacks exceeding max stack size
            // Move origin to reach the max stack size in the target
            const auto move_amount = origin_stack.item->stackSize - target_stack.count;
            origin_stack.count -= move_amount;
            target_stack.count += move_amount;

            return false;
        }


        // Drop 1 to target on right click
        if (mouse_button == 1 && target_stack.count < target_stack.item->stackSize) {
            return target_stack.DropOne(origin_stack);
        }

        return false;
    }

    // Items are not of the same type

    // Items exceeding item stacks
    {
        // It is guaranteed that only one will be a nullptr;
        assert(!(origin_stack.item == nullptr && target_stack.item == nullptr));

        // Origin item exceeding item stack limit
        if (target_stack.item == nullptr) {
            assert(origin_stack.item->stackSize > 0); // Invalid itemstack stacksize

            if (origin_stack.count > origin_stack.item->stackSize) {
                const auto stack_size = origin_stack.item->stackSize;

                origin_stack.count -= stack_size;
                target_stack.count = stack_size;

                target_stack.item = origin_stack.item;
                return false;
            }

            // Drop 1 on right click
            if (mouse_button == 1) {
                return target_stack.DropOne(origin_stack);
            }
        }
        // Target item exceeding item stack limit
        if (origin_stack.item == nullptr) {
            assert(target_stack.item->stackSize > 0); // Invalid itemstack stacksize

            if (target_stack.count > target_stack.item->stackSize) {
                const auto stack_size = target_stack.item->stackSize;

                target_stack.count -= stack_size;
                origin_stack.count = stack_size;

                origin_stack.item = target_stack.item;
                return false;
            }

            // Take half on right click
            if (mouse_button == 1) {
                auto get_take_amount = [&]() -> proto::Item::StackCount {
                    // Never exceed the stack size
                    if (target_stack.count > target_stack.item->stackSize * 2) {
                        return target_stack.item->stackSize;
                    }

                    // Take 1 if there is only 1 remaining
                    if (target_stack.count == 1) {
                        return 1;
                    }

                    return target_stack.count / 2;
                };

                const auto amount = get_take_amount();

                origin_stack.item  = target_stack.item;
                origin_stack.count = amount;

                // Empty?
                target_stack.count -= amount;
                if (target_stack.count == 0)
                    target_stack.item = nullptr;

                return false;
            }
        }
    }

    // Swapping into another stack
    // 1. O -> T: Match target stack's filter filters
    // 2. T -> O: Match origin stack's filter filters
    // 3. O <> T: Match origin && target stack's filter filters
    if ((target_stack.item == nullptr && origin_stack.MatchesFilter(target_stack)) ||
        (origin_stack.item == nullptr && origin_stack.MatchesFilter(target_stack)) ||
        origin_stack.filter == target_stack.filter) {

        const auto item  = origin_stack.item;
        const auto count = origin_stack.count;

        origin_stack.item  = target_stack.item;
        origin_stack.count = target_stack.count;

        target_stack.item  = item;
        target_stack.count = count;
    }

    // Origin item stack is now empty?
    if (origin_stack.count == 0) {
        assert(origin_stack.item == nullptr); // Having no item count must also mean there is no itemstack
        return true;
    }

    return false;
}
