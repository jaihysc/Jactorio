// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 04/07/2020

#include <gtest/gtest.h>

#include "data/prototype/entity/container_entity.h"
#include "data/prototype/entity/mining_drill.h"
#include "data/prototype/entity/transport/transport_belt.h"
#include "game/logic/item_logistics.h"

namespace game
{
	TEST(ItemLogistics, InsertContainerEntity) {
		jactorio::game::WorldData world_data{};
		world_data.AddChunk(jactorio::game::Chunk(0, 0));

		// Deleted by chunk tile layer
		auto* container_data = new jactorio::data::ContainerEntityData(10);

		world_data.GetTile(3, 1)
		          ->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity).uniqueData = container_data;


		jactorio::data::Item item{};
		// Orientation is orientation from origin object
		jactorio::game::InsertContainerEntity({&item, 2},
		                                      *container_data,
		                                      jactorio::data::Orientation::down);

		// Inserted item
		EXPECT_EQ(container_data->inventory[0].first, &item);
		EXPECT_EQ(container_data->inventory[0].second, 2);
	}

	// ======================================================================

	class ItemLogisticsTransportLineTest : public testing::Test
	{
		jactorio::game::WorldData worldData_{};

	protected:
		jactorio::game::TransportSegment* segment_ = nullptr;

		// Creates a transport line with orientation at (4, 4)
		jactorio::data::TransportLineData CreateTransportLine(const jactorio::data::Orientation orientation) {
			worldData_.AddChunk(jactorio::game::Chunk(0, 0));

			segment_ = new jactorio::game::TransportSegment{
				orientation,
				jactorio::game::TransportSegment::TerminationType::straight,
				2
			};

			return jactorio::data::TransportLineData{*segment_};
		}

		static void TransportLineInsert(const jactorio::data::Orientation orientation,
		                                jactorio::data::TransportLineData& line_data) {
			jactorio::data::Item item{};
			jactorio::game::InsertTransportBelt({&item, 1},
			                                    line_data,
			                                    orientation);
		}
	};

	TEST_F(ItemLogisticsTransportLineTest, InsertOffset) {
		auto line_proto             = CreateTransportLine(jactorio::data::Orientation::up);
		line_proto.lineSegmentIndex = 1;
		segment_->itemOffset        = 10;  // Arbitrary itemOffset

		TransportLineInsert(jactorio::data::Orientation::up, line_proto);
		ASSERT_EQ(segment_->right.lane.size(), 1);
		EXPECT_FLOAT_EQ(segment_->right.lane[0].first.getAsDouble(), 1.5f);
	}

	TEST_F(ItemLogisticsTransportLineTest, InsertTransportLineUp) {
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

	TEST_F(ItemLogisticsTransportLineTest, InsertTransportLineRight) {
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

	TEST_F(ItemLogisticsTransportLineTest, InsertTransportLineDown) {
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

	TEST_F(ItemLogisticsTransportLineTest, InsertTransportLineLeft) {
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

	TEST(ItemLogistics, CanAcceptItem) {
		jactorio::game::WorldData world_data{};
		world_data.AddChunk(jactorio::game::Chunk{0, 0});

		// Empty tile cannot be inserted into
		{
			auto* ptr = jactorio::game::CanAcceptItem(world_data, 2, 4);
			EXPECT_EQ(ptr, nullptr);
		}

		// Transport belt can be inserted onto
		{
			jactorio::data::TransportBelt belt{};
			world_data.GetTile(2, 4)
			          ->SetEntityPrototype(jactorio::game::ChunkTile::ChunkLayer::entity, &belt);

			auto* ptr = jactorio::game::CanAcceptItem(world_data, 2, 4);
			EXPECT_EQ(ptr, &jactorio::game::InsertTransportBelt);
		}

		// Mining drill cannot be inserted into 
		{
			jactorio::data::MiningDrill drill{};
			world_data.GetTile(2, 4)
			          ->SetEntityPrototype(jactorio::game::ChunkTile::ChunkLayer::entity, &drill);

			auto* ptr = jactorio::game::CanAcceptItem(world_data, 2, 4);
			EXPECT_EQ(ptr, nullptr);
		}

		// Container can be inserted into
		{
			jactorio::data::ContainerEntity container{};
			world_data.GetTile(2, 4)
			          ->SetEntityPrototype(jactorio::game::ChunkTile::ChunkLayer::entity, &container);

			auto* ptr = jactorio::game::CanAcceptItem(world_data, 2, 4);
			EXPECT_EQ(ptr, &jactorio::game::InsertContainerEntity);
		}

	}

	TEST(ItemLogistics, InsertItemDestination) {
		// Item_insert_destination with custom insertion function
		// .insert() should call insertion function, throwing the exception
		const jactorio::game::ItemInsertDestination::InsertFunc func = [
			](auto& item, auto& unique_data, auto orientation) -> bool {
			EXPECT_EQ(orientation, jactorio::data::Orientation::up);

			throw std::runtime_error("INSERT_FUNC");
		};

		jactorio::data::HealthEntityData unique_data{};
		const jactorio::game::ItemInsertDestination iid{unique_data, func, jactorio::data::Orientation::up};

		jactorio::data::Item item{};
		try {
			iid.Insert({&item, 4});
			FAIL();  // Did not call insert_func
		}
		catch (std::runtime_error& e) {
			const int result = strcmp(e.what(), "INSERT_FUNC");
			EXPECT_EQ(result, 0);  // Ensure exception is one I threw	
		}
	}
}
