// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 01/20/2020

#include "game/logic/inventory_controller.h"

#include <cassert>
#include <utility>

using namespace jactorio;

///
/// \brief Attempts to drop one item from origin item stack to target itme stack
bool DropOneOriginItem(data::Item::Stack& origin_item_stack,
                       data::Item::Stack& target_item_stack) {
	target_item_stack.first = origin_item_stack.first;
	target_item_stack.second++;

	// Empty?
	origin_item_stack.second--;
	if (origin_item_stack.second == 0) {
		origin_item_stack.first = nullptr;
		return true;
	}

	return false;
}

bool game::MoveItemstackToIndex(data::Item::Stack& origin_stack,
                                data::Item::Stack& target_stack,
                                const unsigned short mouse_button) {
	assert(mouse_button == 0 || mouse_button == 1); // Only left and right click are currently supported

	// Moving nothing to nothing
	if (origin_stack.first == nullptr && target_stack.first == nullptr)
		return true;

	// Items are of the same type
	if (origin_stack.first == target_stack.first) {
		assert(origin_stack.first != nullptr); // Invalid itemstack
		assert(target_stack.first != nullptr); // Invalid itemstack
		assert(origin_stack.second != 0);      // Invalid itemstack
		assert(target_stack.second != 0);      // Invalid itemstack

		assert(origin_stack.first->stackSize > 0);      // Invalid itemstack stacksize
		assert(target_stack.first->stackSize > 0);      // Invalid itemstack stacksize

		if (mouse_button == 0) {
			// Not exceeding max stack size
			if (origin_stack.second + target_stack.second <= origin_stack.first->stackSize) {
				// Move the item
				target_stack.second += origin_stack.second;

				// Remove the item from the original location
				origin_stack.first  = nullptr;
				origin_stack.second = 0;

				return true;
			}

			// Swap places if same type, and target is full
			if (target_stack.second == target_stack.first->stackSize) {
				std::swap(target_stack, origin_stack);
				return false;
			}

			// Addition of both stacks exceeding max stack size
			// Move origin to reach the max stack size in the target
			const unsigned short move_amount = origin_stack.first->stackSize - target_stack.second;
			origin_stack.second -= move_amount;
			target_stack.second += move_amount;

			return false;
		}


		// Drop 1 to target on right click
		if (mouse_button == 1 && target_stack.second < target_stack.first->stackSize) {
			return DropOneOriginItem(origin_stack, target_stack);
		}

		return false;
	}

	// Items are not of the same type

	// Items exceeding item stacks
	{
		// It is guaranteed that only one will be a nullptr;
		assert(!(origin_stack.first == nullptr && target_stack.first == nullptr));

		// Origin item exceeding item stack limit
		if (target_stack.first == nullptr) {
			assert(origin_stack.first->stackSize > 0);      // Invalid itemstack stacksize

			if (origin_stack.second > origin_stack.first->stackSize) {
				const unsigned short stack_size = origin_stack.first->stackSize;

				origin_stack.second -= stack_size;
				target_stack.second = stack_size;

				target_stack.first = origin_stack.first;
				return false;
			}

			// Drop 1 on right click
			if (mouse_button == 1) {
				return DropOneOriginItem(origin_stack, target_stack);
			}
		}
		// Target item exceeding item stack limit
		if (origin_stack.first == nullptr) {
			assert(target_stack.first->stackSize > 0);      // Invalid itemstack stacksize

			if (target_stack.second > target_stack.first->stackSize) {
				const unsigned short stack_size = target_stack.first->stackSize;

				target_stack.second -= stack_size;
				origin_stack.second = stack_size;

				origin_stack.first = target_stack.first;
				return false;
			}

			// Take half on right click
			if (mouse_button == 1) {
				unsigned short amount;

				// Never exceed the stack size
				if (target_stack.second > target_stack.first->stackSize * 2) {
					amount = target_stack.first->stackSize;
				}
					// Take 1 if there is only 1 remaining
				else if (target_stack.second == 1) {
					amount = 1;
				}
				else {
					amount = target_stack.second / 2;
				}

				origin_stack.first  = target_stack.first;
				origin_stack.second = amount;

				// Empty?
				target_stack.second -= amount;
				if (target_stack.second == 0)
					target_stack.first = nullptr;

				return false;
			}

		}
	}

	// Swapping 2 items of different types
	std::swap(target_stack, origin_stack);

	// Origin item stack is now empty?
	if (origin_stack.second == 0) {
		assert(origin_stack.first == nullptr); // Having no item count must also mean there is no itemstack
		return true;
	}

	return false;
}

