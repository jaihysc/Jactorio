// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 04/07/2020

#include <gtest/gtest.h>

#include "data/prototype/entity/container_entity.h"
#include "data/prototype/entity/mining_drill.h"
#include "data/prototype/entity/transport/transport_belt.h"
#include "game/logic/item_logistics.h"

namespace game::logic
{
	TEST(item_logistics, insert_container_entity) {
		jactorio::game::World_data world_data{};
		world_data.add_chunk(new jactorio::game::Chunk(0, 0));

		// Deleted by chunk tile layer
		auto* container_data = new jactorio::data::Container_entity_data(10);

		world_data.get_tile_world_coords(3, 1)
		          ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity).unique_data = container_data;


		jactorio::data::Item item{};
		// Orientation is orientation from origin object
		jactorio::game::item_logistics::insert_container_entity({&item, 2},
		                                                        *container_data,
		                                                        jactorio::data::placementOrientation::down);

		// Inserted item
		EXPECT_EQ(container_data->inventory[0].first, &item);
		EXPECT_EQ(container_data->inventory[0].second, 2);
	}

	// Creates a transport line with orientation at (4, 4)
#define CREATE_TRANSPORT_LINE(orientation_)\
		jactorio::game::World_data world_data{};\
		world_data.add_chunk(new jactorio::game::Chunk(0, 0));\
		\
		auto* segment = new jactorio::game::Transport_line_segment{\
			jactorio::game::Transport_line_segment::orientation_,\
			jactorio::game::Transport_line_segment::terminationType::straight,\
			2\
		};\
		auto line_data = jactorio::data::Transport_line_data{*segment};\
		world_data.get_tile_world_coords(4, 4)->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity).unique_data = segment

#define TRANSPORT_LINE_INSERT(orientation_)\
			jactorio::data::Item item{};\
			jactorio::game::item_logistics::insert_transport_belt({&item, 1},\
																  line_data,\
																  jactorio::data::orientation_)

	TEST(item_logistics, insert_transport_line_up) {
		{
			CREATE_TRANSPORT_LINE(moveDir::up);

			TRANSPORT_LINE_INSERT(placementOrientation::up);
			EXPECT_EQ(segment->right.size(), 1);
		}
		{
			CREATE_TRANSPORT_LINE(moveDir::up);

			TRANSPORT_LINE_INSERT(placementOrientation::right);
			EXPECT_EQ(segment->left.size(), 1);
		}
		{
			CREATE_TRANSPORT_LINE(moveDir::up);

			TRANSPORT_LINE_INSERT(placementOrientation::down);
			EXPECT_EQ(segment->left.size(), 0);
			EXPECT_EQ(segment->right.size(), 0);
		}
		{
			CREATE_TRANSPORT_LINE(moveDir::up);

			TRANSPORT_LINE_INSERT(placementOrientation::left);
			EXPECT_EQ(segment->right.size(), 1);
		}
	}

	TEST(item_logistics, insert_transport_line_right) {
		{
			CREATE_TRANSPORT_LINE(moveDir::right);

			TRANSPORT_LINE_INSERT(placementOrientation::up);
			EXPECT_EQ(segment->right.size(), 1);
		}
		{
			CREATE_TRANSPORT_LINE(moveDir::right);

			TRANSPORT_LINE_INSERT(placementOrientation::right);
			EXPECT_EQ(segment->right.size(), 1);
		}
		{
			CREATE_TRANSPORT_LINE(moveDir::right);

			TRANSPORT_LINE_INSERT(placementOrientation::down);
			EXPECT_EQ(segment->left.size(), 1);
		}
		{
			CREATE_TRANSPORT_LINE(moveDir::right);

			TRANSPORT_LINE_INSERT(placementOrientation::left);
			EXPECT_EQ(segment->left.size(), 0);
			EXPECT_EQ(segment->right.size(), 0);
		}
	}

	TEST(item_logistics, insert_transport_line_down) {
		{
			CREATE_TRANSPORT_LINE(moveDir::down);

			TRANSPORT_LINE_INSERT(placementOrientation::up);
			EXPECT_EQ(segment->left.size(), 0);
			EXPECT_EQ(segment->right.size(), 0);
		}
		{
			CREATE_TRANSPORT_LINE(moveDir::down);

			TRANSPORT_LINE_INSERT(placementOrientation::right);
			EXPECT_EQ(segment->right.size(), 1);
		}
		{
			CREATE_TRANSPORT_LINE(moveDir::down);

			TRANSPORT_LINE_INSERT(placementOrientation::down);
			EXPECT_EQ(segment->right.size(), 1);
		}
		{
			CREATE_TRANSPORT_LINE(moveDir::down);

			TRANSPORT_LINE_INSERT(placementOrientation::left);
			EXPECT_EQ(segment->left.size(), 1);
		}
	}

	TEST(item_logistics, insert_transport_line_left) {
		{
			CREATE_TRANSPORT_LINE(moveDir::left);

			TRANSPORT_LINE_INSERT(placementOrientation::up);
			EXPECT_EQ(segment->left.size(), 1);
		}
		{
			CREATE_TRANSPORT_LINE(moveDir::left);

			TRANSPORT_LINE_INSERT(placementOrientation::right);
			EXPECT_EQ(segment->left.size(), 0);
			EXPECT_EQ(segment->right.size(), 0);
		}
		{
			CREATE_TRANSPORT_LINE(moveDir::left);

			TRANSPORT_LINE_INSERT(placementOrientation::down);
			EXPECT_EQ(segment->right.size(), 1);
		}
		{
			CREATE_TRANSPORT_LINE(moveDir::left);

			TRANSPORT_LINE_INSERT(placementOrientation::left);
			EXPECT_EQ(segment->right.size(), 1);
		}
	}
#undef CREATE_TRANSPORT_LINE
#undef TRANSPORT_LINE_INSERT
	// ======================================================================

	TEST(item_logistics, can_accept_item) {
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
			world_data.get_tile_world_coords(2, 4)
			          ->set_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity, &belt);

			auto* ptr = jactorio::game::item_logistics::can_accept_item(world_data, 2, 4);
			EXPECT_EQ(ptr, &jactorio::game::item_logistics::insert_transport_belt);
		}

		// Mining drill cannot be inserted into 
		{
			jactorio::data::Mining_drill drill{};
			world_data.get_tile_world_coords(2, 4)
			          ->set_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity, &drill);

			auto* ptr = jactorio::game::item_logistics::can_accept_item(world_data, 2, 4);
			EXPECT_EQ(ptr, nullptr);
		}

		// Container can be inserted into
		{
			jactorio::data::Container_entity container{};
			world_data.get_tile_world_coords(2, 4)
			          ->set_entity_prototype(jactorio::game::Chunk_tile::chunkLayer::entity, &container);

			auto* ptr = jactorio::game::item_logistics::can_accept_item(world_data, 2, 4);
			EXPECT_EQ(ptr, &jactorio::game::item_logistics::insert_container_entity);
		}

	}

	TEST(item_logistics, insert_item_destination) {
		// Item_insert_destination with custom insertion function
		// .insert() should call insertion function, throwing the exception
		const jactorio::game::Item_insert_destination::insert_func func = [
			](auto& item, auto& unique_data, auto orientation) -> bool {
			EXPECT_EQ(orientation, jactorio::data::placementOrientation::up);

			throw std::runtime_error("INSERT_FUNC");
		};

		jactorio::data::Health_entity_data unique_data{};
		const jactorio::game::Item_insert_destination iid{unique_data, func, jactorio::data::placementOrientation::up};

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
