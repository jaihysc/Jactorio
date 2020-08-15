// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "data/prototype/mining_drill.h"

#include "jactorioTests.h"

#include "data/prototype/container_entity.h"
#include "data/prototype/resource_entity.h"

namespace jactorio::data
{
	class MiningDrillTest : public testing::Test
	{
	protected:
		game::WorldData worldData_{};
		game::LogicData logicData_{};

		MiningDrill drillProto_{};

		Item resourceItem_{};
		ResourceEntity resource_{};

		void SetUp() override {
			worldData_.EmplaceChunk(0, 0);

			drillProto_.tileWidth    = 3;
			drillProto_.tileHeight   = 3;
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
		drill.tileWidth    = 4;
		drill.tileHeight   = 3;
		drill.miningRadius = 2;

		// Has no resource tiles
		EXPECT_FALSE(drill.OnCanBuild(worldData_, {2, 2}));

		// Has resource tiles
		ResourceEntity resource{};
		worldData_.GetTile(0, 0)
		          ->GetLayer(game::ChunkTile::ChunkLayer::resource).prototypeData = &resource;

		EXPECT_TRUE(drill.OnCanBuild(worldData_, {2, 2}));
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
		drill.tileWidth    = 4;
		drill.tileHeight   = 3;
		drill.miningRadius = 2;

		ResourceEntity resource{};
		worldData_.GetTile(7, 6)
		          ->GetLayer(game::ChunkTile::ChunkLayer::resource).prototypeData = &resource;

		EXPECT_TRUE(drill.OnCanBuild(worldData_, {2, 2}));
	}

	TEST_F(MiningDrillTest, FindOutputItem) {

		EXPECT_EQ(drillProto_.FindOutputItem(worldData_, {2, 2}), nullptr);  // No resources 


		worldData_.GetTile(0, 0)
		          ->GetLayer(game::ChunkTile::ChunkLayer::resource).prototypeData = &resource_;

		EXPECT_EQ(drillProto_.FindOutputItem(worldData_, {2, 2}), nullptr);  // No resources in range


		worldData_.GetTile(6, 5)
		          ->GetLayer(game::ChunkTile::ChunkLayer::resource).prototypeData = &resource_;

		EXPECT_EQ(drillProto_.FindOutputItem(worldData_, {2, 2}), nullptr);  // No resources in range

		// ======================================================================

		worldData_.GetTile(5, 5)
		          ->GetLayer(game::ChunkTile::ChunkLayer::resource).prototypeData = &resource_;
		EXPECT_EQ(drillProto_.FindOutputItem(worldData_, {2, 2}), &resourceItem_);

		// Closer to the top left
		{
			Item item2{};
			ResourceEntity resource2{};
			resource2.SetItem(&item2);

			worldData_.GetTile(1, 1)
			          ->GetLayer(game::ChunkTile::ChunkLayer::resource).prototypeData = &resource2;
			EXPECT_EQ(drillProto_.FindOutputItem(worldData_, {2, 2}), &item2);
		}
	}


	TEST_F(MiningDrillTest, BuildAndExtractResource) {
		// Mining drill is built with an item output chest

		drillProto_.miningSpeed          = 2;  // Halves mining time
		drillProto_.resourceOutput.right = {3, 1};
		ContainerEntity container{};


		TestSetupContainer(worldData_, {4, 2}, container);
		auto& tile = TestSetupDrill(worldData_, logicData_,
		                            {1, 1},
		                            resource_, drillProto_, 100);

		auto* data = tile.GetLayer(game::ChunkTile::ChunkLayer::entity).GetUniqueData<MiningDrillData>();

		EXPECT_EQ(data->resourceCoord.x, 0);
		EXPECT_EQ(data->resourceCoord.y, 0);
		EXPECT_EQ(data->resourceOffset, 6);

		// ======================================================================
		// Resource taken from ground
		auto& resource_layer = tile.GetLayer(game::ChunkTile::ChunkLayer::resource);

		EXPECT_EQ(resource_layer.GetUniqueData<ResourceEntityData>()->resourceAmount, 99);

		// ======================================================================
		// Ensure it inserts into the correct entity

		Item item{};
		data->outputTile.DropOff(logicData_, {&item, 1});

		game::ChunkTileLayer& container_layer =
			worldData_.GetTile(4, 2)->GetLayer(game::ChunkTile::ChunkLayer::entity);

		EXPECT_EQ(container_layer.GetUniqueData<ContainerEntityData>()->inventory[0].count, 1);

		// ======================================================================

		logicData_.deferralTimer.DeferralUpdate(worldData_, 30);  // Takes 60 ticks to mine / 2 (since mining speed is 2)

		EXPECT_EQ(container_layer.GetUniqueData<ContainerEntityData>()->inventory[1].count, 1);

		// Another resource taken for next output
		EXPECT_EQ(resource_layer.GetUniqueData<ResourceEntityData>()->resourceAmount, 98);
	}