// ======================================================================
// Can be used by non-player inventories 

bool game::CanAddStack(const data::Item::Inventory& target_inv,
                       const data::Item::Stack& item_stack) {
	// Amount left which needs to be added
	auto remaining_add = item_stack.second;
	for (const auto& slot : target_inv) {
		// Item of same type
		if (slot.first == item_stack.first) {
			// Amount that can be added to fill the slot
			const auto max_add_amount = item_stack.first->stackSize - slot.second;
			if (max_add_amount < 0)
				continue;

			assert(max_add_amount >= 0);

			// Attempting to add more than what is available
			if (max_add_amount >= remaining_add)
				return true;

			remaining_add -= max_add_amount;
		}
			// Empty slot
		else if (slot.first == nullptr)
			return true;
	}

	return false;
}

data::Item::StackCount game::AddStack(data::Item::Inventory& inv,
                                      const data::Item::Stack& item_stack) {
	// Amount left which needs to be added
	auto remaining_add = item_stack.second;
	for (auto& slot : inv) {
		// Item of same type
		if (slot.first == item_stack.first) {
			// Amount that can be added to fill the slot
			const auto max_add_amount = item_stack.first->stackSize - slot.second;
			if (max_add_amount < 0)
				continue;

			assert(max_add_amount >= 0);

			// Attempting to add more than what is available
			if (max_add_amount >= remaining_add) {
				slot.second += remaining_add;
				return 0;
			}

			slot.second += max_add_amount;
			remaining_add -= max_add_amount;
		}
			// Empty slot
		else if (slot.first == nullptr) {
			slot.first  = item_stack.first;
			slot.second = remaining_add;

			return 0;
		}
	}

	return remaining_add;
}

bool game::AddStackSub(data::Item::Inventory& inv,
                       data::Item::Stack& item_stack) {
	const auto remainder = AddStack(inv, item_stack);
	if (remainder == 0) {
		item_stack.second = 0;
		return true;
	}

	// Subtract difference between what is in stack and what remains, since that is what was added
	item_stack.second -= item_stack.second - remainder;
	return false;
}

uint32_t game::GetInvItemCount(const data::Item::Inventory& inv,
                               const data::Item* item) {
	uint32_t count = 0;
	for (const auto& i : inv) {
		if (i.first == item)
			count += i.second;
	}
	return count;
}

const data::Item* game::GetFirstItem(const data::Item::Inventory& inv) {
	for (const auto& i : inv) {
		if (i.first != nullptr) {
			assert(i.second != 0);
			return i.first;
		}
	}
	return nullptr;
}


bool game::RemoveInvItem(data::Item::Inventory& inv,
                         const data::Item* item, const uint32_t remove_amount) {
	// Not enough to remove
	if (GetInvItemCount(inv, item) < remove_amount)
		return false;

	DeleteInvItem(inv, item, remove_amount);
	return true;
}

void game::DeleteInvItem(data::Item::Inventory& inv,
                         const data::Item* item, uint32_t remove_amount) {
	for (auto& inv_i : inv) {
		if (inv_i.first == item) {
			// Enough to remove and move on
			if (remove_amount > inv_i.second) {
				remove_amount -= inv_i.second;
				inv_i.first = nullptr;
			}
				// Not enough to remove and move on
			else {
				inv_i.second -= remove_amount;
				if (inv_i.second == 0)
					inv_i.first = nullptr;

				return;
			}
		}
	}
}
