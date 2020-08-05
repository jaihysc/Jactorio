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

		/// \brief Creates a transport line with orientation
		data::TransportLineData CreateTransportLine(const data::Orientation orientation,
		                                            const TransportSegment::TerminationType ttype =
			                                            TransportSegment::TerminationType::straight) {
			worldData_.AddChunk(Chunk(0, 0));

			const auto segment = std::make_shared<TransportSegment>(
				orientation,
				ttype,
				2
			);

			return data::TransportLineData{segment};
		}

		///
		/// \param orientation Inserter orientation to dropoff
		void TransportLineInsert(const data::Orientation orientation,
		                         data::TransportLineData& line_data) {
			data::Item item{};
			InsertTransportBelt({logicData_, {&item, 1}, line_data, orientation});

			EXPECT_TRUE(CanInsertTransportBelt({logicData_, {}, line_data, data::Orientation::up}));
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
		EXPECT_TRUE(
			InsertContainerEntity({logicData_, {&item, 2}, *container_data, data::Orientation::down})
		);
		EXPECT_TRUE(CanInsertContainerEntity({logicData_, {}, *container_data, data::Orientation::up}));

		// Inserted item
		EXPECT_EQ(container_data->inventory[0].item, &item);
		EXPECT_EQ(container_data->inventory[0].count, 2);
	}

	TEST_F(ItemDropOffTest, InsertOffset) {
		auto line_data             = CreateTransportLine(data::Orientation::up);
		line_data.lineSegmentIndex = 1;
		line_data.lineSegment->itemOffset        = 10;  // Arbitrary itemOffset

		TransportLineInsert(data::Orientation::up, line_data);
		ASSERT_EQ(line_data.lineSegment->right.lane.size(), 1);
		EXPECT_FLOAT_EQ(line_data.lineSegment->right.lane[0].first.getAsDouble(), 1.5f);
	}

	TEST_F(ItemDropOffTest, InsertTransportLineUp) {
		{
			auto line = CreateTransportLine(data::Orientation::up);

			TransportLineInsert(data::Orientation::up, line);
			EXPECT_EQ(line.lineSegment->right.lane.size(), 1);
		}
		{
			auto line = CreateTransportLine(data::Orientation::up);

			TransportLineInsert(data::Orientation::right, line);
			EXPECT_EQ(line.lineSegment->left.lane.size(), 1);
		}
		{
			auto line = CreateTransportLine(data::Orientation::up);

			TransportLineInsert(data::Orientation::down, line);
			EXPECT_EQ(line.lineSegment->left.lane.size(), 0);
			EXPECT_EQ(line.lineSegment->right.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(data::Orientation::up);

			TransportLineInsert(data::Orientation::left, line);
			EXPECT_EQ(line.lineSegment->right.lane.size(), 1);
		}
	}

	TEST_F(ItemDropOffTest, InsertTransportLineRight) {
		{
			auto line = CreateTransportLine(data::Orientation::right);

			TransportLineInsert(data::Orientation::up, line);
			EXPECT_EQ(line.lineSegment->right.lane.size(), 1);
		}
		{
			auto line = CreateTransportLine(data::Orientation::right);

			TransportLineInsert(data::Orientation::right, line);
			EXPECT_EQ(line.lineSegment->right.lane.size(), 1);
		}
		{
			auto line = CreateTransportLine(data::Orientation::right);

			TransportLineInsert(data::Orientation::down, line);
			EXPECT_EQ(line.lineSegment->left.lane.size(), 1);
		}
		{
			auto line = CreateTransportLine(data::Orientation::right);

			TransportLineInsert(data::Orientation::left, line);
			EXPECT_EQ(line.lineSegment->left.lane.size(), 0);
			EXPECT_EQ(line.lineSegment->right.lane.size(), 0);
		}
	}

	TEST_F(ItemDropOffTest, InsertTransportLineDown) {
		{
			auto line = CreateTransportLine(data::Orientation::down);

			TransportLineInsert(data::Orientation::up, line);
			EXPECT_EQ(line.lineSegment->left.lane.size(), 0);
			EXPECT_EQ(line.lineSegment->right.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(data::Orientation::down);

			TransportLineInsert(data::Orientation::right, line);
			EXPECT_EQ(line.lineSegment->right.lane.size(), 1);
		}
		{
			auto line = CreateTransportLine(data::Orientation::down);

			TransportLineInsert(data::Orientation::down, line);
			EXPECT_EQ(line.lineSegment->right.lane.size(), 1);
		}
		{
			auto line = CreateTransportLine(data::Orientation::down);

			TransportLineInsert(data::Orientation::left, line);
			EXPECT_EQ(line.lineSegment->left.lane.size(), 1);
		}
	}

	TEST_F(ItemDropOffTest, InsertTransportLineLeft) {
		{
			auto line = CreateTransportLine(data::Orientation::left);

			TransportLineInsert(data::Orientation::up, line);
			EXPECT_EQ(line.lineSegment->left.lane.size(), 1);
		}
		{
			auto line = CreateTransportLine(data::Orientation::left);

			TransportLineInsert(data::Orientation::right, line);
			EXPECT_EQ(line.lineSegment->left.lane.size(), 0);
			EXPECT_EQ(line.lineSegment->right.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(data::Orientation::left);

			TransportLineInsert(data::Orientation::down, line);
			EXPECT_EQ(line.lineSegment->right.lane.size(), 1);
		}
		{
			auto line = CreateTransportLine(data::Orientation::left);

			TransportLineInsert(data::Orientation::left, line);
			EXPECT_EQ(line.lineSegment->right.lane.size(), 1);
		}
	}

	TEST_F(ItemDropOffTest, DropoffTransportLineNonStraight) {
		// -->
		// ^
		// | <--

		auto right = CreateTransportLine(data::Orientation::right, TransportSegment::TerminationType::straight);
		auto up    = CreateTransportLine(data::Orientation::up, TransportSegment::TerminationType::bend_right);
		auto left  = CreateTransportLine(data::Orientation::left, TransportSegment::TerminationType::bend_right);

		left.lineSegment->targetSegment = up.lineSegment.get();
		up.lineSegment->targetSegment   = right.lineSegment.get();

		left.lineSegmentIndex = 1;


		data::Item item;

		TransportLineInsert(data::Orientation::up, left);
		ASSERT_EQ(left.lineSegment->left.lane.size(), 1);
		EXPECT_FLOAT_EQ(left.lineSegment->left.lane[0].first.getAsDouble(), 0.5 + 0.7);
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
		EXPECT_TRUE(InsertAssemblyMachine({logicData_, {recipe_pack.item2, 10}, asm_data, data::Orientation::up}));

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
		EXPECT_FALSE(InsertAssemblyMachine({logicData_, {recipe_pack.item1, 2}, asm_data, data::Orientation::up}));
	}

	TEST_F(ItemDropOffTest, CanInsertAssemblyMachine) {
		data::AssemblyMachineData asm_data{};

		data::Item item{};

		// No recipe
		EXPECT_FALSE(CanInsertAssemblyMachine({logicData_, {&item, 2}, asm_data, data::Orientation::down}));


		data::PrototypeManager prototype_manager{};

		auto recipe_pack = TestSetupRecipe(prototype_manager);
		asm_data.ChangeRecipe(logicData_, prototype_manager, &recipe_pack.recipe);

		// Has recipe, wrong item
		EXPECT_FALSE(CanInsertAssemblyMachine({logicData_, {&item, 2}, asm_data, data::Orientation::down}));

		// Has recipe, correct item
		DropOffParams args{logicData_, {recipe_pack.item1, 2000}, asm_data, data::Orientation::down};
		EXPECT_TRUE(CanInsertAssemblyMachine(args));

		// Exceeds the max percentage that a slot can be filled
		asm_data.ingredientInv[0].count = 2;
		EXPECT_FALSE(CanInsertAssemblyMachine(args));

		// Exceeds max stack size
		recipe_pack.item1->stackSize    = 50;
		asm_data.ingredientInv[0].count = 50;
		EXPECT_FALSE(CanInsertAssemblyMachine(args));

		// Under the max percentage that a slot can be filled
		asm_data.ingredientInv[0].count = 1;
		EXPECT_TRUE(CanInsertAssemblyMachine(args));
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

		/// Item which will be on transport segments from CreateTransportLine
		data::Item lineItem_{};

		/// \brief Creates a transport line with  1 item on each side
		data::TransportLineData CreateTransportLine(const data::Orientation orientation) {

			const auto segment = std::make_shared<TransportSegment>(
				orientation,
				TransportSegment::TerminationType::straight,
				2
			);

			segment->InsertItem(false, 0.5, lineItem_);
			segment->InsertItem(true, 0.5, lineItem_);

			return data::TransportLineData{segment};
		}


		/// \param orientation Orientation to line being picked up from
		void PickupLine(const data::Orientation orientation,
		                data::TransportLineData& line_data) {
			constexpr int pickup_amount = 1;

			const auto result = PickupTransportBelt({
				logicData_,
				1, data::ToRotationDegree(kMaxInserterDegree),
				pickup_amount,
				line_data,
				orientation
			});

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


		PickupContainerEntity({
			logicData_,
			1, data::ToRotationDegree(179),
			1, container_data,
			data::Orientation::up
		});
		EXPECT_EQ(inv[0].count, 10);  // No items picked up, not 180 degrees


		PickupParams args = {
			logicData_,
			1, data::ToRotationDegree(180),
			2, container_data,
			data::Orientation::up
		};

		EXPECT_EQ(GetPickupContainerEntity(args), &item);

		auto result = PickupContainerEntity(args);
		EXPECT_EQ(inv[0].count, 8);  // At 180 degrees, 2 items picked up
		EXPECT_EQ(result.second.item, &item);
		EXPECT_EQ(result.second.count, 2);
	}

	TEST_F(InserterPickupTest, PickupTransportLineUp) {
		// Line is above inserter
		{
			auto line = CreateTransportLine(data::Orientation::up);

			line.lineSegment->right.index = 10;

			// Should set index to 0 since a item was removed
			PickupLine(data::Orientation::up, line);
			EXPECT_EQ(line.lineSegment->right.lane.size(), 0);
			EXPECT_EQ(line.lineSegment->right.index, 0);
		}
		{
			auto line = CreateTransportLine(data::Orientation::up);

			PickupLine(data::Orientation::right, line);
			EXPECT_EQ(line.lineSegment->right.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(data::Orientation::up);

			PickupLine(data::Orientation::down, line);
			EXPECT_EQ(line.lineSegment->left.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(data::Orientation::up);

			PickupLine(data::Orientation::left, line);
			EXPECT_EQ(line.lineSegment->left.lane.size(), 0);
		}
	}

	TEST_F(InserterPickupTest, PickupTransportLineRight) {
		{
			auto line = CreateTransportLine(data::Orientation::right);

			PickupLine(data::Orientation::up, line);
			EXPECT_EQ(line.lineSegment->left.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(data::Orientation::right);

			PickupLine(data::Orientation::right, line);
			EXPECT_EQ(line.lineSegment->right.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(data::Orientation::right);

			PickupLine(data::Orientation::down, line);
			EXPECT_EQ(line.lineSegment->right.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(data::Orientation::right);

			PickupLine(data::Orientation::left, line);
			EXPECT_EQ(line.lineSegment->left.lane.size(), 0);
		}
	}

	TEST_F(InserterPickupTest, PickupTransportLineDown) {
		{
			auto line = CreateTransportLine(data::Orientation::down);

			PickupLine(data::Orientation::up, line);
			EXPECT_EQ(line.lineSegment->left.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(data::Orientation::down);

			PickupLine(data::Orientation::right, line);
			EXPECT_EQ(line.lineSegment->left.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(data::Orientation::down);

			PickupLine(data::Orientation::down, line);
			EXPECT_EQ(line.lineSegment->right.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(data::Orientation::down);

			PickupLine(data::Orientation::left, line);
			EXPECT_EQ(line.lineSegment->right.lane.size(), 0);
		}
	}

	TEST_F(InserterPickupTest, PickupTransportLineLeft) {
		{
			auto line = CreateTransportLine(data::Orientation::left);

			PickupLine(data::Orientation::up, line);
			EXPECT_EQ(line.lineSegment->right.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(data::Orientation::left);

			PickupLine(data::Orientation::right, line);
			EXPECT_EQ(line.lineSegment->left.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(data::Orientation::left);

			PickupLine(data::Orientation::down, line);
			EXPECT_EQ(line.lineSegment->left.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(data::Orientation::left);

			PickupLine(data::Orientation::left, line);
			EXPECT_EQ(line.lineSegment->right.lane.size(), 0);
		}
	}

	TEST_F(InserterPickupTest, PickupTransportLineNonStraight) {
		// -->
		// ^
		// | <--

		const auto right = std::make_shared<TransportSegment>(
			data::Orientation::right,
			TransportSegment::TerminationType::straight,
			2
		);

		const auto up = std::make_shared<TransportSegment>(
			data::Orientation::up,
			TransportSegment::TerminationType::bend_right,
			2
		);

		const auto left = std::make_shared<TransportSegment>(
			data::Orientation::left,
			TransportSegment::TerminationType::bend_right,
			2
		);

		left->targetSegment = up.get();
		up->targetSegment   = right.get();

		data::TransportLineData line{left};
		line.lineSegmentIndex = 1;


		data::Item item;

		left->AppendItem(true, 0.5 + 0.7, item);
		PickupLine(data::Orientation::up, line);
		EXPECT_EQ(left->left.lane.size(), 0);

		left->AppendItem(false, 0.5 + 0.3, item);
		PickupLine(data::Orientation::up, line);
		EXPECT_EQ(left->right.lane.size(), 0);
	}

	TEST_F(InserterPickupTest, PickupTransportLineAlternativeSide) {
		// If the preferred lane is available, inserter will attempt to pick up from other lane
		auto line = CreateTransportLine(data::Orientation::up);

		EXPECT_EQ(
			GetPickupTransportBelt({
				logicData_,
				1, data::ToRotationDegree(180),
				1, line, data::Orientation::up
				}),
			&lineItem_
		);

		PickupLine(data::Orientation::up, line);
		EXPECT_EQ(line.lineSegment->right.lane.size(), 0);

		// Use alternative side
		EXPECT_EQ(
			GetPickupTransportBelt({
				logicData_,
				1, data::ToRotationDegree(180),
				1, line, data::Orientation::up
				}),
			&lineItem_
		);
		PickupLine(data::Orientation::up, line);
		EXPECT_EQ(line.lineSegment->left.lane.size(), 0);
	}


	TEST_F(InserterPickupTest, PickupAssemblyMachine) {
		data::PrototypeManager prototype_manager{};

		data::AssemblyMachine asm_machine{};
		auto& layer = TestSetupAssemblyMachine(worldData_, {0, 0}, asm_machine);
		auto* data  = layer.GetUniqueData<data::AssemblyMachineData>();

		// Does nothing as there is no recipe yet
		PickupAssemblyMachine({
				logicData_,
				2, data::ToRotationDegree(kMaxInserterDegree),
				2,
				*data, data::Orientation::up
			}
		);

		// ======================================================================

		auto recipe_pack = TestSetupRecipe(prototype_manager);


		data->ChangeRecipe(logicData_, prototype_manager, &recipe_pack.recipe);

		// No items in product inventory
		EXPECT_FALSE(
			PickupAssemblyMachine({logicData_,
				2, data::ToRotationDegree(kMaxInserterDegree),
				2,
				*data, data::Orientation::up
				}).first
		);


		// Has items in product inventory
		data->ingredientInv[0] = {recipe_pack.item1, 1};
		data->ingredientInv[1] = {recipe_pack.item2, 1};

		data->productInv[0] = {recipe_pack.itemProduct, 10, recipe_pack.itemProduct};

		targetProtoData_ = &asm_machine;

		PickupParams args{
			logicData_,
			2, data::ToRotationDegree(kMaxInserterDegree),
			2,
			*data, data::Orientation::up
		};

		EXPECT_EQ(GetPickupAssemblyMachine(args), recipe_pack.itemProduct);
		auto result = PickupAssemblyMachine(args);

		EXPECT_TRUE(result.first);
		EXPECT_EQ(result.second.item, recipe_pack.itemProduct);
		EXPECT_EQ(result.second.count, 2);

		EXPECT_EQ(data->productInv[0].count, 8);

		// Begin crafting since ingredients are met
		EXPECT_NE(data->deferralEntry.second, 0);
	}
}
