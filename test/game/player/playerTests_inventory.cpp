// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "game/player/player.h"

#include "jactorioTests.h"

namespace jactorio::game
{
    class PlayerInventoryTest : public testing::Test
    {
    protected:
        Player::Inventory playerInv_;
        data::PrototypeManager proto_;

        proto::Item* cursor_ = nullptr;

        ///
        /// Creates the cursor prototype which is hardcoded when an item is selected
        void SetupInventoryCursor() {
            cursor_ = &proto_.Make<proto::Item>(proto::Item::kInventorySelectedCursor);
        }
    };

    TEST_F(PlayerInventoryTest, InventoryLclickSelectItemByReference) {
        // Left click on a slot picks up items by reference
        // The inventory slot becomes the cursor
        // The cursor holds the item
        SetupInventoryCursor();

        const proto::Item item;


        // Position 3 should have the 50 items + item prototype after moving
        playerInv_.inventory[0].item  = &item;
        playerInv_.inventory[0].count = 50;

        playerInv_.HandleClick(proto_, 0, 0, true, playerInv_.inventory);

        EXPECT_EQ(playerInv_.inventory[0].item, cursor_);
        EXPECT_EQ(playerInv_.inventory[0].count, 0);

        const auto* cursor_item = playerInv_.GetSelectedItem();
        EXPECT_EQ(cursor_item->item, &item);
        EXPECT_EQ(cursor_item->count, 50);
    }

    TEST_F(PlayerInventoryTest, InventoryLeftClickDeselectReferencedItem) {
        // Left click on a slot picks up items by reference
        // Left / right clicking again on the same slot deselects the item
        SetupInventoryCursor();

        const proto::Item item;

        playerInv_.inventory[0].item  = &item;
        playerInv_.inventory[0].count = 50;

        playerInv_.HandleClick(proto_, 0, 0, true, playerInv_.inventory); // Select

        EXPECT_EQ(playerInv_.inventory[0].item, cursor_);
        EXPECT_EQ(playerInv_.inventory[0].count, 0);


        playerInv_.HandleClick(proto_, 0, 0, true, playerInv_.inventory); // Deselect

        EXPECT_EQ(playerInv_.inventory[0].item, &item);
        EXPECT_EQ(playerInv_.inventory[0].count, 50);

        const auto* cursor_item = playerInv_.GetSelectedItem();
        EXPECT_EQ(cursor_item, nullptr);
    }

    TEST_F(PlayerInventoryTest, InventoryRightClickDeselectReferencedItem) {
        SetupInventoryCursor();

        const proto::Item item;

        playerInv_.inventory[0].item  = &item;
        playerInv_.inventory[0].count = 50;

        playerInv_.HandleClick(proto_, 0, 0, true, playerInv_.inventory); // Select

        const auto* cursor_item = playerInv_.GetSelectedItem();
        EXPECT_EQ(cursor_item->item, &item);
        EXPECT_EQ(cursor_item->count, 50);


        playerInv_.HandleClick(proto_, 0, 1, true, playerInv_.inventory); // Deselect

        EXPECT_EQ(playerInv_.inventory[0].item, &item);
        EXPECT_EQ(playerInv_.inventory[0].count, 50);

        cursor_item = playerInv_.GetSelectedItem();
        EXPECT_EQ(cursor_item, nullptr);
    }

    /*
    TEST_F(PlayerInventoryTest, InventoryDeselectReferencedItem2Inventories) {
        // Left click on a slot picks up items by reference
        // Left / right clicking again on the same slot in another inventory however will not deselect the item
        SetupInventoryCursor();

        const proto::Item item;

        playerInv_.ClearPlayerInventory();
        // Left click into another inventory
        {
            // Position 3 should have the 50 items + item prototype after moving
            playerInv_.inventoryPlayer[0].item  = &item;
            playerInv_.inventoryPlayer[0].count = 50;

            playerInv_.InventoryClick(proto_, 0, 0, true, playerInv_.inventoryPlayer);  // Select

            // Deselect into inv_2
            proto::Item::Inventory inv_2{10};
            playerInv_.InventoryClick(proto_, 0, 0, true, inv_2);  // Deselect

            EXPECT_EQ(inv_2[0].item, &item);
            EXPECT_EQ(inv_2[0].count, 50);

            const auto* cursor_item = playerInv_.GetSelectedItemStack();
            EXPECT_EQ(cursor_item, nullptr);

            // Selected cursor should no longer exist in inventory_player
            EXPECT_EQ(playerInv_.inventoryPlayer[0].item, nullptr);
        }
        playerInv_.Clear();
        // Right click into another inventory
        {
            // Position 3 should have the 50 items + item prototype after moving
            playerInv_.inventoryPlayer[0].item  = &item;
            playerInv_.inventoryPlayer[0].count = 50;

            playerInv_.InventoryClick(proto_, 0, 0, true, playerInv_.inventoryPlayer);  // Select

            // Deselect into inv_2
            proto::Item::Inventory inv_2{10};
            playerInv_.InventoryClick(proto_, 0, 1, true, inv_2);  // Will NOT Deselect since in another inventory

            EXPECT_EQ(inv_2[0].item, &item);
            EXPECT_EQ(inv_2[0].count, 1);

            // Cursor still holds 49
            const auto* cursor_item = playerInv_.GetSelectedItem();
            EXPECT_EQ(cursor_item->item, &item);
            EXPECT_EQ(cursor_item->count, 49);

            // Selected cursor should STILL exist in inventory_player since not deselected
            EXPECT_NE(playerInv_.inventoryPlayer[0].item, nullptr);
        }
    }
 */

