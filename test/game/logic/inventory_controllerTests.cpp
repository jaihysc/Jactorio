#include <gtest/gtest.h>

#include <memory>

#include "game/logic/inventory_controller.h"

namespace game::logic
{
	TEST(inventory_controller, move_stack_to_empty_slot) {
		// Moving from inventory position 0 to position 3
		
		using namespace jactorio::game::inventory_c;

		constexpr unsigned short inv_size = 10;
		jactorio::data::item_stack inv[inv_size];

		auto item = std::make_unique<jactorio::data::Item>();
		item->stack_size = 50;
		
		// Position 3 should have the 50 items + item prototype after moving
		inv[0].first = item.get();
		inv[0].second = 50;
		
		inv[3].first = nullptr;
		inv[3].second = 0;
		
		const bool result = move_itemstack_to_index(inv, 0, inv, 3, 0);
		EXPECT_EQ(result, true);

		EXPECT_EQ(inv[0].first, nullptr);
		EXPECT_EQ(inv[0].second, 0);
		
		EXPECT_EQ(inv[3].first, item.get());
		EXPECT_EQ(inv[3].second, 50);
	}

	TEST(inventory_controller, move_stack_to_partial_filled_slot_non_exceeding) {
		// Moving from inventory position 0 to position 3. not exceeding the max stack size
		// moving 10 items:
		// position 3 already has 30 items, adding 10 from position 0 to equal 40.

		using namespace jactorio::game::inventory_c;

		constexpr unsigned short inv_size = 10;
		jactorio::data::item_stack inv[inv_size];

		auto item = std::make_unique<jactorio::data::Item>();
		item->stack_size = 50;

		inv[0].first = item.get();
		inv[0].second = 10;

		inv[3].first = item.get();
		inv[3].second = 30;

		const bool result = move_itemstack_to_index(inv, 0, inv, 3, 0);
		EXPECT_EQ(result, true);

		EXPECT_EQ(inv[0].first, nullptr);
		EXPECT_EQ(inv[0].second, 0);

		EXPECT_EQ(inv[3].first, item.get());
		EXPECT_EQ(inv[3].second, 40);
	}

	TEST(inventory_controller, move_stack_to_partial_filled_slot_exceeding) {
		// Moving from inventory position 0 to position 3. EXCEEDING the max stack size
		// moving 30 items:
		// position 3 already has 30 items, meaning only 20 can be moved into it to reach the stack size of 50.
		// this leaves 10 in the original location (0) and 50 in the target location (3)

		using namespace jactorio::game::inventory_c;

		constexpr unsigned short inv_size = 10;
		jactorio::data::item_stack inv[inv_size];

		auto item = std::make_unique<jactorio::data::Item>();
		item->stack_size = 50;

		inv[0].first = item.get();
		inv[0].second = 30;

		inv[3].first = item.get();
		inv[3].second = 30;

		const bool result = move_itemstack_to_index(inv, 0, inv, 3, 0);
		EXPECT_EQ(result, false);

		EXPECT_EQ(inv[0].first, item.get());
		EXPECT_EQ(inv[0].second, 10);
		
		EXPECT_EQ(inv[3].first, item.get());
		EXPECT_EQ(inv[3].second, 50);
	}

	TEST(inventory_controller, move_stack_different_item_stacks) {
		// Moving from inventory position 0 to position 3
		// The item stacks are of different items, therefore only swapping positions

		using namespace jactorio::game::inventory_c;

		constexpr unsigned short inv_size = 10;
		jactorio::data::item_stack inv[inv_size];

		auto item = std::make_unique<jactorio::data::Item>();
		item->stack_size = 50;

		auto item2 = std::make_unique<jactorio::data::Item>();
		item2->stack_size = 100;

		// Position 3 should have the 50 items + item prototype after moving
		inv[0].first = item.get();
		inv[0].second = 50;

		inv[3].first = item2.get();
		inv[3].second = 10;

		const bool result = move_itemstack_to_index(inv, 0, inv, 3, 0);
		EXPECT_EQ(result, false);

		EXPECT_EQ(inv[0].first, item2.get());
		EXPECT_EQ(inv[0].second, 10);

		EXPECT_EQ(inv[3].first, item.get());
		EXPECT_EQ(inv[3].second, 50);
	}

