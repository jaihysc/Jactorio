// 
// transport_lineTests.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 03/22/2020
// Last modified: 03/24/2020
// 

#include <gtest/gtest.h>

#include "data/prototype/entity/transport/transport_belt.h"
#include "data/prototype/entity/transport/transport_line.h"
#include "game/logic/transport_line_structure.h"

// ======================================================================
// Tests for the various bend orientations

///
/// \brief Creates the requisite structures and datatypes for the macros below
#define TRANSPORT_LINE_TEST_HEAD\
	jactorio::data::Transport_belt line_proto{};\
	jactorio::game::World_data world_data{};\
	world_data.add_chunk(new jactorio::game::Chunk{0, 0, nullptr});

#define ADD_TRANSPORT_LINE(orientation, x, y)\
{\
	auto& layer = world_data.get_tile_world_coords(x, y)->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);\
	\
	layer.prototype_data = &line_proto;\
	auto* data = new jactorio::data::Transport_line_data();\
	data->set_orientation(jactorio::data::Transport_line_data::orientation);\
	layer.unique_data = data;\
}


/// Creates a transport line with the provided orientation above/right/below/left of 1, 1
#define ADD_TOP_TRANSPORT_LINE(orientation)    ADD_TRANSPORT_LINE(orientation, 1, 0)
#define ADD_RIGHT_TRANSPORT_LINE(orientation)  ADD_TRANSPORT_LINE(orientation, 2, 1)
#define ADD_BOTTOM_TRANSPORT_LINE(orientation) ADD_TRANSPORT_LINE(orientation, 1, 2)
#define ADD_LEFT_TRANSPORT_LINE(orientation)   ADD_TRANSPORT_LINE(orientation, 0, 1)

///
/// \brief Validates that a tile at coords 1,1 with the placement orientation produces the expected line orientation
#define VALIDATE_RESULT_ORIENTATION(placement_orientation, expected_line_orientation)\
{\
	auto pair =\
		std::pair<uint16_t, uint16_t>{\
			static_cast<uint16_t>(jactorio::data::Transport_line_data::expected_line_orientation),\
			0};\
	EXPECT_EQ(\
		line_proto.map_placement_orientation(\
			jactorio::data::placement_orientation, world_data, {1, 1}),\
		pair\
	);\
}


namespace data
{
	TEST(transport_line, set_orientation) {
		// When the orientation is set, the member "set" should also be updated

		jactorio::data::Transport_line_data line_data{};

		line_data.set_orientation(jactorio::data::Transport_line_data::lineOrientation::down);
		EXPECT_EQ(line_data.set, static_cast<uint16_t>(jactorio::data::Transport_line_data::lineOrientation::down));

		line_data.set_orientation(jactorio::data::Transport_line_data::lineOrientation::left_down);
		EXPECT_EQ(line_data.set, static_cast<uint16_t>(jactorio::data::Transport_line_data::lineOrientation::left_down));
	}


	TEST(transport_line, orientation_up) {
		{
			TRANSPORT_LINE_TEST_HEAD
			ADD_LEFT_TRANSPORT_LINE(lineOrientation::right);
			VALIDATE_RESULT_ORIENTATION(placementOrientation::up, lineOrientation::right_up);
		}
		{
			TRANSPORT_LINE_TEST_HEAD
			ADD_RIGHT_TRANSPORT_LINE(lineOrientation::left);
			VALIDATE_RESULT_ORIENTATION(placementOrientation::up, lineOrientation::left_up);
		}
		{
			// Top and bottom points to one line, line should be straight
			TRANSPORT_LINE_TEST_HEAD
			ADD_LEFT_TRANSPORT_LINE(lineOrientation::right);
			ADD_RIGHT_TRANSPORT_LINE(lineOrientation::left);
			VALIDATE_RESULT_ORIENTATION(placementOrientation::up, lineOrientation::up);
		}
		{
			TRANSPORT_LINE_TEST_HEAD
			ADD_LEFT_TRANSPORT_LINE(lineOrientation::right);
			ADD_BOTTOM_TRANSPORT_LINE(lineOrientation::right_down);
			VALIDATE_RESULT_ORIENTATION(placementOrientation::up, lineOrientation::right_up);
		}
		{
			TRANSPORT_LINE_TEST_HEAD
			ADD_LEFT_TRANSPORT_LINE(lineOrientation::right);
			ADD_BOTTOM_TRANSPORT_LINE(lineOrientation::up);
			VALIDATE_RESULT_ORIENTATION(placementOrientation::up, lineOrientation::up);
		}

		{
			/*
			 * < ^ <
			 */
			TRANSPORT_LINE_TEST_HEAD
			ADD_LEFT_TRANSPORT_LINE(lineOrientation::left);
			ADD_RIGHT_TRANSPORT_LINE(lineOrientation::left);
			VALIDATE_RESULT_ORIENTATION(placementOrientation::up, lineOrientation::left_up);
		}
	}

