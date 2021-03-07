// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "game/logistic/inventory.h"

namespace jactorio::game
{
    TEST(ItemStack, Accepts) {
        proto::Item item_1;
        proto::Item item_2;

        //
        EXPECT_FALSE(ItemStack({nullptr, 0}).Accepts({nullptr, 0}));

        EXPECT_TRUE(ItemStack({nullptr, 0}).Accepts({&item_1, 1}));
        EXPECT_FALSE(ItemStack({nullptr, 0}).Accepts({nullptr, 0, &item_1}));
        EXPECT_TRUE(ItemStack({nullptr, 0}).Accepts({&item_1, 1, &item_1}));


        //
        EXPECT_FALSE(ItemStack({nullptr, 0, &item_1}).Accepts({nullptr, 0}));

        EXPECT_TRUE(ItemStack({nullptr, 0, &item_1}).Accepts({&item_1, 1}));
        EXPECT_TRUE(ItemStack({nullptr, 0, &item_1}).Accepts({&item_1, 1, &item_1}));
        EXPECT_FALSE(ItemStack({nullptr, 0, &item_1}).Accepts({nullptr, 0, &item_1}));

        EXPECT_FALSE(ItemStack({nullptr, 0, &item_1}).Accepts({&item_2, 1}));
        EXPECT_FALSE(ItemStack({nullptr, 0, &item_1}).Accepts({&item_2, 1, &item_2}));
        EXPECT_FALSE(ItemStack({nullptr, 0, &item_1}).Accepts({nullptr, 0, &item_2}));


        //
        EXPECT_FALSE(ItemStack({&item_1, 1}).Accepts({nullptr, 0}));


        EXPECT_TRUE(ItemStack({&item_1, 1}).Accepts({&item_1, 1}));
        EXPECT_FALSE(ItemStack({&item_1, 1}).Accepts({nullptr, 0, &item_1}));
        EXPECT_TRUE(ItemStack({&item_1, 1}).Accepts({&item_1, 1, &item_1}));

        EXPECT_FALSE(ItemStack({&item_1, 1}).Accepts({&item_2, 1}));
        EXPECT_FALSE(ItemStack({&item_1, 1}).Accepts({nullptr, 0, &item_2}));
        EXPECT_FALSE(ItemStack({&item_1, 1}).Accepts({&item_2, 1, &item_2}));


        //
        EXPECT_FALSE(ItemStack({&item_1, 1, &item_1}).Accepts({nullptr, 0}));

        EXPECT_TRUE(ItemStack({&item_1, 1, &item_1}).Accepts({&item_1, 1}));
        EXPECT_FALSE(ItemStack({&item_1, 1, &item_1}).Accepts({nullptr, 0, &item_1}));
        EXPECT_TRUE(ItemStack({&item_1, 1, &item_1}).Accepts({&item_1, 0, &item_1}));

        EXPECT_FALSE(ItemStack({&item_1, 1, &item_1}).Accepts({&item_2, 1}));
        EXPECT_FALSE(ItemStack({&item_1, 1, &item_1}).Accepts({nullptr, 0, &item_2}));
        EXPECT_FALSE(ItemStack({&item_1, 1, &item_1}).Accepts({&item_2, 0, &item_2}));
    }

    TEST(ItemStack, Empty) {
        proto::Item item;
        item.stackSize = 10;

        EXPECT_TRUE(ItemStack({nullptr, 0}).Empty());
        EXPECT_FALSE(ItemStack({&item, 9}).Empty());
    }

    TEST(ItemStack, Full) {
        proto::Item item;
        item.stackSize = 10;

        EXPECT_FALSE(ItemStack({&item, 9}).Full());
        EXPECT_TRUE(ItemStack({&item, 10}).Full());
        EXPECT_TRUE(ItemStack({&item, 11}).Full());
    }

    TEST(ItemStack, Overloaded) {
        proto::Item item;
        item.stackSize = 10;

        EXPECT_FALSE(ItemStack({&item, 9}).Overloaded());
        EXPECT_FALSE(ItemStack({&item, 10}).Overloaded());
        EXPECT_TRUE(ItemStack({&item, 11}).Overloaded());
    }

