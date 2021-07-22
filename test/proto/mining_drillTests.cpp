// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "proto/mining_drill.h"

#include "jactorioTests.h"

namespace jactorio::proto
{
    class MiningDrillTest : public testing::Test
    {
    protected:
        game::World world_;
        game::Logic logic_;

        MiningDrill drill_;

        Item resourceItem_;
        ResourceEntity resource_;
        ContainerEntity container_;

        void SetUp() override {
            world_.EmplaceChunk({0, 0});

            drill_.SetWidth(3);
            drill_.SetHeight(3);
            drill_.miningRadius = 1;

            resource_.pickupTime = 1.f;
            resource_.SetItem(&resourceItem_);
        }
    };

    TEST_F(MiningDrillTest, OnCanBuild) {
        /*
         * [ ] [ ] [ ] [ ] [ ] [ ] [ ] [ ]
         * [ ] [ ] [ ] [ ] [ ] [ ] [ ] [ ]
         * [ ] [ ] [X] [x] [x] [x] [ ] [ ]
         * [ ] [ ] [x] [x] [x] [x] [ ] [ ]
         * [ ] [ ] [x] [x] [x] [x] [ ] [ ]
         * [ ] [ ] [ ] [ ] [ ] [ ] [ ] [ ]
         * [ ] [ ] [ ] [ ] [ ] [ ] [ ] [ ]
         */

        MiningDrill drill;
        drill.SetWidth(4);
        drill.SetHeight(3);
        drill.miningRadius = 2;

        // Has no resource tiles
        EXPECT_FALSE(drill.OnCanBuild(world_, {2, 2}, Orientation::up));

        // Has resource tiles
        ResourceEntity resource;
        world_.GetTile({0, 0}, game::TileLayer::resource)->SetPrototype(Orientation::up, &resource);

        EXPECT_TRUE(drill.OnCanBuild(world_, {2, 2}, Orientation::up));
    }

    TEST_F(MiningDrillTest, OnCanBuild2) {
        /*
         * [ ] [ ] [ ] [ ] [ ] [ ] [ ] [ ]
         * [ ] [ ] [ ] [ ] [ ] [ ] [ ] [ ]
         * [ ] [ ] [X] [x] [x] [x] [ ] [ ]
         * [ ] [ ] [x] [x] [x] [x] [ ] [ ]
         * [ ] [ ] [x] [x] [x] [x] [ ] [ ]
         * [ ] [ ] [ ] [ ] [ ] [ ] [ ] [ ]
         * [ ] [ ] [ ] [ ] [ ] [ ] [ ] [ ]
         */

        MiningDrill drill;
        drill.SetWidth(4);
        drill.SetHeight(3);
        drill.miningRadius = 2;

        ResourceEntity resource;
        world_.GetTile({7, 6}, game::TileLayer::resource)->SetPrototype(Orientation::up, &resource);

        EXPECT_TRUE(drill.OnCanBuild(world_, {2, 2}, Orientation::up));
    }

    TEST_F(MiningDrillTest, FindOutputItem) {

        EXPECT_EQ(drill_.FindOutputItem(world_, {2, 2}, Orientation::up), nullptr); // No resources


        world_.GetTile({0, 0}, game::TileLayer::resource)->SetPrototype(Orientation::up, &resource_);
        EXPECT_EQ(drill_.FindOutputItem(world_, {2, 2}, Orientation::up), nullptr); // No resources in range


        world_.GetTile({6, 5}, game::TileLayer::resource)->SetPrototype(Orientation::up, &resource_);
        EXPECT_EQ(drill_.FindOutputItem(world_, {2, 2}, Orientation::up), nullptr); // No resources in range

        // ======================================================================

        world_.GetTile({5, 5}, game::TileLayer::resource)->SetPrototype(Orientation::up, &resource_);
        EXPECT_EQ(drill_.FindOutputItem(world_, {2, 2}, Orientation::up), &resourceItem_);

        // Closer to the top left
        {
            Item item2;
            ResourceEntity resource2;
            resource2.SetItem(&item2);

            world_.GetTile({1, 1}, game::TileLayer::resource)->SetPrototype(Orientation::up, &resource2);
            EXPECT_EQ(drill_.FindOutputItem(world_, {2, 2}, Orientation::up), &item2);
        }
    }