	TEST(transport_line, orientation_right) {
		{
			TRANSPORT_LINE_TEST_HEAD
			ADD_TOP_TRANSPORT_LINE(lineOrientation::down);
			VALIDATE_RESULT_ORIENTATION(placementOrientation::right, lineOrientation::down_right);
		}
		{
			TRANSPORT_LINE_TEST_HEAD
			ADD_BOTTOM_TRANSPORT_LINE(lineOrientation::up);
			VALIDATE_RESULT_ORIENTATION(placementOrientation::right, lineOrientation::up_right);
		}
		{
			// Top and bottom points to one line, line should be straight
			TRANSPORT_LINE_TEST_HEAD
			ADD_TOP_TRANSPORT_LINE(lineOrientation::down);
			ADD_BOTTOM_TRANSPORT_LINE(lineOrientation::up);
			VALIDATE_RESULT_ORIENTATION(placementOrientation::right, lineOrientation::right);
		}
		{
			TRANSPORT_LINE_TEST_HEAD
			ADD_TOP_TRANSPORT_LINE(lineOrientation::down);
			ADD_LEFT_TRANSPORT_LINE(lineOrientation::up);  // Does not point to center
			VALIDATE_RESULT_ORIENTATION(placementOrientation::right, lineOrientation::down_right);
		}
		{
			TRANSPORT_LINE_TEST_HEAD
			ADD_TOP_TRANSPORT_LINE(lineOrientation::down);
			ADD_LEFT_TRANSPORT_LINE(lineOrientation::right);  // Points at center, center now straight 
			VALIDATE_RESULT_ORIENTATION(placementOrientation::right, lineOrientation::right);
		}

		{
			TRANSPORT_LINE_TEST_HEAD
			ADD_TOP_TRANSPORT_LINE(lineOrientation::up);
			ADD_BOTTOM_TRANSPORT_LINE(lineOrientation::up);
			VALIDATE_RESULT_ORIENTATION(placementOrientation::right, lineOrientation::up_right);
		}
	}

	TEST(transport_line, orientation_down) {
		{
			TRANSPORT_LINE_TEST_HEAD
			ADD_LEFT_TRANSPORT_LINE(lineOrientation::right);
			VALIDATE_RESULT_ORIENTATION(placementOrientation::down, lineOrientation::right_down);
		}
		{
			TRANSPORT_LINE_TEST_HEAD
			ADD_RIGHT_TRANSPORT_LINE(lineOrientation::left);
			VALIDATE_RESULT_ORIENTATION(placementOrientation::down, lineOrientation::left_down);
		}
		{
			// Top and bottom points to one line, line should be straight
			TRANSPORT_LINE_TEST_HEAD
			ADD_LEFT_TRANSPORT_LINE(lineOrientation::right);
			ADD_RIGHT_TRANSPORT_LINE(lineOrientation::left);
			VALIDATE_RESULT_ORIENTATION(placementOrientation::down, lineOrientation::down);
		}
		{
			TRANSPORT_LINE_TEST_HEAD
			ADD_LEFT_TRANSPORT_LINE(lineOrientation::right);
			ADD_TOP_TRANSPORT_LINE(lineOrientation::left);
			VALIDATE_RESULT_ORIENTATION(placementOrientation::down, lineOrientation::right_down);
		}
		{
			TRANSPORT_LINE_TEST_HEAD
			ADD_LEFT_TRANSPORT_LINE(lineOrientation::right);
			ADD_TOP_TRANSPORT_LINE(lineOrientation::right_down);
			VALIDATE_RESULT_ORIENTATION(placementOrientation::down, lineOrientation::down);
		}

		{
			TRANSPORT_LINE_TEST_HEAD
			ADD_LEFT_TRANSPORT_LINE(lineOrientation::left);
			ADD_RIGHT_TRANSPORT_LINE(lineOrientation::left);
			VALIDATE_RESULT_ORIENTATION(placementOrientation::down, lineOrientation::left_down);
		}
	}