    TEST(ItemStack, FreeCount) {
        proto::Item item;
        item.stackSize = 10;

        EXPECT_EQ(ItemStack({&item, 1}).FreeCount(), 9);
        EXPECT_EQ(ItemStack({&item, 9}).FreeCount(), 1);
        EXPECT_EQ(ItemStack({&item, 10}).FreeCount(), 0);
        EXPECT_EQ(ItemStack({&item, 11}).FreeCount(), 0);
    }

    TEST(ItemStack, DeleteNowEmpty) {
        proto::Item item;
        item.stackSize = 10;

        ItemStack stack({&item, 1});
        stack.Delete(1);

        EXPECT_EQ(stack, ItemStack({nullptr, 0}));
    }

    TEST(ItemStack, Delete) {
        proto::Item item;
        item.stackSize = 10;

        ItemStack stack({&item, 9});
        stack.Delete(4);

        EXPECT_EQ(stack, ItemStack({&item, 5}));
    }


    // ======================================================================


    /// Moving from inventory position 0 to position 3
    TEST(Inventory, MoveStackToEmptySlot) {
        Inventory inv(10);

        proto::Item item;
        item.stackSize = 50;

        // Position 3 should have the 50 items + item prototype after moving
        inv[0] = {&item, 50};
        inv[3] = {nullptr, 0};

        EXPECT_TRUE(MoveItemstackToIndex(inv[0], inv[3], 0));

        EXPECT_EQ(inv[0].item, nullptr);
        EXPECT_EQ(inv[0].count, 0);

        EXPECT_EQ(inv[3].item, &item);
        EXPECT_EQ(inv[3].count, 50);
    }

    /// Not exceeding the max stack size
    /// moving 10 items:
    /// position 3 already has 30 items, adding 10 from position 0 to equal 40.
    TEST(Inventory, MoveStackToPartialFilledSlotNonExceeding) {
        Inventory inv(10);

        proto::Item item;
        item.stackSize = 50;

        inv[0] = {&item, 10};
        inv[3] = {&item, 30};

        EXPECT_TRUE(MoveItemstackToIndex(inv[0], inv[3], 0));

        EXPECT_EQ(inv[0].item, nullptr);
        EXPECT_EQ(inv[0].count, 0);

        EXPECT_EQ(inv[3].item, &item);
        EXPECT_EQ(inv[3].count, 40);
    }

    /// EXCEEDING the max stack size
    /// moving 30 items:
    /// position 3 already has 30 items, meaning only 20 can be moved into it to reach the stack size of 50.
    /// this leaves 10 in the original location (0) and 50 in the target location (3)
    TEST(Inventory, MoveStackToPartialFilledSlotExceeding) {
        Inventory inv(10);

        proto::Item item;
        item.stackSize = 50;

        inv[0] = {&item, 30};
        inv[3] = {&item, 30};

        EXPECT_FALSE(MoveItemstackToIndex(inv[0], inv[3], 0));

        EXPECT_EQ(inv[0].item, &item);
        EXPECT_EQ(inv[0].count, 10);

        EXPECT_EQ(inv[3].item, &item);
        EXPECT_EQ(inv[3].count, 50);
    }

    /// The item stacks are of different items, therefore only swapping positions
    TEST(Inventory, SwapItemStacks) {
        Inventory inv(10);

        proto::Item item;
        item.stackSize = 50;

        proto::Item item2;
        item2.stackSize = 100;

        // Position 3 should have the 50 items + item prototype after moving
        inv[0] = {&item, 50};
        inv[3] = {&item2, 10};

        EXPECT_FALSE(MoveItemstackToIndex(inv[0], inv[3], 0));

        EXPECT_EQ(inv[0].item, &item2);
        EXPECT_EQ(inv[0].count, 10);

        EXPECT_EQ(inv[3].item, &item);
        EXPECT_EQ(inv[3].count, 50);
    }

