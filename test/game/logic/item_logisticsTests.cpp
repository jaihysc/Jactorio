// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "jactorioTests.h"

#include "data/prototype/entity/container_entity.h"
#include "data/prototype/entity/inserter.h"
#include "data/prototype/entity/mining_drill.h"
#include "data/prototype/entity/transport_belt.h"
#include "game/logic/inserter_controller.h"
#include "game/logic/item_logistics.h"

namespace jactorio::game
{
	class ItemLogisticsTest : public testing::Test
	{
	protected:
		WorldData worldData_{};
		LogicData logicData_{};

		void SetUp() override {
			worldData_.EmplaceChunk(0, 0);
		}
	};

	TEST_F(ItemLogisticsTest, Uninitialize) {
		data::ContainerEntity container_entity{};
		auto& container_layer = TestSetupContainer(worldData_, {2, 4}, container_entity);

		ItemDropOff drop_off{data::Orientation::up};
		ASSERT_TRUE(drop_off.Initialize(worldData_, *container_layer.GetUniqueData(), 2, 4));
		drop_off.Uninitialize();

		EXPECT_FALSE(drop_off.IsInitialized());
	}

	TEST_F(ItemLogisticsTest, DropOffDropItem) {
		data::ContainerEntity container_entity{};
		auto& container_layer = TestSetupContainer(worldData_, {2, 4}, container_entity);

		ItemDropOff drop_off{data::Orientation::up};
		ASSERT_TRUE(drop_off.Initialize(worldData_, *container_layer.GetUniqueData(), 2, 4));

		data::Item item{};
		drop_off.DropOff(logicData_, {&item, 10});

		EXPECT_EQ(
			container_layer.GetUniqueData<data::ContainerEntityData>()->inventory[0].count,
			10);
	}

	TEST_F(ItemLogisticsTest, InserterPickupItem) {
		data::ContainerEntity container_entity{};
		auto& container_layer = TestSetupContainer(worldData_, {2, 4}, container_entity);

		InserterPickup pickup{data::Orientation::up};
		ASSERT_TRUE(pickup.Initialize(worldData_, *container_layer.GetUniqueData(), 2, 4));

		data::Item item{};
		auto& inv = container_layer.GetUniqueData<data::ContainerEntityData>()->inventory;
		inv[0]    = {&item, 10};

		pickup.Pickup(logicData_, 1, data::ToRotationDegree(180.f), 2);

		EXPECT_EQ(inv[0].count, 8);
	}

	// ======================================================================

	///
	/// Inherits ItemDropOff to gain access to insertion methods
	class ItemDropOffTest : public testing::Test, public ItemDropOff
	{
	public:
		explicit ItemDropOffTest()
			: ItemDropOff(data::Orientation::up) {
		}

	protected:
		WorldData worldData_{};
		LogicData logicData_{};

		TransportSegment* segment_ = nullptr;

		// Creates a transport line with orientation at (4, 4)
		data::TransportLineData CreateTransportLine(const data::Orientation orientation) {
			worldData_.AddChunk(Chunk(0, 0));

			const auto segment = std::make_shared<TransportSegment>(
				orientation,
				TransportSegment::TerminationType::straight,
				2
			);
			segment_ = segment.get();

			return data::TransportLineData{segment};
		}

		void TransportLineInsert(const data::Orientation orientation,
		                         data::TransportLineData& line_data) {
			data::Item item{};
			InsertTransportBelt(logicData_, {&item, 1},
			                    line_data,
			                    orientation);
		}
	};

