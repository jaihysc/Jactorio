// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include <memory>

#include "game/logistic/inventory.h"

namespace jactorio::game
{
    TEST(ItemSack, MatchesFilter) {
        proto::Item item_1;
        proto::Item item_2;

        EXPECT_FALSE(ItemStack({&item_1, 1}).MatchesFilter({nullptr, 0, &item_2}));
        EXPECT_TRUE(ItemStack({nullptr, 1}).MatchesFilter({nullptr, 0, &item_2}));
        EXPECT_TRUE(ItemStack({&item_1, 1}).MatchesFilter({nullptr, 0, nullptr}));
    }

    TEST(Inventory, MoveStackToEmptySlot) {
        // Moving from inventory position 0 to position 3

        Inventory inv(10);

        const auto item = std::make_unique<proto::Item>();
        item->stackSize = 50;

        // Position 3 should have the 50 items + item prototype after moving
        inv[0].item  = item.get();
        inv[0].count = 50;

        inv[3].item  = nullptr;
        inv[3].count = 0;

        const bool result = MoveItemstackToIndex(inv[0], inv[3], 0);
        EXPECT_EQ(result, true);

        EXPECT_EQ(inv[0].item, nullptr);
        EXPECT_EQ(inv[0].count, 0);

        EXPECT_EQ(inv[3].item, item.get());
        EXPECT_EQ(inv[3].count, 50);
    }

    TEST(Inventory, MoveStackToPartialFilledSlotNonExceeding) {
        // Moving from inventory position 0 to position 3. not exceeding the max stack size
        // moving 10 items:
        // position 3 already has 30 items, adding 10 from position 0 to equal 40.

        Inventory inv(10);

        const auto item = std::make_unique<proto::Item>();
        item->stackSize = 50;

        inv[0].item  = item.get();
        inv[0].count = 10;

        inv[3].item  = item.get();
        inv[3].count = 30;

        const bool result = MoveItemstackToIndex(inv[0], inv[3], 0);
        EXPECT_EQ(result, true);

        EXPECT_EQ(inv[0].item, nullptr);
        EXPECT_EQ(inv[0].count, 0);

        EXPECT_EQ(inv[3].item, item.get());
        EXPECT_EQ(inv[3].count, 40);
    }

    TEST(Inventory, MoveStackToPartialFilledSlotExceeding) {
        // Moving from inventory position 0 to position 3. EXCEEDING the max stack size
        // moving 30 items:
        // position 3 already has 30 items, meaning only 20 can be moved into it to reach the stack size of 50.
        // this leaves 10 in the original location (0) and 50 in the target location (3)

        Inventory inv(10);

        const auto item = std::make_unique<proto::Item>();
        item->stackSize = 50;

        inv[0].item  = item.get();
        inv[0].count = 30;

        inv[3].item  = item.get();
        inv[3].count = 30;

        const bool result = MoveItemstackToIndex(inv[0], inv[3], 0);
        EXPECT_EQ(result, false);

        EXPECT_EQ(inv[0].item, item.get());
        EXPECT_EQ(inv[0].count, 10);

        EXPECT_EQ(inv[3].item, item.get());
        EXPECT_EQ(inv[3].count, 50);
    }

    TEST(Inventory, SwapItemStacks) {
        // Moving from inventory position 0 to position 3
        // The item stacks are of different items, therefore only swapping positions

        Inventory inv(10);

        const auto item = std::make_unique<proto::Item>();
        item->stackSize = 50;

        const auto item2 = std::make_unique<proto::Item>();
        item2->stackSize = 100;

        // Position 3 should have the 50 items + item prototype after moving
        inv[0].item  = item.get();
        inv[0].count = 50;

        inv[3].item  = item2.get();
        inv[3].count = 10;

        EXPECT_FALSE(MoveItemstackToIndex(inv[0], inv[3], 0));

        EXPECT_EQ(inv[0].item, item2.get());
        EXPECT_EQ(inv[0].count, 10);

        EXPECT_EQ(inv[3].item, item.get());
        EXPECT_EQ(inv[3].count, 50);
    }

