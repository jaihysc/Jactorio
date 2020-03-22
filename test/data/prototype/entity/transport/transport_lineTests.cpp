// 
// transport_lineTests.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 03/22/2020
// Last modified: 03/22/2020
// 

#include <gtest/gtest.h>

#include "data/prototype/entity/transport/transport_belt.h"
#include "data/prototype/entity/transport/transport_line.h"

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

	// ======================================================================
	// Tests for the various bend orientations

	///
	/// \brief Creates the requisite structures and datatypes for the macros below
#define TRANSPORT_LINE_TEST_HEAD\
		jactorio::data::Transport_belt line_proto{};\
		jactorio::game::World_data world_data{};\
		world_data.add_chunk(new jactorio::game::Chunk{0, 0, nullptr});


	/// Creates a transport line with the provided orientation above/right/below/left of 1, 1
#define ADD_TOP_TRANSPORT_LINE(orientation)\
	{\
		auto& layer = world_data.get_tile_world_coords(1, 0)->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);\
		\
		layer.prototype_data = &line_proto;\
		auto* data = new jactorio::data::Transport_line_data();\
		data->set_orientation(jactorio::data::Transport_line_data::orientation);\
		layer.unique_data = data;\
	}
#define ADD_RIGHT_TRANSPORT_LINE(orientation)\
	{\
		auto& layer = world_data.get_tile_world_coords(2, 1)->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);\
		\
		layer.prototype_data = &line_proto;\
		auto* data = new jactorio::data::Transport_line_data();\
		data->set_orientation(jactorio::data::Transport_line_data::orientation);\
		layer.unique_data = data;\
	}
#define ADD_BOTTOM_TRANSPORT_LINE(orientation)\
	{\
		auto& layer = world_data.get_tile_world_coords(1, 2)->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);\
		\
		layer.prototype_data = &line_proto;\
		auto* data = new jactorio::data::Transport_line_data();\
		data->set_orientation(jactorio::data::Transport_line_data::orientation);\
		layer.unique_data = data;\
	}
#define ADD_LEFT_TRANSPORT_LINE(orientation)\
	{\
		auto& layer = world_data.get_tile_world_coords(0, 1)->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);\
		\
		layer.prototype_data = &line_proto;\
		auto* data = new jactorio::data::Transport_line_data();\
		data->set_orientation(jactorio::data::Transport_line_data::orientation);\
		layer.unique_data = data;\
	}

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
}