	TEST_F(MiningDrillTest, ExtractRemoveResourceEntity) {
		drillProto_.resourceOutput.right = {3, 1};
		const ContainerEntity container{};


		TestSetupContainer(worldData_, {4, 2}, container);

		auto& tile  = TestSetupDrill(worldData_, logicData_,
		                             {1, 1},
		                             resource_, drillProto_, 1);
		auto& tile2 = TestSetupResource(worldData_, {3, 4}, resource_, 1);
		auto& tile3 = TestSetupResource(worldData_, {4, 4}, resource_, 1);


		auto& resource_layer  = tile.GetLayer(game::ChunkTile::ChunkLayer::resource);
		auto& resource_layer2 = tile2.GetLayer(game::ChunkTile::ChunkLayer::resource);
		auto& resource_layer3 = tile3.GetLayer(game::ChunkTile::ChunkLayer::resource);

		// ======================================================================

		resource_layer2.Clear();  // Resource 2 was mined by an external source
		logicData_.deferralTimer.DeferralUpdate(worldData_, 60);
		EXPECT_EQ(resource_layer.prototypeData, nullptr);
		EXPECT_EQ(resource_layer.GetUniqueData(), nullptr);

		// Found another resource (resource3)
		logicData_.deferralTimer.DeferralUpdate(worldData_, 120);
		EXPECT_EQ(resource_layer3.prototypeData, nullptr);
		EXPECT_EQ(resource_layer3.GetUniqueData(), nullptr);

		EXPECT_TRUE(logicData_.deferralTimer.GetDebugInfo().callbacks.empty());
		auto* drill_data = tile.GetLayer(game::ChunkTile::ChunkLayer::entity).GetUniqueData<MiningDrillData>();

		EXPECT_FALSE(drill_data->deferralEntry.Valid());
	}

	TEST_F(MiningDrillTest, ExtractResourceOutputBlocked) {
		// If output is blocked, drill attempts to output at next game tick

		drillProto_.resourceOutput.right = {3, 1};
		ContainerEntity container{};


		TestSetupContainer(worldData_, {4, 2}, container, 1);
		TestSetupDrill(worldData_, logicData_, {1, 1}, resource_, drillProto_);

		// ======================================================================

		auto& container_layer = worldData_.GetTile(4, 2)->GetLayer(game::ChunkTile::ChunkLayer::entity);
		auto* container_data  = container_layer.GetUniqueData<ContainerEntityData>();

		// No output since output inventory is full
		Item item;
		item.stackSize = 50;

		container_data->inventory[0] = {&item, 50};

		logicData_.deferralTimer.DeferralUpdate(worldData_, 60);
		EXPECT_EQ(container_data->inventory[0].count, 50);

		// Output has space
		container_data->inventory[0] = {nullptr, 0};
		logicData_.deferralTimer.DeferralUpdate(worldData_, 61);
		EXPECT_EQ(container_data->inventory[0].count, 1);
	}

	TEST_F(MiningDrillTest, BuildMultiTileOutput) {

		drillProto_.resourceOutput.right = {3, 1};

		AssemblyMachine asm_machine{};
		TestSetupAssemblyMachine(worldData_, {4, 1}, asm_machine);

		auto& tile = TestSetupDrill(worldData_, logicData_, {1, 1}, resource_, drillProto_);
		auto* data = tile.GetLayer(game::ChunkTile::ChunkLayer::entity).GetUniqueData<MiningDrillData>();

		EXPECT_TRUE(data->outputTile.IsInitialized());
	}