    TEST(Inventory, SwapItemStacksFilterd) {
        // Moving from inventory position 0 to position 3
        // slot 0 is filtered, therefore no swap occurs

        Inventory inv(10);

        const auto item  = std::make_unique<proto::Item>();
        const auto item2 = std::make_unique<proto::Item>();

        inv[0].item   = item.get();
        inv[0].count  = 2;
        inv[0].filter = item.get();

        inv[3].item  = item2.get();
        inv[3].count = 10;

        EXPECT_FALSE(MoveItemstackToIndex(inv[0], inv[3], 0));

        // 0 <> 3: No swap, filters do not match item
        EXPECT_EQ(inv[0].item, item.get());
        EXPECT_EQ(inv[0].count, 2);
        EXPECT_EQ(inv[0].filter, item.get());

        EXPECT_EQ(inv[3].item, item2.get());
        EXPECT_EQ(inv[3].count, 10);
        EXPECT_EQ(inv[3].filter, nullptr);


        // 0 -> 3: Ok, 3 is empty
        inv[3].item  = nullptr;
        inv[3].count = 0;

        EXPECT_TRUE(MoveItemstackToIndex(inv[0], inv[3], 0));

        EXPECT_EQ(inv[0].item, nullptr);
        EXPECT_EQ(inv[0].count, 0);
        EXPECT_EQ(inv[0].filter, item.get());

        EXPECT_EQ(inv[3].item, item.get());
        EXPECT_EQ(inv[3].count, 2);
        EXPECT_EQ(inv[3].filter, nullptr);


        // 3 -> 0: Ok, 0 is empty
        EXPECT_FALSE(MoveItemstackToIndex(inv[0], inv[3], 0));

        EXPECT_EQ(inv[0].item, item.get());
        EXPECT_EQ(inv[0].count, 2);
        EXPECT_EQ(inv[0].filter, item.get());

        EXPECT_EQ(inv[3].item, nullptr);
        EXPECT_EQ(inv[3].count, 0);
        EXPECT_EQ(inv[3].filter, nullptr);
    }

    TEST(Inventory, MoveStackFullTargetSlot) {
        // Moving from inventory position 0 to position 3.
        // The target slot is full, origin slot has something
        // swap the 2 items
        Inventory inv(10);

        const auto item = std::make_unique<proto::Item>();
        item->stackSize = 50;

        inv[0].item  = item.get();
        inv[0].count = 18;

        inv[3].item  = item.get();
        inv[3].count = 50;

        const bool result = MoveItemstackToIndex(inv[0], inv[3], 0);
        EXPECT_EQ(result, false);

        EXPECT_EQ(inv[0].item, item.get());
        EXPECT_EQ(inv[0].count, 50);

        EXPECT_EQ(inv[3].item, item.get());
        EXPECT_EQ(inv[3].count, 18);
    }

    TEST(Inventory, MoveEmptySlotToEmptySlot) {
        // Moving from inventory position 0 to position 3
        // Moving nothing to nothing results in nothing!

        Inventory inv(10);

        inv[0].item  = nullptr;
        inv[0].count = 0;

        inv[3].item  = nullptr;
        inv[3].count = 0;

        const bool result = MoveItemstackToIndex(inv[0], inv[3], 0);
        EXPECT_EQ(result, true);

        EXPECT_EQ(inv[0].item, nullptr);
        EXPECT_EQ(inv[0].count, 0);

        EXPECT_EQ(inv[3].item, nullptr);
        EXPECT_EQ(inv[3].count, 0);
    }

    // Items somehow exceeding their item stacks

    TEST(Inventory, MoveExceedingStackToEmptySlot) {
        // Moving from inventory position 0 to position 3.
        // The origin item is somehow exceeding its stack size, perhaps a prototype update
        // Move out only the stack size into the empty slot

        Inventory inv(10);

        const auto item = std::make_unique<proto::Item>();
        item->stackSize = 50;

        inv[0].item  = item.get();
        inv[0].count = 9000;

        inv[3].item  = nullptr;
        inv[3].count = 0;

        const bool result = MoveItemstackToIndex(inv[0], inv[3], 0);
        EXPECT_EQ(result, false);

        EXPECT_EQ(inv[0].item, item.get());
        EXPECT_EQ(inv[0].count, 8950);

        EXPECT_EQ(inv[3].item, item.get());
        EXPECT_EQ(inv[3].count, 50);
    }

