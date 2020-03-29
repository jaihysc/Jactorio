// 
// transport_lineTests.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 03/22/2020
// Last modified: 03/29/2020
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
	\
	auto segment = jactorio::game::Transport_line_segment{\
		jactorio::game::Transport_line_segment::moveDir::left,\
		jactorio::game::Transport_line_segment::terminationType::straight,\
		1\
	};\
	auto* data = new jactorio::data::Transport_line_data(segment);\
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

		// Arbitrary segment is fine since no logic updates are performed
		auto segment = jactorio::game::Transport_line_segment{
			jactorio::game::Transport_line_segment::moveDir::left,
			jactorio::game::Transport_line_segment::terminationType::straight,
			1
		};

		jactorio::data::Transport_line_data line_data{segment};

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

	void create_transport_line(jactorio::game::World_data& world_data, jactorio::data::Entity& proto,
	                           const std::pair<uint32_t, uint32_t> world_coords,
	                           const jactorio::data::placementOrientation orientation) {
		auto& layer = world_data.get_tile_world_coords(world_coords.first, world_coords.second)
		                        ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
		layer.prototype_data = &proto;
		proto.on_build(world_data, world_coords, layer, 0, orientation);
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

	TEST(transport_line, on_build_create_transport_line_segment) {
		// Should create a transport line segment and add its chunk to logic chunks
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

	TEST(transport_line, on_build_connect_transport_line_segments) {
		// A transport line pointing to another one will set the target_segment
		jactorio::game::World_data world_data{};
		world_data.add_chunk(new jactorio::game::Chunk{0, 0, nullptr});

		auto proto = jactorio::data::Transport_belt{};
		{
			auto& layer = world_data.get_tile_world_coords(0, 0)
			                        ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &proto;
			proto.on_build(world_data, {0, 0}, layer, 0, jactorio::data::placementOrientation::up);
		}
		{
			// Second transport line should connect to first
			auto& layer = world_data.get_tile_world_coords(1, 0)
			                        ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &proto;
			proto.on_build(world_data, {1, 0}, layer, 0, jactorio::data::placementOrientation::left);
		}

		auto& logic_chunk = world_data.logic_get_all_chunks().at(world_data.get_chunk(0, 0));
		auto& transport_lines =
			logic_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		ASSERT_EQ(transport_lines.size(), 2);
		auto* line = static_cast<jactorio::game::Transport_line_segment*>(transport_lines[1].unique_data);
		EXPECT_EQ(line->target_segment, transport_lines[0].unique_data);
	}

	TEST(transport_line, on_build_connect_transport_line_segments_2) {
		// A transport line placed infront of another one will set the target_segment of the neighbor 
		jactorio::game::World_data world_data{};
		world_data.add_chunk(new jactorio::game::Chunk{0, 0, nullptr});

		auto proto = jactorio::data::Transport_belt{};
		{
			auto& layer = world_data.get_tile_world_coords(1, 0)
			                        ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &proto;
			proto.on_build(world_data, {1, 0}, layer, 0, jactorio::data::placementOrientation::left);
		}
		{
			auto& layer = world_data.get_tile_world_coords(0, 0)
			                        ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &proto;
			proto.on_build(world_data, {0, 0}, layer, 0, jactorio::data::placementOrientation::up);
		}

		auto& logic_chunk = world_data.logic_get_all_chunks().at(world_data.get_chunk(0, 0));
		auto& transport_lines =
			logic_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		ASSERT_EQ(transport_lines.size(), 2);
		auto* line = static_cast<jactorio::game::Transport_line_segment*>(transport_lines[0].unique_data);
		EXPECT_EQ(line->target_segment, transport_lines[1].unique_data);
	}

	TEST(transport_line, on_build_no_connect_transport_line_segments) {
		// Do not connect transport line segments pointed at each other
		jactorio::game::World_data world_data{};
		world_data.add_chunk(new jactorio::game::Chunk{0, 0, nullptr});

		auto proto = jactorio::data::Transport_belt{};
		{
			auto& layer = world_data.get_tile_world_coords(0, 0)
			                        ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &proto;
			proto.on_build(world_data, {0, 0}, layer, 0, jactorio::data::placementOrientation::down);
		}
		{
			auto& layer = world_data.get_tile_world_coords(0, 1)
			                        ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &proto;
			proto.on_build(world_data, {0, 1}, layer, 0, jactorio::data::placementOrientation::up);
		}

		auto& logic_chunk = world_data.logic_get_all_chunks().at(world_data.get_chunk(0, 0));
		auto& transport_lines =
			logic_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		ASSERT_EQ(transport_lines.size(), 2);

		auto* line = static_cast<jactorio::game::Transport_line_segment*>(transport_lines[0].unique_data);
		EXPECT_EQ(line->target_segment, nullptr);

		auto* line_b = static_cast<jactorio::game::Transport_line_segment*>(transport_lines[1].unique_data);
		EXPECT_EQ(line_b->target_segment, nullptr);
	}

	TEST(transport_line, on_build_connect_transport_line_segments_cross_chunks) {
		// A transport line pointing to another one will set the target_segment
		jactorio::game::World_data world_data{};
		world_data.add_chunk(new jactorio::game::Chunk{-1, 0, nullptr});
		world_data.add_chunk(new jactorio::game::Chunk{0, 0, nullptr});

		auto proto = jactorio::data::Transport_belt{};
		{
			auto& layer = world_data.get_tile_world_coords(-1, 0)
			                        ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &proto;
			proto.on_build(world_data, {-1, 0}, layer, 0, jactorio::data::placementOrientation::left);
		}
		{
			// Second transport line should connect to first
			auto& layer = world_data.get_tile_world_coords(0, 0)
			                        ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &proto;
			proto.on_build(world_data, {0, 0}, layer, 0, jactorio::data::placementOrientation::left);
		}


		auto& transport_lines_l =
			world_data.logic_get_all_chunks().at(world_data.get_chunk(-1, 0))
			          .get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		auto& transport_lines_r =
			world_data.logic_get_all_chunks().at(world_data.get_chunk(0, 0))
			          .get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		ASSERT_EQ(transport_lines_l.size(), 1);
		ASSERT_EQ(transport_lines_r.size(), 1);
		auto* line_l = static_cast<jactorio::game::Transport_line_segment*>(transport_lines_l[0].unique_data);
		auto* line_r = static_cast<jactorio::game::Transport_line_segment*>(transport_lines_r[0].unique_data);

		EXPECT_EQ(line_r->target_segment, line_l);
	}

	TEST(transport_line, on_build_bending_transport_line_segment) {
		// Change the transport_line_segment termination type in accordance with orientation when placed behind existing line
		jactorio::game::World_data world_data{};
		world_data.add_chunk(new jactorio::game::Chunk{0, 0, nullptr});

		auto proto = jactorio::data::Transport_belt{};
		{
			auto& layer = world_data.get_tile_world_coords(0, 0)
			                        ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &proto;
			proto.on_build(world_data, {0, 0}, layer, 0, jactorio::data::placementOrientation::down);
		}
		{
			auto& layer = world_data.get_tile_world_coords(1, 0)
			                        ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &proto;
			proto.on_build(world_data, {1, 0}, layer, 0, jactorio::data::placementOrientation::left);
		}


		auto& logic_chunk = world_data.logic_get_all_chunks().at(world_data.get_chunk(0, 0));
		auto& transport_lines =
			logic_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		ASSERT_EQ(transport_lines.size(), 2);

		jactorio::game::Chunk_struct_layer& line_layer = transport_lines[1];

		auto* line = static_cast<jactorio::game::Transport_line_segment*>(line_layer.unique_data);
		EXPECT_EQ(line->termination_type, jactorio::game::Transport_line_segment::terminationType::bend_left);

		// Should have lengthened segment and moved x 1 left
		EXPECT_EQ(line->segment_length, 2);
		EXPECT_FLOAT_EQ(line_layer.position_x, 0.f);
		EXPECT_FLOAT_EQ(line_layer.position_y, 0.f);
	}

	TEST(transport_line, on_build_bending_transport_line_segment_2) {
		// Change the transport_line_segment termination type in accordance with orientation when placed ahead of existing line 
		jactorio::game::World_data world_data{};
		world_data.add_chunk(new jactorio::game::Chunk{0, 0, nullptr});

		auto proto = jactorio::data::Transport_belt{};
		{
			auto& layer = world_data.get_tile_world_coords(1, 0)
			                        ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &proto;
			proto.on_build(world_data, {1, 0}, layer, 0, jactorio::data::placementOrientation::left);
		}
		{
			auto& layer = world_data.get_tile_world_coords(0, 0)
			                        ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &proto;
			proto.on_build(world_data, {0, 0}, layer, 0, jactorio::data::placementOrientation::down);
		}


		auto& logic_chunk = world_data.logic_get_all_chunks().at(world_data.get_chunk(0, 0));
		auto& transport_lines =
			logic_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		ASSERT_EQ(transport_lines.size(), 2);

		jactorio::game::Chunk_struct_layer& line_layer = transport_lines[0];
		auto* line = static_cast<jactorio::game::Transport_line_segment*>(line_layer.unique_data);
		EXPECT_EQ(line->termination_type, jactorio::game::Transport_line_segment::terminationType::bend_left);

		// Should have lengthened segment and moved x 1 left
		EXPECT_EQ(line->segment_length, 2);
		EXPECT_FLOAT_EQ(line_layer.position_x, 0.f);
		EXPECT_FLOAT_EQ(line_layer.position_y, 0.f);
	}

	// Checks that transport_lines.size() == 5, 4th item is right_only, 5th is left_only,
	// Checks that both lines at at center_x, center_y
#define VALIDATE_SIDE_ONLY(center_x, center_y)\
		auto& logic_chunk = world_data.logic_get_all_chunks().at(world_data.get_chunk(0, 0));\
		auto& transport_lines =\
			logic_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);\
		\
		ASSERT_EQ(transport_lines.size(), 5);\
		\
		{\
			jactorio::game::Chunk_struct_layer& line_layer = transport_lines[3];\
			auto* line = static_cast<jactorio::game::Transport_line_segment*>(line_layer.unique_data);\
			\
			EXPECT_EQ(line->termination_type, jactorio::game::Transport_line_segment::terminationType::right_only);\
			EXPECT_EQ(line->segment_length, 2);\
			EXPECT_FLOAT_EQ(line_layer.position_x, center_x);\
			EXPECT_FLOAT_EQ(line_layer.position_y, center_y);\
		}\
		{\
			jactorio::game::Chunk_struct_layer& line_layer = transport_lines[4];\
			auto* line_2 = static_cast<jactorio::game::Transport_line_segment*>(line_layer.unique_data);\
			\
			EXPECT_EQ(line_2->termination_type, jactorio::game::Transport_line_segment::terminationType::left_only);\
			EXPECT_EQ(line_2->segment_length, 2);\
			EXPECT_FLOAT_EQ(line_layer.position_x, center_x);\
			EXPECT_FLOAT_EQ(line_layer.position_y, center_y);\
		}


	TEST(transport_line, on_build_up_side_only_transport_segment) {
		/*
		 *   ^
		 * > ^ < 
		 *   ^
		 */
		jactorio::game::World_data world_data{};
		world_data.add_chunk(new jactorio::game::Chunk{0, 0, nullptr});

		auto proto = jactorio::data::Transport_belt{};

		create_transport_line(world_data, proto, {1, 0}, jactorio::data::placementOrientation::up);
		create_transport_line(world_data, proto, {1, 1}, jactorio::data::placementOrientation::up);
		create_transport_line(world_data, proto, {1, 2}, jactorio::data::placementOrientation::up);

		create_transport_line(world_data, proto, {2, 1}, jactorio::data::placementOrientation::left);
		create_transport_line(world_data, proto, {0, 1}, jactorio::data::placementOrientation::right);

		VALIDATE_SIDE_ONLY(1, 1);
	}

	TEST(transport_line, on_build_right_side_only_transport_segment) {
		/*
		 *   v
		 * > > > 
		 *   ^
		 */
		jactorio::game::World_data world_data{};
		world_data.add_chunk(new jactorio::game::Chunk{0, 0, nullptr});

		auto proto = jactorio::data::Transport_belt{};

		create_transport_line(world_data, proto, {0, 1}, jactorio::data::placementOrientation::right);
		create_transport_line(world_data, proto, {1, 1}, jactorio::data::placementOrientation::right);
		create_transport_line(world_data, proto, {2, 1}, jactorio::data::placementOrientation::right);

		create_transport_line(world_data, proto, {1, 2}, jactorio::data::placementOrientation::up);
		create_transport_line(world_data, proto, {1, 0}, jactorio::data::placementOrientation::down);

		VALIDATE_SIDE_ONLY(1, 1);
	}

	TEST(transport_line, on_build_down_side_only_transport_segment) {
		/*
		 *   v
		 * > v < 
		 *   v 
		 */
		jactorio::game::World_data world_data{};
		world_data.add_chunk(new jactorio::game::Chunk{0, 0, nullptr});

		auto proto = jactorio::data::Transport_belt{};

		create_transport_line(world_data, proto, {1, 0}, jactorio::data::placementOrientation::down);
		create_transport_line(world_data, proto, {1, 1}, jactorio::data::placementOrientation::down);
		create_transport_line(world_data, proto, {1, 2}, jactorio::data::placementOrientation::down);

		create_transport_line(world_data, proto, {0, 1}, jactorio::data::placementOrientation::right);
		create_transport_line(world_data, proto, {2, 1}, jactorio::data::placementOrientation::left);

		VALIDATE_SIDE_ONLY(1, 1);
	}

	TEST(transport_line, on_build_left_side_only_transport_segment) {
		/*
		 *   v
		 * < < <
		 *   ^
		 */
		jactorio::game::World_data world_data{};
		world_data.add_chunk(new jactorio::game::Chunk{0, 0, nullptr});

		auto proto = jactorio::data::Transport_belt{};

		create_transport_line(world_data, proto, {0, 1}, jactorio::data::placementOrientation::left);
		create_transport_line(world_data, proto, {1, 1}, jactorio::data::placementOrientation::left);
		create_transport_line(world_data, proto, {2, 1}, jactorio::data::placementOrientation::left);

		create_transport_line(world_data, proto, {1, 0}, jactorio::data::placementOrientation::down);
		create_transport_line(world_data, proto, {1, 2}, jactorio::data::placementOrientation::up);

		VALIDATE_SIDE_ONLY(1, 1);
	}
#undef VALIDATE_SIDE_ONLY

	TEST(transport_line, transport_line_circle) {
		// Creates a circle of transport lines
		/*
		 * > v
		 * ^ <
		 */

		jactorio::game::World_data world_data{};
		world_data.add_chunk(new jactorio::game::Chunk{0, 0, nullptr});

		auto proto = jactorio::data::Transport_belt{};
		{
			auto& layer = world_data.get_tile_world_coords(0, 0)
			                        ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &proto;
			proto.on_build(world_data, {0, 0}, layer, 0, jactorio::data::placementOrientation::right);
		}
		{
			auto& layer = world_data.get_tile_world_coords(1, 0)
			                        ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &proto;
			proto.on_build(world_data, {1, 0}, layer, 0, jactorio::data::placementOrientation::down);
		}
		{
			auto& layer = world_data.get_tile_world_coords(1, 1)
			                        ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &proto;
			proto.on_build(world_data, {1, 1}, layer, 0, jactorio::data::placementOrientation::left);
		}
		{
			auto& layer = world_data.get_tile_world_coords(0, 1)
			                        ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &proto;
			proto.on_build(world_data, {0, 1}, layer, 0, jactorio::data::placementOrientation::up);
		}


		auto& logic_chunk = world_data.logic_get_all_chunks().at(world_data.get_chunk(0, 0));
		auto& transport_lines =
			logic_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		ASSERT_EQ(transport_lines.size(), 4);

		// Right
		{
			jactorio::game::Chunk_struct_layer& line_layer = transport_lines[0];
			auto* line = static_cast<jactorio::game::Transport_line_segment*>(line_layer.unique_data);
			EXPECT_EQ(line->termination_type, jactorio::game::Transport_line_segment::terminationType::bend_right);

			EXPECT_EQ(line->segment_length, 2);
			EXPECT_FLOAT_EQ(line_layer.position_x, 1.f);
			EXPECT_FLOAT_EQ(line_layer.position_y, 0.f);
		}
		// Down
		{
			jactorio::game::Chunk_struct_layer& line_layer = transport_lines[1];
			auto* line = static_cast<jactorio::game::Transport_line_segment*>(line_layer.unique_data);
			EXPECT_EQ(line->termination_type, jactorio::game::Transport_line_segment::terminationType::bend_right);

			EXPECT_EQ(line->segment_length, 2);
			EXPECT_FLOAT_EQ(line_layer.position_x, 1.f);
			EXPECT_FLOAT_EQ(line_layer.position_y, 1.f);
		}

		// Left
		{
			jactorio::game::Chunk_struct_layer& line_layer = transport_lines[2];
			auto* line = static_cast<jactorio::game::Transport_line_segment*>(line_layer.unique_data);
			EXPECT_EQ(line->termination_type, jactorio::game::Transport_line_segment::terminationType::bend_right);

			EXPECT_EQ(line->segment_length, 2);
			EXPECT_FLOAT_EQ(line_layer.position_x, 0.f);
			EXPECT_FLOAT_EQ(line_layer.position_y, 1.f);
		}

		// Up
		{
			jactorio::game::Chunk_struct_layer& line_layer = transport_lines[3];
			auto* line = static_cast<jactorio::game::Transport_line_segment*>(line_layer.unique_data);
			EXPECT_EQ(line->termination_type, jactorio::game::Transport_line_segment::terminationType::bend_right);

			EXPECT_EQ(line->segment_length, 2);
			EXPECT_FLOAT_EQ(line_layer.position_x, 0.f);
			EXPECT_FLOAT_EQ(line_layer.position_y, 0.f);
		}
	}
}