	TEST(inventory_controller, move_stack_full_target_slot) {
		// Moving from inventory position 0 to position 3.
		// The target slot is full, origin slot has something
		// swap the 2 items
		using namespace jactorio::game::inventory_c;

		constexpr unsigned short inv_size = 10;
		jactorio::data::item_stack inv[inv_size];

		auto item = std::make_unique<jactorio::data::Item>();
		item->stack_size = 50;

		inv[0].first = item.get();
		inv[0].second = 18;

		inv[3].first = item.get();
		inv[3].second = 50;

		const bool result = move_itemstack_to_index(inv, 0, inv, 3, 0);
		EXPECT_EQ(result, false);

		EXPECT_EQ(inv[0].first, item.get());
		EXPECT_EQ(inv[0].second, 50);

		EXPECT_EQ(inv[3].first, item.get());
		EXPECT_EQ(inv[3].second, 18);
	}

	TEST(inventory_controller, move_empty_slot_to_empty_slot) {
		// Moving from inventory position 0 to position 3
		// Moving nothing to nothing results in nothing!

		using namespace jactorio::game::inventory_c;

		constexpr unsigned short inv_size = 10;
		jactorio::data::item_stack inv[inv_size];

		inv[0].first = nullptr;
		inv[0].second = 0;

		inv[3].first = nullptr;
		inv[3].second = 0;

		const bool result = move_itemstack_to_index(inv, 0, inv, 3, 0);
		EXPECT_EQ(result, true);

		EXPECT_EQ(inv[0].first, nullptr);
		EXPECT_EQ(inv[0].second, 0);

		EXPECT_EQ(inv[3].first, nullptr);
		EXPECT_EQ(inv[3].second, 0);
	}

	// Items somehow exceeding their item stacks
	
	TEST(inventory_controller, move_exceeding_stack_to_empty_slot) {
		// Moving from inventory position 0 to position 3.
		// The origin item is somehow exceeding its stack size, perhaps a prototype update
		// Move out only the stack size into the empty slot
		
		using namespace jactorio::game::inventory_c;

		constexpr unsigned short inv_size = 10;
		jactorio::data::item_stack inv[inv_size];

		auto item = std::make_unique<jactorio::data::Item>();
		item->stack_size = 50;

		inv[0].first = item.get();
		inv[0].second = 9000;

		inv[3].first = nullptr;
		inv[3].second = 0;

		const bool result = move_itemstack_to_index(inv, 0, inv, 3, 0);
		EXPECT_EQ(result, false);

		EXPECT_EQ(inv[0].first, item.get());
		EXPECT_EQ(inv[0].second, 8950);

		EXPECT_EQ(inv[3].first, item.get());
		EXPECT_EQ(inv[3].second, 50);
	}

	TEST(inventory_controller, move_empty_slot_to_exceeding_stack) {
		// Moving from inventory position 0 to position 3.
		// The TARGET item is somehow exceeding its stack size, perhaps a prototype update
		// Move out only the stack size into the empty origin slot

		using namespace jactorio::game::inventory_c;

		constexpr unsigned short inv_size = 10;
		jactorio::data::item_stack inv[inv_size];

		auto item = std::make_unique<jactorio::data::Item>();
		item->stack_size = 50;

		inv[0].first = nullptr;
		inv[0].second = 0;
		
		inv[3].first = item.get();
		inv[3].second = 9000;

		const bool result = move_itemstack_to_index(inv, 0, inv, 3, 0);
		EXPECT_EQ(result, false);

		EXPECT_EQ(inv[0].first, item.get());
		EXPECT_EQ(inv[0].second, 50);

		EXPECT_EQ(inv[3].first, item.get());
		EXPECT_EQ(inv[3].second, 8950);
	}

	TEST(inventory_controller, move_exceeding_stack_to_partial_slot) {
		// Moving from inventory position 0 to position 3.
		// The origin item is somehow exceeding its stack size, perhaps a prototype update
		// Move out only 10 to reach the stack size in the target slot

		using namespace jactorio::game::inventory_c;

		constexpr unsigned short inv_size = 10;
		jactorio::data::item_stack inv[inv_size];

		auto item = std::make_unique<jactorio::data::Item>();
		item->stack_size = 50;

		inv[0].first = item.get();
		inv[0].second = 9000;

		inv[3].first = item.get();
		inv[3].second = 40;

		const bool result = move_itemstack_to_index(inv, 0, inv, 3, 0);
		EXPECT_EQ(result, false);

		EXPECT_EQ(inv[0].first, item.get());
		EXPECT_EQ(inv[0].second, 8990);

		EXPECT_EQ(inv[3].first, item.get());
		EXPECT_EQ(inv[3].second, 50);
	}