    /// A slot is filtered, therefore no swap occurs
    TEST(Inventory, SwapItemStacksFilterd) {
        Inventory inv(10);

        const proto::Item item;
        const proto::Item item2;

        inv[0] = {&item, 2, &item};
        inv[3] = {&item2, 10};

        EXPECT_FALSE(MoveItemstackToIndex(inv[0], inv[3], 0));

        // 0 <> 3: No swap, filters do not match item
        EXPECT_EQ(inv[0].item, &item);
        EXPECT_EQ(inv[0].count, 2);
        EXPECT_EQ(inv[0].filter, &item);

        EXPECT_EQ(inv[3].item, &item2);
        EXPECT_EQ(inv[3].count, 10);
        EXPECT_EQ(inv[3].filter, nullptr);


        // 0 -> 3: Ok, 3 is empty
        inv[3] = {nullptr, 0};

        EXPECT_TRUE(MoveItemstackToIndex(inv[0], inv[3], 0));

        EXPECT_EQ(inv[0].item, nullptr);
        EXPECT_EQ(inv[0].count, 0);
        EXPECT_EQ(inv[0].filter, &item);

        EXPECT_EQ(inv[3].item, &item);
        EXPECT_EQ(inv[3].count, 2);
        EXPECT_EQ(inv[3].filter, nullptr);


        // 3 -> 0: Ok, 0 is empty
        EXPECT_FALSE(MoveItemstackToIndex(inv[0], inv[3], 0));

        EXPECT_EQ(inv[0].item, &item);
        EXPECT_EQ(inv[0].count, 2);
        EXPECT_EQ(inv[0].filter, &item);

        EXPECT_EQ(inv[3].item, nullptr);
        EXPECT_EQ(inv[3].count, 0);
        EXPECT_EQ(inv[3].filter, nullptr);
    }

    /// The target slot is full, origin slot has something. Swap the 2 items
    TEST(Inventory, MoveStackFullTargetSlot) {
        Inventory inv(10);

        proto::Item item;
        item.stackSize = 50;

        inv[0] = {&item, 18};
        inv[3] = {&item, 50};

        EXPECT_FALSE(MoveItemstackToIndex(inv[0], inv[3], 0));

        EXPECT_EQ(inv[0].item, &item);
        EXPECT_EQ(inv[0].count, 50);

        EXPECT_EQ(inv[3].item, &item);
        EXPECT_EQ(inv[3].count, 18);
    }

    /// Moving nothing to nothing results in nothing!
    TEST(Inventory, MoveEmptySlotToEmptySlot) {
        Inventory inv(10);

        inv[0] = {nullptr, 0};
        inv[3] = {nullptr, 0};

        EXPECT_TRUE(MoveItemstackToIndex(inv[0], inv[3], 0));

        EXPECT_EQ(inv[0].item, nullptr);
        EXPECT_EQ(inv[0].count, 0);

        EXPECT_EQ(inv[3].item, nullptr);
        EXPECT_EQ(inv[3].count, 0);
    }

    // Items exceeding their item stacks

    /// The origin item is exceeding its stack size
    /// Move out only the stack size into the empty slot
    TEST(Inventory, MoveExceedingStackToEmptySlot) {
        Inventory inv(10);

        proto::Item item;
        item.stackSize = 50;

        inv[0] = {&item, 9000};
        inv[3] = {nullptr, 0};

        EXPECT_FALSE(MoveItemstackToIndex(inv[0], inv[3], 0));

        EXPECT_EQ(inv[0].item, &item);
        EXPECT_EQ(inv[0].count, 8950);

        EXPECT_EQ(inv[3].item, &item);
        EXPECT_EQ(inv[3].count, 50);
    }

    /// The TARGET item is exceeding its stack size
    /// Move out only the stack size into the empty origin slot
    TEST(Inventory, MoveEmptySlotToExceedingStack) {
        Inventory inv(10);

        proto::Item item;
        item.stackSize = 50;

        inv[0] = {nullptr, 0};
        inv[3] = {&item, 9000};

        EXPECT_FALSE(MoveItemstackToIndex(inv[0], inv[3], 0));

        EXPECT_EQ(inv[0].item, &item);
        EXPECT_EQ(inv[0].count, 50);

        EXPECT_EQ(inv[3].item, &item);
        EXPECT_EQ(inv[3].count, 8950);
    }