    TEST_F(PlayerInventoryTest, InventoryMoveReferencedItem) {
        // Left click on a slot picks up items by reference
        // Left click on index 3 to drop off the item at index

        // Index 0 (origin) should be empty
        // Create the cursor prototype
        SetupInventoryCursor();

        const proto::Item item;

        // Left click
        {
            // Position 3 should have the 50 items + item prototype after moving
            playerInv_.inventory[0].item  = &item;
            playerInv_.inventory[0].count = 50;

            playerInv_.HandleClick(proto_, 0, 0, true, playerInv_.inventory); // Select item
            playerInv_.HandleClick(proto_, 3, 0, true, playerInv_.inventory); // Drop item off


            EXPECT_EQ(playerInv_.inventory[0].item, nullptr);
            EXPECT_EQ(playerInv_.inventory[0].count, 0);

            EXPECT_EQ(playerInv_.inventory[3].item, &item);
            EXPECT_EQ(playerInv_.inventory[3].count, 50);

            const auto* cursor_item = playerInv_.GetSelectedItem();
            EXPECT_EQ(cursor_item, nullptr);
        }
    }


    TEST_F(PlayerInventoryTest, InventoryRclickSelectItemByUnique) {
        // Right click on a slot creates a new inventory slot in the cursor and places half from the inventory into it
        const proto::Item item;


        playerInv_.inventory[0].item  = &item;
        playerInv_.inventory[0].count = 40;

        playerInv_.HandleClick(proto_, 0, 1, true, playerInv_.inventory); // Pick up half

        playerInv_.inventory[0].item  = &item;
        playerInv_.inventory[0].count = 20;

        const auto* cursor_item = playerInv_.GetSelectedItem();
        EXPECT_EQ(cursor_item->item, &item);
        EXPECT_EQ(cursor_item->count, 20);
    }

    TEST_F(PlayerInventoryTest, InventoryDropSingleUniqueItemNewStack) {
        // Right click on item to pick up half into cursor
        // Right click on empty slot to drop 1 off from the cursor
        const proto::Item item;

        playerInv_.inventory[0].item  = &item;
        playerInv_.inventory[0].count = 10;

        playerInv_.HandleClick(proto_, 0, 1, true, playerInv_.inventory); // Pick up half

        EXPECT_EQ(playerInv_.inventory[0].item, &item);
        EXPECT_EQ(playerInv_.inventory[0].count, 5);

        const auto* cursor_item = playerInv_.GetSelectedItem();
        EXPECT_EQ(cursor_item->item, &item);
        EXPECT_EQ(cursor_item->count, 5);


        playerInv_.HandleClick(proto_, 3, 1, true, playerInv_.inventory); // Drop 1 at index 3

        // Should remain unchanged
        EXPECT_EQ(playerInv_.inventory[0].item, &item);
        EXPECT_EQ(playerInv_.inventory[0].count, 5);

        // Loses 1
        cursor_item = playerInv_.GetSelectedItem();
        EXPECT_EQ(cursor_item->item, &item);
        EXPECT_EQ(cursor_item->count, 4);

        // Gain 1
        EXPECT_EQ(playerInv_.inventory[3].item, &item);
        EXPECT_EQ(playerInv_.inventory[3].count, 1);
    }

