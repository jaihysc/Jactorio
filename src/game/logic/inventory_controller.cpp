// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 01/20/2020

#include "game/logic/inventory_controller.h"

#include <cassert>
#include <utility>

#include "jactorio.h"

using namespace jactorio;

#ifdef JACTORIO_DEBUG_BUILD

#define J_INVENTORY_VERIFY(inventory__, guard__)\
	InventoryVerify(inventory__);\
	core::CapturingGuard<void()> guard__([&]() { InventoryVerify(inventory__); })
#else

#define J_INVENTORY_VERIFY(inventory__, guard__)

#endif

///
/// \brief Validates that the contents of the inventory are valid
/// Disabled in release builds
void InventoryVerify(const data::Item::Inventory& inv) {
	for (const auto& stack : inv) {
		if (stack.item == nullptr || stack.count == 0) {
			// Inventory selection cursor exempt from having 0 for count
			if (stack.item && stack.item->name == data::Item::kInventorySelectedCursor) {
				continue;
			}

			assert(stack.item == nullptr);
			assert(stack.count == 0);
		}
	}
}

// ======================================================================

///
/// \brief Attempts to drop one item from origin item stack to target itme stack
bool DropOneOriginItem(data::Item::Stack& origin_item_stack,
                       data::Item::Stack& target_item_stack) {
	target_item_stack.item = origin_item_stack.item;
	target_item_stack.count++;

	// Empty?
	origin_item_stack.count--;
	if (origin_item_stack.count == 0) {
		origin_item_stack.item = nullptr;
		return true;
	}

	return false;
}

bool game::StackMatchesFilter(const data::Item::Stack& origin_stack, const data::Item::Stack& target_stack) {
	// Origin stack must match target stack if filter is set and origin stack has an item
	return origin_stack.item == nullptr || target_stack.filter == nullptr || origin_stack.item == target_stack.filter;
}