	TEST(transport_line, orientation_left) {
		{
			TRANSPORT_LINE_TEST_HEAD
			ADD_TOP_TRANSPORT_LINE(lineOrientation::down);
			VALIDATE_RESULT_ORIENTATION(placementOrientation::left, lineOrientation::down_left);
		}
		{
			TRANSPORT_LINE_TEST_HEAD
			ADD_BOTTOM_TRANSPORT_LINE(lineOrientation::up);
			VALIDATE_RESULT_ORIENTATION(placementOrientation::left, lineOrientation::up_left);
		}
		{
			TRANSPORT_LINE_TEST_HEAD
			ADD_TOP_TRANSPORT_LINE(lineOrientation::down);
			ADD_BOTTOM_TRANSPORT_LINE(lineOrientation::up);
			VALIDATE_RESULT_ORIENTATION(placementOrientation::left, lineOrientation::left);
		}
		{
			TRANSPORT_LINE_TEST_HEAD
			ADD_TOP_TRANSPORT_LINE(lineOrientation::down);
			ADD_RIGHT_TRANSPORT_LINE(lineOrientation::up);
			VALIDATE_RESULT_ORIENTATION(placementOrientation::left, lineOrientation::down_left);
		}
		{
			TRANSPORT_LINE_TEST_HEAD
			ADD_TOP_TRANSPORT_LINE(lineOrientation::down);
			ADD_RIGHT_TRANSPORT_LINE(lineOrientation::left);
			VALIDATE_RESULT_ORIENTATION(placementOrientation::left, lineOrientation::left);
		}

		{
			TRANSPORT_LINE_TEST_HEAD
			ADD_TOP_TRANSPORT_LINE(lineOrientation::up);
			ADD_BOTTOM_TRANSPORT_LINE(lineOrientation::up);
			VALIDATE_RESULT_ORIENTATION(placementOrientation::left, lineOrientation::up_left);
		}
	}

	TEST(transport_line, on_build_update_neighboring_lines) {
		TRANSPORT_LINE_TEST_HEAD
		/*
		 * >
		 * ^
		 */
		ADD_TOP_TRANSPORT_LINE(lineOrientation::right);

		auto& layer = world_data.get_tile_world_coords(1, 1)
		                        ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);


		auto proto = jactorio::data::Transport_belt{};
		layer.prototype_data = &proto;


		// Should update line above, turn right to a up-right
		line_proto.on_build(world_data, {1, 1}, layer, 0,
		                    jactorio::data::placementOrientation::up);

		{
			auto& result_layer = world_data.get_tile_world_coords(1, 0)
			                               ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);

			EXPECT_EQ(
				static_cast<jactorio::data::Transport_line_data*>(result_layer.unique_data)->orientation,
				jactorio::data::Transport_line_data::lineOrientation::up_right
			);
		}
	}

	TEST(transport_line, on_remove_update_neighboring_lines) {
		// The on_remove event should update the orientations of the neighboring belts to if the current transport
		// line is not there
		TRANSPORT_LINE_TEST_HEAD
		/*
		 *  v
		 *  >
		 *  ^
		 */
		ADD_TOP_TRANSPORT_LINE(lineOrientation::down);
		ADD_BOTTOM_TRANSPORT_LINE(lineOrientation::up)
		ADD_TRANSPORT_LINE(lineOrientation::right, 1, 1);  // Between the 2 above and below

		auto& layer = world_data.get_tile_world_coords(1, 2)
		                        ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);

		auto proto = jactorio::data::Transport_belt{};
		layer.prototype_data = &proto;


		// Removing the bottom line makes the center one bend down-right
		line_proto.on_remove(world_data, {1, 2}, layer);

		{
			auto& result_layer = world_data.get_tile_world_coords(1, 1)
			                               ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);

			EXPECT_EQ(
				static_cast<jactorio::data::Transport_line_data*>(result_layer.unique_data)->orientation,
				jactorio::data::Transport_line_data::lineOrientation::down_right
			);
		}
	}

	TEST(transport_line, on_build_create_transport_line_structure) {
		// Should create a transport line structure and add its chunk to logic chunks
		jactorio::game::World_data world_data{};
		world_data.add_chunk(new jactorio::game::Chunk{-1, 0, nullptr});

		auto& layer = world_data.get_tile_world_coords(-5, 0)
		                        ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);

		auto proto = jactorio::data::Transport_belt{};
		layer.prototype_data = &proto;

		proto.on_build(world_data, {-5, 0}, layer, 0, jactorio::data::placementOrientation::right);

		// Added current chunk as a logic chunk
		ASSERT_EQ(world_data.logic_get_all_chunks().size(), 1);

		auto& logic_chunk = world_data.logic_get_all_chunks().at(world_data.get_chunk(-1, 0));
		EXPECT_EQ(logic_chunk.chunk, world_data.get_chunk(-1, 0));


		auto& transport_lines =
			logic_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		// Should have created a transport line structure
		ASSERT_EQ(transport_lines.size(), 1);
		ASSERT_TRUE(dynamic_cast<jactorio::game::Transport_line_segment*>(transport_lines.front().unique_data));

		auto* line_data = dynamic_cast<jactorio::game::Transport_line_segment*>(transport_lines.front().unique_data);
		EXPECT_EQ(line_data->direction, jactorio::game::Transport_line_segment::moveDir::right);
		EXPECT_EQ(line_data->termination_type, jactorio::game::Transport_line_segment::terminationType::straight);
		EXPECT_EQ(line_data->segment_length, 1);

		// Position_x / position_y is the distance from the top left of the chunk
		EXPECT_EQ(transport_lines.front().position_x, 27);
		EXPECT_EQ(transport_lines.front().position_y, 0);
	}
}
