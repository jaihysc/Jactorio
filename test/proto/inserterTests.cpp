// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "jactorioTests.h"

#include "proto/container_entity.h"
#include "proto/inserter.h"

namespace jactorio::proto
{
    class InserterTest : public testing::Test
    {
    protected:
        game::World world_;
        game::Logic logicData_;

        Inserter inserterProto_;
        ContainerEntity containerProto_;

        void SetUp() override {
            world_.EmplaceChunk(0, 0);
        }

        game::ChunkTileLayer& BuildInserter(const WorldCoord& coords, const Orientation orientation) {
            return TestSetupInserter(world_, logicData_, coords, orientation, inserterProto_);
        }
    };

    TEST_F(InserterTest, OnBuildCreateDataInvalid) {
        BuildInserter({1, 1}, Orientation::right);

        auto& layer         = world_.GetTile({1, 1})->GetLayer(game::TileLayer::entity);
        auto* inserter_data = layer.GetUniqueData<InserterData>();
        ASSERT_TRUE(inserter_data);

        EXPECT_DOUBLE_EQ(inserter_data->rotationDegree.getAsDouble(), 180.);

        // Does not have both pickup + dropoff, not added
        EXPECT_EQ(world_.LogicGetChunks().size(), 0);
    }

    TEST_F(InserterTest, FindPickupDropoffOnBuild) {
        // Finding pickup and dropoff tiles

        TestSetupContainer(world_, {1, 1}, Orientation::up, containerProto_);
        TestSetupContainer(world_, {3, 1}, Orientation::up, containerProto_);

        auto& layer = BuildInserter({2, 1}, Orientation::left);

        EXPECT_TRUE(layer.GetUniqueData<InserterData>()->pickup.IsInitialized());
        EXPECT_TRUE(layer.GetUniqueData<InserterData>()->dropoff.IsInitialized());
    }

    TEST_F(InserterTest, FindPickupDropoff) {
        // Finding pickup and dropoff tiles

        auto& layer = BuildInserter({2, 1}, Orientation::left);

        EXPECT_FALSE(layer.GetUniqueData<InserterData>()->pickup.IsInitialized());
        EXPECT_FALSE(layer.GetUniqueData<InserterData>()->dropoff.IsInitialized());


        // Dropoff
        TestSetupContainer(world_, {1, 1}, Orientation::up, containerProto_);
        world_.UpdateDispatch({1, 1}, UpdateType::place);

        EXPECT_TRUE(layer.GetUniqueData<InserterData>()->dropoff.IsInitialized());
        EXPECT_EQ(world_.LogicGetChunks().size(), 0);


        // Pickup
        TestSetupContainer(world_, {3, 1}, Orientation::up, containerProto_);
        world_.UpdateDispatch({3, 1}, UpdateType::place);

        EXPECT_TRUE(layer.GetUniqueData<InserterData>()->pickup.IsInitialized());
        EXPECT_EQ(world_.LogicGetChunks().size(), 1); // Added since both are now valid
    }

    TEST_F(InserterTest, FindPickupDropoffFar) {
        // Finding pickup and dropoff tiles when tileReach > 1

        inserterProto_.tileReach = 2;

        auto& layer = BuildInserter({2, 2}, Orientation::up);

        EXPECT_FALSE(layer.GetUniqueData<InserterData>()->pickup.IsInitialized());
        EXPECT_FALSE(layer.GetUniqueData<InserterData>()->dropoff.IsInitialized());


        // Dropoff
        AssemblyMachine asm_machine;
        asm_machine.SetDimensions(2, 2);
        TestSetupAssemblyMachine(world_, {1, 0}, Orientation::up, asm_machine);
        world_.UpdateDispatch({2, 0}, UpdateType::place);

        EXPECT_TRUE(layer.GetUniqueData<InserterData>()->dropoff.IsInitialized());
        EXPECT_EQ(world_.LogicGetChunks().size(), 0);


        // Pickup
        TestSetupContainer(world_, {2, 4}, Orientation::up, containerProto_);
        world_.UpdateDispatch({2, 4}, UpdateType::place);

        EXPECT_TRUE(layer.GetUniqueData<InserterData>()->pickup.IsInitialized());
        EXPECT_EQ(world_.LogicGetChunks().size(), 1); // Added since both are now valid
    }

    TEST_F(InserterTest, RemovePickupDropoff) {
        // Finding pickup and dropoff tiles

        TestSetupContainer(world_, {3, 1}, Orientation::up, containerProto_);

        auto& layer = BuildInserter({2, 1}, Orientation::left);
        EXPECT_TRUE(layer.GetUniqueData<InserterData>()->pickup.IsInitialized());
        EXPECT_FALSE(layer.GetUniqueData<InserterData>()->dropoff.IsInitialized());


        TestSetupContainer(world_, {1, 1}, Orientation::up, containerProto_);
        world_.UpdateDispatch({1, 1}, UpdateType::place);
        EXPECT_EQ(world_.LogicGetChunks().size(), 1);


        // Removed chest

        world_.GetTile({3, 1})->GetLayer(game::TileLayer::entity).Clear();
        world_.UpdateDispatch({3, 1}, UpdateType::place);

        world_.GetTile({1, 1})->GetLayer(game::TileLayer::entity).Clear();
        world_.UpdateDispatch({1, 1}, UpdateType::place);

        EXPECT_FALSE(layer.GetUniqueData<InserterData>()->pickup.IsInitialized());
        EXPECT_FALSE(layer.GetUniqueData<InserterData>()->dropoff.IsInitialized());

        EXPECT_EQ(world_.LogicGetChunks().size(), 0);
    }

    TEST_F(InserterTest, Serialize) {
        auto inserter_data = std::make_unique<InserterData>(Orientation::left);

        inserter_data->rotationDegree = 145.234;
        inserter_data->status         = InserterData::Status::dropoff;
        inserter_data->heldItem       = {nullptr, 32};

        // ======================================================================

        const std::unique_ptr<UniqueDataBase> base_data = std::move(inserter_data);

        const auto result_base  = TestSerializeDeserialize(base_data);
        const auto* result_data = static_cast<InserterData*>(result_base.get());

        EXPECT_EQ(result_data->orientation, Orientation::left);
        EXPECT_DOUBLE_EQ(result_data->rotationDegree.getAsDouble(), 145.234);
        EXPECT_EQ(result_data->status, InserterData::Status::dropoff);
        EXPECT_EQ(result_data->heldItem.count, 32);
    }

    TEST_F(InserterTest, OnDeserialize) {
        inserterProto_.tileReach = 3;

        auto& inserter_layer = BuildInserter({5, 1}, Orientation::right);

        TestSetupContainer(world_, {2, 1}, Orientation::up, containerProto_); // Pickup
        TestSetupContainer(world_, {8, 1}, Orientation::up, containerProto_); // Dropoff

        // Should locate pickup and dropoff

        world_.DeserializePostProcess();

        auto* inserter_data = inserter_layer.GetUniqueData<InserterData>();

        EXPECT_TRUE(inserter_data->pickup.IsInitialized());
        EXPECT_TRUE(inserter_data->dropoff.IsInitialized());
    }
} // namespace jactorio::proto