    /// The origin item is exceeding its stack size
    /// Move out only 10 to reach the stack size in the target slot
    TEST(Inventory, MoveExceedingStackToPartialSlot) {
        Inventory inv(10);

        proto::Item item;
        item.stackSize = 50;

        inv[0] = {&item, 9000};
        inv[3] = {&item, 40};

        EXPECT_FALSE(MoveItemstackToIndex(inv[0], inv[3], 0));

        EXPECT_EQ(inv[0].item, &item);
        EXPECT_EQ(inv[0].count, 8990);

        EXPECT_EQ(inv[3].item, &item);
        EXPECT_EQ(inv[3].count, 50);
    }

    /// First stack is filtered, cannot move second stack into first
    TEST(Inventory, MoveTakeOriginFiltered) {
        Inventory inv(10);

        proto::Item item;
        proto::Item item_2;

        inv[0] = {nullptr, 0, &item_2};
        inv[3] = {&item, 255};

        EXPECT_TRUE(MoveItemstackToIndex(inv[0], inv[3], 0));

        EXPECT_EQ(inv[0].item, nullptr);
        EXPECT_EQ(inv[0].count, 0);

        EXPECT_EQ(inv[3].item, &item);
        EXPECT_EQ(inv[3].count, 255);
    }

    // ------------------------------------------------------
    // RIGHT click tests
    // ------------------------------------------------------

    /// Target has even number of items
    TEST(Inventory, MoveRclickTargetTakeHalfEven) {
        Inventory inv(10);

        proto::Item item;
        item.stackSize = 50;

        // Case 1, even number
        inv[0] = {nullptr, 0};
        inv[3] = {&item, 40};

        EXPECT_FALSE(MoveItemstackToIndex(inv[0], inv[3], 1));

        EXPECT_EQ(inv[0].item, &item);
        EXPECT_EQ(inv[0].count, 20);

        EXPECT_EQ(inv[3].item, &item);
        EXPECT_EQ(inv[3].count, 20);
    }

    /// Target stack has odd number of items
    TEST(Inventory, MoveRclickTargetTakeHalfOdd) {
        Inventory inv(10);

        proto::Item item;
        item.stackSize = 50;

        // Case 2, odd number
        inv[0] = {nullptr, 0};
        inv[3] = {&item, 39};

        EXPECT_FALSE(MoveItemstackToIndex(inv[0], inv[3], 1));

        EXPECT_EQ(inv[0].item, &item);
        EXPECT_EQ(inv[0].count, 19);

        EXPECT_EQ(inv[3].item, &item);
        EXPECT_EQ(inv[3].count, 20);
    }

    /// Target stack only has 1 item
    TEST(Inventory, MoveRclickTargetOnly1ToTake) {
        Inventory inv(10);

        proto::Item item;
        item.stackSize = 50;

        inv[0] = {nullptr, 0};
        inv[3] = {&item, 1};

        EXPECT_FALSE(MoveItemstackToIndex(inv[0], inv[3], 1));

        EXPECT_EQ(inv[0].item, &item);
        EXPECT_EQ(inv[0].count, 1);

        EXPECT_EQ(inv[3].item, nullptr);
        EXPECT_EQ(inv[3].count, 0);
    }

    /// Target stack exceeds stack size
    TEST(Inventory, MoveRclickTargetExceedStack) {
        Inventory inv(10);

        proto::Item item;
        item.stackSize = 50;

        inv[0] = {nullptr, 0};
        inv[3] = {&item, 110};

        EXPECT_FALSE(MoveItemstackToIndex(inv[0], inv[3], 1));

        EXPECT_EQ(inv[0].item, &item);
        EXPECT_EQ(inv[0].count, 50);

        EXPECT_EQ(inv[3].item, &item);
        EXPECT_EQ(inv[3].count, 60);
    }