	// ------------------------------------------------------
	// RIGHT click tests
	// ------------------------------------------------------
	TEST(inventory_controller, move_rclick_empty_origin_inv) {
		// Moving from inventory position 0 to position 3.
		// Origin inventory is empty, right clicking on an item takes half of it
		// round down, unless there is only 1, where one is taken

		using namespace jactorio::game::inventory_c;

		constexpr unsigned short inv_size = 10;
		jactorio::data::item_stack inv[inv_size];

		auto item = std::make_unique<jactorio::data::Item>();
		item->stack_size = 50;

		// Case 1, even number
		{
			inv[0].first = nullptr;
			inv[0].second = 0;

			inv[3].first = item.get();
			inv[3].second = 40;

			const bool result = move_itemstack_to_index(inv, 0, inv, 3, 
			                                            1);
			EXPECT_EQ(result, false);

			EXPECT_EQ(inv[0].first, item.get());
			EXPECT_EQ(inv[0].second, 20);

			EXPECT_EQ(inv[3].first, item.get());
			EXPECT_EQ(inv[3].second, 20);
		}
		// Case 2, odd number
		{
			inv[0].first = nullptr;
			inv[0].second = 0;

			inv[3].first = item.get();
			inv[3].second = 39;

			const bool result = move_itemstack_to_index(inv, 0, inv, 3,
			                                            1);
			EXPECT_EQ(result, false);

			EXPECT_EQ(inv[0].first, item.get());
			EXPECT_EQ(inv[0].second, 19);

			EXPECT_EQ(inv[3].first, item.get());
			EXPECT_EQ(inv[3].second, 20);
		}
		// Case 3, 1 item
		{
			inv[0].first = nullptr;
			inv[0].second = 0;

			inv[3].first = item.get();
			inv[3].second = 1;

			const bool result = move_itemstack_to_index(inv, 0, inv, 3,
			                                            1);
			EXPECT_EQ(result, false);

			EXPECT_EQ(inv[0].first, item.get());
			EXPECT_EQ(inv[0].second, 1);

			EXPECT_EQ(inv[3].first, nullptr);
			EXPECT_EQ(inv[3].second, 0);
		}
		// Case 4, Exceed stack size
		{
			inv[0].first = nullptr;
			inv[0].second = 0;

			inv[3].first = item.get();
			inv[3].second = 110;

			const bool result = move_itemstack_to_index(inv, 0, inv, 3,
			                                            1);
			EXPECT_EQ(result, false);

			EXPECT_EQ(inv[0].first, item.get());
			EXPECT_EQ(inv[0].second, 50);

			EXPECT_EQ(inv[3].first, item.get());
			EXPECT_EQ(inv[3].second, 60);
		}
	}

	TEST(inventory_controller, move_rclick_empty_target_inv) {
		// Moving from inventory position 0 to position 3.
		// Target inventory is empty, right clicking drops 1 item off

		using namespace jactorio::game::inventory_c;

		constexpr unsigned short inv_size = 10;
		jactorio::data::item_stack inv[inv_size];

		auto item = std::make_unique<jactorio::data::Item>();
		item->stack_size = 50;

		// Case 1, > 1 item
		{
			inv[0].first = item.get();
			inv[0].second = 10;

			inv[3].first = nullptr;
			inv[3].second = 0;

			const bool result = move_itemstack_to_index(inv, 0, inv, 3,
			                                            1);
			EXPECT_EQ(result, false);

			EXPECT_EQ(inv[0].first, item.get());
			EXPECT_EQ(inv[0].second, 9);

			EXPECT_EQ(inv[3].first, item.get());
			EXPECT_EQ(inv[3].second, 1);
		}
		// Case 2, 1 item
		{
			inv[0].first = item.get();
			inv[0].second = 1;

			inv[3].first = nullptr;
			inv[3].second = 0;

			const bool result = move_itemstack_to_index(inv, 0, inv, 3,
			                                            1);
			EXPECT_EQ(result, true);

			EXPECT_EQ(inv[0].first, nullptr);
			EXPECT_EQ(inv[0].second, 0);

			EXPECT_EQ(inv[3].first, item.get());
			EXPECT_EQ(inv[3].second, 1);
		}
		// Case 3, Target has 1 of the item
		{
			inv[0].first = item.get();
			inv[0].second = 1;

			inv[3].first = item.get();
			inv[3].second = 1;

			const bool result = move_itemstack_to_index(inv, 0, inv, 3,
			                                            1);
			EXPECT_EQ(result, true);

			EXPECT_EQ(inv[0].first, nullptr);
			EXPECT_EQ(inv[0].second, 0);

			EXPECT_EQ(inv[3].first, item.get());
			EXPECT_EQ(inv[3].second, 2);
		}
		// Case 4, target has > 1 of the item
		{
			inv[0].first = item.get();
			inv[0].second = 10;

			inv[3].first = item.get();
			inv[3].second = 1;

			const bool result = move_itemstack_to_index(inv, 0, inv, 3,
			                                            1);
			EXPECT_EQ(result, false);

			EXPECT_EQ(inv[0].first, item.get());
			EXPECT_EQ(inv[0].second, 9);

			EXPECT_EQ(inv[3].first, item.get());
			EXPECT_EQ(inv[3].second, 2);
		}
	}