bool game::MoveItemstackToIndex(data::Item::Stack& origin_stack,
                                data::Item::Stack& target_stack,
                                const unsigned short mouse_button) {
	assert(mouse_button == 0 || mouse_button == 1); // Only left and right click are currently supported

	// Moving nothing to nothing
	if (origin_stack.item == nullptr && target_stack.item == nullptr)
		return true;

	if (!StackMatchesFilter(origin_stack, target_stack))
		return false;

	// Items are of the same type
	if (origin_stack.item == target_stack.item) {
		assert(origin_stack.item != nullptr); // Invalid itemstack
		assert(target_stack.item != nullptr); // Invalid itemstack
		assert(origin_stack.count != 0);      // Invalid itemstack
		assert(target_stack.count != 0);      // Invalid itemstack

		assert(origin_stack.item->stackSize > 0);      // Invalid itemstack stacksize
		assert(target_stack.item->stackSize > 0);      // Invalid itemstack stacksize

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
			const unsigned short move_amount = origin_stack.item->stackSize - target_stack.count;
			origin_stack.count -= move_amount;
			target_stack.count += move_amount;

			return false;
		}


		// Drop 1 to target on right click
		if (mouse_button == 1 && target_stack.count < target_stack.item->stackSize) {
			return DropOneOriginItem(origin_stack, target_stack);
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
			assert(origin_stack.item->stackSize > 0);      // Invalid itemstack stacksize

			if (origin_stack.count > origin_stack.item->stackSize) {
				const unsigned short stack_size = origin_stack.item->stackSize;

				origin_stack.count -= stack_size;
				target_stack.count = stack_size;

				target_stack.item = origin_stack.item;
				return false;
			}

			// Drop 1 on right click
			if (mouse_button == 1) {
				return DropOneOriginItem(origin_stack, target_stack);
			}
		}
		// Target item exceeding item stack limit
		if (origin_stack.item == nullptr) {
			assert(target_stack.item->stackSize > 0);      // Invalid itemstack stacksize

			if (target_stack.count > target_stack.item->stackSize) {
				const unsigned short stack_size = target_stack.item->stackSize;

				target_stack.count -= stack_size;
				origin_stack.count = stack_size;

				origin_stack.item = target_stack.item;
				return false;
			}

			// Take half on right click
			if (mouse_button == 1) {
				unsigned short amount;

				// Never exceed the stack size
				if (target_stack.count > target_stack.item->stackSize * 2) {
					amount = target_stack.item->stackSize;
				}
					// Take 1 if there is only 1 remaining
				else if (target_stack.count == 1) {
					amount = 1;
				}
				else {
					amount = target_stack.count / 2;
				}

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

	// Swapping 2 items of different types
	std::swap(target_stack, origin_stack);

	// Origin item stack is now empty?
	if (origin_stack.count == 0) {
		assert(origin_stack.item == nullptr); // Having no item count must also mean there is no itemstack
		return true;
	}

	return false;
}

// ======================================================================
// Can be used by non-player inventories 

std::pair<bool, size_t> game::CanAddStack(const data::Item::Inventory& inv,
                                          const data::Item::Stack& item_stack) {
	J_INVENTORY_VERIFY(inv, guard);

	// Amount left which needs to be added
	auto remaining_add = item_stack.count;
	for (size_t i = 0; i < inv.size(); ++i) {
		const auto& slot = inv[i];

		if (!StackMatchesFilter(item_stack, slot))
			continue;

		// Item of same type
		if (slot.item == item_stack.item) {
			// Amount that can be added to fill the slot
			const auto max_add_amount = item_stack.item->stackSize - slot.count;
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

data::Item::StackCount game::AddStack(data::Item::Inventory& inv,
                                      const data::Item::Stack& item_stack) {
	J_INVENTORY_VERIFY(inv, guard);

	// Amount left which needs to be added
	auto remaining_add = item_stack.count;
	for (auto& slot : inv) {
		if (!StackMatchesFilter(item_stack, slot))
			continue;

		// Item of same type
		if (slot.item == item_stack.item) {
			// Amount that can be added to fill the slot
			const auto max_add_amount = item_stack.item->stackSize - slot.count;
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
			slot.item  = item_stack.item;
			slot.count = remaining_add;

			return 0;
		}
	}

	return remaining_add;
}

bool game::AddStackSub(data::Item::Inventory& inv,
                       data::Item::Stack& item_stack) {
	J_INVENTORY_VERIFY(inv, guard);

	const auto remainder = AddStack(inv, item_stack);
	if (remainder == 0) {
		item_stack.count = 0;
		return true;
	}

	// Subtract difference between what is in stack and what remains, since that is what was added
	item_stack.count -= item_stack.count - remainder;
	return false;
}

uint32_t game::GetInvItemCount(const data::Item::Inventory& inv,
                               const data::Item* item) {
	J_INVENTORY_VERIFY(inv, guard);

	uint32_t count = 0;
	for (const auto& i : inv) {
		if (i.item == item)
			count += i.count;
	}
	return count;
}

const data::Item* game::GetFirstItem(const data::Item::Inventory& inv) {
	J_INVENTORY_VERIFY(inv, guard);

	for (const auto& i : inv) {
		if (i.item != nullptr) {
			assert(i.count != 0);
			return i.item;
		}
	}
	return nullptr;
}


bool game::RemoveInvItem(data::Item::Inventory& inv,
                         const data::Item* item, const uint32_t remove_amount) {
	J_INVENTORY_VERIFY(inv, guard);

	// Not enough to remove
	if (GetInvItemCount(inv, item) < remove_amount)
		return false;

	DeleteInvItem(inv, item, remove_amount);
	return true;
}

void game::DeleteInvItem(data::Item::Inventory& inv,
                         const data::Item* item, uint32_t remove_amount) {
	J_INVENTORY_VERIFY(inv, guard);

	for (auto& inv_i : inv) {
		if (inv_i.item == item) {
			// Enough to remove and move on
			if (remove_amount > inv_i.count) {
				remove_amount -= inv_i.count;
				inv_i = {nullptr, 0};
			}
				// Not enough to remove and move on
			else {
				inv_i.count -= remove_amount;
				if (inv_i.count == 0) {
					inv_i = {nullptr, 0};
				}

				return;
			}
		}
	}
}
