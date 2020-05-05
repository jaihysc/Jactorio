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
	/*
	// General tests
	// Neighbor updates
	// Bends
	// Side only
	// Connecting segments
	// Grouping
	*/

	class TransportLineTest : public testing::Test
	{
	protected:
		jactorio::game::World_data world_data_{};
		jactorio::data::Transport_belt line_proto_{};

		void SetUp() override {
			world_data_.add_chunk(new jactorio::game::Chunk{0, 0});
		}

		// ======================================================================

		///
		/// \brief Sets the prototype pointer for a transport line at tile
		void build_transport_line(
			const jactorio::data::Orientation orientation,
			const std::pair<uint32_t, uint32_t> world_coords) {

			auto& layer = world_data_.get_tile(world_coords.first, world_coords.second)
			                         ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &line_proto_;
			tl_build_events(world_coords, orientation);
		}

		///
		/// \brief Creates new Transport line segent alongside data at tile
		void add_transport_line(const jactorio::data::Transport_line_data::LineOrientation orientation,
		                        const jactorio::game::World_data::world_coord x,
		                        const jactorio::game::World_data::world_coord y) {
			auto& layer          = world_data_.get_tile(x, y)->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &line_proto_;

			auto segment = jactorio::game::Transport_line_segment{
				jactorio::data::Orientation::left,
				jactorio::game::Transport_line_segment::TerminationType::straight,
				1
			};
			auto* data = new jactorio::data::Transport_line_data(segment);
			data->set_orientation(orientation);

			layer.unique_data = data;

			tl_build_events({x, y}, jactorio::data::Transport_line_data::to_orientation(orientation));
		}

		void add_transport_line(const jactorio::game::World_data::world_pair& world_coords,
		                        const jactorio::data::Transport_line_data::LineOrientation orientation) {
			add_transport_line(orientation, world_coords.first, world_coords.second);
		}

		/// Creates a transport line with the provided orientation above/right/below/left of 1, 1
		void add_top_transport_line(const jactorio::data::Transport_line_data::LineOrientation orientation) {
			add_transport_line(orientation, 1, 0);
		}

		void add_right_transport_line(const jactorio::data::Transport_line_data::LineOrientation orientation) {
			add_transport_line(orientation, 2, 1);
		}

		void add_bottom_transport_line(const jactorio::data::Transport_line_data::LineOrientation orientation) {
			add_transport_line(orientation, 1, 2);
		}

		void add_left_transport_line(const jactorio::data::Transport_line_data::LineOrientation orientation) {
			add_transport_line(orientation, 0, 1);
		}

		///
		/// \brief Validates that a tile at coords 1,1 with the placement orientation produces the expected line orientation
		void validate_result_orientation(const jactorio::data::Orientation placement_orientation,
		                                 jactorio::data::Transport_line_data::LineOrientation expected_line_orientation) {
			const auto pair = std::pair<uint16_t, uint16_t>{static_cast<uint16_t>(expected_line_orientation), 0};
			EXPECT_EQ(line_proto_.map_placement_orientation(placement_orientation, world_data_, {1, 1}), pair);
		}

		// ======================================================================
	private:
		void dispatch_neighbor_update(const jactorio::game::World_data::world_pair& emit_coords,
		                              const jactorio::game::World_data::world_pair& receive_coords,
		                              const jactorio::data::Orientation emit_orientation) {

			auto* tile = world_data_.get_tile(receive_coords);
			if (!tile)
				return;

			auto& layer = tile->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			if (!layer.prototype_data)
				return;

			static_cast<const jactorio::data::Entity*>(layer.prototype_data)
				->on_neighbor_update(world_data_, emit_coords, receive_coords, emit_orientation);
		}

	protected:
		///
		/// \brief Dispatches the appropriate events for when a transport line is built
		void tl_build_events(const jactorio::game::World_data::world_pair& world_coords,
		                     const jactorio::data::Orientation orientation) {
			auto& layer = world_data_.get_tile(world_coords)
			                         ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);

			line_proto_.on_build(world_data_, world_coords, layer, 0, orientation);

			// Call on_neighbor_update for the 4 sides
			dispatch_neighbor_update(world_coords,
			                         {world_coords.first, world_coords.second - 1}, jactorio::data::Orientation::up);

			dispatch_neighbor_update(world_coords,
			                         {world_coords.first + 1, world_coords.second}, jactorio::data::Orientation::right);

			dispatch_neighbor_update(world_coords,
			                         {world_coords.first, world_coords.second + 1}, jactorio::data::Orientation::down);

			dispatch_neighbor_update(world_coords,
			                         {world_coords.first - 1, world_coords.second}, jactorio::data::Orientation::left);
		}

		///
		/// \brief Dispatches the appropriate events AFTER a transport line is removed 
		void tl_remove_events(const jactorio::game::World_data::world_pair& world_coords) {

			auto& layer = world_data_.get_tile(world_coords)
			                         ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);

			line_proto_.on_remove(world_data_, world_coords, layer);

			// Call on_neighbor_update for the 4 sides
			dispatch_neighbor_update(world_coords,
			                         {world_coords.first, world_coords.second - 1}, jactorio::data::Orientation::up);

			dispatch_neighbor_update(world_coords,
			                         {world_coords.first + 1, world_coords.second}, jactorio::data::Orientation::right);

			dispatch_neighbor_update(world_coords,
			                         {world_coords.first, world_coords.second + 1}, jactorio::data::Orientation::down);

			dispatch_neighbor_update(world_coords,
			                         {world_coords.first - 1, world_coords.second}, jactorio::data::Orientation::left);
		}


		// Grouping

		std::vector<jactorio::game::Chunk_struct_layer>& get_transport_lines(
			const jactorio::game::World_data::world_pair& chunk_coords) {
			return world_data_.logic_get_chunk(world_data_.get_chunk_c(chunk_coords.first, chunk_coords.second))
			                  ->get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);
		}

		J_NODISCARD auto& get_line_data(const jactorio::game::World_data::world_pair& world_coords) const {
			return *static_cast<const jactorio::data::Transport_line_data*>(
				world_data_.get_tile(world_coords)->get_layer(
					jactorio::game::Chunk_tile::chunkLayer::entity).unique_data
			);
		}

		auto get_line_segment_index(const jactorio::game::World_data::world_pair& world_coords) const {
			return get_line_data(world_coords).line_segment_index;
		}


		///
		/// \param first Leading segment
		/// \param second Segment placed behind leading segment
		void group_ahead_validate(const jactorio::game::World_data::world_pair& first,
		                          const jactorio::game::World_data::world_pair& second) {
			ASSERT_EQ(get_transport_lines({0, 0}).size(), 1);  // 0, 0 is chunk coordinate
			EXPECT_EQ(get_line_data(first).line_segment.get().length, 2);

			EXPECT_EQ(get_line_segment_index(first), 0);
			EXPECT_EQ(get_line_segment_index(second), 1);

			EXPECT_EQ(get_line_data(first).line_segment.get().target_segment, nullptr);

			// Ensure the head is at the correct location
			jactorio::data::Transport_line::get_line_struct_layer(
				world_data_,
				first.first, first.second, [&](jactorio::game::Chunk_struct_layer& layer, auto&) {
					EXPECT_FLOAT_EQ(layer.position_x,
					                first.first);  // Validation of logic chunk position only valid when within a single chunk
					EXPECT_FLOAT_EQ(layer.position_y, first.second);
				});
		}
	};


	// ======================================================================
	// General tests

	TEST_F(TransportLineTest, OnBuildCreateTransportLineSegment) {
		// Should create a transport line segment and add its chunk to logic chunks
		world_data_.add_chunk(new jactorio::game::Chunk{-1, 0});

		auto& layer = world_data_.get_tile(-5, 0)
		                         ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
		layer.prototype_data = &line_proto_;

		tl_build_events({-5, 0}, jactorio::data::Orientation::right);

		// ======================================================================

		// Added current chunk as a logic chunk
		ASSERT_EQ(world_data_.logic_get_all_chunks().size(), 1);

		auto& logic_chunk = world_data_.logic_get_all_chunks().at(world_data_.get_chunk_c(-1, 0));
		EXPECT_EQ(logic_chunk.chunk, world_data_.get_chunk_c(-1, 0));


		auto& transport_lines =
			logic_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		// Should have created a transport line structure
		ASSERT_EQ(transport_lines.size(), 1);
		ASSERT_TRUE(dynamic_cast<jactorio::game::Transport_line_segment*>(transport_lines.front().unique_data));

		auto* line_data = dynamic_cast<jactorio::game::Transport_line_segment*>(transport_lines.front().unique_data);
		EXPECT_EQ(line_data->direction, jactorio::data::Orientation::right);
		EXPECT_EQ(line_data->termination_type, jactorio::game::Transport_line_segment::TerminationType::straight);
		EXPECT_EQ(line_data->length, 1);

		// Position_x / position_y is the distance from the top left of the chunk
		EXPECT_EQ(transport_lines.front().position_x, 27);
		EXPECT_EQ(transport_lines.front().position_y, 0);
	}

	TEST_F(TransportLineTest, OnRemoveDeleteTransportLineSegment) {
		// Removing a transport line needs to delete the transport line segment associated with it
		add_transport_line({0, 0}, jactorio::data::Transport_line_data::LineOrientation::left);


		tl_remove_events({0, 0});

		// Transport line structure count should be 0 as it was removed
		jactorio::game::Logic_chunk& logic_chunk = world_data_.logic_get_all_chunks().begin()->second;

		EXPECT_EQ(logic_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line).size(), 0);
	}

	// ======================================================================
	// Neighbor updates

	TEST_F(TransportLineTest, OnBuildNeighborBend) {
		// Removing a transport line must update neighboring transport segments
		// The bottom segment goes from right_only to bend_right

		/*
		 *  > > > 
		 *    ^ 
		 */
		/* Order:
		 * 4 3 2
		 *   1
		 */
		add_transport_line({1, 1}, jactorio::data::Transport_line_data::LineOrientation::up);

		add_transport_line({2, 0}, jactorio::data::Transport_line_data::LineOrientation::right);
		add_transport_line({1, 0}, jactorio::data::Transport_line_data::LineOrientation::right);
		add_transport_line({0, 0}, jactorio::data::Transport_line_data::LineOrientation::right);


		jactorio::game::Logic_chunk& logic_chunk                      = world_data_.logic_get_all_chunks().begin()->second;
		std::vector<jactorio::game::Chunk_struct_layer>& struct_layer =
			logic_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		ASSERT_EQ(struct_layer.size(), 2);
		EXPECT_FLOAT_EQ(struct_layer[0].position_x, 1.f);
		EXPECT_FLOAT_EQ(struct_layer[0].position_y, 0.f);

		auto* line_segment = static_cast<jactorio::game::Transport_line_segment*>(struct_layer[0].unique_data);
		EXPECT_EQ(line_segment->termination_type, jactorio::game::Transport_line_segment::TerminationType::right_only);
		EXPECT_EQ(line_segment->length, 2);
	}

	TEST_F(TransportLineTest, OnRemoveNeighborBend) {
		// Removing a transport line must update neighboring transport segments
		// The bottom segment goes from right_only to bend_right

		/*
		 * /> > > 
		 *    ^ 
		 */
		add_transport_line({0, 0}, jactorio::data::Transport_line_data::LineOrientation::right);
		add_transport_line({1, 0}, jactorio::data::Transport_line_data::LineOrientation::right);
		add_transport_line({2, 0}, jactorio::data::Transport_line_data::LineOrientation::right);

		add_transport_line({1, 1}, jactorio::data::Transport_line_data::LineOrientation::up);


		tl_remove_events({0, 0});


		jactorio::game::Logic_chunk& logic_chunk                      = world_data_.logic_get_all_chunks().begin()->second;
		std::vector<jactorio::game::Chunk_struct_layer>& struct_layer =
			logic_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		ASSERT_EQ(struct_layer.size(), 2);
		EXPECT_FLOAT_EQ(struct_layer[1].position_x, 1.f);
		EXPECT_FLOAT_EQ(struct_layer[1].position_y, 0.f);

		auto* line_segment = static_cast<jactorio::game::Transport_line_segment*>(struct_layer[1].unique_data);
		EXPECT_EQ(line_segment->termination_type, jactorio::game::Transport_line_segment::TerminationType::bend_right);
		EXPECT_EQ(line_segment->length, 2);
	}

	TEST_F(TransportLineTest, OnBuildUpdateNeighboringLines) {

		/*
		 * >
		 * ^
		 */
		add_top_transport_line(jactorio::data::Transport_line_data::LineOrientation::right);

		auto& layer = world_data_.get_tile(1, 1)
		                         ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);


		auto proto           = jactorio::data::Transport_belt{};
		layer.prototype_data = &proto;


		// Should update line above, turn right to a up-right
		tl_build_events({1, 1}, jactorio::data::Orientation::up);

		{
			auto& result_layer = world_data_.get_tile(1, 0)
			                                ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);

			EXPECT_EQ(
				static_cast<jactorio::data::Transport_line_data*>(result_layer.unique_data)->orientation,
				jactorio::data::Transport_line_data::LineOrientation::up_right
			);
		}
	}

	TEST_F(TransportLineTest, OnRemoveUpdateNeighboringLines) {
		// The on_remove event should update the orientations of the neighboring belts to if the current transport
		// line is not there

		world_data_.logic_add_chunk(world_data_.get_chunk(0, 0));
		/*
		 *  v
		 *  >
		 *  ^
		 */
		add_top_transport_line(jactorio::data::Transport_line_data::LineOrientation::down);
		add_bottom_transport_line(jactorio::data::Transport_line_data::LineOrientation::up);
		add_transport_line(jactorio::data::Transport_line_data::LineOrientation::right, 1, 1);  // Between the 2 above and below

		auto& layer = world_data_.get_tile(1, 2)
		                         ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
		layer.prototype_data = &line_proto_;


		// Removing the bottom line makes the center one bend down-right
		tl_remove_events({1, 2});

		{
			auto& result_layer = world_data_.get_tile(1, 1)
			                                ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);

			EXPECT_EQ(
				static_cast<jactorio::data::Transport_line_data*>(result_layer.unique_data)->orientation,
				jactorio::data::Transport_line_data::LineOrientation::down_right
			);
		}
	}

	// ======================================================================
	// Bends 

	// Various custom arrangements of transport lines
	TEST_F(TransportLineTest, OnBuildBendingTransportLineSegmentTrailing) {
		// Change the transport_line_segment termination type in accordance with orientation when placed behind existing line


		auto& down_layer = world_data_.get_tile(0, 0)
		                              ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
		down_layer.prototype_data = &line_proto_;
		tl_build_events({0, 0}, jactorio::data::Orientation::down);

		auto& left_layer = world_data_.get_tile(1, 0)
		                              ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
		left_layer.prototype_data = &line_proto_;
		tl_build_events({1, 0}, jactorio::data::Orientation::left);


		auto& logic_chunk     = world_data_.logic_get_all_chunks().at(world_data_.get_chunk_c(0, 0));
		auto& transport_lines =
			logic_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		ASSERT_EQ(transport_lines.size(), 2);

		jactorio::game::Chunk_struct_layer& line_layer = transport_lines[1];

		auto* line = static_cast<jactorio::game::Transport_line_segment*>(line_layer.unique_data);
		EXPECT_EQ(line->termination_type, jactorio::game::Transport_line_segment::TerminationType::bend_left);

		// Should have lengthened segment and moved x 1 left
		EXPECT_EQ(line->length, 2);
		EXPECT_FLOAT_EQ(line_layer.position_x, 0.f);
		EXPECT_FLOAT_EQ(line_layer.position_y, 0.f);

		EXPECT_EQ(static_cast<jactorio::data::Transport_line_data*>(left_layer.unique_data)->line_segment_index, 1);
	}

	TEST_F(TransportLineTest, OnBuildBendingTransportLineSegmentLeading) {
		// Change the transport_line_segment termination type in accordance with orientation when placed ahead of existing line 

		auto& left_layer = world_data_.get_tile(1, 0)
		                              ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
		left_layer.prototype_data = &line_proto_;
		tl_build_events({1, 0}, jactorio::data::Orientation::left);

		auto& down_layer = world_data_.get_tile(0, 0)
		                              ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
		down_layer.prototype_data = &line_proto_;
		tl_build_events({0, 0}, jactorio::data::Orientation::down);


		auto& logic_chunk     = world_data_.logic_get_all_chunks().at(world_data_.get_chunk_c(0, 0));
		auto& transport_lines =
			logic_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		ASSERT_EQ(transport_lines.size(), 2);

		jactorio::game::Chunk_struct_layer& line_layer = transport_lines[0];
		auto* line = static_cast<jactorio::game::Transport_line_segment*>(line_layer.unique_data);
		EXPECT_EQ(line->termination_type, jactorio::game::Transport_line_segment::TerminationType::bend_left);

		// Should have lengthened segment and moved x 1 left
		EXPECT_EQ(line->length, 2);
		EXPECT_FLOAT_EQ(line_layer.position_x, 0.f);
		EXPECT_FLOAT_EQ(line_layer.position_y, 0.f);

		EXPECT_EQ(static_cast<jactorio::data::Transport_line_data*>(left_layer.unique_data)->line_segment_index, 1);
	}


	TEST_F(TransportLineTest, OnRemoveBend) {
		// After removing a transport line the transport line that connects to it with a bend must become straight,
		// decrement segment_length by 1, and shift its position

		/*
		 * v
		 * >
		 */
		add_transport_line({0, 0}, jactorio::data::Transport_line_data::LineOrientation::down);
		add_transport_line({0, 1}, jactorio::data::Transport_line_data::LineOrientation::right);


		tl_remove_events({0, 1});


		jactorio::game::Logic_chunk& logic_chunk                      = world_data_.logic_get_all_chunks().begin()->second;
		std::vector<jactorio::game::Chunk_struct_layer>& struct_layer =
			logic_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		ASSERT_EQ(struct_layer.size(), 1);
		EXPECT_FLOAT_EQ(struct_layer[0].position_x, 0.f);
		EXPECT_FLOAT_EQ(struct_layer[0].position_y, 0.f);

		auto* line_segment = static_cast<jactorio::game::Transport_line_segment*>(struct_layer[0].unique_data);
		EXPECT_EQ(line_segment->termination_type, jactorio::game::Transport_line_segment::TerminationType::straight);
		EXPECT_EQ(line_segment->length, 1);
	}

	TEST_F(TransportLineTest, SetOrientation) {
		// When the orientation is set, the member "set" should also be updated

		// Arbitrary segment is fine since no logic updates are performed
		auto segment = jactorio::game::Transport_line_segment{
			jactorio::data::Orientation::left,
			jactorio::game::Transport_line_segment::TerminationType::straight,
			1
		};

		jactorio::data::Transport_line_data line_data{segment};

		line_data.set_orientation(jactorio::data::Transport_line_data::LineOrientation::down);
		EXPECT_EQ(line_data.set, static_cast<uint16_t>(jactorio::data::Transport_line_data::LineOrientation::down));

		line_data.set_orientation(jactorio::data::Transport_line_data::LineOrientation::left_down);
		EXPECT_EQ(line_data.set, static_cast<uint16_t>(jactorio::data::Transport_line_data::LineOrientation::left_down));
	}


	TEST_F(TransportLineTest, OrientationUp1) {
		/*
		 * > ^
		 */
		add_left_transport_line(jactorio::data::Transport_line_data::LineOrientation::right);
		validate_result_orientation(jactorio::data::Orientation::up,
		                            jactorio::data::Transport_line_data::LineOrientation::right_up);
	}

	TEST_F(TransportLineTest, OrientationUp2) {
		/*
		 *   ^ <
		 */
		add_right_transport_line(jactorio::data::Transport_line_data::LineOrientation::left);
		validate_result_orientation(jactorio::data::Orientation::up,
		                            jactorio::data::Transport_line_data::LineOrientation::left_up);
	}

	TEST_F(TransportLineTest, OrientationUp3) {
		/*
		 * > ^ <
		 */
		// Top and bottom points to one line, line should be straight

		add_left_transport_line(jactorio::data::Transport_line_data::LineOrientation::right);
		add_right_transport_line(jactorio::data::Transport_line_data::LineOrientation::left);
		validate_result_orientation(jactorio::data::Orientation::up,
		                            jactorio::data::Transport_line_data::LineOrientation::up);
	}

	TEST_F(TransportLineTest, OrientationUp4) {
		/*
		 * > ^ 
		 *   >
		 */
		add_left_transport_line(jactorio::data::Transport_line_data::LineOrientation::right);
		add_bottom_transport_line(jactorio::data::Transport_line_data::LineOrientation::right_down);
		validate_result_orientation(jactorio::data::Orientation::up,
		                            jactorio::data::Transport_line_data::LineOrientation::right_up);
	}

	TEST_F(TransportLineTest, OrientationUp5) {
		/*
		 * > ^ 
		 *   ^
		 */
		add_left_transport_line(jactorio::data::Transport_line_data::LineOrientation::right);
		add_bottom_transport_line(jactorio::data::Transport_line_data::LineOrientation::up);
		validate_result_orientation(jactorio::data::Orientation::up,
		                            jactorio::data::Transport_line_data::LineOrientation::up);
	}

	TEST_F(TransportLineTest, OrientationUp6) {
		/*
		 * < ^ <
		 */

		add_left_transport_line(jactorio::data::Transport_line_data::LineOrientation::left);
		add_right_transport_line(jactorio::data::Transport_line_data::LineOrientation::left);
		validate_result_orientation(jactorio::data::Orientation::up,
		                            jactorio::data::Transport_line_data::LineOrientation::left_up);
	}

	// ===

	TEST_F(TransportLineTest, OrientationRight1) {
		/*
		 *  v
		 *  >
		 */
		add_top_transport_line(jactorio::data::Transport_line_data::LineOrientation::down);
		validate_result_orientation(jactorio::data::Orientation::right,
		                            jactorio::data::Transport_line_data::LineOrientation::down_right);
	}

	TEST_F(TransportLineTest, OrientationRight2) {
		/*
		 * >
		 * ^
		 */
		add_bottom_transport_line(jactorio::data::Transport_line_data::LineOrientation::up);
		validate_result_orientation(jactorio::data::Orientation::right,
		                            jactorio::data::Transport_line_data::LineOrientation::up_right);
	}

	TEST_F(TransportLineTest, OrientationRight3) {
		// Top and bottom points to one line, line should be straight

		add_top_transport_line(jactorio::data::Transport_line_data::LineOrientation::down);
		add_bottom_transport_line(jactorio::data::Transport_line_data::LineOrientation::up);
		validate_result_orientation(jactorio::data::Orientation::right,
		                            jactorio::data::Transport_line_data::LineOrientation::right);
	}

	TEST_F(TransportLineTest, OrientationRight4) {
		/*
		 *   v
		 * ^ >
		 */
		add_top_transport_line(jactorio::data::Transport_line_data::LineOrientation::down);
		add_left_transport_line(jactorio::data::Transport_line_data::LineOrientation::up);  // Does not point to center
		validate_result_orientation(jactorio::data::Orientation::right,
		                            jactorio::data::Transport_line_data::LineOrientation::down_right);
	}

	TEST_F(TransportLineTest, OrientationRight5) {
		/*
		 *   v
		 * > >
		 */
		add_top_transport_line(jactorio::data::Transport_line_data::LineOrientation::down);
		add_left_transport_line(
			jactorio::data::Transport_line_data::LineOrientation::right);  // Points at center, center now straight 
		validate_result_orientation(jactorio::data::Orientation::right,
		                            jactorio::data::Transport_line_data::LineOrientation::right);
	}

	TEST_F(TransportLineTest, OrientationRight6) {
		/*
		 * ^
		 * >
		 * ^
		 */
		add_top_transport_line(jactorio::data::Transport_line_data::LineOrientation::up);
		add_bottom_transport_line(jactorio::data::Transport_line_data::LineOrientation::up);
		validate_result_orientation(jactorio::data::Orientation::right,
		                            jactorio::data::Transport_line_data::LineOrientation::up_right);
	}

	// ===

	TEST_F(TransportLineTest, OrientationDown1) {
		add_left_transport_line(jactorio::data::Transport_line_data::LineOrientation::right);
		validate_result_orientation(jactorio::data::Orientation::down,
		                            jactorio::data::Transport_line_data::LineOrientation::right_down);
	}

	TEST_F(TransportLineTest, OrientationDown2) {
		add_right_transport_line(jactorio::data::Transport_line_data::LineOrientation::left);
		validate_result_orientation(jactorio::data::Orientation::down,
		                            jactorio::data::Transport_line_data::LineOrientation::left_down);
	}

	TEST_F(TransportLineTest, OrientationDown3) {
		// Top and bottom points to one line, line should be straight

		add_left_transport_line(jactorio::data::Transport_line_data::LineOrientation::right);
		add_right_transport_line(jactorio::data::Transport_line_data::LineOrientation::left);
		validate_result_orientation(jactorio::data::Orientation::down,
		                            jactorio::data::Transport_line_data::LineOrientation::down);
	}

	TEST_F(TransportLineTest, OrientationDown4) {
		add_left_transport_line(jactorio::data::Transport_line_data::LineOrientation::right);
		add_top_transport_line(jactorio::data::Transport_line_data::LineOrientation::left);
		validate_result_orientation(jactorio::data::Orientation::down,
		                            jactorio::data::Transport_line_data::LineOrientation::right_down);
	}

	TEST_F(TransportLineTest, OrientationDown5) {
		add_left_transport_line(jactorio::data::Transport_line_data::LineOrientation::right);
		add_top_transport_line(jactorio::data::Transport_line_data::LineOrientation::right_down);
		validate_result_orientation(jactorio::data::Orientation::down,
		                            jactorio::data::Transport_line_data::LineOrientation::down);
	}

	TEST_F(TransportLineTest, OrientationDown6) {
		add_left_transport_line(jactorio::data::Transport_line_data::LineOrientation::left);
		add_right_transport_line(jactorio::data::Transport_line_data::LineOrientation::left);
		validate_result_orientation(jactorio::data::Orientation::down,
		                            jactorio::data::Transport_line_data::LineOrientation::left_down);
	}

	// ===

	TEST_F(TransportLineTest, OrientationLeft1) {
		add_top_transport_line(jactorio::data::Transport_line_data::LineOrientation::down);
		validate_result_orientation(jactorio::data::Orientation::left,
		                            jactorio::data::Transport_line_data::LineOrientation::down_left);
	}

	TEST_F(TransportLineTest, OrientationLeft2) {
		add_bottom_transport_line(jactorio::data::Transport_line_data::LineOrientation::up);
		validate_result_orientation(jactorio::data::Orientation::left,
		                            jactorio::data::Transport_line_data::LineOrientation::up_left);
	}

	TEST_F(TransportLineTest, OrientationLeft3) {
		add_top_transport_line(jactorio::data::Transport_line_data::LineOrientation::down);
		add_bottom_transport_line(jactorio::data::Transport_line_data::LineOrientation::up);
		validate_result_orientation(jactorio::data::Orientation::left,
		                            jactorio::data::Transport_line_data::LineOrientation::left);
	}

	TEST_F(TransportLineTest, OrientationLeft4) {
		add_top_transport_line(jactorio::data::Transport_line_data::LineOrientation::down);
		add_right_transport_line(jactorio::data::Transport_line_data::LineOrientation::up);
		validate_result_orientation(jactorio::data::Orientation::left,
		                            jactorio::data::Transport_line_data::LineOrientation::down_left);
	}

	TEST_F(TransportLineTest, OrientationLeft5) {
		add_top_transport_line(jactorio::data::Transport_line_data::LineOrientation::down);
		add_right_transport_line(jactorio::data::Transport_line_data::LineOrientation::left);
		validate_result_orientation(jactorio::data::Orientation::left,
		                            jactorio::data::Transport_line_data::LineOrientation::left);
	}

	TEST_F(TransportLineTest, OrientationLeft6) {
		add_top_transport_line(jactorio::data::Transport_line_data::LineOrientation::up);
		add_bottom_transport_line(jactorio::data::Transport_line_data::LineOrientation::up);
		validate_result_orientation(jactorio::data::Orientation::left,
		                            jactorio::data::Transport_line_data::LineOrientation::up_left);
	}


	// ======================================================================
	// Side only

	void validate_bend_to_side_only(jactorio::game::World_data& world_data,
	                                const int32_t center_x, const int32_t center_y) {
		auto& logic_chunk     = world_data.logic_get_all_chunks().at(world_data.get_chunk_c(0, 0));
		auto& transport_lines =
			logic_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		ASSERT_EQ(transport_lines.size(), 3);

		{
			jactorio::game::Chunk_struct_layer& line_layer = transport_lines[1];
			auto* line = static_cast<jactorio::game::Transport_line_segment*>(line_layer.unique_data);

			EXPECT_EQ(line->termination_type, jactorio::game::Transport_line_segment::TerminationType::right_only);
			EXPECT_EQ(line->length, 2);
			EXPECT_FLOAT_EQ(line_layer.position_x, center_x);
			EXPECT_FLOAT_EQ(line_layer.position_y, center_y);
		}
		{
			jactorio::game::Chunk_struct_layer& line_layer = transport_lines[2];
			auto* line = static_cast<jactorio::game::Transport_line_segment*>(line_layer.unique_data);

			EXPECT_EQ(line->termination_type, jactorio::game::Transport_line_segment::TerminationType::left_only);
			EXPECT_EQ(line->length, 2);
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
		build_transport_line(jactorio::data::Orientation::up, {1, 0});
		build_transport_line(jactorio::data::Orientation::up, {1, 1});

		build_transport_line(jactorio::data::Orientation::left, {2, 1});
		build_transport_line(jactorio::data::Orientation::right, {0, 1});

		validate_bend_to_side_only(world_data_, 1, 1);
		EXPECT_EQ(get_line_segment_index({0, 1}), 1);
		EXPECT_EQ(get_line_segment_index({2, 1}), 1);
	}

	TEST_F(TransportLineTest, OnBuildRightChangeBendToSideOnly) {

		/*
		 *   v
		 *   > > 
		 *   ^
		 */
		build_transport_line(jactorio::data::Orientation::right, {1, 1});
		build_transport_line(jactorio::data::Orientation::right, {2, 1});

		build_transport_line(jactorio::data::Orientation::up, {1, 2});
		build_transport_line(jactorio::data::Orientation::down, {1, 0});

		validate_bend_to_side_only(world_data_, 1, 1);
	}

	TEST_F(TransportLineTest, OnBuildDownChangeBendToSideOnly) {

		/*
		 * > v < 
		 *   v 
		 */
		build_transport_line(jactorio::data::Orientation::down, {1, 1});
		build_transport_line(jactorio::data::Orientation::down, {1, 2});

		build_transport_line(jactorio::data::Orientation::right, {0, 1});
		build_transport_line(jactorio::data::Orientation::left, {2, 1});

		validate_bend_to_side_only(world_data_, 1, 1);
	}

	TEST_F(TransportLineTest, OnBuildLeftChangeBendToSideOnly) {

		/*
		 *   v
		 * < < 
		 *   ^
		 */
		build_transport_line(jactorio::data::Orientation::left, {0, 1});
		build_transport_line(jactorio::data::Orientation::left, {1, 1});

		build_transport_line(jactorio::data::Orientation::down, {1, 0});
		build_transport_line(jactorio::data::Orientation::up, {1, 2});

		validate_bend_to_side_only(world_data_, 1, 1);
	}


	TEST_F(TransportLineTest, OnBuildUpUpdateNeighboringSegmentToSideOnly) {
		/*   
		 * > ^ < 
		 */
		/* Order:
		 *   
		 * 1 3 2
		 */
		build_transport_line(jactorio::data::Orientation::right, {0, 0});
		build_transport_line(jactorio::data::Orientation::left, {2, 0});

		build_transport_line(jactorio::data::Orientation::up, {1, 0});


		auto& logic_chunk     = world_data_.logic_get_all_chunks().at(world_data_.get_chunk_c(0, 0));
		auto& transport_lines =
			logic_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		ASSERT_EQ(transport_lines.size(), 3);

		{
			jactorio::game::Chunk_struct_layer& line_layer = transport_lines[0];
			EXPECT_FLOAT_EQ(line_layer.position_x, 1);
			EXPECT_FLOAT_EQ(line_layer.position_y, 0);

			auto* line = static_cast<jactorio::game::Transport_line_segment*>(line_layer.unique_data);
			EXPECT_EQ(line->termination_type, jactorio::game::Transport_line_segment::TerminationType::left_only);
		}
		{
			jactorio::game::Chunk_struct_layer& line_layer = transport_lines[1];
			EXPECT_FLOAT_EQ(line_layer.position_x, 1);
			EXPECT_FLOAT_EQ(line_layer.position_y, 0);

			auto* line = static_cast<jactorio::game::Transport_line_segment*>(line_layer.unique_data);
			EXPECT_EQ(line->termination_type, jactorio::game::Transport_line_segment::TerminationType::right_only);
		}

		EXPECT_EQ(get_line_segment_index({0, 0}), 1);
		EXPECT_EQ(get_line_segment_index({2, 0}), 1);
	}

	TEST_F(TransportLineTest, OnBuildRightUpdateNeighboringSegmentToSideOnly) {
		// Line 2 should change to right_only

		/*  v
		 *  > 
		 *  ^
		 */
		/* Order:
		 *  2
		 *  3 
		 *  1
		 */
		build_transport_line(jactorio::data::Orientation::up, {1, 2});
		build_transport_line(jactorio::data::Orientation::down, {1, 0});

		build_transport_line(jactorio::data::Orientation::right, {1, 1});


		auto& logic_chunk     = world_data_.logic_get_all_chunks().at(world_data_.get_chunk_c(0, 0));
		auto& transport_lines =
			logic_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		ASSERT_EQ(transport_lines.size(), 3);

		{
			jactorio::game::Chunk_struct_layer& line_layer = transport_lines[0];
			EXPECT_FLOAT_EQ(line_layer.position_x, 1);
			EXPECT_FLOAT_EQ(line_layer.position_y, 1);

			auto* line = static_cast<jactorio::game::Transport_line_segment*>(line_layer.unique_data);
			EXPECT_EQ(line->termination_type, jactorio::game::Transport_line_segment::TerminationType::right_only);
		}
		{
			jactorio::game::Chunk_struct_layer& line_layer = transport_lines[1];
			EXPECT_FLOAT_EQ(line_layer.position_x, 1);
			EXPECT_FLOAT_EQ(line_layer.position_y, 1);

			auto* line = static_cast<jactorio::game::Transport_line_segment*>(line_layer.unique_data);
			EXPECT_EQ(line->termination_type, jactorio::game::Transport_line_segment::TerminationType::left_only);
		}

		EXPECT_EQ(get_line_segment_index({1, 2}), 1);
		EXPECT_EQ(get_line_segment_index({1, 0}), 1);
	}

	TEST_F(TransportLineTest, OnBuildDownUpdateNeighboringSegmentToSideOnly) {

		/*   
		 * > v < 
		 */
		/* Order:
		 *   
		 * 1 3 2 
		 */
		build_transport_line(jactorio::data::Orientation::right, {0, 0});
		build_transport_line(jactorio::data::Orientation::left, {2, 0});

		build_transport_line(jactorio::data::Orientation::down, {1, 0});


		auto& logic_chunk     = world_data_.logic_get_all_chunks().at(world_data_.get_chunk_c(0, 0));
		auto& transport_lines =
			logic_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		ASSERT_EQ(transport_lines.size(), 3);

		{
			jactorio::game::Chunk_struct_layer& line_layer = transport_lines[0];
			EXPECT_FLOAT_EQ(line_layer.position_x, 1);
			EXPECT_FLOAT_EQ(line_layer.position_y, 0);

			auto* line = static_cast<jactorio::game::Transport_line_segment*>(line_layer.unique_data);
			EXPECT_EQ(line->termination_type, jactorio::game::Transport_line_segment::TerminationType::right_only);
		}
		{
			jactorio::game::Chunk_struct_layer& line_layer = transport_lines[1];
			EXPECT_FLOAT_EQ(line_layer.position_x, 1);
			EXPECT_FLOAT_EQ(line_layer.position_y, 0);

			auto* line = static_cast<jactorio::game::Transport_line_segment*>(line_layer.unique_data);
			EXPECT_EQ(line->termination_type, jactorio::game::Transport_line_segment::TerminationType::left_only);
		}

		EXPECT_EQ(get_line_segment_index({0, 0}), 1);
		EXPECT_EQ(get_line_segment_index({2, 0}), 1);
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
		build_transport_line(jactorio::data::Orientation::down, {0, 0});
		build_transport_line(jactorio::data::Orientation::up, {0, 2});

		build_transport_line(jactorio::data::Orientation::left, {0, 1});


		auto& logic_chunk     = world_data_.logic_get_all_chunks().at(world_data_.get_chunk_c(0, 0));
		auto& transport_lines =
			logic_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		ASSERT_EQ(transport_lines.size(), 3);

		{
			jactorio::game::Chunk_struct_layer& line_layer = transport_lines[0];
			EXPECT_FLOAT_EQ(line_layer.position_x, 0);
			EXPECT_FLOAT_EQ(line_layer.position_y, 1);

			auto* line = static_cast<jactorio::game::Transport_line_segment*>(line_layer.unique_data);
			EXPECT_EQ(line->termination_type, jactorio::game::Transport_line_segment::TerminationType::right_only);
		}
		{
			jactorio::game::Chunk_struct_layer& line_layer = transport_lines[1];
			EXPECT_FLOAT_EQ(line_layer.position_x, 0);
			EXPECT_FLOAT_EQ(line_layer.position_y, 1);

			auto* line = static_cast<jactorio::game::Transport_line_segment*>(line_layer.unique_data);
			EXPECT_EQ(line->termination_type, jactorio::game::Transport_line_segment::TerminationType::left_only);
		}

		EXPECT_EQ(get_line_segment_index({0, 0}), 1);
		EXPECT_EQ(get_line_segment_index({0, 2}), 1);
	}


	// ======================================================================
	// Connecting segments

	TEST_F(TransportLineTest, OnRemoveSetNeighborTargetSegment) {
		// After removing a transport line, anything that points to it as a target_segment needs to be set to NULL
		build_transport_line(jactorio::data::Orientation::left, {0, 0});
		build_transport_line(jactorio::data::Orientation::up, {0, 1});


		tl_remove_events({0, 0});


		jactorio::game::Logic_chunk& logic_chunk                      = world_data_.logic_get_all_chunks().begin()->second;
		std::vector<jactorio::game::Chunk_struct_layer>& struct_layer =
			logic_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		ASSERT_EQ(struct_layer.size(), 1);
		// Set back to nullptr
		EXPECT_EQ(
			static_cast<jactorio::game::Transport_line_segment*>(struct_layer[0].unique_data)->target_segment,
			nullptr);
	}

	TEST_F(TransportLineTest, OnBuildConnectTransportLineSegmentsLeading) {
		// A transport line pointing to another one will set the target_segment
		/*
		 * ^ <
		 * 1 2
		 */

		{
			auto& layer = world_data_.get_tile(0, 0)
			                         ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &line_proto_;
			tl_build_events({0, 0}, jactorio::data::Orientation::up);
		}
		{
			// Second transport line should connect to first
			auto& layer = world_data_.get_tile(1, 0)
			                         ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &line_proto_;
			tl_build_events({1, 0}, jactorio::data::Orientation::left);
		}

		auto& logic_chunk     = world_data_.logic_get_all_chunks().at(world_data_.get_chunk_c(0, 0));
		auto& transport_lines =
			logic_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		ASSERT_EQ(transport_lines.size(), 2);
		auto* line = static_cast<jactorio::game::Transport_line_segment*>(transport_lines[1].unique_data);
		EXPECT_EQ(line->target_segment, transport_lines[0].unique_data);
	}

	TEST_F(TransportLineTest, OnBuildConnectTransportLineSegmentsTrailing) {
		// A transport line placed infront of another one will set the target_segment of the neighbor 
		/*
		 * > ^
		 */
		/*
		 * 1 2
		 */

		{
			auto& layer = world_data_.get_tile(1, 0)
			                         ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &line_proto_;
			tl_build_events({1, 0}, jactorio::data::Orientation::left);
		}
		{
			auto& layer = world_data_.get_tile(0, 0)
			                         ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &line_proto_;
			tl_build_events({0, 0}, jactorio::data::Orientation::up);
		}

		auto& logic_chunk     = world_data_.logic_get_all_chunks().at(world_data_.get_chunk_c(0, 0));
		auto& transport_lines =
			logic_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		ASSERT_EQ(transport_lines.size(), 2);
		auto* line = static_cast<jactorio::game::Transport_line_segment*>(transport_lines[0].unique_data);
		EXPECT_EQ(line->target_segment, transport_lines[1].unique_data);
	}

	TEST_F(TransportLineTest, OnBuildNoConnectTransportLineSegments) {
		// Do not connect transport line segments pointed at each other
		/*
		 * > <
		 */

		{
			auto& layer = world_data_.get_tile(0, 0)
			                         ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &line_proto_;
			tl_build_events({0, 0}, jactorio::data::Orientation::down);
		}
		{
			auto& layer = world_data_.get_tile(0, 1)
			                         ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &line_proto_;
			tl_build_events({0, 1}, jactorio::data::Orientation::up);
		}

		auto& logic_chunk     = world_data_.logic_get_all_chunks().at(world_data_.get_chunk_c(0, 0));
		auto& transport_lines =
			logic_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		ASSERT_EQ(transport_lines.size(), 2);

		auto* line = static_cast<jactorio::game::Transport_line_segment*>(transport_lines[0].unique_data);
		EXPECT_EQ(line->target_segment, nullptr);

		auto* line_b = static_cast<jactorio::game::Transport_line_segment*>(transport_lines[1].unique_data);
		EXPECT_EQ(line_b->target_segment, nullptr);
	}

	// ======================================================================
	// Grouping

	TEST_F(TransportLineTest, OnBuildUpGroupAhead) {
		// When placed behind a line with the same orientation, join the previous line by extending its length
		/*
		 * ^
		 * ^
		 */
		build_transport_line(jactorio::data::Orientation::up, {0, 0});
		build_transport_line(jactorio::data::Orientation::up, {0, 1});

		group_ahead_validate({0, 0}, {0, 1});
	}

	TEST_F(TransportLineTest, OnBuildUpGroupBehind) {
		// When placed ahead of a line with the same orientation, shift the head to the current position 
		/*
		 * ^
		 * ^
		 */
		build_transport_line(jactorio::data::Orientation::up, {0, 1});
		build_transport_line(jactorio::data::Orientation::up, {0, 0});

		group_ahead_validate({0, 0}, {0, 1});
	}

	TEST_F(TransportLineTest, OnBuildUpGroupAheadCrossChunk) {
		// Since grouping ahead requires adjustment of a position within the current logic chunk, crossing chunks requires special logic
		world_data_.add_chunk(new jactorio::game::Chunk{0, -1});

		build_transport_line(jactorio::data::Orientation::up, {0, -1});
		build_transport_line(jactorio::data::Orientation::up, {0, 0});

		bool found = false;
		jactorio::data::Transport_line::get_line_struct_layer(
			world_data_, 0, 0,
			[&found](auto& s_layer, auto&) mutable {
				found      = true;
				auto& data = *static_cast<jactorio::game::Transport_line_segment*>(s_layer.unique_data);

				EXPECT_EQ(data.length, 1);
			});

		EXPECT_TRUE(found);
	}

	TEST_F(TransportLineTest, OnBuildUpGroupBehindCrossChunk) {
		// Since grouping ahead requires adjustment of a position within the current logic chunk, crossing chunks requires special logic
		world_data_.add_chunk(new jactorio::game::Chunk{0, -1});

		build_transport_line(jactorio::data::Orientation::up, {0, 0});
		build_transport_line(jactorio::data::Orientation::up, {0, -1});

		bool found = false;
		jactorio::data::Transport_line::get_line_struct_layer(
			world_data_, 0, -1,
			[&found](auto& s_layer, auto&) mutable {
				found      = true;
				auto& data = *static_cast<jactorio::game::Transport_line_segment*>(s_layer.unique_data);

				EXPECT_EQ(data.length, 1);
			});

		EXPECT_TRUE(found);
	}

	TEST_F(TransportLineTest, OnBuildRightGroupAhead) {
		/*
		 * > >
		 */
		build_transport_line(jactorio::data::Orientation::right, {1, 0});
		build_transport_line(jactorio::data::Orientation::right, {0, 0});

		group_ahead_validate({1, 0}, {0, 0});
	}

	TEST_F(TransportLineTest, OnBuildRightGroupBehind) {
		/*
		 * > >
		 */
		build_transport_line(jactorio::data::Orientation::right, {0, 0});
		build_transport_line(jactorio::data::Orientation::right, {1, 0});

		group_ahead_validate({1, 0}, {0, 0});
	}

	TEST_F(TransportLineTest, OnBuildDownGroupAhead) {
		/*
		 * v
		 * v
		 */
		build_transport_line(jactorio::data::Orientation::down, {0, 1});
		build_transport_line(jactorio::data::Orientation::down, {0, 0});

		group_ahead_validate({0, 1}, {0, 0});
	}

	TEST_F(TransportLineTest, OnBuildDownGroupBehind) {
		/*
		 * v
		 * v
		 */
		build_transport_line(jactorio::data::Orientation::down, {0, 0});
		build_transport_line(jactorio::data::Orientation::down, {0, 1});

		group_ahead_validate({0, 1}, {0, 0});
	}

	TEST_F(TransportLineTest, OnBuildLeftGroupAhead) {
		/*
		 * < <
		 */
		build_transport_line(jactorio::data::Orientation::left, {0, 0});
		build_transport_line(jactorio::data::Orientation::left, {1, 0});

		group_ahead_validate({0, 0}, {1, 0});
	}

	TEST_F(TransportLineTest, OnBuildLeftGroupBehind) {
		/*
		 * < <
		 */
		build_transport_line(jactorio::data::Orientation::left, {1, 0});
		build_transport_line(jactorio::data::Orientation::left, {0, 0});

		group_ahead_validate({0, 0}, {1, 0});
	}

	TEST_F(TransportLineTest, OnRemoveGroupBegin) {
		// Removing beginning of grouped transport segment
		// Create new segment, do not shorten segment ahead

		/*
		 * > >  
		 */
		add_transport_line({0, 0}, jactorio::data::Transport_line_data::LineOrientation::right);
		add_transport_line({1, 0}, jactorio::data::Transport_line_data::LineOrientation::right);

		tl_remove_events({1, 0});

		std::vector<jactorio::game::Chunk_struct_layer>& struct_layer =
			world_data_.logic_get_all_chunks().begin()->second
			           .get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		ASSERT_EQ(struct_layer.size(), 1);
		EXPECT_EQ(static_cast<jactorio::game::Transport_line_segment*>(struct_layer[0].unique_data)->length, 1);
	}

	TEST_F(TransportLineTest, OnRemoveGroupBeginBend) {
		/*
		 * > > v
		 */
		add_transport_line({0, 0}, jactorio::data::Transport_line_data::LineOrientation::right);
		add_transport_line({1, 0}, jactorio::data::Transport_line_data::LineOrientation::right);
		add_transport_line({2, 0}, jactorio::data::Transport_line_data::LineOrientation::down);

		tl_remove_events({1, 0});

		std::vector<jactorio::game::Chunk_struct_layer>& struct_layer =
			world_data_.logic_get_all_chunks().begin()->second
			           .get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		ASSERT_EQ(struct_layer.size(), 2);
		EXPECT_EQ(static_cast<jactorio::game::Transport_line_segment*>(struct_layer[0].unique_data)->length, 1);
	}

	TEST_F(TransportLineTest, OnRemoveGroupMiddle) {
		// Removing middle of grouped transport segment
		// Create new segment, shorten segment ahead

		/*
		 * > /> > >
		 */
		add_transport_line({0, 0}, jactorio::data::Transport_line_data::LineOrientation::right);
		add_transport_line({1, 0}, jactorio::data::Transport_line_data::LineOrientation::right);
		add_transport_line({2, 0}, jactorio::data::Transport_line_data::LineOrientation::right);
		add_transport_line({3, 0}, jactorio::data::Transport_line_data::LineOrientation::right);

		tl_remove_events({1, 0});

		std::vector<jactorio::game::Chunk_struct_layer>& struct_layer =
			world_data_.logic_get_all_chunks().begin()->second
			           .get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		ASSERT_EQ(struct_layer.size(), 2);
		EXPECT_EQ(static_cast<jactorio::game::Transport_line_segment*>(struct_layer[0].unique_data)->length, 2);
		EXPECT_EQ(static_cast<jactorio::game::Transport_line_segment*>(struct_layer[1].unique_data)->length, 1);
	}

	TEST_F(TransportLineTest, OnRemoveGroupEnd) {
		// Removing end of grouped transport segment
		// Create no new segment, shorten segment ahead

		/*
		 * > >  
		 */
		add_transport_line({0, 0}, jactorio::data::Transport_line_data::LineOrientation::right);
		add_transport_line({1, 0}, jactorio::data::Transport_line_data::LineOrientation::right);

		tl_remove_events({0, 0});

		std::vector<jactorio::game::Chunk_struct_layer>& struct_layer =
			world_data_.logic_get_all_chunks().begin()->second
			           .get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		ASSERT_EQ(struct_layer.size(), 1);
		EXPECT_EQ(static_cast<jactorio::game::Transport_line_segment*>(struct_layer[0].unique_data)->length, 1);
	}

	// ======================================================================
	// Various custom arrangements of transport lines

	TEST_F(TransportLineTest, TransportLineCircle) {
		// Creates a circle of transport lines

		/*
		 * > v
		 * ^ <
		 */

		{
			auto& layer = world_data_.get_tile(0, 0)
			                         ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &line_proto_;
			tl_build_events({0, 0}, jactorio::data::Orientation::right);
		}
		{
			auto& layer = world_data_.get_tile(1, 0)
			                         ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &line_proto_;
			tl_build_events({1, 0}, jactorio::data::Orientation::down);
		}
		{
			auto& layer = world_data_.get_tile(1, 1)
			                         ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &line_proto_;
			tl_build_events({1, 1}, jactorio::data::Orientation::left);
		}
		{
			auto& layer = world_data_.get_tile(0, 1)
			                         ->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);
			layer.prototype_data = &line_proto_;
			tl_build_events({0, 1}, jactorio::data::Orientation::up);
		}


		auto& logic_chunk     = world_data_.logic_get_all_chunks().at(world_data_.get_chunk_c(0, 0));
		auto& transport_lines =
			logic_chunk.get_struct(jactorio::game::Logic_chunk::structLayer::transport_line);

		ASSERT_EQ(transport_lines.size(), 4);

		// Right
		{
			jactorio::game::Chunk_struct_layer& line_layer = transport_lines[0];
			auto* line = static_cast<jactorio::game::Transport_line_segment*>(line_layer.unique_data);
			EXPECT_EQ(line->termination_type, jactorio::game::Transport_line_segment::TerminationType::bend_right);

			EXPECT_EQ(line->length, 2);
			EXPECT_FLOAT_EQ(line_layer.position_x, 1.f);
			EXPECT_FLOAT_EQ(line_layer.position_y, 0.f);
		}
		// Down
		{
			jactorio::game::Chunk_struct_layer& line_layer = transport_lines[1];
			auto* line = static_cast<jactorio::game::Transport_line_segment*>(line_layer.unique_data);
			EXPECT_EQ(line->termination_type, jactorio::game::Transport_line_segment::TerminationType::bend_right);

			EXPECT_EQ(line->length, 2);
			EXPECT_FLOAT_EQ(line_layer.position_x, 1.f);
			EXPECT_FLOAT_EQ(line_layer.position_y, 1.f);
		}

		// Left
		{
			jactorio::game::Chunk_struct_layer& line_layer = transport_lines[2];
			auto* line = static_cast<jactorio::game::Transport_line_segment*>(line_layer.unique_data);
			EXPECT_EQ(line->termination_type, jactorio::game::Transport_line_segment::TerminationType::bend_right);

			EXPECT_EQ(line->length, 2);
			EXPECT_FLOAT_EQ(line_layer.position_x, 0.f);
			EXPECT_FLOAT_EQ(line_layer.position_y, 1.f);
		}

		// Up
		{
			jactorio::game::Chunk_struct_layer& line_layer = transport_lines[3];
			auto* line = static_cast<jactorio::game::Transport_line_segment*>(line_layer.unique_data);
			EXPECT_EQ(line->termination_type, jactorio::game::Transport_line_segment::TerminationType::bend_right);

			EXPECT_EQ(line->length, 2);
			EXPECT_FLOAT_EQ(line_layer.position_x, 0.f);
			EXPECT_FLOAT_EQ(line_layer.position_y, 0.f);
		}
	}
}
