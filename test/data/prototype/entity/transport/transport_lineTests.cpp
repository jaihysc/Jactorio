// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#include <gtest/gtest.h>

#include "data/prototype/entity/transport/transport_belt.h"
#include "data/prototype/entity/transport/transport_line.h"
#include "game/logic/transport_line_structure.h"

// ======================================================================
// Tests for the various bend orientations

namespace data::prototype
{
	class TransportLineTest : public testing::Test
	{
	protected:
		jactorio::game::World_data world_data_{};
		jactorio::data::Transport_belt line_proto_{};

		void SetUp() override {
			world_data_.add_chunk(new jactorio::game::Chunk{0, 0});
		}


		void add_transport_line(const jactorio::data::Transport_line_data::lineOrientation orientation,
		                        const jactorio::game::World_data::world_coord x,
		                        const jactorio::game::World_data::world_coord y) const {
			auto& layer = world_data_.get_tile_world_coords(x, y)->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &line_proto_;

			auto segment = jactorio::game::Transport_line_segment{
				jactorio::game::Transport_line_segment::moveDir::left,
				jactorio::game::Transport_line_segment::terminationType::straight,
				1
			};
			auto* data = new jactorio::data::Transport_line_data(segment);
			data->set_orientation(orientation);

			layer.unique_data = data;
		}

		/// Creates a transport line with the provided orientation above/right/below/left of 1, 1
		void add_top_transport_line(const jactorio::data::Transport_line_data::lineOrientation orientation) const {
			add_transport_line(orientation, 1, 0);
		}

		void add_right_transport_line(const jactorio::data::Transport_line_data::lineOrientation orientation) const {
			add_transport_line(orientation, 2, 1);
		}

		void add_bottom_transport_line(const jactorio::data::Transport_line_data::lineOrientation orientation) const {
			add_transport_line(orientation, 1, 2);
		}

		void add_left_transport_line(const jactorio::data::Transport_line_data::lineOrientation orientation) const {
			add_transport_line(orientation, 0, 1);
		}

		///
		/// \brief Validates that a tile at coords 1,1 with the placement orientation produces the expected line orientation
		void validate_result_orientation(const jactorio::data::placementOrientation placement_orientation,
		                                 jactorio::data::Transport_line_data::lineOrientation expected_line_orientation) {
			const auto pair = std::pair<uint16_t, uint16_t>{static_cast<uint16_t>(expected_line_orientation), 0};
			EXPECT_EQ(line_proto_.map_placement_orientation(placement_orientation, world_data_, {1, 1}), pair);
		}
	};

	// ======================================================================
	// Placement logic for the various orientations and bends

