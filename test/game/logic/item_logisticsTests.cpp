// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 04/07/2020

#include <gtest/gtest.h>

#include "jactorioTests.h"

#include "data/prototype/entity/container_entity.h"
#include "data/prototype/entity/inserter.h"
#include "data/prototype/entity/mining_drill.h"
#include "data/prototype/entity/transport/transport_belt.h"
#include "game/logic/inserter_controller.h"
#include "game/logic/item_logistics.h"

namespace jactorio::game
{
	class ItemLogisticsTest : public testing::Test
	{
	protected:
		WorldData worldData_{};

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
		drop_off.DropOff({&item, 10});

		EXPECT_EQ(
			container_layer.GetUniqueData<data::ContainerEntityData>()->inventory[0].second,
			10);
	}

	TEST_F(ItemLogisticsTest, InserterPickupItem) {
		data::ContainerEntity container_entity{};
		auto& container_layer = TestSetupContainer(worldData_, {2, 4}, container_entity);

		InserterPickup pickup{data::Orientation::up};
		ASSERT_TRUE(pickup.Initialize(worldData_, *container_layer.GetUniqueData(), 2, 4));

		data::Item item{};
		auto* inv = container_layer.GetUniqueData<data::ContainerEntityData>()->inventory;
		inv[0]    = {&item, 10};

		data::ItemStack out_item_stack;
		pickup.Pickup(1, data::ToRotationDegree(180.f), 2, out_item_stack);

		EXPECT_EQ(inv[0].second, 8);
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

		void TransportLineInsert(const data::Orientation orientation,
		                         data::TransportLineData& line_data) const {
			data::Item item{};
			InsertTransportBelt({&item, 1},
			                    line_data,
			                    orientation);
		}

	protected:
		WorldData worldData_{};
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
	};

	TEST_F(ItemDropOffTest, GetInsertFunc) {
		worldData_.EmplaceChunk(0, 0);

		// For passing into ItemDropOff::Initialize
		data::HealthEntityData mock_unique_data{};


		// Empty tile cannot be inserted into
		EXPECT_FALSE(this->Initialize(worldData_, mock_unique_data, {2, 4}));
		EXPECT_FALSE(this->IsInitialized());


		// Transport belt can be inserted onto
		data::TransportBelt belt{};
		worldData_.GetTile(2, 4)
		          ->SetEntityPrototype(ChunkTile::ChunkLayer::entity, &belt);

		EXPECT_TRUE(this->Initialize(worldData_, mock_unique_data, {2, 4}));
		EXPECT_TRUE(this->IsInitialized());


		// Mining drill cannot be inserted into 
		data::MiningDrill drill{};
		worldData_.GetTile(2, 4)
		          ->SetEntityPrototype(ChunkTile::ChunkLayer::entity, &drill);

		EXPECT_FALSE(this->Initialize(worldData_, mock_unique_data, {2, 4}));
		EXPECT_TRUE(this->IsInitialized());  // Still initialized from transport belt


		// Container can be inserted into
		data::ContainerEntity container{};
		worldData_.GetTile(2, 4)
		          ->SetEntityPrototype(ChunkTile::ChunkLayer::entity, &container);

		EXPECT_TRUE(this->Initialize(worldData_, mock_unique_data, {2, 4}));
		EXPECT_TRUE(this->IsInitialized());
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
		InsertContainerEntity({&item, 2},
		                      *container_data,
		                      data::Orientation::down);

		// Inserted item
		EXPECT_EQ(container_data->inventory[0].first, &item);
		EXPECT_EQ(container_data->inventory[0].second, 2);
	}

	// ======================================================================


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
		                data::TransportLineData& line_data) const {
			constexpr int pickup_amount = 1;

			data::ItemStack out_item_stack;
			const bool result = PickupTransportBelt(1, data::ToRotationDegree(kMaxInserterDegree),
			                                        pickup_amount,
			                                        line_data,
			                                        orientation,
			                                        out_item_stack);

			EXPECT_TRUE(result);

			EXPECT_NE(out_item_stack.first, nullptr);
			EXPECT_EQ(out_item_stack.second, pickup_amount);
		}
	};

	TEST_F(InserterPickupTest, PickupContainerEntity) {
		data::ContainerEntity container_entity{};
		auto& container_layer = TestSetupContainer(worldData_, {2, 4}, container_entity);
		auto& container_data  = *container_layer.GetUniqueData();

		data::Item item{};
		auto* inv = container_layer.GetUniqueData<data::ContainerEntityData>()->inventory;
		inv[0]    = {&item, 10};

		data::ItemStack out_item_stack;

		PickupContainerEntity(1, data::ToRotationDegree(179),
		                      1, container_data,
		                      data::Orientation::up,
		                      out_item_stack);

		EXPECT_EQ(inv[0].second, 10);  // No items picked up, not 180 degrees

		PickupContainerEntity(1, data::ToRotationDegree(180),
		                      2, container_data,
		                      data::Orientation::up,
		                      out_item_stack);
		EXPECT_EQ(inv[0].second, 8);  // 2 items picked up

		EXPECT_NE(out_item_stack.first, nullptr);
		EXPECT_EQ(out_item_stack.second, 2);
	}

	TEST_F(InserterPickupTest, PickupContainerTransportLineUp) {
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

	TEST_F(InserterPickupTest, PickupContainerTransportLineRight) {
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

	TEST_F(InserterPickupTest, PickupContainerTransportLineDown) {
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

	TEST_F(InserterPickupTest, PickupContainerTransportLineLeft) {
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
}
