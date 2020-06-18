// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 01/20/2020

#include "game/logic/inventory_controller.h"

#include <cassert>
#include <utility>

/**
 * Attempts to drop one item from origin item stack to target itme stack
 */
bool DropOneOriginItem(jactorio::data::ItemStack& origin_item_stack,
                          jactorio::data::ItemStack& target_item_stack) {
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

bool jactorio::game::MoveItemstackToIndex(
	data::ItemStack* origin_inv, const uint16_t origin_inv_index,
	data::ItemStack* target_inv, const uint16_t target_inv_index, const unsigned short mouse_button) {
	assert(mouse_button == 0 || mouse_button == 1); // Only left and right click are currently supported

	auto& origin_item_stack = origin_inv[origin_inv_index];
	auto& target_item_stack = target_inv[target_inv_index];

	// Moving nothing to nothing
	if (origin_item_stack.first == nullptr && target_item_stack.first == nullptr)
		return true;

	// Items are of the same type
	if (origin_item_stack.first == target_item_stack.first) {
		assert(origin_item_stack.first != nullptr); // Invalid itemstack
		assert(target_item_stack.first != nullptr); // Invalid itemstack
		assert(origin_item_stack.second != 0);      // Invalid itemstack
		assert(target_item_stack.second != 0);      // Invalid itemstack

		assert(origin_item_stack.first->stackSize > 0);      // Invalid itemstack stacksize
		assert(target_item_stack.first->stackSize > 0);      // Invalid itemstack stacksize

		if (mouse_button == 0) {
			// Not exceeding max stack size
			if (origin_item_stack.second + target_item_stack.second <= origin_item_stack.first->stackSize) {
				// Move the item
				target_item_stack.second += origin_item_stack.second;

				// Remove the item from the original location
				origin_item_stack.first  = nullptr;
				origin_item_stack.second = 0;

				return true;
			}

			// Swap places if same type, and target is full
			if (target_item_stack.second == target_item_stack.first->stackSize) {
				std::swap(target_item_stack, origin_item_stack);
				return false;
			}

			// Addition of both stacks exceeding max stack size
			// Move origin to reach the max stack size in the target
			const unsigned short move_amount = origin_item_stack.first->stackSize - target_item_stack.second;
			origin_item_stack.second -= move_amount;
			target_item_stack.second += move_amount;

			return false;
		}


		// Drop 1 to target on right click
		if (mouse_button == 1 && target_item_stack.second < target_item_stack.first->stackSize) {
			return DropOneOriginItem(origin_item_stack, target_item_stack);
		}

		return false;
	}

	// Items are not of the same type

	// Items exceeding item stacks
	{
		// It is guaranteed that only one will be a nullptr;
		assert(!(origin_item_stack.first == nullptr && target_item_stack.first == nullptr));

		// Origin item exceeding item stack limit
		if (target_item_stack.first == nullptr) {
			assert(origin_item_stack.first->stackSize > 0);      // Invalid itemstack stacksize

			if (origin_item_stack.second > origin_item_stack.first->stackSize) {
				const unsigned short stack_size = origin_item_stack.first->stackSize;

				origin_item_stack.second -= stack_size;
				target_item_stack.second = stack_size;

				target_item_stack.first = origin_item_stack.first;
				return false;
			}

			// Drop 1 on right click
			if (mouse_button == 1) {
				return DropOneOriginItem(origin_item_stack, target_item_stack);
			}
		}
		// Target item exceeding item stack limit
		if (origin_item_stack.first == nullptr) {
			assert(target_item_stack.first->stackSize > 0);      // Invalid itemstack stacksize

			if (target_item_stack.second > target_item_stack.first->stackSize) {
				const unsigned short stack_size = target_item_stack.first->stackSize;

				target_item_stack.second -= stack_size;
				origin_item_stack.second = stack_size;

				origin_item_stack.first = target_item_stack.first;
				return false;
			}

			// Take half on right click
			if (mouse_button == 1) {
				unsigned short amount;

				// Never exceed the stack size
				if (target_item_stack.second > target_item_stack.first->stackSize * 2) {
					amount = target_item_stack.first->stackSize;
				}
					// Take 1 if there is only 1 remaining
				else if (target_item_stack.second == 1) {
					amount = 1;
				}
				else {
					amount = target_item_stack.second / 2;
				}

				origin_item_stack.first  = target_item_stack.first;
				origin_item_stack.second = amount;

				// Empty?
				target_item_stack.second -= amount;
				if (target_item_stack.second == 0)
					target_item_stack.first = nullptr;

				return false;
			}

		}
	}

	// Swapping 2 items of different types
	std::swap(target_item_stack, origin_item_stack);

	// Origin item stack is now empty?
	if (origin_item_stack.second == 0) {
		assert(origin_item_stack.first == nullptr); // Having no item count must also mean there is no itemstack
		return true;
	}

	return false;
}

// ======================================================================
// Can be used by non-player inventories 

bool jactorio::game::CanAddStack(const data::ItemStack* target_inv, const uint16_t target_inv_size,
                                 const data::ItemStack& item_stack) {
	assert(target_inv != nullptr); // Invalid item_stack to add

	// Amount left which needs to be added
	auto remaining_add = item_stack.second;
	for (int i = 0; i < target_inv_size; ++i) {
		const data::ItemStack& slot = target_inv[i];

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

decltype(jactorio::data::ItemStack::second) jactorio::game::AddStack(
	data::ItemStack* target_inv, const uint16_t target_inv_size, const data::ItemStack& item_stack) {

	assert(target_inv != nullptr); // Invalid item_stack to add

	// Amount left which needs to be added
	auto remaining_add = item_stack.second;
	for (int i = 0; i < target_inv_size; ++i) {
		data::ItemStack& slot = target_inv[i];

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

bool jactorio::game::AddStackSub(data::ItemStack* target_inv, const uint16_t target_inv_size,
                                 data::ItemStack& item_stack) {
	assert(target_inv != nullptr); // Invalid item_stack to add

	const auto remainder = AddStack(target_inv, target_inv_size, item_stack);
	if (remainder == 0) {
		item_stack.second = 0;
		return true;
	}

	// Subtract difference between what is in stack and what remains, since that is what was added
	item_stack.second -= item_stack.second - remainder;
	return false;
}

uint32_t jactorio::game::GetInvItemCount(const data::ItemStack* inv, const uint16_t inv_size,
                                         const data::Item* item) {
	uint32_t count = 0;
	for (int i = 0; i < inv_size; ++i) {
		if (inv[i].first == item)
			count += inv[i].second;
	}
	return count;
}

const jactorio::data::Item* jactorio::game::GetFirstItem(const data::ItemStack* inv, const uint16_t inv_size) {
	assert(inv != nullptr);

	for (int i = 0; i < inv_size; ++i) {
		if (inv[i].first != nullptr) {
			assert(inv[i].second != 0);
			return inv[i].first;
		}
	}
	return nullptr;
}


bool jactorio::game::RemoveInvItem(data::ItemStack* inv, const uint16_t inv_size,
                                   const data::Item* item, const uint32_t remove_amount) {
	assert(inv != nullptr);

	// Not enough to remove
	if (GetInvItemCount(inv, inv_size, item) < remove_amount)
		return false;

	DeleteInvItem(inv, inv_size, item, remove_amount);
	return true;
}

void jactorio::game::DeleteInvItem(data::ItemStack* inv, const uint16_t inv_size,
                                   const data::Item* item, uint32_t remove_amount) {
	assert(inv != nullptr);

	for (int i = 0; i < inv_size; ++i) {
		auto& inv_i = inv[i];
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