	TEST_F(ItemDropOffTest, GetInsertFunc) {
		auto set_prototype = [&](data::Entity& entity_proto) {
			worldData_.GetTile(2, 4)
			          ->SetEntityPrototype(&entity_proto);
		};

		worldData_.EmplaceChunk(0, 0);

		// For passing into ItemDropOff::Initialize
		data::HealthEntityData mock_unique_data{};


		// No: Empty tile cannot be inserted into
		EXPECT_FALSE(this->Initialize(worldData_, mock_unique_data, {2, 4}));
		EXPECT_FALSE(this->IsInitialized());


		// Ok: Transport belt can be inserted onto
		data::TransportBelt belt{};
		set_prototype(belt);

		EXPECT_TRUE(this->Initialize(worldData_, mock_unique_data, {2, 4}));
		EXPECT_TRUE(this->IsInitialized());


		// No: Mining drill 
		data::MiningDrill drill{};
		set_prototype(drill);

		EXPECT_FALSE(this->Initialize(worldData_, mock_unique_data, {2, 4}));
		EXPECT_TRUE(this->IsInitialized());  // Still initialized from transport belt


		// Ok: Container
		data::ContainerEntity container{};
		set_prototype(container);

		EXPECT_TRUE(this->Initialize(worldData_, mock_unique_data, {2, 4}));
		EXPECT_TRUE(this->IsInitialized());


		// Ok: Assembly machine
		data::AssemblyMachine assembly_machine{};
		TestSetupAssemblyMachine(worldData_, {2, 4}, assembly_machine);

		EXPECT_TRUE(this->Initialize(worldData_, mock_unique_data, {3, 5}));
		EXPECT_TRUE(this->IsInitialized());
		EXPECT_EQ(this->targetProtoData_, &assembly_machine);
	}

	// ======================================================================

	TEST_F(ItemDropOffTest, InsertContainerEntity) {
		WorldData world_data{};
		world_data.AddChunk(Chunk(0, 0));

		auto& layer = world_data.GetTile(3, 1)
		                        ->GetLayer(ChunkTile::ChunkLayer::entity);

		layer.MakeUniqueData<data::ContainerEntityData>(10);

		auto* container_data = layer.GetUniqueData<data::ContainerEntityData>();


		data::Item item{};
		// Orientation is orientation from origin object
		InsertContainerEntity(logicData_, {&item, 2},
		                      *container_data,
		                      data::Orientation::down);

		// Inserted item
		EXPECT_EQ(container_data->inventory[0].item, &item);
		EXPECT_EQ(container_data->inventory[0].count, 2);
	}

	TEST_F(ItemDropOffTest, InsertOffset) {
		auto line_proto             = CreateTransportLine(data::Orientation::up);
		line_proto.lineSegmentIndex = 1;
		segment_->itemOffset        = 10;  // Arbitrary itemOffset

		TransportLineInsert(data::Orientation::up, line_proto);
		ASSERT_EQ(segment_->right.lane.size(), 1);
		EXPECT_FLOAT_EQ(segment_->right.lane[0].first.getAsDouble(), 1.5f);
	}

