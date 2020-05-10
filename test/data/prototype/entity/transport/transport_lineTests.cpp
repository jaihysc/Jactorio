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
		jactorio::game::WorldData worldData_{};
		jactorio::data::TransportBelt lineProto_{};

		void SetUp() override {
			worldData_.AddChunk(jactorio::game::Chunk{0, 0});
		}

		// ======================================================================

		///
		/// \brief Sets the prototype pointer for a transport line at tile
		void BuildTransportLine(
			const jactorio::data::Orientation orientation,
			const std::pair<uint32_t, uint32_t> world_coords) {

			auto& layer = worldData_.GetTile(world_coords.first, world_coords.second)
			                        ->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity);
			layer.prototypeData = &lineProto_;
			TlBuildEvents(world_coords, orientation);
		}

		///
		/// \brief Creates new Transport line segment alongside data at tile
		void AddTransportLine(const jactorio::data::TransportLineData::LineOrientation orientation,
		                      const jactorio::game::WorldData::WorldCoord x,
		                      const jactorio::game::WorldData::WorldCoord y) {
			auto& layer         = worldData_.GetTile(x, y)->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity);
			layer.prototypeData = &lineProto_;

			auto segment = jactorio::game::TransportLineSegment{
				jactorio::data::Orientation::left,
				jactorio::game::TransportLineSegment::TerminationType::straight,
				1
			};
			auto* data = new jactorio::data::TransportLineData(segment);
			data->SetOrientation(orientation);

			layer.uniqueData = data;

			TlBuildEvents({x, y}, jactorio::data::TransportLineData::ToOrientation(orientation));
		}

		void AddTransportLine(const jactorio::game::WorldData::WorldPair& world_coords,
		                      const jactorio::data::TransportLineData::LineOrientation orientation) {
			AddTransportLine(orientation, world_coords.first, world_coords.second);
		}

		/// Creates a transport line with the provided orientation above/right/below/left of 1, 1
		void AddTopTransportLine(const jactorio::data::TransportLineData::LineOrientation orientation) {
			AddTransportLine(orientation, 1, 0);
		}

		void AddRightTransportLine(const jactorio::data::TransportLineData::LineOrientation orientation) {
			AddTransportLine(orientation, 2, 1);
		}

		void AddBottomTransportLine(const jactorio::data::TransportLineData::LineOrientation orientation) {
			AddTransportLine(orientation, 1, 2);
		}

		void AddLeftTransportLine(const jactorio::data::TransportLineData::LineOrientation orientation) {
			AddTransportLine(orientation, 0, 1);
		}

		///
		/// \brief Validates that a tile at coords 1,1 with the placement orientation produces the expected line orientation
		void ValidateResultOrientation(const jactorio::data::Orientation placement_orientation,
		                               jactorio::data::TransportLineData::LineOrientation expected_line_orientation) {
			const auto pair = std::pair<uint16_t, uint16_t>{static_cast<uint16_t>(expected_line_orientation), 0};
			EXPECT_EQ(lineProto_.MapPlacementOrientation(placement_orientation, worldData_, {1, 1}), pair);
		}

		// ======================================================================
	private:
		void DispatchNeighborUpdate(const jactorio::game::WorldData::WorldPair& emit_coords,
		                            const jactorio::game::WorldData::WorldPair& receive_coords,
		                            const jactorio::data::Orientation emit_orientation) {

			auto* tile = worldData_.GetTile(receive_coords);
			if (!tile)
				return;

			auto& layer = tile->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity);
			if (!layer.prototypeData)
				return;

			static_cast<const jactorio::data::Entity*>(layer.prototypeData)
				->OnNeighborUpdate(worldData_, emit_coords, receive_coords, emit_orientation);
		}

	protected:
		///
		/// \brief Dispatches the appropriate events for when a transport line is built
		void TlBuildEvents(const jactorio::game::WorldData::WorldPair& world_coords,
		                     const jactorio::data::Orientation orientation) {
			auto& layer = worldData_.GetTile(world_coords)
			                        ->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity);

			lineProto_.OnBuild(worldData_, world_coords, layer, orientation);

			// Call on_neighbor_update for the 4 sides
			DispatchNeighborUpdate(world_coords,
			                       {world_coords.first, world_coords.second - 1}, jactorio::data::Orientation::up);

			DispatchNeighborUpdate(world_coords,
			                       {world_coords.first + 1, world_coords.second}, jactorio::data::Orientation::right);

			DispatchNeighborUpdate(world_coords,
			                       {world_coords.first, world_coords.second + 1}, jactorio::data::Orientation::down);

			DispatchNeighborUpdate(world_coords,
			                       {world_coords.first - 1, world_coords.second}, jactorio::data::Orientation::left);
		}

		///
		/// \brief Dispatches the appropriate events AFTER a transport line is removed 
		void TlRemoveEvents(const jactorio::game::WorldData::WorldPair& world_coords) {

			auto& layer = worldData_.GetTile(world_coords)
			                        ->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity);

			lineProto_.OnRemove(worldData_, world_coords, layer);

			// Call on_neighbor_update for the 4 sides
			DispatchNeighborUpdate(world_coords,
			                       {world_coords.first, world_coords.second - 1}, jactorio::data::Orientation::up);

			DispatchNeighborUpdate(world_coords,
			                       {world_coords.first + 1, world_coords.second}, jactorio::data::Orientation::right);

			DispatchNeighborUpdate(world_coords,
			                       {world_coords.first, world_coords.second + 1}, jactorio::data::Orientation::down);

			DispatchNeighborUpdate(world_coords,
			                       {world_coords.first - 1, world_coords.second}, jactorio::data::Orientation::left);
		}


		// Grouping

		std::vector<jactorio::game::ChunkStructLayer>& GetTransportLines(
			const jactorio::game::Chunk::ChunkPair& chunk_coords) {
			return worldData_.LogicGetChunk(worldData_.GetChunkC(chunk_coords.first, chunk_coords.second))
			                 ->GetStruct(jactorio::game::LogicChunk::StructLayer::transport_line);
		}

		J_NODISCARD auto& GetLineData(const jactorio::game::WorldData::WorldPair& world_coords) const {
			return *static_cast<const jactorio::data::TransportLineData*>(
				worldData_.GetTile(world_coords)->GetLayer(
					jactorio::game::ChunkTile::ChunkLayer::entity).uniqueData
			);
		}

		auto GetLineSegmentIndex(const jactorio::game::WorldData::WorldPair& world_coords) const {
			return GetLineData(world_coords).lineSegmentIndex;
		}


		///
		/// \param first Leading segment
		/// \param second Segment placed behind leading segment
		void GroupAheadValidate(const jactorio::game::WorldData::WorldPair& first,
		                          const jactorio::game::WorldData::WorldPair& second) {
			ASSERT_EQ(GetTransportLines({0, 0}).size(), 1);  // 0, 0 is chunk coordinate
			EXPECT_EQ(GetLineData(first).lineSegment.get().length, 2);

			EXPECT_EQ(GetLineSegmentIndex(first), 0);
			EXPECT_EQ(GetLineSegmentIndex(second), 1);

			EXPECT_EQ(GetLineData(first).lineSegment.get().targetSegment, nullptr);

			// Ensure the head is at the correct location
			jactorio::data::TransportLine::GetLineStructLayer(
				worldData_,
				first.first, first.second, [&](jactorio::game::ChunkStructLayer& layer, auto&) {
					EXPECT_FLOAT_EQ(layer.positionX,
					                first.first);  // Validation of logic chunk position only valid when within a single chunk
					EXPECT_FLOAT_EQ(layer.positionY, first.second);
				});
		}
	};


	// ======================================================================
	// General tests

	TEST_F(TransportLineTest, OnBuildCreateTransportLineSegment) {
		// Should create a transport line segment and add its chunk to logic chunks
		worldData_.AddChunk(jactorio::game::Chunk{-1, 0});

		auto& layer = worldData_.GetTile(-5, 0)
		                        ->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity);
		layer.prototypeData = &lineProto_;

		TlBuildEvents({-5, 0}, jactorio::data::Orientation::right);

		// ======================================================================

		// Added current chunk as a logic chunk
		ASSERT_EQ(worldData_.LogicGetAllChunks().size(), 1);

		auto& logic_chunk = worldData_.LogicGetAllChunks().at(worldData_.GetChunkC(-1, 0));
		EXPECT_EQ(logic_chunk.chunk, worldData_.GetChunkC(-1, 0));


		auto& transport_lines =
			logic_chunk.GetStruct(jactorio::game::LogicChunk::StructLayer::transport_line);

		// Should have created a transport line structure
		ASSERT_EQ(transport_lines.size(), 1);
		ASSERT_TRUE(dynamic_cast<jactorio::game::TransportLineSegment*>(transport_lines.front().uniqueData));

		auto* line_data = dynamic_cast<jactorio::game::TransportLineSegment*>(transport_lines.front().uniqueData);
		EXPECT_EQ(line_data->direction, jactorio::data::Orientation::right);
		EXPECT_EQ(line_data->terminationType, jactorio::game::TransportLineSegment::TerminationType::straight);
		EXPECT_EQ(line_data->length, 1);

		// Position_x / position_y is the distance from the top left of the chunk
		EXPECT_EQ(transport_lines.front().positionX, 27);
		EXPECT_EQ(transport_lines.front().positionY, 0);
	}

	TEST_F(TransportLineTest, OnRemoveDeleteTransportLineSegment) {
		// Removing a transport line needs to delete the transport line segment associated with it
		AddTransportLine({0, 0}, jactorio::data::TransportLineData::LineOrientation::left);


		TlRemoveEvents({0, 0});

		// Transport line structure count should be 0 as it was removed
		jactorio::game::LogicChunk& logic_chunk = worldData_.LogicGetAllChunks().begin()->second;

		EXPECT_EQ(logic_chunk.GetStruct(jactorio::game::LogicChunk::StructLayer::transport_line).size(), 0);
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
		AddTransportLine({1, 1}, jactorio::data::TransportLineData::LineOrientation::up);

		AddTransportLine({2, 0}, jactorio::data::TransportLineData::LineOrientation::right);
		AddTransportLine({1, 0}, jactorio::data::TransportLineData::LineOrientation::right);
		AddTransportLine({0, 0}, jactorio::data::TransportLineData::LineOrientation::right);


		jactorio::game::LogicChunk& logic_chunk                     = worldData_.LogicGetAllChunks().begin()->second;
		std::vector<jactorio::game::ChunkStructLayer>& struct_layer =
			logic_chunk.GetStruct(jactorio::game::LogicChunk::StructLayer::transport_line);

		ASSERT_EQ(struct_layer.size(), 2);
		EXPECT_FLOAT_EQ(struct_layer[0].positionX, 1.f);
		EXPECT_FLOAT_EQ(struct_layer[0].positionY, 0.f);

		auto* line_segment = static_cast<jactorio::game::TransportLineSegment*>(struct_layer[0].uniqueData);
		EXPECT_EQ(line_segment->terminationType, jactorio::game::TransportLineSegment::TerminationType::right_only);
		EXPECT_EQ(line_segment->length, 2);
	}

	TEST_F(TransportLineTest, OnRemoveNeighborBend) {
		// Removing a transport line must update neighboring transport segments
		// The bottom segment goes from right_only to bend_right

		/*
		 * /> > > 
		 *    ^ 
		 */
		AddTransportLine({0, 0}, jactorio::data::TransportLineData::LineOrientation::right);
		AddTransportLine({1, 0}, jactorio::data::TransportLineData::LineOrientation::right);
		AddTransportLine({2, 0}, jactorio::data::TransportLineData::LineOrientation::right);

		AddTransportLine({1, 1}, jactorio::data::TransportLineData::LineOrientation::up);


		TlRemoveEvents({0, 0});


		jactorio::game::LogicChunk& logic_chunk                     = worldData_.LogicGetAllChunks().begin()->second;
		std::vector<jactorio::game::ChunkStructLayer>& struct_layer =
			logic_chunk.GetStruct(jactorio::game::LogicChunk::StructLayer::transport_line);

		ASSERT_EQ(struct_layer.size(), 2);
		EXPECT_FLOAT_EQ(struct_layer[1].positionX, 1.f);
		EXPECT_FLOAT_EQ(struct_layer[1].positionY, 0.f);

		auto* line_segment = static_cast<jactorio::game::TransportLineSegment*>(struct_layer[1].uniqueData);
		EXPECT_EQ(line_segment->terminationType, jactorio::game::TransportLineSegment::TerminationType::bend_right);
		EXPECT_EQ(line_segment->length, 2);
	}

	TEST_F(TransportLineTest, OnBuildUpdateNeighboringLines) {

		/*
		 * >
		 * ^
		 */
		AddTopTransportLine(jactorio::data::TransportLineData::LineOrientation::right);

		auto& layer = worldData_.GetTile(1, 1)
		                        ->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity);


		auto proto          = jactorio::data::TransportBelt{};
		layer.prototypeData = &proto;


		// Should update line above, turn right to a up-right
		TlBuildEvents({1, 1}, jactorio::data::Orientation::up);

		{
			auto& result_layer = worldData_.GetTile(1, 0)
			                               ->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity);

			EXPECT_EQ(
				static_cast<jactorio::data::TransportLineData*>(result_layer.uniqueData)->orientation,
				jactorio::data::TransportLineData::LineOrientation::up_right
			);
		}
	}

	TEST_F(TransportLineTest, OnRemoveUpdateNeighboringLines) {
		// The on_remove event should update the orientations of the neighboring belts to if the current transport
		// line is not there

		worldData_.LogicAddChunk(worldData_.GetChunk(0, 0));
		/*
		 *  v
		 *  >
		 *  ^
		 */
		AddTopTransportLine(jactorio::data::TransportLineData::LineOrientation::down);
		AddBottomTransportLine(jactorio::data::TransportLineData::LineOrientation::up);
		AddTransportLine(jactorio::data::TransportLineData::LineOrientation::right, 1, 1);  // Between the 2 above and below

		auto& layer = worldData_.GetTile(1, 2)
		                        ->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity);
		layer.prototypeData = &lineProto_;


		// Removing the bottom line makes the center one bend down-right
		TlRemoveEvents({1, 2});

		{
			auto& result_layer = worldData_.GetTile(1, 1)
			                               ->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity);

			EXPECT_EQ(
				static_cast<jactorio::data::TransportLineData*>(result_layer.uniqueData)->orientation,
				jactorio::data::TransportLineData::LineOrientation::down_right
			);
		}
	}

	// ======================================================================
	// Bends 

	// Various custom arrangements of transport lines
	TEST_F(TransportLineTest, OnBuildBendingTransportLineSegmentTrailing) {
		// Change the transport_line_segment termination type in accordance with orientation when placed behind existing line


		auto& down_layer = worldData_.GetTile(0, 0)
		                             ->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity);
		down_layer.prototypeData = &lineProto_;
		TlBuildEvents({0, 0}, jactorio::data::Orientation::down);

		auto& left_layer = worldData_.GetTile(1, 0)
		                             ->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity);
		left_layer.prototypeData = &lineProto_;
		TlBuildEvents({1, 0}, jactorio::data::Orientation::left);


		auto& logic_chunk     = worldData_.LogicGetAllChunks().at(worldData_.GetChunkC(0, 0));
		auto& transport_lines =
			logic_chunk.GetStruct(jactorio::game::LogicChunk::StructLayer::transport_line);

		ASSERT_EQ(transport_lines.size(), 2);

		jactorio::game::ChunkStructLayer& line_layer = transport_lines[1];

		auto* line = static_cast<jactorio::game::TransportLineSegment*>(line_layer.uniqueData);
		EXPECT_EQ(line->terminationType, jactorio::game::TransportLineSegment::TerminationType::bend_left);

		// Should have lengthened segment and moved x 1 left
		EXPECT_EQ(line->length, 2);
		EXPECT_FLOAT_EQ(line_layer.positionX, 0.f);
		EXPECT_FLOAT_EQ(line_layer.positionY, 0.f);

		EXPECT_EQ(static_cast<jactorio::data::TransportLineData*>(left_layer.uniqueData)->lineSegmentIndex, 1);
	}

	TEST_F(TransportLineTest, OnBuildBendingTransportLineSegmentLeading) {
		// Change the transport_line_segment termination type in accordance with orientation when placed ahead of existing line 

		auto& left_layer = worldData_.GetTile(1, 0)
		                             ->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity);
		left_layer.prototypeData = &lineProto_;
		TlBuildEvents({1, 0}, jactorio::data::Orientation::left);

		auto& down_layer = worldData_.GetTile(0, 0)
		                             ->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity);
		down_layer.prototypeData = &lineProto_;
		TlBuildEvents({0, 0}, jactorio::data::Orientation::down);


		auto& logic_chunk     = worldData_.LogicGetAllChunks().at(worldData_.GetChunkC(0, 0));
		auto& transport_lines =
			logic_chunk.GetStruct(jactorio::game::LogicChunk::StructLayer::transport_line);

		ASSERT_EQ(transport_lines.size(), 2);

		jactorio::game::ChunkStructLayer& line_layer = transport_lines[0];
		auto* line                                   = static_cast<jactorio::game::TransportLineSegment*>(line_layer.uniqueData);
		EXPECT_EQ(line->terminationType, jactorio::game::TransportLineSegment::TerminationType::bend_left);

		// Should have lengthened segment and moved x 1 left
		EXPECT_EQ(line->length, 2);
		EXPECT_FLOAT_EQ(line_layer.positionX, 0.f);
		EXPECT_FLOAT_EQ(line_layer.positionY, 0.f);

		EXPECT_EQ(static_cast<jactorio::data::TransportLineData*>(left_layer.uniqueData)->lineSegmentIndex, 1);
	}


	TEST_F(TransportLineTest, OnRemoveBend) {
		// After removing a transport line the transport line that connects to it with a bend must become straight,
		// decrement segment_length by 1, and shift its position

		/*
		 * v
		 * >
		 */
		AddTransportLine({0, 0}, jactorio::data::TransportLineData::LineOrientation::down);
		AddTransportLine({0, 1}, jactorio::data::TransportLineData::LineOrientation::right);


		TlRemoveEvents({0, 1});


		jactorio::game::LogicChunk& logic_chunk                     = worldData_.LogicGetAllChunks().begin()->second;
		std::vector<jactorio::game::ChunkStructLayer>& struct_layer =
			logic_chunk.GetStruct(jactorio::game::LogicChunk::StructLayer::transport_line);

		ASSERT_EQ(struct_layer.size(), 1);
		EXPECT_FLOAT_EQ(struct_layer[0].positionX, 0.f);
		EXPECT_FLOAT_EQ(struct_layer[0].positionY, 0.f);

		auto* line_segment = static_cast<jactorio::game::TransportLineSegment*>(struct_layer[0].uniqueData);
		EXPECT_EQ(line_segment->terminationType, jactorio::game::TransportLineSegment::TerminationType::straight);
		EXPECT_EQ(line_segment->length, 1);
	}

	TEST_F(TransportLineTest, SetOrientation) {
		// When the orientation is set, the member "set" should also be updated

		// Arbitrary segment is fine since no logic updates are performed
		auto segment = jactorio::game::TransportLineSegment{
			jactorio::data::Orientation::left,
			jactorio::game::TransportLineSegment::TerminationType::straight,
			1
		};

		jactorio::data::TransportLineData line_data{segment};

		line_data.SetOrientation(jactorio::data::TransportLineData::LineOrientation::down);
		EXPECT_EQ(line_data.set, static_cast<uint16_t>(jactorio::data::TransportLineData::LineOrientation::down));

		line_data.SetOrientation(jactorio::data::TransportLineData::LineOrientation::left_down);
		EXPECT_EQ(line_data.set, static_cast<uint16_t>(jactorio::data::TransportLineData::LineOrientation::left_down));
	}


	TEST_F(TransportLineTest, OrientationUp1) {
		/*
		 * > ^
		 */
		AddLeftTransportLine(jactorio::data::TransportLineData::LineOrientation::right);
		ValidateResultOrientation(jactorio::data::Orientation::up,
		                          jactorio::data::TransportLineData::LineOrientation::right_up);
	}

	TEST_F(TransportLineTest, OrientationUp2) {
		/*
		 *   ^ <
		 */
		AddRightTransportLine(jactorio::data::TransportLineData::LineOrientation::left);
		ValidateResultOrientation(jactorio::data::Orientation::up,
		                          jactorio::data::TransportLineData::LineOrientation::left_up);
	}

	TEST_F(TransportLineTest, OrientationUp3) {
		/*
		 * > ^ <
		 */
		// Top and bottom points to one line, line should be straight

		AddLeftTransportLine(jactorio::data::TransportLineData::LineOrientation::right);
		AddRightTransportLine(jactorio::data::TransportLineData::LineOrientation::left);
		ValidateResultOrientation(jactorio::data::Orientation::up,
		                          jactorio::data::TransportLineData::LineOrientation::up);
	}

	TEST_F(TransportLineTest, OrientationUp4) {
		/*
		 * > ^ 
		 *   >
		 */
		AddLeftTransportLine(jactorio::data::TransportLineData::LineOrientation::right);
		AddBottomTransportLine(jactorio::data::TransportLineData::LineOrientation::right_down);
		ValidateResultOrientation(jactorio::data::Orientation::up,
		                          jactorio::data::TransportLineData::LineOrientation::right_up);
	}

	TEST_F(TransportLineTest, OrientationUp5) {
		/*
		 * > ^ 
		 *   ^
		 */
		AddLeftTransportLine(jactorio::data::TransportLineData::LineOrientation::right);
		AddBottomTransportLine(jactorio::data::TransportLineData::LineOrientation::up);
		ValidateResultOrientation(jactorio::data::Orientation::up,
		                          jactorio::data::TransportLineData::LineOrientation::up);
	}

	TEST_F(TransportLineTest, OrientationUp6) {
		/*
		 * < ^ <
		 */

		AddLeftTransportLine(jactorio::data::TransportLineData::LineOrientation::left);
		AddRightTransportLine(jactorio::data::TransportLineData::LineOrientation::left);
		ValidateResultOrientation(jactorio::data::Orientation::up,
		                          jactorio::data::TransportLineData::LineOrientation::left_up);
	}

	// ===

	TEST_F(TransportLineTest, OrientationRight1) {
		/*
		 *  v
		 *  >
		 */
		AddTopTransportLine(jactorio::data::TransportLineData::LineOrientation::down);
		ValidateResultOrientation(jactorio::data::Orientation::right,
		                          jactorio::data::TransportLineData::LineOrientation::down_right);
	}

	TEST_F(TransportLineTest, OrientationRight2) {
		/*
		 * >
		 * ^
		 */
		AddBottomTransportLine(jactorio::data::TransportLineData::LineOrientation::up);
		ValidateResultOrientation(jactorio::data::Orientation::right,
		                          jactorio::data::TransportLineData::LineOrientation::up_right);
	}

	TEST_F(TransportLineTest, OrientationRight3) {
		// Top and bottom points to one line, line should be straight

		AddTopTransportLine(jactorio::data::TransportLineData::LineOrientation::down);
		AddBottomTransportLine(jactorio::data::TransportLineData::LineOrientation::up);
		ValidateResultOrientation(jactorio::data::Orientation::right,
		                          jactorio::data::TransportLineData::LineOrientation::right);
	}

	TEST_F(TransportLineTest, OrientationRight4) {
		/*
		 *   v
		 * ^ >
		 */
		AddTopTransportLine(jactorio::data::TransportLineData::LineOrientation::down);
		AddLeftTransportLine(jactorio::data::TransportLineData::LineOrientation::up);  // Does not point to center
		ValidateResultOrientation(jactorio::data::Orientation::right,
		                          jactorio::data::TransportLineData::LineOrientation::down_right);
	}

	TEST_F(TransportLineTest, OrientationRight5) {
		/*
		 *   v
		 * > >
		 */
		AddTopTransportLine(jactorio::data::TransportLineData::LineOrientation::down);
		AddLeftTransportLine(
			jactorio::data::TransportLineData::LineOrientation::right);  // Points at center, center now straight 
		ValidateResultOrientation(jactorio::data::Orientation::right,
		                          jactorio::data::TransportLineData::LineOrientation::right);
	}

	TEST_F(TransportLineTest, OrientationRight6) {
		/*
		 * ^
		 * >
		 * ^
		 */
		AddTopTransportLine(jactorio::data::TransportLineData::LineOrientation::up);
		AddBottomTransportLine(jactorio::data::TransportLineData::LineOrientation::up);
		ValidateResultOrientation(jactorio::data::Orientation::right,
		                          jactorio::data::TransportLineData::LineOrientation::up_right);
	}

	// ===

	TEST_F(TransportLineTest, OrientationDown1) {
		AddLeftTransportLine(jactorio::data::TransportLineData::LineOrientation::right);
		ValidateResultOrientation(jactorio::data::Orientation::down,
		                          jactorio::data::TransportLineData::LineOrientation::right_down);
	}

	TEST_F(TransportLineTest, OrientationDown2) {
		AddRightTransportLine(jactorio::data::TransportLineData::LineOrientation::left);
		ValidateResultOrientation(jactorio::data::Orientation::down,
		                          jactorio::data::TransportLineData::LineOrientation::left_down);
	}

	TEST_F(TransportLineTest, OrientationDown3) {
		// Top and bottom points to one line, line should be straight

		AddLeftTransportLine(jactorio::data::TransportLineData::LineOrientation::right);
		AddRightTransportLine(jactorio::data::TransportLineData::LineOrientation::left);
		ValidateResultOrientation(jactorio::data::Orientation::down,
		                          jactorio::data::TransportLineData::LineOrientation::down);
	}

	TEST_F(TransportLineTest, OrientationDown4) {
		AddLeftTransportLine(jactorio::data::TransportLineData::LineOrientation::right);
		AddTopTransportLine(jactorio::data::TransportLineData::LineOrientation::left);
		ValidateResultOrientation(jactorio::data::Orientation::down,
		                          jactorio::data::TransportLineData::LineOrientation::right_down);
	}

	TEST_F(TransportLineTest, OrientationDown5) {
		AddLeftTransportLine(jactorio::data::TransportLineData::LineOrientation::right);
		AddTopTransportLine(jactorio::data::TransportLineData::LineOrientation::right_down);
		ValidateResultOrientation(jactorio::data::Orientation::down,
		                          jactorio::data::TransportLineData::LineOrientation::down);
	}

	TEST_F(TransportLineTest, OrientationDown6) {
		AddLeftTransportLine(jactorio::data::TransportLineData::LineOrientation::left);
		AddRightTransportLine(jactorio::data::TransportLineData::LineOrientation::left);
		ValidateResultOrientation(jactorio::data::Orientation::down,
		                          jactorio::data::TransportLineData::LineOrientation::left_down);
	}

	// ===

	TEST_F(TransportLineTest, OrientationLeft1) {
		AddTopTransportLine(jactorio::data::TransportLineData::LineOrientation::down);
		ValidateResultOrientation(jactorio::data::Orientation::left,
		                          jactorio::data::TransportLineData::LineOrientation::down_left);
	}

	TEST_F(TransportLineTest, OrientationLeft2) {
		AddBottomTransportLine(jactorio::data::TransportLineData::LineOrientation::up);
		ValidateResultOrientation(jactorio::data::Orientation::left,
		                          jactorio::data::TransportLineData::LineOrientation::up_left);
	}

	TEST_F(TransportLineTest, OrientationLeft3) {
		AddTopTransportLine(jactorio::data::TransportLineData::LineOrientation::down);
		AddBottomTransportLine(jactorio::data::TransportLineData::LineOrientation::up);
		ValidateResultOrientation(jactorio::data::Orientation::left,
		                          jactorio::data::TransportLineData::LineOrientation::left);
	}

	TEST_F(TransportLineTest, OrientationLeft4) {
		AddTopTransportLine(jactorio::data::TransportLineData::LineOrientation::down);
		AddRightTransportLine(jactorio::data::TransportLineData::LineOrientation::up);
		ValidateResultOrientation(jactorio::data::Orientation::left,
		                          jactorio::data::TransportLineData::LineOrientation::down_left);
	}

	TEST_F(TransportLineTest, OrientationLeft5) {
		AddTopTransportLine(jactorio::data::TransportLineData::LineOrientation::down);
		AddRightTransportLine(jactorio::data::TransportLineData::LineOrientation::left);
		ValidateResultOrientation(jactorio::data::Orientation::left,
		                          jactorio::data::TransportLineData::LineOrientation::left);
	}

	TEST_F(TransportLineTest, OrientationLeft6) {
		AddTopTransportLine(jactorio::data::TransportLineData::LineOrientation::up);
		AddBottomTransportLine(jactorio::data::TransportLineData::LineOrientation::up);
		ValidateResultOrientation(jactorio::data::Orientation::left,
		                          jactorio::data::TransportLineData::LineOrientation::up_left);
	}


	// ======================================================================
	// Side only

	void ValidateBendToSideOnly(jactorio::game::WorldData& world_data,
	                                const int32_t center_x, const int32_t center_y) {
		auto& logic_chunk     = world_data.LogicGetAllChunks().at(world_data.GetChunkC(0, 0));
		auto& transport_lines =
			logic_chunk.GetStruct(jactorio::game::LogicChunk::StructLayer::transport_line);

		ASSERT_EQ(transport_lines.size(), 3);

		{
			jactorio::game::ChunkStructLayer& line_layer = transport_lines[1];
			auto* line = static_cast<jactorio::game::TransportLineSegment*>(line_layer.uniqueData);

			EXPECT_EQ(line->terminationType, jactorio::game::TransportLineSegment::TerminationType::right_only);
			EXPECT_EQ(line->length, 2);
			EXPECT_FLOAT_EQ(line_layer.positionX, center_x);
			EXPECT_FLOAT_EQ(line_layer.positionY, center_y);
		}
		{
			jactorio::game::ChunkStructLayer& line_layer = transport_lines[2];
			auto* line = static_cast<jactorio::game::TransportLineSegment*>(line_layer.uniqueData);

			EXPECT_EQ(line->terminationType, jactorio::game::TransportLineSegment::TerminationType::left_only);
			EXPECT_EQ(line->length, 2);
			EXPECT_FLOAT_EQ(line_layer.positionX, center_x);
			EXPECT_FLOAT_EQ(line_layer.positionY, center_y);
		}

	}

	// Change bend to side only as initially, it forms a bend without the line on top / bottom
	TEST_F(TransportLineTest, OnBuildUpChangeBendToSideOnly) {

		/*
		 *   ^
		 * > ^ < 
		 */
		BuildTransportLine(jactorio::data::Orientation::up, {1, 0});
		BuildTransportLine(jactorio::data::Orientation::up, {1, 1});

		BuildTransportLine(jactorio::data::Orientation::left, {2, 1});
		BuildTransportLine(jactorio::data::Orientation::right, {0, 1});

		ValidateBendToSideOnly(worldData_, 1, 1);
		EXPECT_EQ(GetLineSegmentIndex({0, 1}), 1);
		EXPECT_EQ(GetLineSegmentIndex({2, 1}), 1);
	}

	TEST_F(TransportLineTest, OnBuildRightChangeBendToSideOnly) {

		/*
		 *   v
		 *   > > 
		 *   ^
		 */
		BuildTransportLine(jactorio::data::Orientation::right, {1, 1});
		BuildTransportLine(jactorio::data::Orientation::right, {2, 1});

		BuildTransportLine(jactorio::data::Orientation::up, {1, 2});
		BuildTransportLine(jactorio::data::Orientation::down, {1, 0});

		ValidateBendToSideOnly(worldData_, 1, 1);
	}

	TEST_F(TransportLineTest, OnBuildDownChangeBendToSideOnly) {

		/*
		 * > v < 
		 *   v 
		 */
		BuildTransportLine(jactorio::data::Orientation::down, {1, 1});
		BuildTransportLine(jactorio::data::Orientation::down, {1, 2});

		BuildTransportLine(jactorio::data::Orientation::right, {0, 1});
		BuildTransportLine(jactorio::data::Orientation::left, {2, 1});

		ValidateBendToSideOnly(worldData_, 1, 1);
	}

	TEST_F(TransportLineTest, OnBuildLeftChangeBendToSideOnly) {

		/*
		 *   v
		 * < < 
		 *   ^
		 */
		BuildTransportLine(jactorio::data::Orientation::left, {0, 1});
		BuildTransportLine(jactorio::data::Orientation::left, {1, 1});

		BuildTransportLine(jactorio::data::Orientation::down, {1, 0});
		BuildTransportLine(jactorio::data::Orientation::up, {1, 2});

		ValidateBendToSideOnly(worldData_, 1, 1);
	}


	TEST_F(TransportLineTest, OnBuildUpUpdateNeighboringSegmentToSideOnly) {
		/*   
		 * > ^ < 
		 */
		/* Order:
		 *   
		 * 1 3 2
		 */
		BuildTransportLine(jactorio::data::Orientation::right, {0, 0});
		BuildTransportLine(jactorio::data::Orientation::left, {2, 0});

		BuildTransportLine(jactorio::data::Orientation::up, {1, 0});


		auto& logic_chunk     = worldData_.LogicGetAllChunks().at(worldData_.GetChunkC(0, 0));
		auto& transport_lines =
			logic_chunk.GetStruct(jactorio::game::LogicChunk::StructLayer::transport_line);

		ASSERT_EQ(transport_lines.size(), 3);

		{
			jactorio::game::ChunkStructLayer& line_layer = transport_lines[0];
			EXPECT_FLOAT_EQ(line_layer.positionX, 1);
			EXPECT_FLOAT_EQ(line_layer.positionY, 0);

			auto* line = static_cast<jactorio::game::TransportLineSegment*>(line_layer.uniqueData);
			EXPECT_EQ(line->terminationType, jactorio::game::TransportLineSegment::TerminationType::left_only);
		}
		{
			jactorio::game::ChunkStructLayer& line_layer = transport_lines[1];
			EXPECT_FLOAT_EQ(line_layer.positionX, 1);
			EXPECT_FLOAT_EQ(line_layer.positionY, 0);

			auto* line = static_cast<jactorio::game::TransportLineSegment*>(line_layer.uniqueData);
			EXPECT_EQ(line->terminationType, jactorio::game::TransportLineSegment::TerminationType::right_only);
		}

		EXPECT_EQ(GetLineSegmentIndex({0, 0}), 1);
		EXPECT_EQ(GetLineSegmentIndex({2, 0}), 1);
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
		BuildTransportLine(jactorio::data::Orientation::up, {1, 2});
		BuildTransportLine(jactorio::data::Orientation::down, {1, 0});

		BuildTransportLine(jactorio::data::Orientation::right, {1, 1});


		auto& logic_chunk     = worldData_.LogicGetAllChunks().at(worldData_.GetChunkC(0, 0));
		auto& transport_lines =
			logic_chunk.GetStruct(jactorio::game::LogicChunk::StructLayer::transport_line);

		ASSERT_EQ(transport_lines.size(), 3);

		{
			jactorio::game::ChunkStructLayer& line_layer = transport_lines[0];
			EXPECT_FLOAT_EQ(line_layer.positionX, 1);
			EXPECT_FLOAT_EQ(line_layer.positionY, 1);

			auto* line = static_cast<jactorio::game::TransportLineSegment*>(line_layer.uniqueData);
			EXPECT_EQ(line->terminationType, jactorio::game::TransportLineSegment::TerminationType::right_only);
		}
		{
			jactorio::game::ChunkStructLayer& line_layer = transport_lines[1];
			EXPECT_FLOAT_EQ(line_layer.positionX, 1);
			EXPECT_FLOAT_EQ(line_layer.positionY, 1);

			auto* line = static_cast<jactorio::game::TransportLineSegment*>(line_layer.uniqueData);
			EXPECT_EQ(line->terminationType, jactorio::game::TransportLineSegment::TerminationType::left_only);
		}

		EXPECT_EQ(GetLineSegmentIndex({1, 2}), 1);
		EXPECT_EQ(GetLineSegmentIndex({1, 0}), 1);
	}

	TEST_F(TransportLineTest, OnBuildDownUpdateNeighboringSegmentToSideOnly) {

		/*   
		 * > v < 
		 */
		/* Order:
		 *   
		 * 1 3 2 
		 */
		BuildTransportLine(jactorio::data::Orientation::right, {0, 0});
		BuildTransportLine(jactorio::data::Orientation::left, {2, 0});

		BuildTransportLine(jactorio::data::Orientation::down, {1, 0});


		auto& logic_chunk     = worldData_.LogicGetAllChunks().at(worldData_.GetChunkC(0, 0));
		auto& transport_lines =
			logic_chunk.GetStruct(jactorio::game::LogicChunk::StructLayer::transport_line);

		ASSERT_EQ(transport_lines.size(), 3);

		{
			jactorio::game::ChunkStructLayer& line_layer = transport_lines[0];
			EXPECT_FLOAT_EQ(line_layer.positionX, 1);
			EXPECT_FLOAT_EQ(line_layer.positionY, 0);

			auto* line = static_cast<jactorio::game::TransportLineSegment*>(line_layer.uniqueData);
			EXPECT_EQ(line->terminationType, jactorio::game::TransportLineSegment::TerminationType::right_only);
		}
		{
			jactorio::game::ChunkStructLayer& line_layer = transport_lines[1];
			EXPECT_FLOAT_EQ(line_layer.positionX, 1);
			EXPECT_FLOAT_EQ(line_layer.positionY, 0);

			auto* line = static_cast<jactorio::game::TransportLineSegment*>(line_layer.uniqueData);
			EXPECT_EQ(line->terminationType, jactorio::game::TransportLineSegment::TerminationType::left_only);
		}

		EXPECT_EQ(GetLineSegmentIndex({0, 0}), 1);
		EXPECT_EQ(GetLineSegmentIndex({2, 0}), 1);
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
		BuildTransportLine(jactorio::data::Orientation::down, {0, 0});
		BuildTransportLine(jactorio::data::Orientation::up, {0, 2});

		BuildTransportLine(jactorio::data::Orientation::left, {0, 1});


		auto& logic_chunk     = worldData_.LogicGetAllChunks().at(worldData_.GetChunkC(0, 0));
		auto& transport_lines =
			logic_chunk.GetStruct(jactorio::game::LogicChunk::StructLayer::transport_line);

		ASSERT_EQ(transport_lines.size(), 3);

		{
			jactorio::game::ChunkStructLayer& line_layer = transport_lines[0];
			EXPECT_FLOAT_EQ(line_layer.positionX, 0);
			EXPECT_FLOAT_EQ(line_layer.positionY, 1);

			auto* line = static_cast<jactorio::game::TransportLineSegment*>(line_layer.uniqueData);
			EXPECT_EQ(line->terminationType, jactorio::game::TransportLineSegment::TerminationType::right_only);
		}
		{
			jactorio::game::ChunkStructLayer& line_layer = transport_lines[1];
			EXPECT_FLOAT_EQ(line_layer.positionX, 0);
			EXPECT_FLOAT_EQ(line_layer.positionY, 1);

			auto* line = static_cast<jactorio::game::TransportLineSegment*>(line_layer.uniqueData);
			EXPECT_EQ(line->terminationType, jactorio::game::TransportLineSegment::TerminationType::left_only);
		}

		EXPECT_EQ(GetLineSegmentIndex({0, 0}), 1);
		EXPECT_EQ(GetLineSegmentIndex({0, 2}), 1);
	}


	// ======================================================================
	// Connecting segments

	TEST_F(TransportLineTest, OnRemoveSetNeighborTargetSegment) {
		// After removing a transport line, anything that points to it as a target_segment needs to be set to NULL
		BuildTransportLine(jactorio::data::Orientation::left, {0, 0});
		BuildTransportLine(jactorio::data::Orientation::up, {0, 1});


		TlRemoveEvents({0, 0});


		jactorio::game::LogicChunk& logic_chunk                     = worldData_.LogicGetAllChunks().begin()->second;
		std::vector<jactorio::game::ChunkStructLayer>& struct_layer =
			logic_chunk.GetStruct(jactorio::game::LogicChunk::StructLayer::transport_line);

		ASSERT_EQ(struct_layer.size(), 1);
		// Set back to nullptr
		EXPECT_EQ(
			static_cast<jactorio::game::TransportLineSegment*>(struct_layer[0].uniqueData)->targetSegment,
			nullptr);
	}

	TEST_F(TransportLineTest, OnBuildConnectTransportLineSegmentsLeading) {
		// A transport line pointing to another one will set the target_segment
		/*
		 * ^ <
		 * 1 2
		 */

		{
			auto& layer = worldData_.GetTile(0, 0)
			                        ->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity);
			layer.prototypeData = &lineProto_;
			TlBuildEvents({0, 0}, jactorio::data::Orientation::up);
		}
		{
			// Second transport line should connect to first
			auto& layer = worldData_.GetTile(1, 0)
			                        ->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity);
			layer.prototypeData = &lineProto_;
			TlBuildEvents({1, 0}, jactorio::data::Orientation::left);
		}

		auto& logic_chunk     = worldData_.LogicGetAllChunks().at(worldData_.GetChunkC(0, 0));
		auto& transport_lines =
			logic_chunk.GetStruct(jactorio::game::LogicChunk::StructLayer::transport_line);

		ASSERT_EQ(transport_lines.size(), 2);
		auto* line = static_cast<jactorio::game::TransportLineSegment*>(transport_lines[1].uniqueData);
		EXPECT_EQ(line->targetSegment, transport_lines[0].uniqueData);
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
			auto& layer = worldData_.GetTile(1, 0)
			                        ->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity);
			layer.prototypeData = &lineProto_;
			TlBuildEvents({1, 0}, jactorio::data::Orientation::left);
		}
		{
			auto& layer = worldData_.GetTile(0, 0)
			                        ->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity);
			layer.prototypeData = &lineProto_;
			TlBuildEvents({0, 0}, jactorio::data::Orientation::up);
		}

		auto& logic_chunk     = worldData_.LogicGetAllChunks().at(worldData_.GetChunkC(0, 0));
		auto& transport_lines =
			logic_chunk.GetStruct(jactorio::game::LogicChunk::StructLayer::transport_line);

		ASSERT_EQ(transport_lines.size(), 2);
		auto* line = static_cast<jactorio::game::TransportLineSegment*>(transport_lines[0].uniqueData);
		EXPECT_EQ(line->targetSegment, transport_lines[1].uniqueData);
	}

	TEST_F(TransportLineTest, OnBuildNoConnectTransportLineSegments) {
		// Do not connect transport line segments pointed at each other
		/*
		 * > <
		 */

		{
			auto& layer = worldData_.GetTile(0, 0)
			                        ->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity);
			layer.prototypeData = &lineProto_;
			TlBuildEvents({0, 0}, jactorio::data::Orientation::down);
		}
		{
			auto& layer = worldData_.GetTile(0, 1)
			                        ->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity);
			layer.prototypeData = &lineProto_;
			TlBuildEvents({0, 1}, jactorio::data::Orientation::up);
		}

		auto& logic_chunk     = worldData_.LogicGetAllChunks().at(worldData_.GetChunkC(0, 0));
		auto& transport_lines =
			logic_chunk.GetStruct(jactorio::game::LogicChunk::StructLayer::transport_line);

		ASSERT_EQ(transport_lines.size(), 2);

		auto* line = static_cast<jactorio::game::TransportLineSegment*>(transport_lines[0].uniqueData);
		EXPECT_EQ(line->targetSegment, nullptr);

		auto* line_b = static_cast<jactorio::game::TransportLineSegment*>(transport_lines[1].uniqueData);
		EXPECT_EQ(line_b->targetSegment, nullptr);
	}

	// ======================================================================
	// Grouping

	TEST_F(TransportLineTest, OnBuildUpGroupAhead) {
		// When placed behind a line with the same orientation, join the previous line by extending its length
		/*
		 * ^
		 * ^
		 */
		BuildTransportLine(jactorio::data::Orientation::up, {0, 0});
		BuildTransportLine(jactorio::data::Orientation::up, {0, 1});

		GroupAheadValidate({0, 0}, {0, 1});
	}

	TEST_F(TransportLineTest, OnBuildUpGroupBehind) {
		// When placed ahead of a line with the same orientation, shift the head to the current position 
		/*
		 * ^
		 * ^
		 */
		BuildTransportLine(jactorio::data::Orientation::up, {0, 1});
		BuildTransportLine(jactorio::data::Orientation::up, {0, 0});

		GroupAheadValidate({0, 0}, {0, 1});
	}

	TEST_F(TransportLineTest, OnBuildUpGroupAheadCrossChunk) {
		// Since grouping ahead requires adjustment of a position within the current logic chunk, crossing chunks requires special logic
		worldData_.AddChunk(jactorio::game::Chunk{0, -1});

		BuildTransportLine(jactorio::data::Orientation::up, {0, -1});
		BuildTransportLine(jactorio::data::Orientation::up, {0, 0});

		bool found = false;
		jactorio::data::TransportLine::GetLineStructLayer(
			worldData_, 0, 0,
			[&found](auto& s_layer, auto&) mutable {
				found      = true;
				auto& data = *static_cast<jactorio::game::TransportLineSegment*>(s_layer.uniqueData);

				EXPECT_EQ(data.length, 1);
			});

		EXPECT_TRUE(found);
	}

	TEST_F(TransportLineTest, OnBuildUpGroupBehindCrossChunk) {
		// Since grouping ahead requires adjustment of a position within the current logic chunk, crossing chunks requires special logic
		worldData_.AddChunk(jactorio::game::Chunk{0, -1});

		BuildTransportLine(jactorio::data::Orientation::up, {0, 0});
		BuildTransportLine(jactorio::data::Orientation::up, {0, -1});

		bool found = false;
		jactorio::data::TransportLine::GetLineStructLayer(
			worldData_, 0, -1,
			[&found](auto& s_layer, auto&) mutable {
				found      = true;
				auto& data = *static_cast<jactorio::game::TransportLineSegment*>(s_layer.uniqueData);

				EXPECT_EQ(data.length, 1);
			});

		EXPECT_TRUE(found);
	}

	TEST_F(TransportLineTest, OnBuildRightGroupAhead) {
		/*
		 * > >
		 */
		BuildTransportLine(jactorio::data::Orientation::right, {1, 0});
		BuildTransportLine(jactorio::data::Orientation::right, {0, 0});

		GroupAheadValidate({1, 0}, {0, 0});
	}

	TEST_F(TransportLineTest, OnBuildRightGroupBehind) {
		/*
		 * > >
		 */
		BuildTransportLine(jactorio::data::Orientation::right, {0, 0});
		BuildTransportLine(jactorio::data::Orientation::right, {1, 0});

		GroupAheadValidate({1, 0}, {0, 0});
	}

	TEST_F(TransportLineTest, OnBuildDownGroupAhead) {
		/*
		 * v
		 * v
		 */
		BuildTransportLine(jactorio::data::Orientation::down, {0, 1});
		BuildTransportLine(jactorio::data::Orientation::down, {0, 0});

		GroupAheadValidate({0, 1}, {0, 0});
	}

	TEST_F(TransportLineTest, OnBuildDownGroupBehind) {
		/*
		 * v
		 * v
		 */
		BuildTransportLine(jactorio::data::Orientation::down, {0, 0});
		BuildTransportLine(jactorio::data::Orientation::down, {0, 1});

		GroupAheadValidate({0, 1}, {0, 0});
	}

	TEST_F(TransportLineTest, OnBuildLeftGroupAhead) {
		/*
		 * < <
		 */
		BuildTransportLine(jactorio::data::Orientation::left, {0, 0});
		BuildTransportLine(jactorio::data::Orientation::left, {1, 0});

		GroupAheadValidate({0, 0}, {1, 0});
	}

	TEST_F(TransportLineTest, OnBuildLeftGroupBehind) {
		/*
		 * < <
		 */
		BuildTransportLine(jactorio::data::Orientation::left, {1, 0});
		BuildTransportLine(jactorio::data::Orientation::left, {0, 0});

		GroupAheadValidate({0, 0}, {1, 0});
	}

	TEST_F(TransportLineTest, OnRemoveGroupBegin) {
		// Removing beginning of grouped transport segment
		// Create new segment, do not shorten segment ahead

		/*
		 * > >  
		 */
		AddTransportLine({0, 0}, jactorio::data::TransportLineData::LineOrientation::right);
		AddTransportLine({1, 0}, jactorio::data::TransportLineData::LineOrientation::right);

		TlRemoveEvents({1, 0});

		const auto& struct_layer = GetTransportLines({0, 0});
		ASSERT_EQ(struct_layer.size(), 1);
		EXPECT_EQ(static_cast<jactorio::game::TransportLineSegment*>(struct_layer[0].uniqueData)->length, 1);
	}

	TEST_F(TransportLineTest, OnRemoveGroupBeginBend) {
		/*
		 * > > v
		 */
		AddTransportLine({0, 0}, jactorio::data::TransportLineData::LineOrientation::right);
		AddTransportLine({1, 0}, jactorio::data::TransportLineData::LineOrientation::right);
		AddTransportLine({2, 0}, jactorio::data::TransportLineData::LineOrientation::down);

		TlRemoveEvents({1, 0});

		const auto& struct_layer = GetTransportLines({0, 0});
		ASSERT_EQ(struct_layer.size(), 2);
		EXPECT_EQ(static_cast<jactorio::game::TransportLineSegment*>(struct_layer[0].uniqueData)->length, 1);
	}

	TEST_F(TransportLineTest, OnRemoveGroupMiddle) {
		// Removing middle of grouped transport segment
		// Create new segment, shorten segment ahead

		/*
		 * > /> > >
		 */
		AddTransportLine({0, 0}, jactorio::data::TransportLineData::LineOrientation::right);
		AddTransportLine({1, 0}, jactorio::data::TransportLineData::LineOrientation::right);
		AddTransportLine({2, 0}, jactorio::data::TransportLineData::LineOrientation::right);
		AddTransportLine({3, 0}, jactorio::data::TransportLineData::LineOrientation::right);

		TlRemoveEvents({1, 0});

		const auto& struct_layer = GetTransportLines({0, 0});
		ASSERT_EQ(struct_layer.size(), 2);
		EXPECT_EQ(static_cast<jactorio::game::TransportLineSegment*>(struct_layer[0].uniqueData)->length, 2);
		EXPECT_EQ(static_cast<jactorio::game::TransportLineSegment*>(struct_layer[1].uniqueData)->length, 1);
	}

	TEST_F(TransportLineTest, OnRemoveGroupMiddleUpdateTargetSegment) {
		// The new segment created when removing a group needs to update target segments so point to the newly created segment
		worldData_.AddChunk(jactorio::game::Chunk(-1, 0));

		/*
		 * > > /> >
		 *   ^ 
		 */
		AddTransportLine({0, 0}, jactorio::data::TransportLineData::LineOrientation::right);
		AddTransportLine({1, 0}, jactorio::data::TransportLineData::LineOrientation::right);
		AddTransportLine({2, 0}, jactorio::data::TransportLineData::LineOrientation::right);

		AddTransportLine({-1, 0}, jactorio::data::TransportLineData::LineOrientation::right);
		AddTransportLine({0, 1}, jactorio::data::TransportLineData::LineOrientation::up);

		TlRemoveEvents({1, 0});

		const auto& struct_layer = GetTransportLines({0, 0});
		ASSERT_EQ(struct_layer.size(), 3);
		EXPECT_EQ(GetLineData({0, 1}).lineSegment.get().targetSegment,
		          static_cast<jactorio::game::TransportLineSegment*>(struct_layer[1].uniqueData)->targetSegment);


		const auto& struct_layer_left = GetTransportLines({-1, 0});
		ASSERT_EQ(struct_layer_left.size(), 1);
		EXPECT_EQ(GetLineData({-1, 0}).lineSegment.get().targetSegment,
		          static_cast<jactorio::game::TransportLineSegment*>(struct_layer[1].uniqueData)->targetSegment);
	}

	TEST_F(TransportLineTest, OnRemoveGroupUpdateIndex) {
		// The segment index must be updated when a formally bend segment becomes straight
		/*
		 * />
		 * ^ 
		 */
		AddTransportLine({0, 0}, jactorio::data::TransportLineData::LineOrientation::right);
		AddTransportLine({0, 1}, jactorio::data::TransportLineData::LineOrientation::up);

		TlRemoveEvents({0, 0});

		const auto& struct_layer = GetTransportLines({0, 0});
		ASSERT_EQ(struct_layer.size(), 1);

		EXPECT_EQ(GetLineData({0, 1}).lineSegmentIndex, 0);
	}

	TEST_F(TransportLineTest, OnRemoveGroupEnd) {
		// Removing end of grouped transport segment
		// Create no new segment, shorten segment ahead

		/*
		 * > >  
		 */
		AddTransportLine({0, 0}, jactorio::data::TransportLineData::LineOrientation::right);
		AddTransportLine({1, 0}, jactorio::data::TransportLineData::LineOrientation::right);

		TlRemoveEvents({0, 0});

		const auto& struct_layer = GetTransportLines({0, 0});
		ASSERT_EQ(struct_layer.size(), 1);
		EXPECT_EQ(static_cast<jactorio::game::TransportLineSegment*>(struct_layer[0].uniqueData)->length, 1);
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
			auto& layer = worldData_.GetTile(0, 0)
			                        ->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity);
			layer.prototypeData = &lineProto_;
			TlBuildEvents({0, 0}, jactorio::data::Orientation::right);
		}
		{
			auto& layer = worldData_.GetTile(1, 0)
			                        ->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity);
			layer.prototypeData = &lineProto_;
			TlBuildEvents({1, 0}, jactorio::data::Orientation::down);
		}
		{
			auto& layer = worldData_.GetTile(1, 1)
			                        ->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity);
			layer.prototypeData = &lineProto_;
			TlBuildEvents({1, 1}, jactorio::data::Orientation::left);
		}
		{
			auto& layer = worldData_.GetTile(0, 1)
			                        ->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity);
			layer.prototypeData = &lineProto_;
			TlBuildEvents({0, 1}, jactorio::data::Orientation::up);
		}


		auto& logic_chunk     = worldData_.LogicGetAllChunks().at(worldData_.GetChunkC(0, 0));
		auto& transport_lines =
			logic_chunk.GetStruct(jactorio::game::LogicChunk::StructLayer::transport_line);

		ASSERT_EQ(transport_lines.size(), 4);

		// Right
		{
			jactorio::game::ChunkStructLayer& line_layer = transport_lines[0];
			auto* line = static_cast<jactorio::game::TransportLineSegment*>(line_layer.uniqueData);
			EXPECT_EQ(line->terminationType, jactorio::game::TransportLineSegment::TerminationType::bend_right);

			EXPECT_EQ(line->length, 2);
			EXPECT_FLOAT_EQ(line_layer.positionX, 1.f);
			EXPECT_FLOAT_EQ(line_layer.positionY, 0.f);
		}
		// Down
		{
			jactorio::game::ChunkStructLayer& line_layer = transport_lines[1];
			auto* line = static_cast<jactorio::game::TransportLineSegment*>(line_layer.uniqueData);
			EXPECT_EQ(line->terminationType, jactorio::game::TransportLineSegment::TerminationType::bend_right);

			EXPECT_EQ(line->length, 2);
			EXPECT_FLOAT_EQ(line_layer.positionX, 1.f);
			EXPECT_FLOAT_EQ(line_layer.positionY, 1.f);
		}

		// Left
		{
			jactorio::game::ChunkStructLayer& line_layer = transport_lines[2];
			auto* line = static_cast<jactorio::game::TransportLineSegment*>(line_layer.uniqueData);
			EXPECT_EQ(line->terminationType, jactorio::game::TransportLineSegment::TerminationType::bend_right);

			EXPECT_EQ(line->length, 2);
			EXPECT_FLOAT_EQ(line_layer.positionX, 0.f);
			EXPECT_FLOAT_EQ(line_layer.positionY, 1.f);
		}

		// Up
		{
			jactorio::game::ChunkStructLayer& line_layer = transport_lines[3];
			auto* line = static_cast<jactorio::game::TransportLineSegment*>(line_layer.uniqueData);
			EXPECT_EQ(line->terminationType, jactorio::game::TransportLineSegment::TerminationType::bend_right);

			EXPECT_EQ(line->length, 2);
			EXPECT_FLOAT_EQ(line_layer.positionX, 0.f);
			EXPECT_FLOAT_EQ(line_layer.positionY, 0.f);
		}
	}
}