    /// Target stack exceeds stack size, taking half is still within stack size
    TEST(Inventory, MoveRclickTargetExceedStackHalfWithinStack) {
        Inventory inv(10);

        proto::Item item;
        item.stackSize = 50;

        inv[0] = {nullptr, 0};
        inv[3] = {&item, 62};

        EXPECT_FALSE(MoveItemstackToIndex(inv[0], inv[3], 1));

        EXPECT_EQ(inv[0].item, &item);
        EXPECT_EQ(inv[0].count, 31);

        EXPECT_EQ(inv[3].item, &item);
        EXPECT_EQ(inv[3].count, 31);
    }

    /// Target stack empty, right clicking drops 1 item off
    TEST(Inventory, MoveRclickEmptyTargetDrop1) {
        Inventory inv(10);

        proto::Item item;
        item.stackSize = 50;

        inv[0] = {&item, 10};
        inv[3] = {nullptr, 0};

        EXPECT_FALSE(MoveItemstackToIndex(inv[0], inv[3], 1));

        EXPECT_EQ(inv[0].item, &item);
        EXPECT_EQ(inv[0].count, 9);

        EXPECT_EQ(inv[3].item, &item);
        EXPECT_EQ(inv[3].count, 1);
    }

    /// Target stack empty, right clicking drops 1 item off, origin stack is now empty
    TEST(Inventory, MoveRclickEmptyTargetDrop1Has1) {
        Inventory inv(10);

        proto::Item item;
        item.stackSize = 50;

        inv[0] = {&item, 1};
        inv[3] = {nullptr, 0};

        EXPECT_TRUE(MoveItemstackToIndex(inv[0], inv[3], 1));

        EXPECT_EQ(inv[0].item, nullptr);
        EXPECT_EQ(inv[0].count, 0);

        EXPECT_EQ(inv[3].item, &item);
        EXPECT_EQ(inv[3].count, 1);
    }

    TEST(Inventory, MoveRclickDropIntoFiltered) {
        Inventory inv(10);

        proto::Item item;
        proto::Item item_2;

        inv[0] = {&item, 10};
        inv[3] = {nullptr, 0, &item_2};

        EXPECT_FALSE(MoveItemstackToIndex(inv[0], inv[3], 1));

        EXPECT_EQ(inv[0].item, &item);
        EXPECT_EQ(inv[0].count, 10);

        EXPECT_EQ(inv[3].item, nullptr);
        EXPECT_EQ(inv[3].count, 0);
    }

    // ======================================================================

    TEST(Inventory, Sort) {
        proto::Item item;
        item.stackSize = 50;

        proto::Item item_2;
        item_2.stackSize = 10;

        Inventory inv{40};

        // Item 1
        inv[0]  = {&item, 10};
        inv[10] = {&item, 25};
        inv[20] = {&item, 25};
        inv[13] = {&item, 20};
        inv[14] = {&item, 30};

        // Item 2
        inv[31] = {&item_2, 4};
        inv[32] = {&item_2, 6};
        inv[22] = {&item_2, 1};


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

        for (std::size_t i = 5; i < inv.Size(); ++i) {
            EXPECT_TRUE(inv[i].Empty());
        }
    }