    TEST_F(MiningDrillTest, BuildAndExtractResource) {
        // Mining drill is built with an item output chest

        drill_.miningSpeed          = 2; // Halves mining time
        drill_.resourceOutput.right = {3, 1};


        auto& container_tile = TestSetupContainer(world_, {4, 2}, Orientation::up, container_);
        auto& resource_tile  = TestSetupResource(world_, {1, 1}, resource_, 100);
        auto& drill_tile     = TestSetupDrill(world_, logic_, {1, 1}, Orientation::right, drill_);

        auto* data = drill_tile.GetUniqueData<MiningDrillData>();

        EXPECT_EQ(data->resourceCoord.x, 0);
        EXPECT_EQ(data->resourceCoord.y, 0);
        EXPECT_EQ(data->resourceOffset, 6);

        // Resource taken from ground
        EXPECT_EQ(resource_tile.GetUniqueData<ResourceEntityData>()->resourceAmount, 99);

        // Ensure it inserts into the correct entity
        Item item;
        data->output.DropOff(logic_, {&item, 1});

        EXPECT_EQ(container_tile.GetUniqueData<ContainerEntityData>()->inventory[0].count, 1);

        // ======================================================================

        logic_.DeferralUpdate(world_, 30); // Takes 60 ticks to mine / 2 (since mining speed is 2)

        EXPECT_EQ(container_tile.GetUniqueData<ContainerEntityData>()->inventory[1].count, 1);

        // Another resource taken for next output
        EXPECT_EQ(resource_tile.GetUniqueData<ResourceEntityData>()->resourceAmount, 98);
    }

    TEST_F(MiningDrillTest, ExtractRemoveResourceEntity) {
        drill_.resourceOutput.right = {3, 1};


        TestSetupContainer(world_, {4, 2}, Orientation::up, container_);

        // The moment the drill is created, it has an output
        // thus, it immediately deducts a resource and registers the callback for outputting
        world_.SetTexCoordId({1, 1}, game::TileLayer::resource, 1234);

        TestSetupResource(world_, {1, 1}, resource_, 1);
        auto& drill_tile = TestSetupDrill(world_, logic_, {1, 1}, Orientation::right, drill_);
        auto& r_tile1    = *world_.GetTile({1, 1}, game::TileLayer::resource);

        auto& r_tile2 = TestSetupResource(world_, {3, 4}, resource_, 1);
        auto& r_tile3 = TestSetupResource(world_, {4, 4}, resource_, 1);


        // ======================================================================

        r_tile2.Clear(); // Resource 2 was mined by an external source
        logic_.DeferralUpdate(world_, 60);
        EXPECT_EQ(r_tile1.GetPrototype(), nullptr);
        EXPECT_EQ(r_tile1.GetUniqueData(), nullptr);
        EXPECT_EQ(world_.GetTexCoordId({1, 1}, game::TileLayer::resource), 0);

        // Found another resource (resource3)
        logic_.DeferralUpdate(world_, 120);
        EXPECT_EQ(r_tile3.GetPrototype(), nullptr);
        EXPECT_EQ(r_tile3.GetUniqueData(), nullptr);

        EXPECT_TRUE(logic_.deferralTimer.GetDebugInfo().callbacks.empty());
        auto* drill_data = drill_tile.GetUniqueData<MiningDrillData>();

        EXPECT_FALSE(drill_data->deferralEntry.Valid());
    }

    TEST_F(MiningDrillTest, ExtractResourceOutputBlocked) {
        // If output is blocked, drill attempts to output at next game tick

        drill_.resourceOutput.right = {3, 1};


        TestSetupContainer(world_, {4, 2}, Orientation::up, container_, 1);
        TestSetupResource(world_, {1, 1}, resource_, 100);
        TestSetupDrill(world_, logic_, {1, 1}, Orientation::right, drill_);

        auto* container_data = world_.GetTile({4, 2}, game::TileLayer::entity)->GetUniqueData<ContainerEntityData>();

        // No output since output inventory is full
        Item item;
        item.stackSize = 50;

        container_data->inventory[0] = {&item, 50};

        logic_.DeferralUpdate(world_, 60);
        EXPECT_EQ(container_data->inventory[0].count, 50);

        // Output has space
        container_data->inventory[0] = {nullptr, 0};
        logic_.DeferralUpdate(world_, 61);
        EXPECT_EQ(container_data->inventory[0].count, 1);
    }

    TEST_F(MiningDrillTest, BuildMultiTileOutput) {

        drill_.resourceOutput.right = {3, 1};

        AssemblyMachine asm_machine;
        asm_machine.SetDimension({2, 2});
        TestSetupAssemblyMachine(world_, {4, 1}, Orientation::up, asm_machine);

        TestSetupResource(world_, {1, 1}, resource_, 100);
        auto& tile = TestSetupDrill(world_, logic_, {1, 1}, Orientation::right, drill_);
        auto* data = tile.GetUniqueData<MiningDrillData>();

        EXPECT_TRUE(data->output.IsInitialized());
    }

    TEST_F(MiningDrillTest, BuildNoOutput) {
        // Mining drill is built without anywhere to output items
        // Should do nothing until an output is built

        drill_.resourceOutput.right = {3, 1};


        TestSetupResource(world_, {1, 1}, resource_, 100);
        auto& drill_tile = TestSetupDrill(world_, logic_, {1, 1}, Orientation::right, drill_);
        TestSetupContainer(world_, {4, 2}, Orientation::up, container_);

        // Detects it has an entity to output to
        drill_.OnNeighborUpdate(world_, logic_, {4, 2}, {1, 1}, Orientation::right);
        auto* data = drill_tile.GetUniqueData<MiningDrillData>();

        // Ensure it inserts into the correct entity
        Item item;
        data->output.DropOff(logic_, {&item, 1});

        auto* container_tile = world_.GetTile({4, 2}, game::TileLayer::entity);
        EXPECT_EQ(container_tile->GetUniqueData<ContainerEntityData>()->inventory[0].count, 1);
    }