    TEST_F(PlayerInventoryTest, InventoryDropSingleItemOriginalStack) {
        const proto::Item item;

        playerInv_.inventory[0].item  = &item;
        playerInv_.inventory[0].count = 10;

        playerInv_.HandleClick(proto_, 0, 1, true, playerInv_.inventory); // Pick up half
        playerInv_.HandleClick(proto_, 0, 1, true, playerInv_.inventory); // Drop 1 one the stack it picked up from

        // Loses 1
        const auto* cursor_item = playerInv_.GetSelectedItem();
        EXPECT_EQ(cursor_item->item, &item);
        EXPECT_EQ(cursor_item->count, 4);

        // Half (5) plus 1 = 6
        EXPECT_EQ(playerInv_.inventory[0].item, &item);
        EXPECT_EQ(playerInv_.inventory[0].count, 6);
    }

    TEST_F(PlayerInventoryTest, InventoryDropStackUniqueItem) {
        // Right click on item to pick up half into cursor
        // Left click on empty slot to drop entire stack off from the cursor
        const proto::Item item;


        playerInv_.inventory[0].item  = &item;
        playerInv_.inventory[0].count = 10;


        playerInv_.HandleClick(proto_, 0, 1, true, playerInv_.inventory); // Pick up half

        EXPECT_EQ(playerInv_.inventory[0].item, &item);
        EXPECT_EQ(playerInv_.inventory[0].count, 5);

        const auto* cursor_item = playerInv_.GetSelectedItem();
        EXPECT_EQ(cursor_item->item, &item);
        EXPECT_EQ(cursor_item->count, 5);


        playerInv_.HandleClick(proto_, 3, 0, true, playerInv_.inventory); // Drop stack at index 3

        // Should remain unchanged
        EXPECT_EQ(playerInv_.inventory[0].item, &item);
        EXPECT_EQ(playerInv_.inventory[0].count, 5);

        // Empty
        cursor_item = playerInv_.GetSelectedItem();
        EXPECT_EQ(cursor_item, nullptr);

        // Gain 10
        EXPECT_EQ(playerInv_.inventory[3].item, &item);
        EXPECT_EQ(playerInv_.inventory[3].count, 5);
    }

    TEST_F(PlayerInventoryTest, InventoryClickEmptySlot) {
        // Left click on empty slot
        // Should remain unchanged
        playerInv_.inventory[0].item  = nullptr;
        playerInv_.inventory[0].count = 0;

        playerInv_.HandleClick(proto_, 0, 0, true, playerInv_.inventory);

        EXPECT_EQ(playerInv_.inventory[0].item, nullptr);
        EXPECT_EQ(playerInv_.inventory[0].count, 0);

        const auto* cursor_item = playerInv_.GetSelectedItem();
        EXPECT_EQ(cursor_item, nullptr);
    }

    TEST_F(PlayerInventoryTest, InventoryClickFilters) {
        // A inventory click at a stack location with a filter requires that the item attempting to insert into it
        // matches the filter

        proto::Item filtered_item{};
        proto::Item not_filtered_item{};

        // Slot 0 is filtered
        proto::Item::Inventory inv{10};
        inv[0].filter = &filtered_item;

        // Has item not matching filter
        playerInv_.SetSelectedItem({&not_filtered_item, 10});

        // Cannot insert into slot 0
        playerInv_.HandleClick(proto_, 0, 0, false, inv);
        EXPECT_EQ(inv[0].item, nullptr);
    }


    // ======================================================================


    // Increment / decrement selected item
    TEST_F(PlayerInventoryTest, IncrementSelectedItem) {
        // If player selects item by "unique" or "reference",
        // It should function the same as it only modifies the cursor item stack
        proto::Item item;
        item.stackSize = 50;

        playerInv_.inventory[0].item  = &item;
        playerInv_.inventory[0].count = 10;

        // Pickup
        {
            // Pick up 5 of item, now selected
            playerInv_.HandleClick(proto_, 0, 1, true, playerInv_.inventory);

            // Check if item was incremented
            EXPECT_EQ(playerInv_.IncrementSelectedItem(), true);

            EXPECT_EQ(playerInv_.inventory[0].item, &item);
            EXPECT_EQ(playerInv_.inventory[0].count, 5);

            const auto* cursor_item = playerInv_.GetSelectedItem();
            EXPECT_EQ(cursor_item->item, &item);
            EXPECT_EQ(cursor_item->count, 6); // This incremented by 1
        }

        // Drop item down at inv slot 1
        {
            playerInv_.HandleClick(proto_, 1, 0, true, playerInv_.inventory);

            // Inv now empty, contents in inv slot 1
            EXPECT_EQ(playerInv_.inventory[1].item, &item);
            EXPECT_EQ(playerInv_.inventory[1].count, 6);

            const auto* cursor_item = playerInv_.GetSelectedItem();
            EXPECT_EQ(cursor_item, nullptr);
        }
    }

