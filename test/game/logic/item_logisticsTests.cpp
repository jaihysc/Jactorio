// 
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
		jactorio::game::World_data world_data{};
		world_data.add_chunk(new jactorio::game::Chunk(0, 0));

		// Deleted by chunk tile layer
		auto* container_data = new jactorio::data::Container_entity_data(10);

		world_data.get_tile(3, 1)
		          ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity).unique_data = container_data;


		jactorio::data::Item item{};
		// Orientation is orientation from origin object
		jactorio::game::item_logistics::insert_container_entity({&item, 2},
		                                                        *container_data,
		                                                        jactorio::data::Orientation::down);

		// Inserted item
		EXPECT_EQ(container_data->inventory[0].first, &item);
		EXPECT_EQ(container_data->inventory[0].second, 2);
	}

	// ======================================================================

	class ItemLogisticsTransportLineTest : public testing::Test
	{
		jactorio::game::World_data world_data_{};

	protected:
		jactorio::game::Transport_line_segment* segment_ = nullptr;

		// Creates a transport line with orientation at (4, 4)
		jactorio::data::Transport_line_data create_transport_line(const jactorio::data::Orientation orientation) {
			world_data_.add_chunk(new jactorio::game::Chunk(0, 0));

			segment_ = new jactorio::game::Transport_line_segment{
				orientation,
				jactorio::game::Transport_line_segment::TerminationType::straight,
				2
			};

			world_data_.get_tile(4, 4)
			          ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity).unique_data = segment_;

			return jactorio::data::Transport_line_data{*segment_};
		}

		void transport_line_insert(const jactorio::data::Orientation orientation,
		jactorio::data::Transport_line_data& line_data) const {
			jactorio::data::Item item{};
			jactorio::game::item_logistics::insert_transport_belt({&item, 1},
			                                                      line_data,
			                                                      orientation);
		}
	};

	TEST_F(ItemLogisticsTransportLineTest, InsertTransportLineUp) {
		{
			auto line = create_transport_line(jactorio::data::Orientation::up);

			transport_line_insert(jactorio::data::Orientation::up, line);
			EXPECT_EQ(segment_->right.size(), 1);
		}
		{
			auto line = create_transport_line(jactorio::data::Orientation::up);

			transport_line_insert(jactorio::data::Orientation::right, line);
			EXPECT_EQ(segment_->left.size(), 1);
		}
		{
			auto line = create_transport_line(jactorio::data::Orientation::up);

			transport_line_insert(jactorio::data::Orientation::down, line);
			EXPECT_EQ(segment_->left.size(), 0);
			EXPECT_EQ(segment_->right.size(), 0);
		}
		{
			auto line = create_transport_line(jactorio::data::Orientation::up);

			transport_line_insert(jactorio::data::Orientation::left, line);
			EXPECT_EQ(segment_->right.size(), 1);
		}
	}

	TEST_F(ItemLogisticsTransportLineTest, InsertTransportLineRight) {
		{
			auto line = create_transport_line(jactorio::data::Orientation::right);

			transport_line_insert(jactorio::data::Orientation::up, line);
			EXPECT_EQ(segment_->right.size(), 1);
		}
		{
			auto line = create_transport_line(jactorio::data::Orientation::right);

			transport_line_insert(jactorio::data::Orientation::right, line);
			EXPECT_EQ(segment_->right.size(), 1);
		}
		{
			auto line = create_transport_line(jactorio::data::Orientation::right);

			transport_line_insert(jactorio::data::Orientation::down, line);
			EXPECT_EQ(segment_->left.size(), 1);
		}
		{
			auto line = create_transport_line(jactorio::data::Orientation::right);

			transport_line_insert(jactorio::data::Orientation::left, line);
			EXPECT_EQ(segment_->left.size(), 0);
			EXPECT_EQ(segment_->right.size(), 0);
		}
	}

	TEST_F(ItemLogisticsTransportLineTest, InsertTransportLineDown) {
		{
			auto line = create_transport_line(jactorio::data::Orientation::down);

			transport_line_insert(jactorio::data::Orientation::up, line);
			EXPECT_EQ(segment_->left.size(), 0);
			EXPECT_EQ(segment_->right.size(), 0);
		}
		{
			auto line = create_transport_line(jactorio::data::Orientation::down);

			transport_line_insert(jactorio::data::Orientation::right, line);
			EXPECT_EQ(segment_->right.size(), 1);
		}
		{
			auto line = create_transport_line(jactorio::data::Orientation::down);

			transport_line_insert(jactorio::data::Orientation::down, line);
			EXPECT_EQ(segment_->right.size(), 1);
		}
		{
			auto line = create_transport_line(jactorio::data::Orientation::down);

			transport_line_insert(jactorio::data::Orientation::left, line);
			EXPECT_EQ(segment_->left.size(), 1);
		}
	}

	TEST_F(ItemLogisticsTransportLineTest, InsertTransportLineLeft) {
		{
			auto line = create_transport_line(jactorio::data::Orientation::left);

			transport_line_insert(jactorio::data::Orientation::up, line);
			EXPECT_EQ(segment_->left.size(), 1);
		}
		{
			auto line = create_transport_line(jactorio::data::Orientation::left);

			transport_line_insert(jactorio::data::Orientation::right, line);
			EXPECT_EQ(segment_->left.size(), 0);
			EXPECT_EQ(segment_->right.size(), 0);
		}
		{
			auto line = create_transport_line(jactorio::data::Orientation::left);

			transport_line_insert(jactorio::data::Orientation::down, line);
			EXPECT_EQ(segment_->right.size(), 1);
		}
		{
			auto line = create_transport_line(jactorio::data::Orientation::left);

			transport_line_insert(jactorio::data::Orientation::left, line);
			EXPECT_EQ(segment_->right.size(), 1);
		}
	}

	// ======================================================================

	TEST(ItemLogistics, CanAcceptItem) {
		jactorio::game::World_data world_data{};
		world_data.add_chunk(new jactorio::game::Chunk{0, 0});

		// Empty tile cannot be inserted into
		{
			auto* ptr = jactorio::game::item_logistics::can_accept_item(world_data, 2, 4);
			EXPECT_EQ(ptr, nullptr);
		}

		// Transport belt can be inserted onto
		{
			jactorio::data::Transport_belt belt{};
			world_data.get_tile(2, 4)
			          ->set_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity, &belt);

			auto* ptr = jactorio::game::item_logistics::can_accept_item(world_data, 2, 4);
			EXPECT_EQ(ptr, &jactorio::game::item_logistics::insert_transport_belt);
		}

		// Mining drill cannot be inserted into 
		{
			jactorio::data::Mining_drill drill{};
			world_data.get_tile(2, 4)
			          ->set_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity, &drill);

			auto* ptr = jactorio::game::item_logistics::can_accept_item(world_data, 2, 4);
			EXPECT_EQ(ptr, nullptr);
		}

		// Container can be inserted into
		{
			jactorio::data::Container_entity container{};
			world_data.get_tile(2, 4)
			          ->set_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity, &container);

			auto* ptr = jactorio::game::item_logistics::can_accept_item(world_data, 2, 4);
			EXPECT_EQ(ptr, &jactorio::game::item_logistics::insert_container_entity);
		}

	}

	TEST(ItemLogistics, InsertItemDestination) {
		// Item_insert_destination with custom insertion function
		// .insert() should call insertion function, throwing the exception
		const jactorio::game::Item_insert_destination::insert_func func = [
			](auto& item, auto& unique_data, auto orientation) -> bool {
			EXPECT_EQ(orientation, jactorio::data::Orientation::up);

			throw std::runtime_error("INSERT_FUNC");
		};

		jactorio::data::Health_entity_data unique_data{};
		const jactorio::game::Item_insert_destination iid{unique_data, func, jactorio::data::Orientation::up};

		jactorio::data::Item item{};
		try {
			iid.insert({&item, 4});
			FAIL();  // Did not call insert_func
		}
		catch (std::runtime_error& e) {
			const int result = strcmp(e.what(), "INSERT_FUNC");
			EXPECT_EQ(result, 0);  // Ensure exception is one I threw	
		}
	}
}