    TEST_F(MiningDrillTest, RemoveDrill) {
        // When the mining drill is removed, it needs to unregister the defer update
        // callback to the unique_data which now no longer exists

        drill_.resourceOutput.right = {3, 1};


        TestSetupContainer(world_, {4, 2}, Orientation::up, container_);
        TestSetupResource(world_, {1, 1}, resource_, 100);
        auto& tile = TestSetupDrill(world_, logic_, {1, 1}, Orientation::right, drill_);

        drill_.OnRemove(world_, logic_, {1, 1});

        tile.Clear(); // Deletes drill data

        // Should no longer be valid
        logic_.DeferralUpdate(world_, 60);
    }

    TEST_F(MiningDrillTest, RemoveOutputEntity) {
        // When the mining drill's output entity is removed, it needs to unregister the defer update

        drill_.resourceOutput.right = {3, 1};


        auto& container_tile = TestSetupContainer(world_, {4, 2}, Orientation::up, container_);
        TestSetupResource(world_, {1, 1}, resource_, 100);
        TestSetupDrill(world_, logic_, {1, 1}, Orientation::right, drill_);

        // Remove chest
        container_tile.Clear();

        // Should only remove the callback once
        drill_.OnNeighborUpdate(world_, logic_, {4, 2}, {1, 1}, Orientation::right);
        drill_.OnNeighborUpdate(world_, logic_, {4, 2}, {1, 1}, Orientation::right);
        drill_.OnNeighborUpdate(world_, logic_, {4, 2}, {1, 1}, Orientation::right);

        // Should no longer be valid
        logic_.DeferralUpdate(world_, 60);
    }

    TEST_F(MiningDrillTest, UpdateNonOutput) {
        // Mining drill should ignore on_neighbor_update from tiles other than the item output tile

        drill_.resourceOutput.up    = {1, -1};
        drill_.resourceOutput.right = {3, 1};
        TestSetupResource(world_, {1, 1}, resource_, 100);
        TestSetupDrill(world_, logic_, {1, 1}, Orientation::right, drill_);

        // ======================================================================

        TestSetupContainer(world_, {2, 0}, Orientation::up, container_);
        TestSetupContainer(world_, {4, 1}, Orientation::up, container_);

        drill_.OnNeighborUpdate(world_, logic_, {2, 0}, {1, 1}, Orientation::up);
        drill_.OnNeighborUpdate(world_, logic_, {4, 1}, {1, 1}, Orientation::right);

        logic_.DeferralUpdate(world_, 60);

        // If the on_neighbor_update event was ignored, no items will be added
        {
            auto* container_tile = world_.GetTile({2, 0}, game::TileLayer::entity);

            EXPECT_EQ(container_tile->GetUniqueData<ContainerEntityData>()->inventory[0].count, 0);
        }
        {
            auto* container_tile = world_.GetTile({4, 1}, game::TileLayer::entity);

            EXPECT_EQ(container_tile->GetUniqueData<ContainerEntityData>()->inventory[0].count, 0);
        }
    }

    TEST_F(MiningDrillTest, UpdateNoResource) {
        // Mining drill has an output built, but has no resources
        // Should do nothing

        drill_.resourceOutput.right = {3, 1};

        auto& resource_tile = TestSetupResource(world_, {1, 1}, resource_, 1); // A resource so drill gets built
        TestSetupDrill(world_, logic_, {1, 1}, Orientation::right, drill_);

        resource_tile.Clear();
        TestSetupContainer(world_, {4, 2}, Orientation::up, container_);
        drill_.OnNeighborUpdate(world_, logic_, {4, 2}, {1, 1}, Orientation::right);

        EXPECT_EQ(logic_.deferralTimer.GetDebugInfo().callbacks.size(), 0);
    }

    TEST_F(MiningDrillTest, Serialize) {
        auto derived    = std::make_unique<MiningDrillData>(Orientation::down);
        derived->health = 4392;

        const std::unique_ptr<UniqueDataBase> base = std::move(derived);

        // ======================================================================

        const auto result_base     = TestSerializeDeserialize(base);
        const auto* result_derived = static_cast<const MiningDrillData*>(result_base.get());

        EXPECT_EQ(result_derived->health, 4392);
    }

    TEST_F(MiningDrillTest, OnDeserialize) {
        drill_.resourceOutput.down = {1, 3};

        TestSetupResource(world_, {1, 1}, resource_, 100);
        auto& tile = TestSetupDrill(world_, logic_, {1, 1}, Orientation::down, drill_);

        // Drill's output is current uninitialized

        TestSetupContainer(world_, {2, 4}, Orientation::up, container_);

        world_.DeserializePostProcess();

        // Now initialized

        EXPECT_TRUE(tile.GetUniqueData<MiningDrillData>()->output.IsInitialized());
    }
} // namespace jactorio::proto
