// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 04/07/2020

#include <gtest/gtest.h>

#include "data/prototype/entity/container_entity.h"
#include "data/prototype/entity/mining_drill.h"
#include "data/prototype/entity/transport/transport_belt.h"
#include "game/logic/inserter_controller.h"
#include "game/logic/item_logistics.h"

namespace game
{
	class ItemLogisticsTest : public testing::Test
	{
	protected:
		jactorio::game::WorldData worldData_{};

		void SetUp() override {
			worldData_.EmplaceChunk(0, 0);
		}
	};

#define ITEM_LOGISTICS_SETUP_CHEST\
	jactorio::data::ContainerEntity container{};\
	auto& layer = worldData_.GetTile(2, 4)->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity);\
	layer.prototypeData = &container;\
	layer.uniqueData    = new jactorio::data::ContainerEntityData(10);

	TEST_F(ItemLogisticsTest, DropOffDropItem) {
		ITEM_LOGISTICS_SETUP_CHEST

		jactorio::game::ItemDropOff drop_off{jactorio::data::Orientation::up};
		ASSERT_TRUE(drop_off.Initialize(worldData_, *layer.uniqueData, 2, 4));

		jactorio::data::Item item{};
		drop_off.DropOff({&item, 10});

		EXPECT_EQ(
			static_cast<jactorio::data::ContainerEntityData*>(layer.uniqueData)->inventory[0].second,
			10);
	}

	TEST_F(ItemLogisticsTest, InserterPickupItem) {
		ITEM_LOGISTICS_SETUP_CHEST

		jactorio::game::InserterPickup pickup{jactorio::data::Orientation::up};
		ASSERT_TRUE(pickup.Initialize(worldData_, *layer.uniqueData, 2, 4));

		jactorio::data::Item item{};
		auto* inv = static_cast<jactorio::data::ContainerEntityData*>(layer.uniqueData)->inventory;
		inv[0]    = {&item, 10};

		pickup.Pickup(jactorio::data::InserterData::ToRotationDegree(180.f), 2);

		EXPECT_EQ(inv[0].second, 8);
	}

	// ======================================================================

	///
	/// Inherits ItemDropOff to gain access to insertion methods
	class ItemDropOffTest : public testing::Test, public jactorio::game::ItemDropOff
	{
	public:
		explicit ItemDropOffTest()
			: ItemDropOff(jactorio::data::Orientation::up) {
		}

		void TransportLineInsert(const jactorio::data::Orientation orientation,
		                         jactorio::data::TransportLineData& line_data) const {
			jactorio::data::Item item{};
			InsertTransportBelt({&item, 1},
			                    line_data,
			                    orientation);
		}

	protected:
		jactorio::game::WorldData worldData_{};
		jactorio::game::TransportSegment* segment_ = nullptr;

		// Creates a transport line with orientation at (4, 4)
		jactorio::data::TransportLineData CreateTransportLine(const jactorio::data::Orientation orientation) {
			worldData_.AddChunk(jactorio::game::Chunk(0, 0));

			segment_ = new jactorio::game::TransportSegment{
				orientation,
				jactorio::game::TransportSegment::TerminationType::straight,
				2
			};

			return jactorio::data::TransportLineData{*segment_};  // < -- Manages segment_
		}
	};

	TEST_F(ItemDropOffTest, GetInsertFunc) {
		worldData_.EmplaceChunk(0, 0);

		// For passing into ItemDropOff::Initialize
		jactorio::data::HealthEntityData mock_unique_data{};


		// Empty tile cannot be inserted into
		EXPECT_FALSE(this->Initialize(worldData_, mock_unique_data, {2, 4}));


		// Transport belt can be inserted onto
		jactorio::data::TransportBelt belt{};
		worldData_.GetTile(2, 4)
		          ->SetEntityPrototype(jactorio::game::ChunkTile::ChunkLayer::entity, &belt);

		EXPECT_TRUE(this->Initialize(worldData_, mock_unique_data, {2, 4}));


		// Mining drill cannot be inserted into 
		jactorio::data::MiningDrill drill{};
		worldData_.GetTile(2, 4)
		          ->SetEntityPrototype(jactorio::game::ChunkTile::ChunkLayer::entity, &drill);

		EXPECT_FALSE(this->Initialize(worldData_, mock_unique_data, {2, 4}));


		// Container can be inserted into
		jactorio::data::ContainerEntity container{};
		worldData_.GetTile(2, 4)
		          ->SetEntityPrototype(jactorio::game::ChunkTile::ChunkLayer::entity, &container);

		EXPECT_TRUE(this->Initialize(worldData_, mock_unique_data, {2, 4}));
	}

	// ======================================================================

	TEST_F(ItemDropOffTest, InsertContainerEntity) {
		jactorio::game::WorldData world_data{};
		world_data.AddChunk(jactorio::game::Chunk(0, 0));

		// Deleted by chunk tile layer
		auto* container_data = new jactorio::data::ContainerEntityData(10);

		world_data.GetTile(3, 1)
		          ->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity).uniqueData = container_data;


		jactorio::data::Item item{};
		// Orientation is orientation from origin object
		InsertContainerEntity({&item, 2},
		                      *container_data,
		                      jactorio::data::Orientation::down);

		// Inserted item
		EXPECT_EQ(container_data->inventory[0].first, &item);
		EXPECT_EQ(container_data->inventory[0].second, 2);
	}

	// ======================================================================


	TEST_F(ItemDropOffTest, InsertOffset) {
		auto line_proto             = CreateTransportLine(jactorio::data::Orientation::up);
		line_proto.lineSegmentIndex = 1;
		segment_->itemOffset        = 10;  // Arbitrary itemOffset

		TransportLineInsert(jactorio::data::Orientation::up, line_proto);
		ASSERT_EQ(segment_->right.lane.size(), 1);
		EXPECT_FLOAT_EQ(segment_->right.lane[0].first.getAsDouble(), 1.5f);
	}

	TEST_F(ItemDropOffTest, InsertTransportLineUp) {
		{
			auto line = CreateTransportLine(jactorio::data::Orientation::up);

			TransportLineInsert(jactorio::data::Orientation::up, line);
			EXPECT_EQ(segment_->right.lane.size(), 1);
		}
		{
			auto line = CreateTransportLine(jactorio::data::Orientation::up);

			TransportLineInsert(jactorio::data::Orientation::right, line);
			EXPECT_EQ(segment_->left.lane.size(), 1);
		}
		{
			auto line = CreateTransportLine(jactorio::data::Orientation::up);

			TransportLineInsert(jactorio::data::Orientation::down, line);
			EXPECT_EQ(segment_->left.lane.size(), 0);
			EXPECT_EQ(segment_->right.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(jactorio::data::Orientation::up);

			TransportLineInsert(jactorio::data::Orientation::left, line);
			EXPECT_EQ(segment_->right.lane.size(), 1);
		}
	}

	TEST_F(ItemDropOffTest, InsertTransportLineRight) {
		{
			auto line = CreateTransportLine(jactorio::data::Orientation::right);

			TransportLineInsert(jactorio::data::Orientation::up, line);
			EXPECT_EQ(segment_->right.lane.size(), 1);
		}
		{
			auto line = CreateTransportLine(jactorio::data::Orientation::right);

			TransportLineInsert(jactorio::data::Orientation::right, line);
			EXPECT_EQ(segment_->right.lane.size(), 1);
		}
		{
			auto line = CreateTransportLine(jactorio::data::Orientation::right);

			TransportLineInsert(jactorio::data::Orientation::down, line);
			EXPECT_EQ(segment_->left.lane.size(), 1);
		}
		{
			auto line = CreateTransportLine(jactorio::data::Orientation::right);

			TransportLineInsert(jactorio::data::Orientation::left, line);
			EXPECT_EQ(segment_->left.lane.size(), 0);
			EXPECT_EQ(segment_->right.lane.size(), 0);
		}
	}

	TEST_F(ItemDropOffTest, InsertTransportLineDown) {
		{
			auto line = CreateTransportLine(jactorio::data::Orientation::down);

			TransportLineInsert(jactorio::data::Orientation::up, line);
			EXPECT_EQ(segment_->left.lane.size(), 0);
			EXPECT_EQ(segment_->right.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(jactorio::data::Orientation::down);

			TransportLineInsert(jactorio::data::Orientation::right, line);
			EXPECT_EQ(segment_->right.lane.size(), 1);
		}
		{
			auto line = CreateTransportLine(jactorio::data::Orientation::down);

			TransportLineInsert(jactorio::data::Orientation::down, line);
			EXPECT_EQ(segment_->right.lane.size(), 1);
		}
		{
			auto line = CreateTransportLine(jactorio::data::Orientation::down);

			TransportLineInsert(jactorio::data::Orientation::left, line);
			EXPECT_EQ(segment_->left.lane.size(), 1);
		}
	}

	TEST_F(ItemDropOffTest, InsertTransportLineLeft) {
		{
			auto line = CreateTransportLine(jactorio::data::Orientation::left);

			TransportLineInsert(jactorio::data::Orientation::up, line);
			EXPECT_EQ(segment_->left.lane.size(), 1);
		}
		{
			auto line = CreateTransportLine(jactorio::data::Orientation::left);

			TransportLineInsert(jactorio::data::Orientation::right, line);
			EXPECT_EQ(segment_->left.lane.size(), 0);
			EXPECT_EQ(segment_->right.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(jactorio::data::Orientation::left);

			TransportLineInsert(jactorio::data::Orientation::down, line);
			EXPECT_EQ(segment_->right.lane.size(), 1);
		}
		{
			auto line = CreateTransportLine(jactorio::data::Orientation::left);

			TransportLineInsert(jactorio::data::Orientation::left, line);
			EXPECT_EQ(segment_->right.lane.size(), 1);
		}
	}

	// ======================================================================

	class InserterPickupTest : public testing::Test, public jactorio::game::InserterPickup
	{
	public:
		explicit InserterPickupTest()
			: InserterPickup(jactorio::data::Orientation::up) {
		}

		void SetUp() override {
			worldData_.EmplaceChunk(0, 0);
		}

	protected:
		jactorio::game::WorldData worldData_{};

		jactorio::data::Inserter inserterProto_{};
		jactorio::game::TransportSegment* segment_ = nullptr;


		// Creates a transport line 1 item on each side
		jactorio::data::TransportLineData CreateTransportLine(const jactorio::data::Orientation orientation) {
			segment_ = new jactorio::game::TransportSegment{
				orientation,
				jactorio::game::TransportSegment::TerminationType::straight,
				2
			};

			jactorio::data::Item item{};  // Item is not used for tests

			segment_->InsertItem(false, 0, &item);
			segment_->InsertItem(true, 0, &item);

			return jactorio::data::TransportLineData{*segment_};  // <-- Manages segment_
		}


		void PickupLine(const jactorio::data::Orientation orientation,
		                jactorio::data::TransportLineData& line_data) const {
			PickupTransportBelt(jactorio::data::InserterData::ToRotationDegree(jactorio::game::kMaxInserterDegree),
			                    1,
			                    line_data,
			                    orientation);
		}
	};

	TEST_F(InserterPickupTest, PickupContainerEntity) {
		worldData_.EmplaceChunk(0, 0);

		ITEM_LOGISTICS_SETUP_CHEST

		jactorio::data::Item item{};
		auto* inv = static_cast<jactorio::data::ContainerEntityData*>(layer.uniqueData)->inventory;
		inv[0]    = {&item, 10};


		PickupContainerEntity(jactorio::data::InserterData::ToRotationDegree(179),
		                      1, *layer.uniqueData,
		                      jactorio::data::Orientation::up);

		EXPECT_EQ(inv[0].second, 10);  // No items picked up, not 180 degrees

		PickupContainerEntity(jactorio::data::InserterData::ToRotationDegree(180),
		                      2, *layer.uniqueData,
		                      jactorio::data::Orientation::up);
		EXPECT_EQ(inv[0].second, 8);  // 2 items picked up
	}

	TEST_F(InserterPickupTest, PickupContainerTransportLineUp) {
		// Line is above inserter
		{
			auto line = CreateTransportLine(jactorio::data::Orientation::up);

			PickupLine(jactorio::data::Orientation::up, line);
			EXPECT_EQ(segment_->right.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(jactorio::data::Orientation::up);

			PickupLine(jactorio::data::Orientation::right, line);
			EXPECT_EQ(segment_->right.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(jactorio::data::Orientation::up);

			PickupLine(jactorio::data::Orientation::down, line);
			EXPECT_EQ(segment_->left.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(jactorio::data::Orientation::up);

			PickupLine(jactorio::data::Orientation::left, line);
			EXPECT_EQ(segment_->left.lane.size(), 0);
		}
	}

	TEST_F(InserterPickupTest, PickupContainerTransportLineRight) {
		{
			auto line = CreateTransportLine(jactorio::data::Orientation::right);

			PickupLine(jactorio::data::Orientation::up, line);
			EXPECT_EQ(segment_->left.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(jactorio::data::Orientation::right);

			PickupLine(jactorio::data::Orientation::right, line);
			EXPECT_EQ(segment_->right.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(jactorio::data::Orientation::right);

			PickupLine(jactorio::data::Orientation::down, line);
			EXPECT_EQ(segment_->right.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(jactorio::data::Orientation::right);

			PickupLine(jactorio::data::Orientation::left, line);
			EXPECT_EQ(segment_->left.lane.size(), 0);
		}
	}

	TEST_F(InserterPickupTest, PickupContainerTransportLineDown) {
		{
			auto line = CreateTransportLine(jactorio::data::Orientation::down);

			PickupLine(jactorio::data::Orientation::up, line);
			EXPECT_EQ(segment_->left.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(jactorio::data::Orientation::down);

			PickupLine(jactorio::data::Orientation::right, line);
			EXPECT_EQ(segment_->left.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(jactorio::data::Orientation::down);

			PickupLine(jactorio::data::Orientation::down, line);
			EXPECT_EQ(segment_->right.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(jactorio::data::Orientation::down);

			PickupLine(jactorio::data::Orientation::left, line);
			EXPECT_EQ(segment_->right.lane.size(), 0);
		}
	}

	TEST_F(InserterPickupTest, PickupContainerTransportLineLeft) {
		{
			auto line = CreateTransportLine(jactorio::data::Orientation::left);

			PickupLine(jactorio::data::Orientation::up, line);
			EXPECT_EQ(segment_->right.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(jactorio::data::Orientation::left);

			PickupLine(jactorio::data::Orientation::right, line);
			EXPECT_EQ(segment_->left.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(jactorio::data::Orientation::left);

			PickupLine(jactorio::data::Orientation::down, line);
			EXPECT_EQ(segment_->left.lane.size(), 0);
		}
		{
			auto line = CreateTransportLine(jactorio::data::Orientation::left);

			PickupLine(jactorio::data::Orientation::left, line);
			EXPECT_EQ(segment_->right.lane.size(), 0);
		}
	}
}
