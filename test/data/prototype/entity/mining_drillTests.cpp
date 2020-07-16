// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "data/prototype/entity/mining_drill.h"

#include "jactorioTests.h"

#include "data/prototype/entity/container_entity.h"
#include "data/prototype/entity/resource_entity.h"

namespace jactorio::data
{
	class MiningDrillTest : public testing::Test
	{
	protected:
		game::WorldData worldData_{};
		game::LogicData logicData_{};

		MiningDrill drillProto_{};

		Item resourceItem_{};\
		ResourceEntity resource_{};\

		void SetUp() override {
			worldData_.EmplaceChunk(0, 0);

			drillProto_.tileWidth    = 3;
			drillProto_.tileHeight   = 3;
			drillProto_.miningRadius = 1;

			resource_.pickupTime = 1.f;
			resource_.SetItem(&resourceItem_);
		}


		///
		/// \brief Creates a chest in the world
		static void SetupChest(game::WorldData& world_data,
		                       ContainerEntity& container,
		                       const int world_x = 4, const int world_y = 2) {
			game::ChunkTileLayer& container_layer =
				world_data.GetTile(world_x, world_y)
				          ->GetLayer(game::ChunkTile::ChunkLayer::entity);

			container_layer.prototypeData = &container;
			container_layer.MakeUniqueData<ContainerEntityData>(20);
		}

		///
		/// \brief Creates a drill in the world, calling on_build
		static void SetupDrill(game::WorldData& world_data,
		                       game::LogicData& logic_data,
		                       ResourceEntity& resource,
		                       MiningDrill& drill) {

			game::ChunkTile* tile                                               = world_data.GetTile(1, 1);
			tile->GetLayer(game::ChunkTile::ChunkLayer::resource).prototypeData = &resource;

			drill.OnBuild(world_data, logic_data,
			              {1, 1}, tile->GetLayer(game::ChunkTile::ChunkLayer::entity), Orientation::right);
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

		drillProto_.resourceOutput.right = {3, 1};
		ContainerEntity container{};


		SetupChest(worldData_, container);

		SetupDrill(worldData_, logicData_, resource_, drillProto_);


		// ======================================================================
		// Unique data created by on_build()
		game::ChunkTile* tile = worldData_.GetTile(1, 1);

		game::ChunkTileLayer& layer = tile->GetLayer(game::ChunkTile::ChunkLayer::entity);
		auto* data                  = layer.GetUniqueData<MiningDrillData>();

		// Ensure it inserts into the correct entity
		Item item{};
		data->outputTile.DropOff(logicData_, {&item, 1});

		game::ChunkTileLayer& container_layer = worldData_.GetTile(4, 2)
		                                                  ->GetLayer(
			                                                  game::ChunkTile::ChunkLayer::entity);

		EXPECT_EQ(container_layer.GetUniqueData<jactorio::data::ContainerEntityData>()->inventory[0].count,
		          1);

		// ======================================================================
		logicData_.deferralTimer.DeferralUpdate(worldData_, 60);  // Takes 60 ticks to mine

		EXPECT_EQ(container_layer.GetUniqueData<jactorio::data::ContainerEntityData>()->inventory[1].count,
		          1);
	}

	TEST_F(MiningDrillTest, BuildMultiTileOutput) {

		drillProto_.resourceOutput.right = {3, 1};

		AssemblyMachine asm_machine{};
		TestSetupAssemblyMachine(worldData_, {4, 1}, asm_machine);

		SetupDrill(worldData_, logicData_, resource_, drillProto_);

		auto* data = worldData_.GetTile(1, 1)
		                       ->GetLayer(game::ChunkTile::ChunkLayer::entity).GetUniqueData<MiningDrillData>();

		EXPECT_TRUE(data->outputTile.IsInitialized());
	}

	TEST_F(MiningDrillTest, BuildNoOutput) {
		// Mining drill is built without anywhere to output items
		// Should do nothing until an output is built

		drillProto_.resourceOutput.right = {3, 1};
		ContainerEntity container{};


		SetupDrill(worldData_, logicData_, resource_, drillProto_);

		SetupChest(worldData_, container);

		drillProto_.OnNeighborUpdate(worldData_, logicData_,
		                             {4, 2},
		                             {1, 1}, Orientation::right);

		// ======================================================================
		// Should now insert as it has an entity to output to
		game::ChunkTile* tile = worldData_.GetTile(1, 1);

		auto* data =
			static_cast<MiningDrillData*>(
				tile->GetLayer(game::ChunkTile::ChunkLayer::entity).GetUniqueData());

		// Ensure it inserts into the correct entity
		Item item{};
		data->outputTile.DropOff(logicData_, {&item, 1});

		game::ChunkTileLayer& container_layer = worldData_.GetTile(4, 2)
		                                                  ->GetLayer(
			                                                  game::ChunkTile::ChunkLayer::entity);

		EXPECT_EQ(container_layer.GetUniqueData<jactorio::data::ContainerEntityData>()->inventory[0].count,
		          1);
	}

	TEST_F(MiningDrillTest, RemoveDrill) {
		// When the mining drill is removed, it needs to unregister the defer update
		// callback to the unique_data which now no longer exists

		drillProto_.resourceOutput.right = {3, 1};
		ContainerEntity container{};

		SetupChest(worldData_, container);
		SetupDrill(worldData_, logicData_, resource_, drillProto_);

		// Remove
		game::ChunkTile* tile = worldData_.GetTile(1, 1);
		drillProto_.OnRemove(worldData_, logicData_,
		                     {1, 1}, tile->GetLayer(game::ChunkTile::ChunkLayer::entity));

		tile->GetLayer(game::ChunkTile::ChunkLayer::entity).Clear();  // Deletes drill data

		// Should no longer be valid
		logicData_.deferralTimer.DeferralUpdate(worldData_, 60);
	}

	TEST_F(MiningDrillTest, RemoveOutputEntity) {
		// When the mining drill's output entity is removed, it needs to unregister the defer update

		drillProto_.resourceOutput.right = {3, 1};
		ContainerEntity container{};

		SetupChest(worldData_, container);
		SetupDrill(worldData_, logicData_, resource_, drillProto_);

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
		SetupDrill(worldData_, logicData_, resource_, drillProto_);

		// ======================================================================
		ContainerEntity container{};
		SetupChest(worldData_, container, 2, 0);
		SetupChest(worldData_, container, 4, 1);

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

			EXPECT_EQ(container_layer.GetUniqueData<jactorio::data::ContainerEntityData>()->inventory[0].count,
			          0);
		}
		{
			game::ChunkTileLayer& container_layer =
				worldData_.GetTile(4, 1)
				          ->GetLayer(game::ChunkTile::ChunkLayer::entity);

			EXPECT_EQ(container_layer.GetUniqueData<jactorio::data::ContainerEntityData>()->inventory[0].count,
			          0);
		}
	}
}
