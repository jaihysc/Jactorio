#include <gtest/gtest.h>

#include <memory>

#include "game/logic/inventory_controller.h"

namespace game
{
	TEST(inventory_controller, move_stack_to_empty_slot) {
		// Moving from inventory position 0 to position 3
		
		using namespace jactorio::game::inventory_controller;

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

		using namespace jactorio::game::inventory_controller;

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

		using namespace jactorio::game::inventory_controller;

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

		using namespace jactorio::game::inventory_controller;

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
		using namespace jactorio::game::inventory_controller;

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

		using namespace jactorio::game::inventory_controller;

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
		
		using namespace jactorio::game::inventory_controller;

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

		using namespace jactorio::game::inventory_controller;

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

		using namespace jactorio::game::inventory_controller;

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

		using namespace jactorio::game::inventory_controller;

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

		using namespace jactorio::game::inventory_controller;

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
}