    TEST_F(PlayerInventoryTest, IncrementSelectedItemExceedItemStack) {
        // Attempting to increment an item exceeding item stack returns false and fails the increment
        proto::Item item;
        item.stackSize = 50;

        playerInv_.inventory[0].item  = &item;
        playerInv_.inventory[0].count = 50;

        // Pickup
        playerInv_.HandleClick(proto_, 0, 0, true, playerInv_.inventory);

        // Failed to add item: Item stack already full
        EXPECT_EQ(playerInv_.IncrementSelectedItem(), false);

        const auto* cursor_item = playerInv_.GetSelectedItem();
        EXPECT_EQ(cursor_item->item, &item);
        EXPECT_EQ(cursor_item->count, 50); // This unchanged
    }


    TEST_F(PlayerInventoryTest, DecrementSelectedItemUnique) {
        // If player selects item by "unique"
        // If decremented to 0, deselect the cursor item
        proto::Item item;
        item.stackSize = 50;

        playerInv_.inventory[0].item  = &item;
        playerInv_.inventory[0].count = 10;

        // Pickup
        {
            // Pick up 5 of item, now selected
            playerInv_.HandleClick(proto_, 0, 1, true, playerInv_.inventory);

            // Check if item was incremented
            EXPECT_EQ(playerInv_.DecrementSelectedItem(), true);

            EXPECT_EQ(playerInv_.inventory[0].item, &item);
            EXPECT_EQ(playerInv_.inventory[0].count, 5);

            const auto* cursor_item = playerInv_.GetSelectedItem();
            EXPECT_EQ(cursor_item->item, &item);
            EXPECT_EQ(cursor_item->count, 4); // This decremented by 1
        }

        // Drop item down at inv slot 1
        {
            playerInv_.HandleClick(proto_, 1, 0, true, playerInv_.inventory);

            // Inv now empty, contents in inv slot 1
            EXPECT_EQ(playerInv_.inventory[1].item, &item);
            EXPECT_EQ(playerInv_.inventory[1].count, 4);

            const auto* cursor_item = playerInv_.GetSelectedItem();
            EXPECT_EQ(cursor_item, nullptr);
        }
    }

    TEST_F(PlayerInventoryTest, DeselectSelectedItem) {
        // If player selects item by "unique"
        // If decremented to 0, deselect the cursor item
        proto::Item item;
        item.stackSize = 50;

        playerInv_.inventory[0].item  = &item;
        playerInv_.inventory[0].count = 10;

        playerInv_.HandleClick(proto_, 0, 0, true, playerInv_.inventory);

        EXPECT_TRUE(playerInv_.DeselectSelectedItem());

        // Returned to where selection cursor was
        EXPECT_EQ(playerInv_.inventory[0].item, &item);
        EXPECT_EQ(playerInv_.inventory[0].count, 10);

        const auto* cursor_item = playerInv_.GetSelectedItem();
        EXPECT_EQ(cursor_item, nullptr);
    }

    TEST_F(PlayerInventoryTest, DecrementSelectedItemReachZeroReference) {
        // Selected by reference
        // If decremented to 0, deselect the cursor item
        // If the selected item is empty after decrementing, return false
        SetupInventoryCursor();

        proto::Item item;
        item.stackSize = 50;

        playerInv_.inventory[0].item  = &item;
        playerInv_.inventory[0].count = 1;

        // Pickup
        playerInv_.HandleClick(proto_, 0, 0, true, playerInv_.inventory);

        EXPECT_EQ(playerInv_.DecrementSelectedItem(), false);

        // Cursor is nullptr: no item selected
        const auto* cursor_item = playerInv_.GetSelectedItem();
        EXPECT_EQ(cursor_item, nullptr);

        // Should remove the selection cursor on the item
        EXPECT_EQ(playerInv_.inventory[0].item, nullptr);
        EXPECT_EQ(playerInv_.inventory[0].count, 0);
    }

