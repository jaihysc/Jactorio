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


bool game::ItemStack::Accepts(const ItemStack& other) const {
    assert(&other != this);

    // No point accepting empty stacks
    if (other.Empty()) {
        return false;
    }

    assert(other.item != nullptr);
    // Check other stack's item when matching filter, since other guaranteed to have item
    // whereas filter may be nullptr

    if (Empty()) {
        if (filter == nullptr) {
            return true;
        }

        return filter == other.item;
    }

    assert(item != nullptr);
    return item == other.item;
}

void game::ItemStack::Clear() noexcept {
    item  = nullptr;
    count = 0;
}

bool game::ItemStack::Empty() const noexcept {
    return item == nullptr;
}

bool game::ItemStack::Full() const noexcept {
    assert(item != nullptr);
    return count >= item->stackSize;
}

bool game::ItemStack::Overloaded() const noexcept {
    assert(item != nullptr);
    return count > item->stackSize;
}


uint16_t game::ItemStack::FreeCount() const noexcept {
    if (Overloaded())
        return 0;

    return item->stackSize - count;
}

void game::ItemStack::Delete(const proto::Item::StackCount amount) noexcept {
    assert(count >= amount);

    count -= amount;

    if (count == 0) {
        Clear();
    }
}

bool game::ItemStack::DropOne(ItemStack& stack) noexcept {
    assert(!stack.Empty());

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


                const auto target_free_count = target_stack.FreeCount();

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

    assert(!stack.Empty());

    // Amount left which needs to be added
    auto remaining_add = stack.count;
    for (size_t i = 0; i < inv_.size(); ++i) {
        const auto& target_stack = inv_[i];

        if (!target_stack.Accepts(stack)) {
            continue;
        }

        if (target_stack.item == stack.item) {
            const auto target_free_count = target_stack.FreeCount();

            // Attempting to add more than what is available
            if (target_free_count >= remaining_add)
                return {true, i};

            remaining_add -= target_free_count;
        }
        else if (target_stack.Empty()) {
            return {true, i};
        }
    }

    return {false, 0};
}

proto::Item::StackCount game::Inventory::Add(const ItemStack& stack) {
    J_INVENTORY_VERIFY(guard);

    assert(!stack.Empty());

    // Amount left which needs to be added
    auto remaining_add = stack.count;

    for (auto& inv_stack : inv_) {
        if (!inv_stack.Accepts(stack))
            continue;

        if (inv_stack.item == stack.item) {
            const auto free_count = inv_stack.FreeCount();

            // Inv stack can hold amount to add
            if (free_count >= remaining_add) {
                inv_stack.count += remaining_add;
                return 0;
            }

            inv_stack.count += free_count;
            remaining_add -= free_count;
        }
        else if (inv_stack.Empty()) {
            inv_stack.item  = stack.item;
            inv_stack.count = remaining_add;

            return 0;
        }
    }

    return remaining_add;
}

uint32_t game::Inventory::Count(const proto::Item& item) const {
    J_INVENTORY_VERIFY(guard);

    uint32_t count = 0;
    for (const auto& stack : inv_) {
        if (stack.item == &item) {
            count += stack.count;
        }
    }
    return count;
}

const proto::Item* game::Inventory::First() const {
    J_INVENTORY_VERIFY(guard);

    for (const auto& stack : inv_) {
        if (!stack.Empty()) {
            assert(stack.count != 0);
            return stack.item.Get();
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

    for (auto& stack : inv_) {
        if (stack.item == &item) {
            // Delete stack, still has more to delete
            if (amount > stack.count) {
                amount -= stack.count;
                stack.Clear();
            }
            // Done deleting at this stack
            else {
                stack.Delete(amount);
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


bool MoveSameItem(game::ItemStack& origin_stack, game::ItemStack& target_stack, const bool right_click) {
    assert(!origin_stack.Empty());
    assert(!target_stack.Empty());

    if (!right_click) {
        // Not exceeding max stack size
        if (target_stack.FreeCount() >= origin_stack.count) {
            target_stack.count += origin_stack.count;
            origin_stack.Clear();

            return true;
        }

        // Swap places if same type, and target is full
        if (target_stack.Full()) {
            std::swap(target_stack, origin_stack);
            return false;
        }

        // Target stack does not have enough space to fit origin stack
        // Move origin to reach the max stack size in the target
        const auto free_count = target_stack.FreeCount();
        origin_stack.count -= free_count;
        target_stack.count += free_count;

        return false;
    }


    // Drop 1 to target on right click
    if (right_click && !target_stack.Full()) {
        return target_stack.DropOne(origin_stack);
    }

    return false;
}

bool MoveDiffType(game::ItemStack& origin_stack, game::ItemStack& target_stack, const bool right_click) {
    // At most one will be a empty
    assert(!(origin_stack.Empty() && target_stack.Empty()));

    auto swap_contents = [](game::ItemStack& lhs, game::ItemStack& rhs) {
        std::swap(lhs.item, rhs.item);
        std::swap(lhs.count, rhs.count);
    };

    if (target_stack.Empty()) {
        if (!target_stack.Accepts(origin_stack)) {
            return false;
        }

        if (origin_stack.Overloaded()) {
            const auto stack_size = origin_stack.item->stackSize;

            origin_stack.count -= stack_size;
            target_stack.count = stack_size;

            target_stack.item = origin_stack.item;
            return false;
        }

        if (!right_click) {
            swap_contents(origin_stack, target_stack);
            return true;
        }
        // Right click
        return target_stack.DropOne(origin_stack);
    }

    if (origin_stack.Empty()) {
        if (!origin_stack.Accepts(target_stack)) {
            return true;
        }

        if (!right_click) {
            if (target_stack.Overloaded()) {
                const auto stack_size = target_stack.item->stackSize;

                target_stack.count -= stack_size;
                origin_stack.count = stack_size;

                origin_stack.item = target_stack.item;
                return false;
            }

            swap_contents(origin_stack, target_stack);
            return false;
        }

        // Right click
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

        target_stack.Delete(amount);

        return false;
    }

    // Both stacks non empty
    if (target_stack.filter == origin_stack.filter) {
        swap_contents(origin_stack, target_stack);
    }

    return false;
}

bool game::MoveItemstackToIndex(ItemStack& origin_stack, ItemStack& target_stack, const unsigned short mouse_button) {
    assert(mouse_button == 0 || mouse_button == 1); // Only left and right click are currently supported

    // Moving nothing to nothing
    if (origin_stack.Empty() && target_stack.Empty())
        return true;

    if (origin_stack.item == target_stack.item) {
        return MoveSameItem(origin_stack, target_stack, mouse_button != 0);
    }
    return MoveDiffType(origin_stack, target_stack, mouse_button != 0);
}