	TEST_F(ItemDropOffTest, InsertTransportLineUp) {
		{
			auto line = CreateTransportLine(data::Orientation::up);

			TransportLineInsert(data::Orientation::up, line);
			EXPECT_EQ(segment_->right.lane.size(), 1);
		}
		{
			auto line = CreateTransportLine(data::Orientation::up);

			TransportLineInsert(data::Orientation::right, line);
			EXPECT_EQ(segment_->left.lane.size(), 1);
		}
		{
			auto line = CreateTransportLine(data::Orientation::up);

			TransportLineInsert(data::Orientation::down, line);
			EXPECT_EQ(segment_->left.lane.size(), 0);
			EXPECT_EQ(segment_->right.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(data::Orientation::up);

			TransportLineInsert(data::Orientation::left, line);
			EXPECT_EQ(segment_->right.lane.size(), 1);
		}
	}

	TEST_F(ItemDropOffTest, InsertTransportLineRight) {
		{
			auto line = CreateTransportLine(data::Orientation::right);

			TransportLineInsert(data::Orientation::up, line);
			EXPECT_EQ(segment_->right.lane.size(), 1);
		}
		{
			auto line = CreateTransportLine(data::Orientation::right);

			TransportLineInsert(data::Orientation::right, line);
			EXPECT_EQ(segment_->right.lane.size(), 1);
		}
		{
			auto line = CreateTransportLine(data::Orientation::right);

			TransportLineInsert(data::Orientation::down, line);
			EXPECT_EQ(segment_->left.lane.size(), 1);
		}
		{
			auto line = CreateTransportLine(data::Orientation::right);

			TransportLineInsert(data::Orientation::left, line);
			EXPECT_EQ(segment_->left.lane.size(), 0);
			EXPECT_EQ(segment_->right.lane.size(), 0);
		}
	}

	TEST_F(ItemDropOffTest, InsertTransportLineDown) {
		{
			auto line = CreateTransportLine(data::Orientation::down);

			TransportLineInsert(data::Orientation::up, line);
			EXPECT_EQ(segment_->left.lane.size(), 0);
			EXPECT_EQ(segment_->right.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(data::Orientation::down);

			TransportLineInsert(data::Orientation::right, line);
			EXPECT_EQ(segment_->right.lane.size(), 1);
		}
		{
			auto line = CreateTransportLine(data::Orientation::down);

			TransportLineInsert(data::Orientation::down, line);
			EXPECT_EQ(segment_->right.lane.size(), 1);
		}
		{
			auto line = CreateTransportLine(data::Orientation::down);

			TransportLineInsert(data::Orientation::left, line);
			EXPECT_EQ(segment_->left.lane.size(), 1);
		}
	}

	TEST_F(ItemDropOffTest, InsertTransportLineLeft) {
		{
			auto line = CreateTransportLine(data::Orientation::left);

			TransportLineInsert(data::Orientation::up, line);
			EXPECT_EQ(segment_->left.lane.size(), 1);
		}
		{
			auto line = CreateTransportLine(data::Orientation::left);

			TransportLineInsert(data::Orientation::right, line);
			EXPECT_EQ(segment_->left.lane.size(), 0);
			EXPECT_EQ(segment_->right.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(data::Orientation::left);

			TransportLineInsert(data::Orientation::down, line);
			EXPECT_EQ(segment_->right.lane.size(), 1);
		}
		{
			auto line = CreateTransportLine(data::Orientation::left);

			TransportLineInsert(data::Orientation::left, line);
			EXPECT_EQ(segment_->right.lane.size(), 1);
		}
	}

	TEST_F(ItemDropOffTest, InsertAssemblyMachine) {
		data::PrototypeManager prototype_manager{};

		auto recipe_pack = TestSetupRecipe(prototype_manager);

		data::AssemblyMachineData asm_data{};
		asm_data.ChangeRecipe(logicData_, prototype_manager, &recipe_pack.recipe);

		asm_data.ingredientInv[0] = {recipe_pack.item1, 5, recipe_pack.item1};
		asm_data.ingredientInv[1] = {nullptr, 0, recipe_pack.item2};


		// Needs prototype data to register crafting callback
		data::AssemblyMachine assembly_machine{};
		targetProtoData_ = &assembly_machine;

		// Orientation doesn't matter
		EXPECT_TRUE(InsertAssemblyMachine(logicData_, {recipe_pack.item2, 10}, asm_data, data::Orientation::up));

		EXPECT_EQ(asm_data.ingredientInv[1].item, recipe_pack.item2);
		EXPECT_EQ(asm_data.ingredientInv[1].count, 9);  // 1 used to begin crafting

		// Registered to start crafting
		EXPECT_NE(asm_data.deferralEntry.second, 0);
	}

	TEST_F(ItemDropOffTest, InsertAssemblyMachineExceedStack) {
		// Inserters will not insert into assembly machines if it will exceed the current item's stack limit

		data::PrototypeManager prototype_manager{};
		auto recipe_pack = TestSetupRecipe(prototype_manager);

		data::AssemblyMachineData asm_data{};
		asm_data.ingredientInv.resize(2);
		asm_data.ingredientInv[0] = {recipe_pack.item1, 49, recipe_pack.item1};
		asm_data.ingredientInv[1] = {nullptr, 0, recipe_pack.item2};

		// 49 + 2 > 50
		EXPECT_FALSE(InsertAssemblyMachine(logicData_, {recipe_pack.item1, 2}, asm_data, data::Orientation::up));
	}

	// ======================================================================

	class InserterPickupTest : public testing::Test, public InserterPickup
	{
	public:
		explicit InserterPickupTest()
			: InserterPickup(data::Orientation::up) {
		}

		void SetUp() override {
			worldData_.EmplaceChunk(0, 0);
		}

	protected:
		WorldData worldData_{};
		LogicData logicData_{};

		data::Inserter inserterProto_{};
		TransportSegment* segment_ = nullptr;


		// Creates a transport line 1 item on each side
		data::TransportLineData CreateTransportLine(const data::Orientation orientation) {
			data::Item item{};  // Item is not used for tests

			const auto segment = std::make_shared<TransportSegment>(
				orientation,
				TransportSegment::TerminationType::straight,
				2
			);

			segment_ = segment.get();
			segment_->InsertItem(false, 0.5, &item);
			segment_->InsertItem(true, 0.5, &item);

			return data::TransportLineData{segment};
		}


		void PickupLine(const data::Orientation orientation,
		                data::TransportLineData& line_data) {
			constexpr int pickup_amount = 1;

			const auto result = PickupTransportBelt(logicData_,
			                                        1, data::ToRotationDegree(kMaxInserterDegree),
			                                        pickup_amount,
			                                        line_data,
			                                        orientation);

			EXPECT_TRUE(result.first);

			EXPECT_NE(result.second.item, nullptr);
			EXPECT_EQ(result.second.count, pickup_amount);
		}
	};

	TEST_F(InserterPickupTest, GetPickupFunc) {
		auto set_prototype = [&](data::Entity& entity_proto) {
			worldData_.GetTile(2, 4)
			          ->SetEntityPrototype(&entity_proto);
		};

		worldData_.EmplaceChunk(0, 0);

		// For passing into ItemPickup::Initialize
		data::HealthEntityData mock_unique_data{};


		// No: Empty tile cannot be picked up from
		EXPECT_FALSE(this->Initialize(worldData_, mock_unique_data, {2, 4}));
		EXPECT_FALSE(this->IsInitialized());


		// Ok: Transport belt can be picked up from
		data::TransportBelt belt{};
		set_prototype(belt);

		EXPECT_TRUE(this->Initialize(worldData_, mock_unique_data, {2, 4}));
		EXPECT_TRUE(this->IsInitialized());


		// No: Mining drill 
		data::MiningDrill drill{};
		set_prototype(drill);

		EXPECT_FALSE(this->Initialize(worldData_, mock_unique_data, {2, 4}));
		EXPECT_TRUE(this->IsInitialized());  // Still initialized from transport belt


		// Ok: Container
		data::ContainerEntity container{};
		set_prototype(container);

		EXPECT_TRUE(this->Initialize(worldData_, mock_unique_data, {2, 4}));
		EXPECT_TRUE(this->IsInitialized());


		// Ok: Assembly machine
		data::AssemblyMachine assembly_machine{};
		TestSetupAssemblyMachine(worldData_, {2, 4}, assembly_machine);

		EXPECT_TRUE(this->Initialize(worldData_, mock_unique_data, {3, 5}));
		EXPECT_TRUE(this->IsInitialized());
		EXPECT_EQ(this->targetProtoData_, &assembly_machine);
	}

	TEST_F(InserterPickupTest, PickupContainerEntity) {
		data::ContainerEntity container_entity{};
		auto& container_layer = TestSetupContainer(worldData_, {2, 4}, container_entity);
		auto& container_data  = *container_layer.GetUniqueData();

		data::Item item{};
		auto& inv = container_layer.GetUniqueData<data::ContainerEntityData>()->inventory;
		inv[0]    = {&item, 10};

		PickupContainerEntity(logicData_,
		                      1, data::ToRotationDegree(179),
		                      1, container_data,
		                      data::Orientation::up);

		EXPECT_EQ(inv[0].count, 10);  // No items picked up, not 180 degrees

		auto result = PickupContainerEntity(logicData_,
		                                    1, data::ToRotationDegree(180),
		                                    2, container_data,
		                                    data::Orientation::up);
		EXPECT_EQ(inv[0].count, 8);  // 2 items picked up

		EXPECT_NE(result.second.item, nullptr);
		EXPECT_EQ(result.second.count, 2);
	}

	TEST_F(InserterPickupTest, PickupTransportLineUp) {
		// Line is above inserter
		{
			auto line = CreateTransportLine(data::Orientation::up);

			segment_->right.index = 10;

			// Should set index to 0 since a item was removed
			PickupLine(data::Orientation::up, line);
			EXPECT_EQ(segment_->right.lane.size(), 0);
			EXPECT_EQ(segment_->right.index, 0);
		}
		{
			auto line = CreateTransportLine(data::Orientation::up);

			PickupLine(data::Orientation::right, line);
			EXPECT_EQ(segment_->right.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(data::Orientation::up);

			PickupLine(data::Orientation::down, line);
			EXPECT_EQ(segment_->left.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(data::Orientation::up);

			PickupLine(data::Orientation::left, line);
			EXPECT_EQ(segment_->left.lane.size(), 0);
		}
	}

	TEST_F(InserterPickupTest, PickupTransportLineRight) {
		{
			auto line = CreateTransportLine(data::Orientation::right);

			PickupLine(data::Orientation::up, line);
			EXPECT_EQ(segment_->left.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(data::Orientation::right);

			PickupLine(data::Orientation::right, line);
			EXPECT_EQ(segment_->right.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(data::Orientation::right);

			PickupLine(data::Orientation::down, line);
			EXPECT_EQ(segment_->right.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(data::Orientation::right);

			PickupLine(data::Orientation::left, line);
			EXPECT_EQ(segment_->left.lane.size(), 0);
		}
	}

	TEST_F(InserterPickupTest, PickupTransportLineDown) {
		{
			auto line = CreateTransportLine(data::Orientation::down);

			PickupLine(data::Orientation::up, line);
			EXPECT_EQ(segment_->left.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(data::Orientation::down);

			PickupLine(data::Orientation::right, line);
			EXPECT_EQ(segment_->left.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(data::Orientation::down);

			PickupLine(data::Orientation::down, line);
			EXPECT_EQ(segment_->right.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(data::Orientation::down);

			PickupLine(data::Orientation::left, line);
			EXPECT_EQ(segment_->right.lane.size(), 0);
		}
	}

	TEST_F(InserterPickupTest, PickupTransportLineLeft) {
		{
			auto line = CreateTransportLine(data::Orientation::left);

			PickupLine(data::Orientation::up, line);
			EXPECT_EQ(segment_->right.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(data::Orientation::left);

			PickupLine(data::Orientation::right, line);
			EXPECT_EQ(segment_->left.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(data::Orientation::left);

			PickupLine(data::Orientation::down, line);
			EXPECT_EQ(segment_->left.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(data::Orientation::left);

			PickupLine(data::Orientation::left, line);
			EXPECT_EQ(segment_->right.lane.size(), 0);
		}
	}

	TEST_F(InserterPickupTest, PickupAssemblyMachine) {
		data::PrototypeManager prototype_manager{};

		data::AssemblyMachine asm_machine{};
		auto& layer = TestSetupAssemblyMachine(worldData_, {0, 0}, asm_machine);
		auto* data  = layer.GetUniqueData<data::AssemblyMachineData>();

		// Does nothing as there is no recipe yet
		PickupAssemblyMachine(logicData_,
		                      2, data::ToRotationDegree(kMaxInserterDegree),
		                      2,
		                      *data, data::Orientation::up
		);

		// ======================================================================

		auto recipe_pack = TestSetupRecipe(prototype_manager);


		data->ChangeRecipe(logicData_, prototype_manager, &recipe_pack.recipe);

		// No items in product inventory
		EXPECT_FALSE(
			PickupAssemblyMachine(logicData_,
				2, data::ToRotationDegree(kMaxInserterDegree),
				2,
				*data, data::Orientation::up).first);


		// Has items in product inventory
		data->ingredientInv[0] = {recipe_pack.item1, 1};
		data->ingredientInv[1] = {recipe_pack.item2, 1};

		data->productInv[0] = {recipe_pack.itemProduct, 10, recipe_pack.itemProduct};

		targetProtoData_ = &asm_machine;
		auto result      = PickupAssemblyMachine(logicData_,
		                                         2, data::ToRotationDegree(kMaxInserterDegree),
		                                         2,
		                                         *data, data::Orientation::up
		);

		EXPECT_TRUE(result.first);
		EXPECT_EQ(result.second.item, recipe_pack.itemProduct);
		EXPECT_EQ(result.second.count, 2);

		EXPECT_EQ(data->productInv[0].count, 8);

		// Begin crafting since ingredients are met
		EXPECT_NE(data->deferralEntry.second, 0);
	}
}