	TEST_F(TransportLineTest, SetOrientation) {
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


	TEST_F(TransportLineTest, OrientationUp1) {
		/*
		 * > ^
		 */
		add_left_transport_line(jactorio::data::Transport_line_data::lineOrientation::right);
		validate_result_orientation(jactorio::data::placementOrientation::up,
		                            jactorio::data::Transport_line_data::lineOrientation::right_up);
	}

	TEST_F(TransportLineTest, OrientationUp2) {
		/*
		 *   ^ <
		 */
		add_right_transport_line(jactorio::data::Transport_line_data::lineOrientation::left);
		validate_result_orientation(jactorio::data::placementOrientation::up,
		                            jactorio::data::Transport_line_data::lineOrientation::left_up);
	}

	TEST_F(TransportLineTest, OrientationUp3) {
		/*
		 * > ^ <
		 */
		// Top and bottom points to one line, line should be straight

		add_left_transport_line(jactorio::data::Transport_line_data::lineOrientation::right);
		add_right_transport_line(jactorio::data::Transport_line_data::lineOrientation::left);
		validate_result_orientation(jactorio::data::placementOrientation::up,
		                            jactorio::data::Transport_line_data::lineOrientation::up);
	}

	TEST_F(TransportLineTest, OrientationUp4) {
		/*
		 * > ^ 
		 *   >
		 */
		add_left_transport_line(jactorio::data::Transport_line_data::lineOrientation::right);
		add_bottom_transport_line(jactorio::data::Transport_line_data::lineOrientation::right_down);
		validate_result_orientation(jactorio::data::placementOrientation::up,
		                            jactorio::data::Transport_line_data::lineOrientation::right_up);
	}

	TEST_F(TransportLineTest, OrientationUp5) {
		/*
		 * > ^ 
		 *   ^
		 */
		add_left_transport_line(jactorio::data::Transport_line_data::lineOrientation::right);
		add_bottom_transport_line(jactorio::data::Transport_line_data::lineOrientation::up);
		validate_result_orientation(jactorio::data::placementOrientation::up,
		                            jactorio::data::Transport_line_data::lineOrientation::up);
	}

	TEST_F(TransportLineTest, OrientationUp6) {
		/*
		 * < ^ <
		 */

		add_left_transport_line(jactorio::data::Transport_line_data::lineOrientation::left);
		add_right_transport_line(jactorio::data::Transport_line_data::lineOrientation::left);
		validate_result_orientation(jactorio::data::placementOrientation::up,
		                            jactorio::data::Transport_line_data::lineOrientation::left_up);
	}

	// ===

	TEST_F(TransportLineTest, OrientationRight1) {
		/*
		 *  v
		 *  >
		 */
		add_top_transport_line(jactorio::data::Transport_line_data::lineOrientation::down);
		validate_result_orientation(jactorio::data::placementOrientation::right,
		                            jactorio::data::Transport_line_data::lineOrientation::down_right);
	}

	TEST_F(TransportLineTest, OrientationRight2) {
		/*
		 * >
		 * ^
		 */
		add_bottom_transport_line(jactorio::data::Transport_line_data::lineOrientation::up);
		validate_result_orientation(jactorio::data::placementOrientation::right,
		                            jactorio::data::Transport_line_data::lineOrientation::up_right);
	}

	TEST_F(TransportLineTest, OrientationRight3) {
		// Top and bottom points to one line, line should be straight

		add_top_transport_line(jactorio::data::Transport_line_data::lineOrientation::down);
		add_bottom_transport_line(jactorio::data::Transport_line_data::lineOrientation::up);
		validate_result_orientation(jactorio::data::placementOrientation::right,
		                            jactorio::data::Transport_line_data::lineOrientation::right);
	}

	TEST_F(TransportLineTest, OrientationRight4) {
		/*
		 *   v
		 * ^ >
		 */
		add_top_transport_line(jactorio::data::Transport_line_data::lineOrientation::down);
		add_left_transport_line(jactorio::data::Transport_line_data::lineOrientation::up);  // Does not point to center
		validate_result_orientation(jactorio::data::placementOrientation::right,
		                            jactorio::data::Transport_line_data::lineOrientation::down_right);
	}

	TEST_F(TransportLineTest, OrientationRight5) {
		/*
		 *   v
		 * > >
		 */
		add_top_transport_line(jactorio::data::Transport_line_data::lineOrientation::down);
		add_left_transport_line(
			jactorio::data::Transport_line_data::lineOrientation::right);  // Points at center, center now straight 
		validate_result_orientation(jactorio::data::placementOrientation::right,
		                            jactorio::data::Transport_line_data::lineOrientation::right);
	}

	TEST_F(TransportLineTest, OrientationRight6) {
		/*
		 * ^
		 * >
		 * ^
		 */
		add_top_transport_line(jactorio::data::Transport_line_data::lineOrientation::up);
		add_bottom_transport_line(jactorio::data::Transport_line_data::lineOrientation::up);
		validate_result_orientation(jactorio::data::placementOrientation::right,
		                            jactorio::data::Transport_line_data::lineOrientation::up_right);
	}

	// ===

	TEST_F(TransportLineTest, OrientationDown1) {
		add_left_transport_line(jactorio::data::Transport_line_data::lineOrientation::right);
		validate_result_orientation(jactorio::data::placementOrientation::down,
		                            jactorio::data::Transport_line_data::lineOrientation::right_down);
	}

	TEST_F(TransportLineTest, OrientationDown2) {
		add_right_transport_line(jactorio::data::Transport_line_data::lineOrientation::left);
		validate_result_orientation(jactorio::data::placementOrientation::down,
		                            jactorio::data::Transport_line_data::lineOrientation::left_down);
	}

	TEST_F(TransportLineTest, OrientationDown3) {
		// Top and bottom points to one line, line should be straight

		add_left_transport_line(jactorio::data::Transport_line_data::lineOrientation::right);
		add_right_transport_line(jactorio::data::Transport_line_data::lineOrientation::left);
		validate_result_orientation(jactorio::data::placementOrientation::down,
		                            jactorio::data::Transport_line_data::lineOrientation::down);
	}

	TEST_F(TransportLineTest, OrientationDown4) {
		add_left_transport_line(jactorio::data::Transport_line_data::lineOrientation::right);
		add_top_transport_line(jactorio::data::Transport_line_data::lineOrientation::left);
		validate_result_orientation(jactorio::data::placementOrientation::down,
		                            jactorio::data::Transport_line_data::lineOrientation::right_down);
	}

	TEST_F(TransportLineTest, OrientationDown5) {
		add_left_transport_line(jactorio::data::Transport_line_data::lineOrientation::right);
		add_top_transport_line(jactorio::data::Transport_line_data::lineOrientation::right_down);
		validate_result_orientation(jactorio::data::placementOrientation::down,
		                            jactorio::data::Transport_line_data::lineOrientation::down);
	}

	TEST_F(TransportLineTest, OrientationDown6) {
		add_left_transport_line(jactorio::data::Transport_line_data::lineOrientation::left);
		add_right_transport_line(jactorio::data::Transport_line_data::lineOrientation::left);
		validate_result_orientation(jactorio::data::placementOrientation::down,
		                            jactorio::data::Transport_line_data::lineOrientation::left_down);
	}

	// ===

	TEST_F(TransportLineTest, OrientationLeft1) {
		add_top_transport_line(jactorio::data::Transport_line_data::lineOrientation::down);
		validate_result_orientation(jactorio::data::placementOrientation::left,
		                            jactorio::data::Transport_line_data::lineOrientation::down_left);
	}

	TEST_F(TransportLineTest, OrientationLeft2) {
		add_bottom_transport_line(jactorio::data::Transport_line_data::lineOrientation::up);
		validate_result_orientation(jactorio::data::placementOrientation::left,
		                            jactorio::data::Transport_line_data::lineOrientation::up_left);
	}

	TEST_F(TransportLineTest, OrientationLeft3) {
		add_top_transport_line(jactorio::data::Transport_line_data::lineOrientation::down);
		add_bottom_transport_line(jactorio::data::Transport_line_data::lineOrientation::up);
		validate_result_orientation(jactorio::data::placementOrientation::left,
		                            jactorio::data::Transport_line_data::lineOrientation::left);
	}

	TEST_F(TransportLineTest, OrientationLeft4) {
		add_top_transport_line(jactorio::data::Transport_line_data::lineOrientation::down);
		add_right_transport_line(jactorio::data::Transport_line_data::lineOrientation::up);
		validate_result_orientation(jactorio::data::placementOrientation::left,
		                            jactorio::data::Transport_line_data::lineOrientation::down_left);
	}

	TEST_F(TransportLineTest, OrientationLeft5) {
		add_top_transport_line(jactorio::data::Transport_line_data::lineOrientation::down);
		add_right_transport_line(jactorio::data::Transport_line_data::lineOrientation::left);
		validate_result_orientation(jactorio::data::placementOrientation::left,
		                            jactorio::data::Transport_line_data::lineOrientation::left);
	}

	TEST_F(TransportLineTest, OrientationLeft6) {
		add_top_transport_line(jactorio::data::Transport_line_data::lineOrientation::up);
		add_bottom_transport_line(jactorio::data::Transport_line_data::lineOrientation::up);
		validate_result_orientation(jactorio::data::placementOrientation::left,
		                            jactorio::data::Transport_line_data::lineOrientation::up_left);
	}

	// ======================================================================

	void create_transport_line(jactorio::game::World_data& world_data, jactorio::data::Entity& proto,
	                           const std::pair<uint32_t, uint32_t> world_coords,
	                           const jactorio::data::placementOrientation orientation) {
		auto& layer = world_data.get_tile_world_coords(world_coords.first, world_coords.second)
		                        ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
		layer.prototype_data = &proto;
		proto.on_build(world_data, world_coords, layer, 0, orientation);
	}

	TEST_F(TransportLineTest, OnBuildUpdateNeighboringLines) {

		/*
		 * >
		 * ^
		 */
		add_top_transport_line(jactorio::data::Transport_line_data::lineOrientation::right);

		auto& layer = world_data_.get_tile_world_coords(1, 1)
		                        ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);


		auto proto = jactorio::data::Transport_belt{};
		layer.prototype_data = &proto;


		// Should update line above, turn right to a up-right
		line_proto_.on_build(world_data_, {1, 1}, layer, 0,
		                    jactorio::data::placementOrientation::up);

		{
			auto& result_layer = world_data_.get_tile_world_coords(1, 0)
			                               ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);

			EXPECT_EQ(
				static_cast<jactorio::data::Transport_line_data*>(result_layer.unique_data)->orientation,
				jactorio::data::Transport_line_data::lineOrientation::up_right
			);
		}
	}

	TEST_F(TransportLineTest, OnRemoveUpdateNeighboringLines) {
		// The on_remove event should update the orientations of the neighboring belts to if the current transport
		// line is not there

		world_data_.logic_add_chunk(world_data_.get_chunk_world_coords(0, 0));
		/*
		 *  v
		 *  >
		 *  ^
		 */
		add_top_transport_line(jactorio::data::Transport_line_data::lineOrientation::down);
		add_bottom_transport_line(jactorio::data::Transport_line_data::lineOrientation::up);
		add_transport_line(jactorio::data::Transport_line_data::lineOrientation::right, 1, 1);  // Between the 2 above and below

		auto& layer = world_data_.get_tile_world_coords(1, 2)
		                        ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);

		auto proto = jactorio::data::Transport_belt{};
		layer.prototype_data = &proto;


		// Removing the bottom line makes the center one bend down-right
		line_proto_.on_remove(world_data_, {1, 2}, layer);

		{
			auto& result_layer = world_data_.get_tile_world_coords(1, 1)
			                               ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);

			EXPECT_EQ(
				static_cast<jactorio::data::Transport_line_data*>(result_layer.unique_data)->orientation,
				jactorio::data::Transport_line_data::lineOrientation::down_right
			);
		}
	}


	// ======================================================================


	TEST_F(TransportLineTest, OnBuildCreateTransportLineSegment) {
		// Should create a transport line segment and add its chunk to logic chunks
		jactorio::game::World_data world_data{};
		world_data.add_chunk(new jactorio::game::Chunk{-1, 0});

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

	TEST_F(TransportLineTest, OnBuildConnectTransportLineSegments) {
		// A transport line pointing to another one will set the target_segment


		auto proto = jactorio::data::Transport_belt{};
		{
			auto& layer = world_data_.get_tile_world_coords(0, 0)
			                        ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &proto;
			proto.on_build(world_data_, {0, 0}, layer, 0, jactorio::data::placementOrientation::up);
		}
		{
			// Second transport line should connect to first
			auto& layer = world_data_.get_tile_world_coords(1, 0)
			                        ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &proto;
			proto.on_build(world_data_, {1, 0}, layer, 0, jactorio::data::placementOrientation::left);
		}

		auto& logic_chunk = world_data_.logic_get_all_chunks().at(world_data_.get_chunk(0, 0));
		auto& transport_lines =
			logic_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		ASSERT_EQ(transport_lines.size(), 2);
		auto* line = static_cast<jactorio::game::Transport_line_segment*>(transport_lines[1].unique_data);
		EXPECT_EQ(line->target_segment, transport_lines[0].unique_data);
	}

	TEST_F(TransportLineTest, OnBuildConnectTransportLineSegments2) {
		// A transport line placed infront of another one will set the target_segment of the neighbor 


		auto proto = jactorio::data::Transport_belt{};
		{
			auto& layer = world_data_.get_tile_world_coords(1, 0)
			                        ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &proto;
			proto.on_build(world_data_, {1, 0}, layer, 0, jactorio::data::placementOrientation::left);
		}
		{
			auto& layer = world_data_.get_tile_world_coords(0, 0)
			                        ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &proto;
			proto.on_build(world_data_, {0, 0}, layer, 0, jactorio::data::placementOrientation::up);
		}

		auto& logic_chunk = world_data_.logic_get_all_chunks().at(world_data_.get_chunk(0, 0));
		auto& transport_lines =
			logic_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		ASSERT_EQ(transport_lines.size(), 2);
		auto* line = static_cast<jactorio::game::Transport_line_segment*>(transport_lines[0].unique_data);
		EXPECT_EQ(line->target_segment, transport_lines[1].unique_data);
	}

	TEST_F(TransportLineTest, OnBuildNoConnectTransportLineSegments) {
		// Do not connect transport line segments pointed at each other


		auto proto = jactorio::data::Transport_belt{};
		{
			auto& layer = world_data_.get_tile_world_coords(0, 0)
			                        ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &proto;
			proto.on_build(world_data_, {0, 0}, layer, 0, jactorio::data::placementOrientation::down);
		}
		{
			auto& layer = world_data_.get_tile_world_coords(0, 1)
			                        ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &proto;
			proto.on_build(world_data_, {0, 1}, layer, 0, jactorio::data::placementOrientation::up);
		}

		auto& logic_chunk = world_data_.logic_get_all_chunks().at(world_data_.get_chunk(0, 0));
		auto& transport_lines =
			logic_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		ASSERT_EQ(transport_lines.size(), 2);

		auto* line = static_cast<jactorio::game::Transport_line_segment*>(transport_lines[0].unique_data);
		EXPECT_EQ(line->target_segment, nullptr);

		auto* line_b = static_cast<jactorio::game::Transport_line_segment*>(transport_lines[1].unique_data);
		EXPECT_EQ(line_b->target_segment, nullptr);
	}

	TEST_F(TransportLineTest, OnBuildConnectTransportLineSegmentsCrossChunks) {
		// A transport line pointing to another one will set the target_segment
		jactorio::game::World_data world_data{};
		world_data.add_chunk(new jactorio::game::Chunk{-1, 0});
		world_data.add_chunk(new jactorio::game::Chunk{0, 0});

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

	// ======================================================================

	TEST_F(TransportLineTest, OnBuildUpUpdateNeighboringSegmentToSideOnly) {

		/*   
		 * > ^ < 
		 *   ^ 
		 */
		/* Order:
		 *   
		 * 2 4 3
		 *   1
		 */
		auto proto = jactorio::data::Transport_belt{};
		create_transport_line(world_data_, proto, {1, 1}, jactorio::data::placementOrientation::up);

		create_transport_line(world_data_, proto, {0, 0}, jactorio::data::placementOrientation::right);
		create_transport_line(world_data_, proto, {2, 0}, jactorio::data::placementOrientation::left);

		create_transport_line(world_data_, proto, {1, 0}, jactorio::data::placementOrientation::up);


		auto& logic_chunk = world_data_.logic_get_all_chunks().at(world_data_.get_chunk(0, 0));
		auto& transport_lines =
			logic_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		ASSERT_EQ(transport_lines.size(), 4);

		{
			jactorio::game::Chunk_struct_layer& line_layer = transport_lines[1];
			EXPECT_FLOAT_EQ(line_layer.position_x, 1);
			EXPECT_FLOAT_EQ(line_layer.position_y, 0);

			auto* line = static_cast<jactorio::game::Transport_line_segment*>(line_layer.unique_data);
			EXPECT_EQ(line->termination_type, jactorio::game::Transport_line_segment::terminationType::left_only);
		}
		{
			jactorio::game::Chunk_struct_layer& line_layer = transport_lines[2];
			EXPECT_FLOAT_EQ(line_layer.position_x, 1);
			EXPECT_FLOAT_EQ(line_layer.position_y, 0);

			auto* line = static_cast<jactorio::game::Transport_line_segment*>(line_layer.unique_data);
			EXPECT_EQ(line->termination_type, jactorio::game::Transport_line_segment::terminationType::right_only);
		}
	}

	TEST_F(TransportLineTest, OnBuildRightUpdateNeighboringSegmentToSideOnly) {
		// Line 2 should change to right_only

		/*   v
		 * > > 
		 *   ^
		 */
		/* Order:
		 *   3
		 * 1 4 
		 *   2
		 */
		auto proto = jactorio::data::Transport_belt{};
		create_transport_line(world_data_, proto, {0, 1}, jactorio::data::placementOrientation::right);

		create_transport_line(world_data_, proto, {1, 2}, jactorio::data::placementOrientation::up);
		create_transport_line(world_data_, proto, {1, 0}, jactorio::data::placementOrientation::down);

		create_transport_line(world_data_, proto, {1, 1}, jactorio::data::placementOrientation::right);


		auto& logic_chunk = world_data_.logic_get_all_chunks().at(world_data_.get_chunk(0, 0));
		auto& transport_lines =
			logic_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		ASSERT_EQ(transport_lines.size(), 4);

		{
			jactorio::game::Chunk_struct_layer& line_layer = transport_lines[1];
			EXPECT_FLOAT_EQ(line_layer.position_x, 1);
			EXPECT_FLOAT_EQ(line_layer.position_y, 1);

			auto* line = static_cast<jactorio::game::Transport_line_segment*>(line_layer.unique_data);
			EXPECT_EQ(line->termination_type, jactorio::game::Transport_line_segment::terminationType::right_only);
		}
		{
			jactorio::game::Chunk_struct_layer& line_layer = transport_lines[2];
			EXPECT_FLOAT_EQ(line_layer.position_x, 1);
			EXPECT_FLOAT_EQ(line_layer.position_y, 1);

			auto* line = static_cast<jactorio::game::Transport_line_segment*>(line_layer.unique_data);
			EXPECT_EQ(line->termination_type, jactorio::game::Transport_line_segment::terminationType::left_only);
		}
	}

	TEST_F(TransportLineTest, OnBuildDownUpdateNeighboringSegmentToSideOnly) {

		/*   
		 * > v < 
		 */
		/* Order:
		 *   
		 * 1 3 2 
		 */
		auto proto = jactorio::data::Transport_belt{};
		create_transport_line(world_data_, proto, {0, 0}, jactorio::data::placementOrientation::right);
		create_transport_line(world_data_, proto, {2, 0}, jactorio::data::placementOrientation::left);

		create_transport_line(world_data_, proto, {1, 0}, jactorio::data::placementOrientation::down);


		auto& logic_chunk = world_data_.logic_get_all_chunks().at(world_data_.get_chunk(0, 0));
		auto& transport_lines =
			logic_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		ASSERT_EQ(transport_lines.size(), 3);

		{
			jactorio::game::Chunk_struct_layer& line_layer = transport_lines[0];
			EXPECT_FLOAT_EQ(line_layer.position_x, 1);
			EXPECT_FLOAT_EQ(line_layer.position_y, 0);

			auto* line = static_cast<jactorio::game::Transport_line_segment*>(line_layer.unique_data);
			EXPECT_EQ(line->termination_type, jactorio::game::Transport_line_segment::terminationType::right_only);
		}
		{
			jactorio::game::Chunk_struct_layer& line_layer = transport_lines[1];
			EXPECT_FLOAT_EQ(line_layer.position_x, 1);
			EXPECT_FLOAT_EQ(line_layer.position_y, 0);

			auto* line = static_cast<jactorio::game::Transport_line_segment*>(line_layer.unique_data);
			EXPECT_EQ(line->termination_type, jactorio::game::Transport_line_segment::terminationType::left_only);
		}
	}

	TEST_F(TransportLineTest, OnBuildLeftUpdateNeighboringSegmentToSideOnly) {
		// Line 2 should change to right_only

		/*   v
		 *   < 
		 *   ^
		 */
		/* Order:
		 *   1 
		 *   3 
		 *   2
		 */
		auto proto = jactorio::data::Transport_belt{};
		create_transport_line(world_data_, proto, {0, 0}, jactorio::data::placementOrientation::down);
		create_transport_line(world_data_, proto, {0, 2}, jactorio::data::placementOrientation::up);

		create_transport_line(world_data_, proto, {0, 1}, jactorio::data::placementOrientation::left);


		auto& logic_chunk = world_data_.logic_get_all_chunks().at(world_data_.get_chunk(0, 0));
		auto& transport_lines =
			logic_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		ASSERT_EQ(transport_lines.size(), 3);

		{
			jactorio::game::Chunk_struct_layer& line_layer = transport_lines[0];
			EXPECT_FLOAT_EQ(line_layer.position_x, 0);
			EXPECT_FLOAT_EQ(line_layer.position_y, 1);

			auto* line = static_cast<jactorio::game::Transport_line_segment*>(line_layer.unique_data);
			EXPECT_EQ(line->termination_type, jactorio::game::Transport_line_segment::terminationType::right_only);
		}
		{
			jactorio::game::Chunk_struct_layer& line_layer = transport_lines[1];
			EXPECT_FLOAT_EQ(line_layer.position_x, 0);
			EXPECT_FLOAT_EQ(line_layer.position_y, 1);

			auto* line = static_cast<jactorio::game::Transport_line_segment*>(line_layer.unique_data);
			EXPECT_EQ(line->termination_type, jactorio::game::Transport_line_segment::terminationType::left_only);
		}
	}

	// ======================================================================

	TEST_F(TransportLineTest, OnBuildBendingTransportLineSegment) {
		// Change the transport_line_segment termination type in accordance with orientation when placed behind existing line


		auto proto = jactorio::data::Transport_belt{};
		{
			auto& layer = world_data_.get_tile_world_coords(0, 0)
			                        ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &proto;
			proto.on_build(world_data_, {0, 0}, layer, 0, jactorio::data::placementOrientation::down);
		}
		{
			auto& layer = world_data_.get_tile_world_coords(1, 0)
			                        ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &proto;
			proto.on_build(world_data_, {1, 0}, layer, 0, jactorio::data::placementOrientation::left);
		}


		auto& logic_chunk = world_data_.logic_get_all_chunks().at(world_data_.get_chunk(0, 0));
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

	TEST_F(TransportLineTest, OnBuildBendingTransportLineSegment2) {
		// Change the transport_line_segment termination type in accordance with orientation when placed ahead of existing line 


		auto proto = jactorio::data::Transport_belt{};
		{
			auto& layer = world_data_.get_tile_world_coords(1, 0)
			                        ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &proto;
			proto.on_build(world_data_, {1, 0}, layer, 0, jactorio::data::placementOrientation::left);
		}
		{
			auto& layer = world_data_.get_tile_world_coords(0, 0)
			                        ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &proto;
			proto.on_build(world_data_, {0, 0}, layer, 0, jactorio::data::placementOrientation::down);
		}


		auto& logic_chunk = world_data_.logic_get_all_chunks().at(world_data_.get_chunk(0, 0));
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

	TEST_F(TransportLineTest, OnBuildNeighborBend) {
		// Removing a transport line must update neighboring transport segments
		// The bottom segment goes from right_only to bend_right


		auto proto = jactorio::data::Transport_belt{};

		/*
		 *  > > > 
		 *    ^ 
		 */
		/* Order:
		 * 4 3 2
		 *   1
		 */
		create_transport_line(world_data_, proto, {1, 1}, jactorio::data::placementOrientation::up);

		create_transport_line(world_data_, proto, {2, 0}, jactorio::data::placementOrientation::right);
		create_transport_line(world_data_, proto, {1, 0}, jactorio::data::placementOrientation::right);
		create_transport_line(world_data_, proto, {0, 0}, jactorio::data::placementOrientation::right);


		jactorio::game::Logic_chunk& logic_chunk = world_data_.logic_get_all_chunks().begin()->second;
		std::vector<jactorio::game::Chunk_struct_layer>& struct_layer =
			logic_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		ASSERT_EQ(struct_layer.size(), 4);
		EXPECT_FLOAT_EQ(struct_layer[0].position_x, 1.f);
		EXPECT_FLOAT_EQ(struct_layer[0].position_y, 0.f);

		auto* line_segment = static_cast<jactorio::game::Transport_line_segment*>(struct_layer[0].unique_data);
		EXPECT_EQ(line_segment->termination_type, jactorio::game::Transport_line_segment::terminationType::right_only);
		EXPECT_EQ(line_segment->segment_length, 2);
	}

	// ======================================================================

	// Checks that transport_lines.size() == 5, 4th item is right_only, 5th is left_only,
	// Checks that both lines at at center_x, center_y
	void validate_side_only(jactorio::game::World_data& world_data,
	                        const int32_t center_x, const int32_t center_y) {
		auto& logic_chunk = world_data.logic_get_all_chunks().at(world_data.get_chunk(0, 0));
		auto& transport_lines =
			logic_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		ASSERT_EQ(transport_lines.size(), 5);

		{
			jactorio::game::Chunk_struct_layer& line_layer = transport_lines[3];
			auto* line = static_cast<jactorio::game::Transport_line_segment*>(line_layer.unique_data);

			EXPECT_EQ(line->termination_type, jactorio::game::Transport_line_segment::terminationType::right_only);
			EXPECT_EQ(line->segment_length, 2);
			EXPECT_FLOAT_EQ(line_layer.position_x, center_x);
			EXPECT_FLOAT_EQ(line_layer.position_y, center_y);
		}
		{
			jactorio::game::Chunk_struct_layer& line_layer = transport_lines[4];
			auto* line_2 = static_cast<jactorio::game::Transport_line_segment*>(line_layer.unique_data);

			EXPECT_EQ(line_2->termination_type, jactorio::game::Transport_line_segment::terminationType::left_only);
			EXPECT_EQ(line_2->segment_length, 2);
			EXPECT_FLOAT_EQ(line_layer.position_x, center_x);
			EXPECT_FLOAT_EQ(line_layer.position_y, center_y);
		}

	}

	TEST_F(TransportLineTest, OnBuildUpSideOnlyTransportSegment) {

		/*
		 *   ^
		 * > ^ < 
		 *   ^
		 */
		auto proto = jactorio::data::Transport_belt{};

		create_transport_line(world_data_, proto, {1, 0}, jactorio::data::placementOrientation::up);
		create_transport_line(world_data_, proto, {1, 1}, jactorio::data::placementOrientation::up);
		create_transport_line(world_data_, proto, {1, 2}, jactorio::data::placementOrientation::up);

		create_transport_line(world_data_, proto, {2, 1}, jactorio::data::placementOrientation::left);
		create_transport_line(world_data_, proto, {0, 1}, jactorio::data::placementOrientation::right);

		validate_side_only(world_data_, 1, 1);
	}

	TEST_F(TransportLineTest, OnBuildRightSideOnlyTransportSegment) {

		/*
		 *   v
		 * > > > 
		 *   ^
		 */
		auto proto = jactorio::data::Transport_belt{};

		create_transport_line(world_data_, proto, {0, 1}, jactorio::data::placementOrientation::right);
		create_transport_line(world_data_, proto, {1, 1}, jactorio::data::placementOrientation::right);
		create_transport_line(world_data_, proto, {2, 1}, jactorio::data::placementOrientation::right);

		create_transport_line(world_data_, proto, {1, 2}, jactorio::data::placementOrientation::up);
		create_transport_line(world_data_, proto, {1, 0}, jactorio::data::placementOrientation::down);

		validate_side_only(world_data_, 1, 1);
	}

	TEST_F(TransportLineTest, OnBuildDownSideOnlyTransportSegment) {

		/*
		 *   v
		 * > v < 
		 *   v 
		 */
		auto proto = jactorio::data::Transport_belt{};

		create_transport_line(world_data_, proto, {1, 0}, jactorio::data::placementOrientation::down);
		create_transport_line(world_data_, proto, {1, 1}, jactorio::data::placementOrientation::down);
		create_transport_line(world_data_, proto, {1, 2}, jactorio::data::placementOrientation::down);

		create_transport_line(world_data_, proto, {0, 1}, jactorio::data::placementOrientation::right);
		create_transport_line(world_data_, proto, {2, 1}, jactorio::data::placementOrientation::left);

		validate_side_only(world_data_, 1, 1);
	}

	TEST_F(TransportLineTest, OnBuildLeftSideOnlyTransportSegment) {

		/*
		 *   v
		 * < < <
		 *   ^
		 */
		auto proto = jactorio::data::Transport_belt{};

		create_transport_line(world_data_, proto, {0, 1}, jactorio::data::placementOrientation::left);
		create_transport_line(world_data_, proto, {1, 1}, jactorio::data::placementOrientation::left);
		create_transport_line(world_data_, proto, {2, 1}, jactorio::data::placementOrientation::left);

		create_transport_line(world_data_, proto, {1, 0}, jactorio::data::placementOrientation::down);
		create_transport_line(world_data_, proto, {1, 2}, jactorio::data::placementOrientation::up);

		validate_side_only(world_data_, 1, 1);
	}


	// ======================================================================

	void validate_bend_to_side_only(jactorio::game::World_data& world_data,
	                                const int32_t center_x, const int32_t center_y) {
		auto& logic_chunk = world_data.logic_get_all_chunks().at(world_data.get_chunk(0, 0));
		auto& transport_lines =
			logic_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		ASSERT_EQ(transport_lines.size(), 4);

		{
			jactorio::game::Chunk_struct_layer& line_layer = transport_lines[2];
			auto* line = static_cast<jactorio::game::Transport_line_segment*>(line_layer.unique_data);

			EXPECT_EQ(line->termination_type, jactorio::game::Transport_line_segment::terminationType::right_only);
			EXPECT_EQ(line->segment_length, 2);
			EXPECT_FLOAT_EQ(line_layer.position_x, center_x);
			EXPECT_FLOAT_EQ(line_layer.position_y, center_y);
		}
		{
			jactorio::game::Chunk_struct_layer& line_layer = transport_lines[3];
			auto* line = static_cast<jactorio::game::Transport_line_segment*>(line_layer.unique_data);

			EXPECT_EQ(line->termination_type, jactorio::game::Transport_line_segment::terminationType::left_only);
			EXPECT_EQ(line->segment_length, 2);
			EXPECT_FLOAT_EQ(line_layer.position_x, center_x);
			EXPECT_FLOAT_EQ(line_layer.position_y, center_y);
		}

	}

	// Change bend to side only as initially, it forms a bend without the line on top / bottom
	TEST_F(TransportLineTest, OnBuildUpChangeBendToSideOnly) {

		/*
		 *   ^
		 * > ^ < 
		 */

		auto proto = jactorio::data::Transport_belt{};

		create_transport_line(world_data_, proto, {1, 0}, jactorio::data::placementOrientation::up);
		create_transport_line(world_data_, proto, {1, 1}, jactorio::data::placementOrientation::up);

		create_transport_line(world_data_, proto, {2, 1}, jactorio::data::placementOrientation::left);
		create_transport_line(world_data_, proto, {0, 1}, jactorio::data::placementOrientation::right);

		validate_bend_to_side_only(world_data_, 1, 1);
	}

	TEST_F(TransportLineTest, OnBuildRightChangeBendToSideOnly) {

		/*
		 *   v
		 *   > > 
		 *   ^
		 */

		auto proto = jactorio::data::Transport_belt{};

		create_transport_line(world_data_, proto, {1, 1}, jactorio::data::placementOrientation::right);
		create_transport_line(world_data_, proto, {2, 1}, jactorio::data::placementOrientation::right);

		create_transport_line(world_data_, proto, {1, 2}, jactorio::data::placementOrientation::up);
		create_transport_line(world_data_, proto, {1, 0}, jactorio::data::placementOrientation::down);

		validate_bend_to_side_only(world_data_, 1, 1);
	}

	TEST_F(TransportLineTest, OnBuildDownChangeBendToSideOnly) {

		/*
		 * > v < 
		 *   v 
		 */

		auto proto = jactorio::data::Transport_belt{};

		create_transport_line(world_data_, proto, {1, 1}, jactorio::data::placementOrientation::down);
		create_transport_line(world_data_, proto, {1, 2}, jactorio::data::placementOrientation::down);

		create_transport_line(world_data_, proto, {0, 1}, jactorio::data::placementOrientation::right);
		create_transport_line(world_data_, proto, {2, 1}, jactorio::data::placementOrientation::left);

		validate_bend_to_side_only(world_data_, 1, 1);
	}

	TEST_F(TransportLineTest, OnBuildLeftChangeBendToSideOnly) {

		/*
		 *   v
		 * < < 
		 *   ^
		 */

		auto proto = jactorio::data::Transport_belt{};

		create_transport_line(world_data_, proto, {0, 1}, jactorio::data::placementOrientation::left);
		create_transport_line(world_data_, proto, {1, 1}, jactorio::data::placementOrientation::left);

		create_transport_line(world_data_, proto, {1, 0}, jactorio::data::placementOrientation::down);
		create_transport_line(world_data_, proto, {1, 2}, jactorio::data::placementOrientation::up);

		validate_bend_to_side_only(world_data_, 1, 1);
	}

	// ======================================================================

	TEST_F(TransportLineTest, TransportLineCircle) {
		// Creates a circle of transport lines

		/*
		 * > v
		 * ^ <
		 */

		auto proto = jactorio::data::Transport_belt{};
		{
			auto& layer = world_data_.get_tile_world_coords(0, 0)
			                        ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &proto;
			proto.on_build(world_data_, {0, 0}, layer, 0, jactorio::data::placementOrientation::right);
		}
		{
			auto& layer = world_data_.get_tile_world_coords(1, 0)
			                        ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &proto;
			proto.on_build(world_data_, {1, 0}, layer, 0, jactorio::data::placementOrientation::down);
		}
		{
			auto& layer = world_data_.get_tile_world_coords(1, 1)
			                        ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &proto;
			proto.on_build(world_data_, {1, 1}, layer, 0, jactorio::data::placementOrientation::left);
		}
		{
			auto& layer = world_data_.get_tile_world_coords(0, 1)
			                        ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &proto;
			proto.on_build(world_data_, {0, 1}, layer, 0, jactorio::data::placementOrientation::up);
		}


		auto& logic_chunk = world_data_.logic_get_all_chunks().at(world_data_.get_chunk(0, 0));
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


	// ======================================================================


	TEST_F(TransportLineTest, OnRemoveDeleteTransportLineSegment) {
		// Removing a transport line needs to delete the transport line segment associated with it


		auto proto = jactorio::data::Transport_belt{};
		create_transport_line(world_data_, proto, {0, 0}, jactorio::data::placementOrientation::left);


		auto& tile_layer = world_data_.get_tile_world_coords(0, 0)
		                             ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);

		proto.on_remove(world_data_, {0, 0}, tile_layer);

		// Transport line structure count should be 0 as it was removed
		jactorio::game::Logic_chunk& logic_chunk = world_data_.logic_get_all_chunks().begin()->second;

		EXPECT_EQ(logic_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line).size(), 0);
	}

	TEST_F(TransportLineTest, OnRemoveSetNeighborTargetSegment) {
		// After removing a transport line, anything that points to it as a target_segment needs to be set to NULL


		auto proto = jactorio::data::Transport_belt{};

		create_transport_line(world_data_, proto, {0, 0}, jactorio::data::placementOrientation::left);
		create_transport_line(world_data_, proto, {1, 0}, jactorio::data::placementOrientation::left);


		auto& tile_layer = world_data_.get_tile_world_coords(0, 0)
		                             ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);

		proto.on_remove(world_data_, {0, 0}, tile_layer);


		jactorio::game::Logic_chunk& logic_chunk = world_data_.logic_get_all_chunks().begin()->second;
		std::vector<jactorio::game::Chunk_struct_layer>& struct_layer =
			logic_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		ASSERT_EQ(struct_layer.size(), 1);
		// Set back to nullptr
		EXPECT_EQ(
			static_cast<jactorio::game::Transport_line_segment*>(struct_layer[0].unique_data)->target_segment,
			nullptr);
	}

	TEST_F(TransportLineTest, OnRemoveBend) {
		// After removing a transport line the transport line that connects to it with a bend must become straight,
		// decrement segment_length by 1, and shift its position


		auto proto = jactorio::data::Transport_belt{};

		/*
		 * v
		 * >
		 */
		create_transport_line(world_data_, proto, {0, 0}, jactorio::data::placementOrientation::down);
		create_transport_line(world_data_, proto, {0, 1}, jactorio::data::placementOrientation::right);

		{
			auto& tile_layer = world_data_.get_tile_world_coords(0, 1)
			                             ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);

			proto.on_remove(world_data_, {0, 1}, tile_layer);
		}


		jactorio::game::Logic_chunk& logic_chunk = world_data_.logic_get_all_chunks().begin()->second;
		std::vector<jactorio::game::Chunk_struct_layer>& struct_layer =
			logic_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		ASSERT_EQ(struct_layer.size(), 1);
		EXPECT_FLOAT_EQ(struct_layer[0].position_x, 0.f);
		EXPECT_FLOAT_EQ(struct_layer[0].position_y, 0.f);

		auto* line_segment = static_cast<jactorio::game::Transport_line_segment*>(struct_layer[0].unique_data);
		EXPECT_EQ(line_segment->termination_type, jactorio::game::Transport_line_segment::terminationType::straight);
		EXPECT_EQ(line_segment->segment_length, 1);
	}

	TEST_F(TransportLineTest, OnRemoveNeighborBend) {
		// Removing a transport line must update neighboring transport segments
		// The bottom segment goes from right_only to bend_right


		auto proto = jactorio::data::Transport_belt{};

		/*
		 * /> > > 
		 *    ^ 
		 */
		create_transport_line(world_data_, proto, {0, 0}, jactorio::data::placementOrientation::right);
		create_transport_line(world_data_, proto, {1, 0}, jactorio::data::placementOrientation::right);
		create_transport_line(world_data_, proto, {2, 0}, jactorio::data::placementOrientation::right);

		create_transport_line(world_data_, proto, {1, 1}, jactorio::data::placementOrientation::up);

		{
			auto& tile_layer = world_data_.get_tile_world_coords(0, 0)
			                             ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);

			proto.on_remove(world_data_, {0, 0}, tile_layer);
		}


		jactorio::game::Logic_chunk& logic_chunk = world_data_.logic_get_all_chunks().begin()->second;
		std::vector<jactorio::game::Chunk_struct_layer>& struct_layer =
			logic_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		ASSERT_EQ(struct_layer.size(), 3);
		EXPECT_FLOAT_EQ(struct_layer[2].position_x, 1.f);
		EXPECT_FLOAT_EQ(struct_layer[2].position_y, 0.f);

		auto* line_segment = static_cast<jactorio::game::Transport_line_segment*>(struct_layer[2].unique_data);
		EXPECT_EQ(line_segment->termination_type, jactorio::game::Transport_line_segment::terminationType::bend_right);
		EXPECT_EQ(line_segment->segment_length, 2);
	}
}
