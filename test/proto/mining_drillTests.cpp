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

        MiningDrill drillProto_;

        Item resourceItem_;
        ResourceEntity resource_;
        ContainerEntity container_;

        void SetUp() override {
            world_.EmplaceChunk({0, 0});

            drillProto_.SetWidth(3);
            drillProto_.SetHeight(3);
            drillProto_.miningRadius = 1;

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

        MiningDrill drill{};
        drill.SetWidth(4);
        drill.SetHeight(3);
        drill.miningRadius = 2;

        // Has no resource tiles
        EXPECT_FALSE(drill.OnCanBuild(world_, {2, 2}, Orientation::up));

        // Has resource tiles
        ResourceEntity resource{};
        world_.GetTile({0, 0})->GetLayer(game::TileLayer::resource).SetPrototype(Orientation::up, &resource);

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

        MiningDrill drill{};
        drill.SetWidth(4);
        drill.SetHeight(3);
        drill.miningRadius = 2;

        ResourceEntity resource{};
        world_.GetTile({7, 6})->GetLayer(game::TileLayer::resource).SetPrototype(Orientation::up, &resource);

        EXPECT_TRUE(drill.OnCanBuild(world_, {2, 2}, Orientation::up));
    }

    TEST_F(MiningDrillTest, FindOutputItem) {

        EXPECT_EQ(drillProto_.FindOutputItem(world_, {2, 2}, Orientation::up), nullptr); // No resources


        world_.GetTile({0, 0})->GetLayer(game::TileLayer::resource).SetPrototype(Orientation::up, &resource_);
        EXPECT_EQ(drillProto_.FindOutputItem(world_, {2, 2}, Orientation::up), nullptr); // No resources in range


        world_.GetTile({6, 5})->GetLayer(game::TileLayer::resource).SetPrototype(Orientation::up, &resource_);
        EXPECT_EQ(drillProto_.FindOutputItem(world_, {2, 2}, Orientation::up), nullptr); // No resources in range

        // ======================================================================

        world_.GetTile({5, 5})->GetLayer(game::TileLayer::resource).SetPrototype(Orientation::up, &resource_);
        EXPECT_EQ(drillProto_.FindOutputItem(world_, {2, 2}, Orientation::up), &resourceItem_);

        // Closer to the top left
        {
            Item item2;
            ResourceEntity resource2;
            resource2.SetItem(&item2);

            world_.GetTile({1, 1})->GetLayer(game::TileLayer::resource).SetPrototype(Orientation::up, &resource2);
            EXPECT_EQ(drillProto_.FindOutputItem(world_, {2, 2}, Orientation::up), &item2);
        }
    }


    TEST_F(MiningDrillTest, BuildAndExtractResource) {
        // Mining drill is built with an item output chest

        drillProto_.miningSpeed          = 2; // Halves mining time
        drillProto_.resourceOutput.right = {3, 1};


        TestSetupContainer(world_, {4, 2}, Orientation::up, container_);
        auto& tile = TestSetupDrill(world_, logic_, {1, 1}, Orientation::right, resource_, drillProto_, 100);

        auto* data = tile.GetLayer(game::TileLayer::entity).GetUniqueData<MiningDrillData>();

        EXPECT_EQ(data->resourceCoord.x, 0);
        EXPECT_EQ(data->resourceCoord.y, 0);
        EXPECT_EQ(data->resourceOffset, 6);

        // ======================================================================
        // Resource taken from ground
        auto& resource_layer = tile.GetLayer(game::TileLayer::resource);

        EXPECT_EQ(resource_layer.GetUniqueData<ResourceEntityData>()->resourceAmount, 99);

        // ======================================================================
        // Ensure it inserts into the correct entity

        Item item{};
        data->output.DropOff(logic_, {&item, 1});

        game::ChunkTileLayer& container_layer = world_.GetTile({4, 2})->GetLayer(game::TileLayer::entity);

        EXPECT_EQ(container_layer.GetUniqueData<ContainerEntityData>()->inventory[0].count, 1);

        // ======================================================================

        logic_.DeferralUpdate(world_, 30); // Takes 60 ticks to mine / 2 (since mining speed is 2)

        EXPECT_EQ(container_layer.GetUniqueData<ContainerEntityData>()->inventory[1].count, 1);

        // Another resource taken for next output
        EXPECT_EQ(resource_layer.GetUniqueData<ResourceEntityData>()->resourceAmount, 98);
    }

    TEST_F(MiningDrillTest, ExtractRemoveResourceEntity) {
        drillProto_.resourceOutput.right = {3, 1};


        TestSetupContainer(world_, {4, 2}, Orientation::up, container_);

        auto& tile  = TestSetupDrill(world_, logic_, {1, 1}, Orientation::right, resource_, drillProto_, 1);
        auto& tile2 = TestSetupResource(world_, {3, 4}, resource_, 1);
        auto& tile3 = TestSetupResource(world_, {4, 4}, resource_, 1);


        auto& resource_layer  = tile.GetLayer(game::TileLayer::resource);
        auto& resource_layer2 = tile2.GetLayer(game::TileLayer::resource);
        auto& resource_layer3 = tile3.GetLayer(game::TileLayer::resource);

        // ======================================================================

        resource_layer2.Clear(); // Resource 2 was mined by an external source
        logic_.DeferralUpdate(world_, 60);
        EXPECT_EQ(resource_layer.GetPrototype(), nullptr);
        EXPECT_EQ(resource_layer.GetUniqueData(), nullptr);

        // Found another resource (resource3)
        logic_.DeferralUpdate(world_, 120);
        EXPECT_EQ(resource_layer3.GetPrototype(), nullptr);
        EXPECT_EQ(resource_layer3.GetUniqueData(), nullptr);

        EXPECT_TRUE(logic_.deferralTimer.GetDebugInfo().callbacks.empty());
        auto* drill_data = tile.GetLayer(game::TileLayer::entity).GetUniqueData<MiningDrillData>();

        EXPECT_FALSE(drill_data->deferralEntry.Valid());
    }

    TEST_F(MiningDrillTest, ExtractResourceOutputBlocked) {
        // If output is blocked, drill attempts to output at next game tick

        drillProto_.resourceOutput.right = {3, 1};


        TestSetupContainer(world_, {4, 2}, Orientation::up, container_, 1);
        TestSetupDrill(world_, logic_, {1, 1}, Orientation::right, resource_, drillProto_);

        // ======================================================================

        auto& container_layer = world_.GetTile({4, 2})->GetLayer(game::TileLayer::entity);
        auto* container_data  = container_layer.GetUniqueData<ContainerEntityData>();

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

        drillProto_.resourceOutput.right = {3, 1};

        AssemblyMachine asm_machine;
        asm_machine.SetDimensions(2, 2);
        TestSetupAssemblyMachine(world_, {4, 1}, Orientation::up, asm_machine);

        auto& tile = TestSetupDrill(world_, logic_, {1, 1}, Orientation::right, resource_, drillProto_);
        auto* data = tile.GetLayer(game::TileLayer::entity).GetUniqueData<MiningDrillData>();

        EXPECT_TRUE(data->output.IsInitialized());
    }

    TEST_F(MiningDrillTest, BuildNoOutput) {
        // Mining drill is built without anywhere to output items
        // Should do nothing until an output is built

        drillProto_.resourceOutput.right = {3, 1};


        auto& tile = TestSetupDrill(world_, logic_, {1, 1}, Orientation::right, resource_, drillProto_);
        TestSetupContainer(world_, {4, 2}, Orientation::up, container_);

        drillProto_.OnNeighborUpdate(world_, logic_, {4, 2}, {1, 1}, Orientation::right);

        // ======================================================================
        // Should now insert as it has an entity to output to

        auto* data = tile.GetLayer(game::TileLayer::entity).GetUniqueData<MiningDrillData>();

        // Ensure it inserts into the correct entity
        Item item{};
        data->output.DropOff(logic_, {&item, 1});

        game::ChunkTileLayer& container_layer = world_.GetTile({4, 2})->GetLayer(game::TileLayer::entity);

        EXPECT_EQ(container_layer.GetUniqueData<ContainerEntityData>()->inventory[0].count, 1);
    }

    TEST_F(MiningDrillTest, RemoveDrill) {
        // When the mining drill is removed, it needs to unregister the defer update
        // callback to the unique_data which now no longer exists

        drillProto_.resourceOutput.right = {3, 1};


        TestSetupContainer(world_, {4, 2}, Orientation::up, container_);
        auto& tile = TestSetupDrill(world_, logic_, {1, 1}, Orientation::right, resource_, drillProto_);

        drillProto_.OnRemove(world_, logic_, {1, 1}, game::TileLayer::entity);

        tile.GetLayer(game::TileLayer::entity).Clear(); // Deletes drill data

        // Should no longer be valid
        logic_.DeferralUpdate(world_, 60);
    }

    TEST_F(MiningDrillTest, RemoveOutputEntity) {
        // When the mining drill's output entity is removed, it needs to unregister the defer update

        drillProto_.resourceOutput.right = {3, 1};


        TestSetupContainer(world_, {4, 2}, Orientation::up, container_);
        TestSetupDrill(world_, logic_, {1, 1}, Orientation::right, resource_, drillProto_);

        // Remove chest
        game::ChunkTile* tile = world_.GetTile({4, 2});
        tile->GetLayer(game::TileLayer::entity).Clear(); // Remove container

        // Should only remove the callback once
        drillProto_.OnNeighborUpdate(world_, logic_, {4, 2}, {1, 1}, Orientation::right);
        drillProto_.OnNeighborUpdate(world_, logic_, {4, 2}, {1, 1}, Orientation::right);
        drillProto_.OnNeighborUpdate(world_, logic_, {4, 2}, {1, 1}, Orientation::right);

        // Should no longer be valid
        logic_.DeferralUpdate(world_, 60);
    }

    TEST_F(MiningDrillTest, UpdateNonOutput) {
        // Mining drill should ignore on_neighbor_update from tiles other than the item output tile

        drillProto_.resourceOutput.up    = {1, -1};
        drillProto_.resourceOutput.right = {3, 1};
        TestSetupDrill(world_, logic_, {1, 1}, Orientation::right, resource_, drillProto_);

        // ======================================================================

        TestSetupContainer(world_, {2, 0}, Orientation::up, container_);
        TestSetupContainer(world_, {4, 1}, Orientation::up, container_);

        drillProto_.OnNeighborUpdate(world_, logic_, {2, 0}, {1, 1}, Orientation::up);
        drillProto_.OnNeighborUpdate(world_, logic_, {4, 1}, {1, 1}, Orientation::right);

        logic_.DeferralUpdate(world_, 60);

        // If the on_neighbor_update event was ignored, no items will be added
        {
            game::ChunkTileLayer& container_layer = world_.GetTile({2, 0})->GetLayer(game::TileLayer::entity);

            EXPECT_EQ(container_layer.GetUniqueData<ContainerEntityData>()->inventory[0].count, 0);
        }
        {
            game::ChunkTileLayer& container_layer = world_.GetTile({4, 1})->GetLayer(game::TileLayer::entity);

            EXPECT_EQ(container_layer.GetUniqueData<ContainerEntityData>()->inventory[0].count, 0);
        }
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
        drillProto_.resourceOutput.down = {1, 3};

        auto& tile = TestSetupDrill(world_, logic_, {1, 1}, Orientation::down, resource_, drillProto_);

        // Drill's output is current uninitialized

        TestSetupContainer(world_, {2, 4}, Orientation::up, container_);

        world_.DeserializePostProcess();

        // Now initialized

        EXPECT_TRUE(tile.GetLayer(game::TileLayer::entity).GetUniqueData<MiningDrillData>()->output.IsInitialized());
    }
} // namespace jactorio::proto