    TEST_F(PlayerInventoryTest, PlayerInventorySort) {
        proto::Item item;
        item.stackSize = 50;

        proto::Item item_2;
        item_2.stackSize = 10;

        // Item 1
        playerInv_.inventory[0].item  = &item;
        playerInv_.inventory[0].count = 10;

        playerInv_.inventory[10].item  = &item;
        playerInv_.inventory[10].count = 25;

        playerInv_.inventory[20].item  = &item;
        playerInv_.inventory[20].count = 25;

        playerInv_.inventory[13].item  = &item;
        playerInv_.inventory[13].count = 20;

        playerInv_.inventory[14].item  = &item;
        playerInv_.inventory[14].count = 30;

        // Item 2
        playerInv_.inventory[31].item  = &item_2;
        playerInv_.inventory[31].count = 4;

        playerInv_.inventory[32].item  = &item_2;
        playerInv_.inventory[32].count = 6;

        playerInv_.inventory[22].item  = &item_2;
        playerInv_.inventory[22].count = 1;


        // Sorted inventory should be as follows
        // Item(count)
        // 1(50), 1(50), 1(10), 2(10), 2(1)
        playerInv_.InventorySort(playerInv_.inventory);

        EXPECT_EQ(playerInv_.inventory[0].item, &item);
        EXPECT_EQ(playerInv_.inventory[0].count, 50);
        EXPECT_EQ(playerInv_.inventory[1].item, &item);
        EXPECT_EQ(playerInv_.inventory[1].count, 50);
        EXPECT_EQ(playerInv_.inventory[2].item, &item);
        EXPECT_EQ(playerInv_.inventory[2].count, 10);

        EXPECT_EQ(playerInv_.inventory[3].item, &item_2);
        EXPECT_EQ(playerInv_.inventory[3].count, 10);
        EXPECT_EQ(playerInv_.inventory[4].item, &item_2);
        EXPECT_EQ(playerInv_.inventory[4].count, 1);
    }

    TEST_F(PlayerInventoryTest, PlayerInventorySort2) {
        // Sorting will not move the item with inventory_selected_cursor_iname (to prevent breaking the inventory logic)
        SetupInventoryCursor();

        playerInv_.inventory[10].item  = cursor_;
        playerInv_.inventory[10].count = 0;


        playerInv_.InventorySort(playerInv_.inventory);

        EXPECT_EQ(playerInv_.inventory[10].item, cursor_);
        EXPECT_EQ(playerInv_.inventory[10].count, 0);

        // There should have been no new cursors created anywhere
        for (int i = 0; i < playerInv_.inventory.size(); ++i) {
            if (i == 10)
                continue;
            EXPECT_NE(playerInv_.inventory[i].item, cursor_);
        }
    }

    TEST_F(PlayerInventoryTest, PlayerInventorySortFull) {
        // Sorting the inventory when it is full should also work
        proto::Item item;
        item.stackSize = 50;

        for (auto& i : playerInv_.inventory) {
            i.item  = &item;
            i.count = 50;
        }

        playerInv_.InventorySort(playerInv_.inventory);


        // There should have been no new cursors created anywhere
        for (auto& i : playerInv_.inventory) {
            EXPECT_EQ(i.item, &item);
            EXPECT_EQ(i.count, 50);
        }
    }

    TEST_F(PlayerInventoryTest, PlayerInventorySortItemExcedingStack) {
        // If there is an item which exceeds its stack size, do not attempt to stack into it
        proto::Item item;
        item.stackSize = 50;

        playerInv_.inventory[10].item  = &item;
        playerInv_.inventory[10].count = 100;

        playerInv_.inventory[11].item  = &item;
        playerInv_.inventory[11].count = 100;

        playerInv_.inventory[12].item  = &item;
        playerInv_.inventory[12].count = 10;

        playerInv_.InventorySort(playerInv_.inventory);

        EXPECT_EQ(playerInv_.inventory[0].item, &item);
        EXPECT_EQ(playerInv_.inventory[0].count, 100);

        EXPECT_EQ(playerInv_.inventory[1].item, &item);
        EXPECT_EQ(playerInv_.inventory[1].count, 100);

        EXPECT_EQ(playerInv_.inventory[2].item, &item);
        EXPECT_EQ(playerInv_.inventory[2].count, 10);
    }

    TEST_F(PlayerInventoryTest, Serialize) {
        playerInv_.inventory[3].count = 43;

        auto result = TestSerializeDeserialize(playerInv_);

        EXPECT_EQ(result.inventory[3].count, 43);
    }

    TEST_F(PlayerInventoryTest, SerializeSelectedItem) {
        SetupInventoryCursor();

        auto& item = proto_.Make<proto::Item>();

        playerInv_.inventory[8].item  = &item;
        playerInv_.inventory[8].count = 20;
        playerInv_.HandleClick(proto_, 8, 0, true, playerInv_.inventory);


        proto_.GenerateRelocationTable();
        data::active_prototype_manager = &proto_;

        auto result = TestSerializeDeserialize(playerInv_);


        const auto* selected_item = result.GetSelectedItem();
        ASSERT_NE(selected_item, nullptr);
        EXPECT_EQ(selected_item->item, &item);
        EXPECT_EQ(selected_item->count, 20);

        result.HandleClick(proto_, 8, 0, true, result.inventory);
        EXPECT_EQ(result.GetSelectedItem(), nullptr);
    }
} // namespace jactorio::game