    TEST(Inventory, MoveEmptySlotToExceedingStack) {
        // Moving from inventory position 0 to position 3.
        // The TARGET item is somehow exceeding its stack size, perhaps a prototype update
        // Move out only the stack size into the empty origin slot

        Inventory inv(10);

        const auto item = std::make_unique<proto::Item>();
        item->stackSize = 50;

        inv[0].item  = nullptr;
        inv[0].count = 0;

        inv[3].item  = item.get();
        inv[3].count = 9000;

        const bool result = MoveItemstackToIndex(inv[0], inv[3], 0);
        EXPECT_EQ(result, false);

        EXPECT_EQ(inv[0].item, item.get());
        EXPECT_EQ(inv[0].count, 50);

        EXPECT_EQ(inv[3].item, item.get());
        EXPECT_EQ(inv[3].count, 8950);
    }

    TEST(Inventory, MoveExceedingStackToPartialSlot) {
        // Moving from inventory position 0 to position 3.
        // The origin item is somehow exceeding its stack size, perhaps a prototype update
        // Move out only 10 to reach the stack size in the target slot

        Inventory inv(10);

        const auto item = std::make_unique<proto::Item>();
        item->stackSize = 50;

        inv[0].item  = item.get();
        inv[0].count = 9000;

        inv[3].item  = item.get();
        inv[3].count = 40;

        const bool result = MoveItemstackToIndex(inv[0], inv[3], 0);
        EXPECT_EQ(result, false);

        EXPECT_EQ(inv[0].item, item.get());
        EXPECT_EQ(inv[0].count, 8990);

        EXPECT_EQ(inv[3].item, item.get());
        EXPECT_EQ(inv[3].count, 50);
    }

    // ------------------------------------------------------
    // RIGHT click tests
    // ------------------------------------------------------
    TEST(Inventory, MoveRclickEmptyOriginInv) {
        // Moving from inventory position 0 to position 3.
        // Origin inventory is empty, right clicking on an item takes half of it
        // round down, unless there is only 1, where one is taken

        Inventory inv(10);

        auto item       = std::make_unique<proto::Item>();
        item->stackSize = 50;

        // Case 1, even number
        {
            inv[0].item  = nullptr;
            inv[0].count = 0;

            inv[3].item  = item.get();
            inv[3].count = 40;

            const bool result = MoveItemstackToIndex(inv[0], inv[3], 1);
            EXPECT_EQ(result, false);

            EXPECT_EQ(inv[0].item, item.get());
            EXPECT_EQ(inv[0].count, 20);

            EXPECT_EQ(inv[3].item, item.get());
            EXPECT_EQ(inv[3].count, 20);
        }
        // Case 2, odd number
        {
            inv[0].item  = nullptr;
            inv[0].count = 0;

            inv[3].item  = item.get();
            inv[3].count = 39;

            const bool result = MoveItemstackToIndex(inv[0], inv[3], 1);
            EXPECT_EQ(result, false);

            EXPECT_EQ(inv[0].item, item.get());
            EXPECT_EQ(inv[0].count, 19);

            EXPECT_EQ(inv[3].item, item.get());
            EXPECT_EQ(inv[3].count, 20);
        }
        // Case 3, 1 item
        {
            inv[0].item  = nullptr;
            inv[0].count = 0;

            inv[3].item  = item.get();
            inv[3].count = 1;

            const bool result = MoveItemstackToIndex(inv[0], inv[3], 1);
            EXPECT_EQ(result, false);

            EXPECT_EQ(inv[0].item, item.get());
            EXPECT_EQ(inv[0].count, 1);

            EXPECT_EQ(inv[3].item, nullptr);
            EXPECT_EQ(inv[3].count, 0);
        }
        // Case 4, Exceed stack size
        {
            inv[0].item  = nullptr;
            inv[0].count = 0;

            inv[3].item  = item.get();
            inv[3].count = 110;

            const bool result = MoveItemstackToIndex(inv[0], inv[3], 1);
            EXPECT_EQ(result, false);

            EXPECT_EQ(inv[0].item, item.get());
            EXPECT_EQ(inv[0].count, 50);

            EXPECT_EQ(inv[3].item, item.get());
            EXPECT_EQ(inv[3].count, 60);
        }
    }

