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

bool game::ItemStack::DropOne(ItemStack& stack) {
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
    std::vector<ItemStack> sorted_inv;
    sorted_inv.reserve(inv_.size());
    for (const auto& stack : inv_) {
        // Skip the cursor
        if (stack.item == nullptr || stack.item->GetLocalizedName() == proto::Item::kInventorySelectedCursor) {
            continue;
        }

        sorted_inv.push_back(stack);
    }

    // Sort temp inventory (does not contain cursor)
    std::sort(sorted_inv.begin(), sorted_inv.end(), [](const ItemStack a, const ItemStack b) {
        const auto& a_name = a.item->GetLocalizedName();
        const auto& b_name = b.item->GetLocalizedName();

        return a_name < b_name;
    });

    // Compress item stacks
    if (sorted_inv.size() > 1) {
        // Cannot compress slot 0 with anything before it
        for (auto sort_inv_index = sorted_inv.size() - 1; sort_inv_index > 0; --sort_inv_index) {

            auto buffer_item_count = sorted_inv[sort_inv_index].count;
            const auto stack_size  = sorted_inv[sort_inv_index].item->stackSize;

            // Find index which the same item type begins
            auto stack_compress_begin = sort_inv_index;
            while (sorted_inv[stack_compress_begin - 1].item == sorted_inv[sort_inv_index].item) {
                stack_compress_begin--;
                if (stack_compress_begin == 0)
                    break;
            }


            // Compress stacks
            for (; stack_compress_begin < sort_inv_index; ++stack_compress_begin) {

                // If item somehow exceeds stack do not attempt to stack into it
                if (sorted_inv[stack_compress_begin].count > stack_size)
                    continue;

                // Amount which can be dropped is space left until reaching stack size
                const uint16_t max_drop_amount = stack_size - sorted_inv[stack_compress_begin].count;

                // Has enough to max current stack and move on
                if (buffer_item_count > max_drop_amount) {
                    sorted_inv[stack_compress_begin].count = stack_size;
                    buffer_item_count -= max_drop_amount;
                }
                // Not enough to drop and move on
                else {
                    sorted_inv[stack_compress_begin].count += buffer_item_count;
                    sorted_inv[sort_inv_index].item = nullptr;
                    buffer_item_count               = 0;
                    break;
                }
            }
            // Did not drop all items off
            if (buffer_item_count > 0) {
                sorted_inv[sort_inv_index].count = buffer_item_count;
            }
        }
    }


    // Copy sorted inventory back into origin inventory
    bool has_empty_slot = false;

    std::size_t start          = 0; // The index of the first blank slot post sorting
    std::size_t inv_temp_index = 0;

    for (size_t i = 0; i < inv_.size(); ++i) {
        // Skip the cursor
        if (inv_[i].item != nullptr && inv_[i].item->GetLocalizedName() == proto::Item::kInventorySelectedCursor)
            continue;

        // Iterated through every item in the sorted inventory
        if (inv_temp_index >= sorted_inv.size()) {
            has_empty_slot = true;
            start          = i;
            goto loop_exit;
        }
        // Omit empty gaps in sorted inv from the compressing process
        while (sorted_inv[inv_temp_index].item == nullptr) {
            inv_temp_index++;
            if (inv_temp_index >= sorted_inv.size()) {
                has_empty_slot = true;
                start          = i;
                goto loop_exit;
            }
        }

        inv_[i] = sorted_inv[inv_temp_index++];
    }
loop_exit:

    if (!has_empty_slot)
        return;

    // Copy empty spaces into the remainder of the slots
    for (auto i = start; i < inv_.size(); ++i) {
        // Skip the cursor
        if (inv_[i].item != nullptr && inv_[i].item->GetLocalizedName() == proto::Item::kInventorySelectedCursor)
            continue;

        inv_[i] = {nullptr, 0};
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

uint32_t game::Inventory::Count(const proto::Item* item) const {
    J_INVENTORY_VERIFY(guard);

    uint32_t count = 0;
    for (const auto& i : inv_) {
        if (i.item == item)
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


bool game::Inventory::Remove(const proto::Item* item, const uint32_t amount) {
    J_INVENTORY_VERIFY(guard);

    // Not enough to remove
    if (Count(item) < amount)
        return false;

    Delete(item, amount);
    return true;
}

void game::Inventory::Delete(const proto::Item* item, uint32_t amount) {
    J_INVENTORY_VERIFY(guard);

    for (auto& inv_i : inv_) {
        if (inv_i.item == item) {
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
        if (stack.item == nullptr || stack.count == 0) {
            // Inventory selection cursor exempt from having 0 for count
            if (stack.item != nullptr && stack.item->name == proto::Item::kInventorySelectedCursor) {
                continue;
            }

            assert(stack.item == nullptr);
            assert(stack.count == 0);
        }
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