	//
	//
	//
	//
	//
	//
	//
	//
	// add_itemstack_to_inv
	TEST(inventory_controller, add_itemstack_to_inv_add_to_empty_slot) {
		// Should find the first empty slot and add the item there
		// Slots, 0, 1 Will be with another item
		// Should place in slot 2

		using namespace jactorio::game::inventory_c;

		constexpr unsigned short inv_size = 10;
		jactorio::data::item_stack inv[inv_size];

		auto item = std::make_unique<jactorio::data::Item>();
		auto item2 = std::make_unique<jactorio::data::Item>();

		// Another item
		inv[0].first = item.get();
		inv[0].second = 10;
		inv[1].first = item.get();
		inv[1].second = 21;

		auto add_item = jactorio::data::item_stack(item2.get(), 20);
		EXPECT_EQ(add_itemstack_to_inv(inv, inv_size, add_item), true);

		EXPECT_EQ(add_item.second, 0);
		
		// Did not modify existing items
		EXPECT_EQ(inv[0].first, item.get());
		EXPECT_EQ(inv[0].second, 10);
		EXPECT_EQ(inv[1].first, item.get());
		EXPECT_EQ(inv[1].second, 21);

		// Added itemstack
		EXPECT_EQ(inv[2].first, item2.get());
		EXPECT_EQ(inv[2].second, 20);
	}

	TEST(inventory_controller, add_itemstack_to_inv_add_to_existing_slot) {
		// Should find slot with item of same type, respecting max stack size, add the remaining at the next
		// available slot which is another item of the same type
		// 
		// Slots, 0 Will be with another item
		// Should place in slot 1 2 3 | Add amounts: (10, 10, 30)

		using namespace jactorio::game::inventory_c;

		constexpr unsigned short inv_size = 10;
		jactorio::data::item_stack inv[inv_size];

		auto another_item = std::make_unique<jactorio::data::Item>();
		auto item_we_add_to = std::make_unique<jactorio::data::Item>();
		item_we_add_to->stack_size = 50;
		
		inv[0].first = another_item.get();
		inv[0].second = 10;

		// Will fill up first 2, then dump the remaining in 3
		inv[1].first = item_we_add_to.get();
		inv[1].second = 40;
		inv[2].first = item_we_add_to.get();
		inv[2].second = 40;
		
		inv[3].first = item_we_add_to.get();
		inv[3].second = 20;

		auto add_item = jactorio::data::item_stack(item_we_add_to.get(), 50);
		EXPECT_EQ(add_itemstack_to_inv(inv, inv_size, add_item), true);

		EXPECT_EQ(add_item.second, 0);

		// Did not modify other items
		EXPECT_EQ(inv[0].first, another_item.get());
		EXPECT_EQ(inv[0].second, 10);

		// Added correctly??
		EXPECT_EQ(inv[1].first, item_we_add_to.get());
		EXPECT_EQ(inv[1].second, 50);
		EXPECT_EQ(inv[2].first, item_we_add_to.get());
		EXPECT_EQ(inv[2].second, 50);
		
		EXPECT_EQ(inv[3].first, item_we_add_to.get());
		EXPECT_EQ(inv[3].second, 50);

		// No items should be added to the 4th slot index
		EXPECT_EQ(inv[4].first, nullptr);
		EXPECT_EQ(inv[4].second, 0);
	}

	TEST(inventory_controller, add_itemstack_to_inv_no_available_slots) {
		// Slots 1 is full, inv size is 1, will return false
		using namespace jactorio::game::inventory_c;

		constexpr unsigned short inv_size = 1;
		jactorio::data::item_stack inv[inv_size];

		auto item = std::make_unique<jactorio::data::Item>();
		auto item2 = std::make_unique<jactorio::data::Item>();

		inv[0].first = item.get();
		inv[0].second = 10;

		auto add_item = jactorio::data::item_stack(item2.get(), 20);
		EXPECT_EQ(add_itemstack_to_inv(inv, inv_size, add_item), false);

		EXPECT_EQ(add_item.second, 20);

		// Did not modify existing items
		EXPECT_EQ(inv[0].first, item.get());
		EXPECT_EQ(inv[0].second, 10);
	}
}