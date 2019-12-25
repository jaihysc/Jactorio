#include "game/logic/inventory_controller.h"

#include <cassert>
#include <utility>

bool drop_one_origin_item(jactorio::data::item_stack& origin_item_stack,
                          jactorio::data::item_stack& target_item_stack) {
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

bool jactorio::game::inventory_controller::move_itemstack_to_index(
	data::item_stack* origin_inv, const unsigned short origin_inv_index,
	data::item_stack* target_inv, const unsigned short target_inv_index, unsigned short mouse_button) {

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

		if (mouse_button == 0) {
			// Not exceeding max stack size
			if (origin_item_stack.second + target_item_stack.second <= origin_item_stack.first->stack_size) {
				// Move the item
				target_item_stack.second += origin_item_stack.second;

				// Remove the item from the original location
				origin_item_stack.first = nullptr;
				origin_item_stack.second = 0;

				return true;
			}

			// Swap places if same type, and target is full
			if (target_item_stack.second == target_item_stack.first->stack_size) {
				std::swap(target_item_stack, origin_item_stack);
				return false;
			}
			
			// Addition of both stacks exceeding max stack size
			// Move origin to reach the max stack size in the target
			const unsigned short move_amount = origin_item_stack.first->stack_size - target_item_stack.second;;
			origin_item_stack.second -= move_amount;
			target_item_stack.second += move_amount;

			return false;
		}


		// Drop 1 to target on right click
		if (mouse_button == 1 && target_item_stack.second < target_item_stack.first->stack_size) {
			return drop_one_origin_item(origin_item_stack, target_item_stack);
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
			if (origin_item_stack.second > origin_item_stack.first->stack_size) {
				const unsigned short stack_size = origin_item_stack.first->stack_size;

				origin_item_stack.second -= stack_size;
				target_item_stack.second = stack_size;

				target_item_stack.first = origin_item_stack.first;
				return false;
			}
			
			// Drop 1 on right click
			if (mouse_button == 1) {
				return drop_one_origin_item(origin_item_stack, target_item_stack);
			}
		}
		// Target item exceeding item stack limit
		if (origin_item_stack.first == nullptr) {
			if (target_item_stack.second > target_item_stack.first->stack_size) {
				const unsigned short stack_size = target_item_stack.first->stack_size;

				target_item_stack.second -= stack_size;
				origin_item_stack.second = stack_size;

				origin_item_stack.first = target_item_stack.first;
				return false;
			}

			// Take half on right click
			if (mouse_button == 1) {
				unsigned short amount;

				// Never exceed the stack size
				if (target_item_stack.second > target_item_stack.first->stack_size * 2) {
					amount = target_item_stack.first->stack_size;
				}
					// Take 1 if there is only 1 remaining
				else if (target_item_stack.second == 1) {
					amount = 1;
				}
				else {
					amount = target_item_stack.second / 2;
				}

				origin_item_stack.first = target_item_stack.first;
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

// assert(origin_inv_index < origin_inv_size);  // Inventory index out of range
// assert(target_inv_index < target_inv_size);  // Inventory index out of range