    TEST(Inventory, SortFull) {
        proto::Item item;
        item.stackSize = 50;

        Inventory inv{30};
        for (auto& i : inv) {
            i = {&item, 50};
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

        inv[0]  = {&item, 10};
        inv[10] = {&item, 100};
        inv[11] = {&item, 100};
        inv[12] = {&item, 10};

        inv.Sort();

        EXPECT_EQ(inv[0].item, &item);
        EXPECT_EQ(inv[0].count, 50);

        EXPECT_EQ(inv[1].item, &item);
        EXPECT_EQ(inv[1].count, 100);

        EXPECT_EQ(inv[2].item, &item);
        EXPECT_EQ(inv[2].count, 70);
    }

    ///
    /// Should find the first empty slot and add the item there
    /// Slots, 0, 1 Will be with another item
    /// Should place in slot 2
    TEST(Inventory, AddStackToEmptySlot) {
        Inventory inv(10);

        const proto::Item item;
        const proto::Item item2;

        // Another item
        inv[0] = {&item, 10};
        inv[1] = {&item, 21};

        ItemStack add_item{&item2, 20};
        EXPECT_TRUE(inv.CanAdd(add_item).first);
        EXPECT_EQ(inv.Add(add_item), 0);

        // Did not modify existing items
        EXPECT_EQ(inv[0].item, &item);
        EXPECT_EQ(inv[0].count, 10);
        EXPECT_EQ(inv[1].item, &item);
        EXPECT_EQ(inv[1].count, 21);

        // Added itemstack
        EXPECT_EQ(inv[2].item, &item2);
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

        const proto::Item another_item;

        proto::Item item_we_add_to;
        item_we_add_to.stackSize = 50;

        inv[0] = {&another_item, 10};

        // Will fill up first 2, then dump the remaining in 3
        inv[1] = {&item_we_add_to, 40};
        inv[2] = {&item_we_add_to, 40};
        inv[3] = {&item_we_add_to, 20};

        ItemStack add_item{&item_we_add_to, 50};
        EXPECT_TRUE(inv.CanAdd(add_item).first);
        EXPECT_EQ(inv.Add(add_item), 0);

        // Did not modify other items
        EXPECT_EQ(inv[0].item, &another_item);
        EXPECT_EQ(inv[0].count, 10);

        // Added correctly??
        EXPECT_EQ(inv[1].item, &item_we_add_to);
        EXPECT_EQ(inv[1].count, 50);
        EXPECT_EQ(inv[2].item, &item_we_add_to);
        EXPECT_EQ(inv[2].count, 50);

        EXPECT_EQ(inv[3].item, &item_we_add_to);
        EXPECT_EQ(inv[3].count, 50);

        // No items should be added to the 4th slot index
        EXPECT_EQ(inv[4].item, nullptr);
        EXPECT_EQ(inv[4].count, 0);
    }

    /// Slots 1 is full, inv size is 1, will return false
    TEST(Inventory, AddStackNoAvailableSlots) {
        Inventory inv(1);

        const proto::Item item;
        const proto::Item item2;

        inv[0] = {&item, 10};

        ItemStack add_item{&item2, 20};
        EXPECT_FALSE(inv.CanAdd(add_item).first);
        EXPECT_EQ(inv.Add(add_item), 20);

        // Did not modify existing items
        EXPECT_EQ(inv[0].item, &item);
        EXPECT_EQ(inv[0].count, 10);
    }

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
        const proto::Item item;
        const proto::Item item2;

        Inventory inv(30);

        // Count these to a sum of 101
        inv[0]  = {&item, 20};
        inv[5]  = {&item, 30};
        inv[8]  = {&item, 20};
        inv[14] = {&item, 20};
        inv[20] = {&item, 5};
        inv[29] = {&item, 6};

        // Should not count these
        inv[3]  = {&item2, 200};
        inv[9]  = {&item2, 200};
        inv[13] = {&item2, 200};
        inv[28] = {&item2, 200};

        EXPECT_EQ(inv.Count(item), 101);
    }

    TEST(Inventory, GetFirstItem) {
        const proto::Item item;
        const proto::Item item2;

        Inventory inv(10);

        EXPECT_EQ(inv.First(), nullptr); // No items

        inv[3] = {&item2, 32};
        inv[4] = {&item, 2};

        EXPECT_EQ(inv.First(), &item2);
    }

    TEST(Inventory, Remove) {
        Inventory inv(30);

        proto::Item item;
        inv[20] = {&item, 5};
        inv[23] = {&item, 5};
        inv[24] = {&item, 1};

        EXPECT_TRUE(inv.Remove(item, 10));

        for (std::size_t i = 0; i < inv.Size(); ++i) {
            if (i == 24) {
                continue;
            }
            EXPECT_TRUE(inv[i].Empty());
        }

        EXPECT_EQ(inv[24].item, &item);
        EXPECT_EQ(inv[24].count, 1);
    }

    TEST(Inventory, RemoveNotEnough) {
        Inventory inv(30);

        proto::Item item;
        inv[20] = {&item, 5};

        // Attempting to remove 10 when only 5 exists
        EXPECT_FALSE(inv.Remove(item, 10));

        // Inventory unchanged
        EXPECT_EQ(inv[20].item, &item);
        EXPECT_EQ(inv[20].count, 5);
    }
} // namespace jactorio::game