    TEST(Inventory, MoveRclickEmptyTargetInv) {
        // Moving from inventory position 0 to position 3.
        // Target inventory is empty, right clicking drops 1 item off

        Inventory inv(10);

        auto item       = std::make_unique<proto::Item>();
        item->stackSize = 50;

        // Case 1, > 1 item
        {
            inv[0].item  = item.get();
            inv[0].count = 10;

            inv[3].item  = nullptr;
            inv[3].count = 0;

            const bool result = MoveItemstackToIndex(inv[0], inv[3], 1);
            EXPECT_EQ(result, false);

            EXPECT_EQ(inv[0].item, item.get());
            EXPECT_EQ(inv[0].count, 9);

            EXPECT_EQ(inv[3].item, item.get());
            EXPECT_EQ(inv[3].count, 1);
        }
        // Case 2, 1 item
        {
            inv[0].item  = item.get();
            inv[0].count = 1;

            inv[3].item  = nullptr;
            inv[3].count = 0;

            const bool result = MoveItemstackToIndex(inv[0], inv[3], 1);
            EXPECT_EQ(result, true);

            EXPECT_EQ(inv[0].item, nullptr);
            EXPECT_EQ(inv[0].count, 0);

            EXPECT_EQ(inv[3].item, item.get());
            EXPECT_EQ(inv[3].count, 1);
        }
        // Case 3, Target has 1 of the item
        {
            inv[0].item  = item.get();
            inv[0].count = 1;

            inv[3].item  = item.get();
            inv[3].count = 1;

            const bool result = MoveItemstackToIndex(inv[0], inv[3], 1);
            EXPECT_EQ(result, true);

            EXPECT_EQ(inv[0].item, nullptr);
            EXPECT_EQ(inv[0].count, 0);

            EXPECT_EQ(inv[3].item, item.get());
            EXPECT_EQ(inv[3].count, 2);
        }
        // Case 4, target has > 1 of the item
        {
            inv[0].item  = item.get();
            inv[0].count = 10;

            inv[3].item  = item.get();
            inv[3].count = 1;

            const bool result = MoveItemstackToIndex(inv[0], inv[3], 1);
            EXPECT_EQ(result, false);

            EXPECT_EQ(inv[0].item, item.get());
            EXPECT_EQ(inv[0].count, 9);

            EXPECT_EQ(inv[3].item, item.get());
            EXPECT_EQ(inv[3].count, 2);
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
    TEST(Inventory, Sort) {
        proto::Item item;
        item.stackSize = 50;

        proto::Item item_2;
        item_2.stackSize = 10;

        Inventory inv{40};

        // Item 1
        inv[0].item  = &item;
        inv[0].count = 10;

        inv[10].item  = &item;
        inv[10].count = 25;

        inv[20].item  = &item;
        inv[20].count = 25;

        inv[13].item  = &item;
        inv[13].count = 20;

        inv[14].item  = &item;
        inv[14].count = 30;

        // Item 2
        inv[31].item  = &item_2;
        inv[31].count = 4;

        inv[32].item  = &item_2;
        inv[32].count = 6;

        inv[22].item  = &item_2;
        inv[22].count = 1;


        // Sorted inventory should be as follows
        // Item(count)
        // 1(50), 1(50), 1(10), 2(10), 2(1)
        inv.Sort();

        EXPECT_EQ(inv[0].item, &item);
        EXPECT_EQ(inv[0].count, 50);
        EXPECT_EQ(inv[1].item, &item);
        EXPECT_EQ(inv[1].count, 50);
        EXPECT_EQ(inv[2].item, &item);
        EXPECT_EQ(inv[2].count, 10);

        EXPECT_EQ(inv[3].item, &item_2);
        EXPECT_EQ(inv[3].count, 10);
        EXPECT_EQ(inv[4].item, &item_2);
        EXPECT_EQ(inv[4].count, 1);
    }

    TEST(Inventory, SortFull) {
        proto::Item item;
        item.stackSize = 50;

        Inventory inv{30};
        for (auto& i : inv) {
            i.item  = &item;
            i.count = 50;
        }

        inv.Sort();

        for (auto& i : inv) {
            EXPECT_EQ(i.item, &item);
            EXPECT_EQ(i.count, 50);
        }
    }

    ///
    /// If there is an item which exceeds its stack size, do not attempt to stack into it
    TEST(Inventory, SortItemExcedingStack) {
        proto::Item item;
        item.stackSize = 50;

        Inventory inv{30};

        inv[10].item  = &item;
        inv[10].count = 100;

        inv[11].item  = &item;
        inv[11].count = 100;

        inv[12].item  = &item;
        inv[12].count = 10;

        inv.Sort();

        EXPECT_EQ(inv[0].item, &item);
        EXPECT_EQ(inv[0].count, 100);

        EXPECT_EQ(inv[1].item, &item);
        EXPECT_EQ(inv[1].count, 100);

        EXPECT_EQ(inv[2].item, &item);
        EXPECT_EQ(inv[2].count, 10);
    }

    ///
    /// Should find the first empty slot and add the item there
    /// Slots, 0, 1 Will be with another item
    /// Should place in slot 2

    TEST(Inventory, AddStackToEmptySlot) {
        Inventory inv(10);

        const auto item  = std::make_unique<proto::Item>();
        const auto item2 = std::make_unique<proto::Item>();

        // Another item
        inv[0].item  = item.get();
        inv[0].count = 10;
        inv[1].item  = item.get();
        inv[1].count = 21;

        auto add_item = ItemStack{item2.get(), 20};
        EXPECT_TRUE(inv.CanAdd(add_item).first);
        EXPECT_TRUE(inv.AddSub(add_item));

        EXPECT_EQ(add_item.count, 0);

        // Did not modify existing items
        EXPECT_EQ(inv[0].item, item.get());
        EXPECT_EQ(inv[0].count, 10);
        EXPECT_EQ(inv[1].item, item.get());
        EXPECT_EQ(inv[1].count, 21);

        // Added itemstack
        EXPECT_EQ(inv[2].item, item2.get());
        EXPECT_EQ(inv[2].count, 20);
    }

    ///
    /// Should find slot with item of same type, respecting max stack size, add the remaining at the next
    /// available slot which is another item of the same type
    ///
    /// Slots, 0 Will be with another item
    /// Should place in slot 1 2 3 | Add amounts: (10, 10, 30)
    TEST(Inventory, AddStackToExistingSlot) {

        Inventory inv(10);

        const auto another_item   = std::make_unique<proto::Item>();
        const auto item_we_add_to = std::make_unique<proto::Item>();
        item_we_add_to->stackSize = 50;

        inv[0].item  = another_item.get();
        inv[0].count = 10;

        // Will fill up first 2, then dump the remaining in 3
        inv[1].item  = item_we_add_to.get();
        inv[1].count = 40;
        inv[2].item  = item_we_add_to.get();
        inv[2].count = 40;

        inv[3].item  = item_we_add_to.get();
        inv[3].count = 20;

        auto add_item = ItemStack{item_we_add_to.get(), 50};
        EXPECT_TRUE(inv.CanAdd(add_item).first);
        EXPECT_TRUE(inv.AddSub(add_item));

        EXPECT_EQ(add_item.count, 0);

        // Did not modify other items
        EXPECT_EQ(inv[0].item, another_item.get());
        EXPECT_EQ(inv[0].count, 10);

        // Added correctly??
        EXPECT_EQ(inv[1].item, item_we_add_to.get());
        EXPECT_EQ(inv[1].count, 50);
        EXPECT_EQ(inv[2].item, item_we_add_to.get());
        EXPECT_EQ(inv[2].count, 50);

        EXPECT_EQ(inv[3].item, item_we_add_to.get());
        EXPECT_EQ(inv[3].count, 50);

        // No items should be added to the 4th slot index
        EXPECT_EQ(inv[4].item, nullptr);
        EXPECT_EQ(inv[4].count, 0);
    }

    ///
    /// Slots 1 is full, inv size is 1, will return false
    TEST(Inventory, AddStackNoAvailableSlots) {
        Inventory inv(1);

        const auto item  = std::make_unique<proto::Item>();
        const auto item2 = std::make_unique<proto::Item>();

        inv[0].item  = item.get();
        inv[0].count = 10;

        auto add_item = ItemStack{item2.get(), 20};
        EXPECT_FALSE(inv.CanAdd(add_item).first);
        EXPECT_FALSE(inv.AddSub(add_item));

        EXPECT_EQ(add_item.count, 20);

        // Did not modify existing items
        EXPECT_EQ(inv[0].item, item.get());
        EXPECT_EQ(inv[0].count, 10);
    }

    ///
    /// Item must match filter, otherwise it cannot be at the slot with the filter
    TEST(Inventory, AddStackFiltered) {
        proto::Item filtered_item;
        proto::Item not_filtered_item;


        Inventory inv(2);

        inv[0].filter = &filtered_item;

        // Cannot insert into slot 0
        EXPECT_EQ(inv.CanAdd({&not_filtered_item, 10}).second, 1);
        inv.Add({&not_filtered_item, 10});

        EXPECT_EQ(inv[0].item, nullptr);
        EXPECT_EQ(inv[1].item, &not_filtered_item);
    }

    TEST(Inventory, CountItem) {
        const auto item  = std::make_unique<proto::Item>();
        const auto item2 = std::make_unique<proto::Item>();

        Inventory inv(30);

        // Count these to a sum of 101
        inv[0]  = {item.get(), 20};
        inv[5]  = {item.get(), 30};
        inv[8]  = {item.get(), 20};
        inv[14] = {item.get(), 20};
        inv[20] = {item.get(), 5};
        inv[29] = {item.get(), 6};

        // Should not count these
        inv[3]  = {item2.get(), 200};
        inv[9]  = {item2.get(), 200};
        inv[13] = {item2.get(), 200};
        inv[28] = {item2.get(), 200};

        EXPECT_EQ(inv.Count(item.get()), 101);
    }

    TEST(Inventory, GetFirstItem) {
        const auto item  = std::make_unique<proto::Item>();
        const auto item2 = std::make_unique<proto::Item>();

        Inventory inv(10);

        EXPECT_EQ(inv.First(), nullptr); // No items

        inv[3] = {item2.get(), 32};
        inv[4] = {item.get(), 2};

        EXPECT_EQ(inv.First(), item2.get());
    }

    TEST(Inventory, Remove) {
        Inventory inv(30);

        const auto item = std::make_unique<proto::Item>();
        inv[20]         = {item.get(), 5};
        inv[23]         = {item.get(), 5};

        EXPECT_TRUE(inv.Remove(item.get(), 10));

        // Inventory should be empty
        for (auto& i : inv) {
            EXPECT_EQ(i.item, nullptr);
            // EXPECT_EQ(i.second, 0);
        }
    }

    TEST(Inventory, RemoveNotEnough) {
        Inventory inv(30);

        const auto item = std::make_unique<proto::Item>();
        inv[20]         = {item.get(), 5};

        // Attempting to remove 10 when only 5 exists
        EXPECT_FALSE(inv.Remove(item.get(), 10));

        // Inventory unchanged
        EXPECT_EQ(inv[20].item, item.get());
        EXPECT_EQ(inv[20].count, 5);
    }
} // namespace jactorio::game