	TEST_F(MiningDrillTest, BuildNoOutput) {
		// Mining drill is built without anywhere to output items
		// Should do nothing until an output is built

		drillProto_.resourceOutput.right = {3, 1};
		ContainerEntity container{};


		auto& tile = TestSetupDrill(worldData_, logicData_, {1, 1}, resource_, drillProto_);
		TestSetupContainer(worldData_, {4, 2}, container);

		drillProto_.OnNeighborUpdate(worldData_, logicData_,
		                             {4, 2},
		                             {1, 1}, Orientation::right);

		// ======================================================================
		// Should now insert as it has an entity to output to

		auto* data = tile.GetLayer(game::ChunkTile::ChunkLayer::entity).GetUniqueData<MiningDrillData>();

		// Ensure it inserts into the correct entity
		Item item{};
		data->outputTile.DropOff(logicData_, {&item, 1});

		game::ChunkTileLayer& container_layer = worldData_.GetTile(4, 2)
		                                                  ->GetLayer(
			                                                  game::ChunkTile::ChunkLayer::entity);

		EXPECT_EQ(container_layer.GetUniqueData<ContainerEntityData>()->inventory[0].count,
		          1);
	}

	TEST_F(MiningDrillTest, RemoveDrill) {
		// When the mining drill is removed, it needs to unregister the defer update
		// callback to the unique_data which now no longer exists

		drillProto_.resourceOutput.right = {3, 1};
		ContainerEntity container{};

		TestSetupContainer(worldData_, {4, 2}, container);
		auto& tile = TestSetupDrill(worldData_, logicData_, {1, 1}, resource_, drillProto_);

		drillProto_.OnRemove(worldData_, logicData_,
		                     {1, 1}, tile.GetLayer(game::ChunkTile::ChunkLayer::entity));

		tile.GetLayer(game::ChunkTile::ChunkLayer::entity).Clear();  // Deletes drill data

		// Should no longer be valid
		logicData_.deferralTimer.DeferralUpdate(worldData_, 60);
	}

	TEST_F(MiningDrillTest, RemoveOutputEntity) {
		// When the mining drill's output entity is removed, it needs to unregister the defer update

		drillProto_.resourceOutput.right = {3, 1};
		ContainerEntity container{};

		TestSetupContainer(worldData_, {4, 2}, container);
		TestSetupDrill(worldData_, logicData_, {1, 1}, resource_, drillProto_);

		// Remove chest
		game::ChunkTile* tile = worldData_.GetTile(4, 2);
		tile->GetLayer(game::ChunkTile::ChunkLayer::entity).Clear();  // Remove container 

		// Should only remove the callback once
		drillProto_.OnNeighborUpdate(worldData_, logicData_,
		                             {4, 2},
		                             {1, 1}, Orientation::right);
		drillProto_.OnNeighborUpdate(worldData_, logicData_,
		                             {4, 2},
		                             {1, 1}, Orientation::right);
		drillProto_.OnNeighborUpdate(worldData_, logicData_,
		                             {4, 2},
		                             {1, 1}, Orientation::right);

		// Should no longer be valid
		logicData_.deferralTimer.DeferralUpdate(worldData_, 60);
	}

	TEST_F(MiningDrillTest, UpdateNonOutput) {
		// Mining drill should ignore on_neighbor_update from tiles other than the item output tile

		drillProto_.resourceOutput.up    = {1, -1};
		drillProto_.resourceOutput.right = {3, 1};
		TestSetupDrill(worldData_, logicData_, {1, 1}, resource_, drillProto_);

		// ======================================================================
		ContainerEntity container{};
		TestSetupContainer(worldData_, {2, 0}, container);
		TestSetupContainer(worldData_, {4, 1}, container);

		drillProto_.OnNeighborUpdate(worldData_, logicData_,
		                             {2, 0},
		                             {1, 1}, Orientation::up);
		drillProto_.OnNeighborUpdate(worldData_, logicData_,
		                             {4, 1},
		                             {1, 1}, Orientation::right);

		logicData_.deferralTimer.DeferralUpdate(worldData_, 60);

		// If the on_neighbor_update event was ignored, no items will be added
		{
			game::ChunkTileLayer& container_layer =
				worldData_.GetTile(2, 0)
				          ->GetLayer(game::ChunkTile::ChunkLayer::entity);

			EXPECT_EQ(container_layer.GetUniqueData<ContainerEntityData>()->inventory[0].count,
			          0);
		}
		{
			game::ChunkTileLayer& container_layer =
				worldData_.GetTile(4, 1)
				          ->GetLayer(game::ChunkTile::ChunkLayer::entity);

			EXPECT_EQ(container_layer.GetUniqueData<ContainerEntityData>()->inventory[0].count,
			          0);
		}
	}
}
