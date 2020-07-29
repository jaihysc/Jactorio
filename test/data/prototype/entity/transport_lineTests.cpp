// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "data/prototype/entity/transport_belt.h"
#include "data/prototype/entity/transport_line.h"
#include "game/logic/transport_segment.h"

// ======================================================================
// Tests for the various bend orientations

namespace jactorio::data
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
		game::WorldData worldData_{};
		game::LogicData logicData_{};

		TransportBelt lineProto_{};

		void SetUp() override {
			worldData_.AddChunk(game::Chunk{0, 0});
		}

		// ======================================================================

		static game::TransportSegment& GetSegment(game::ChunkTileLayer* tile_layer) {
			return *tile_layer->GetUniqueData<TransportLineData>()->lineSegment;
		}


		///
		/// \brief Sets the prototype pointer for a transport line at tile
		void BuildTransportLine(
			const WorldCoord world_coords,
			const Orientation orientation) {

			auto& layer = worldData_.GetTile(world_coords.x, world_coords.y)
			                        ->GetLayer(game::ChunkTile::ChunkLayer::entity);
			layer.prototypeData = &lineProto_;
			TlBuildEvents(world_coords, orientation);
		}

		///
		/// \brief Creates new Transport line segment alongside data at tile
		void AddTransportLine(const TransportLineData::LineOrientation orientation,
		                      const WorldCoordAxis x,
		                      const WorldCoordAxis y) {
			auto& layer         = worldData_.GetTile(x, y)->GetLayer(game::ChunkTile::ChunkLayer::entity);
			layer.prototypeData = &lineProto_;

			TlBuildEvents({x, y}, TransportLineData::ToOrientation(orientation));
		}

		void AddTransportLine(const WorldCoord& world_coords,
		                      const TransportLineData::LineOrientation orientation) {
			AddTransportLine(orientation, world_coords.x, world_coords.y);
		}

		/// Creates a transport line with the provided orientation above/right/below/left of 1, 1
		void AddTopTransportLine(const TransportLineData::LineOrientation orientation) {
			AddTransportLine(orientation, 1, 0);
		}

		void AddRightTransportLine(const TransportLineData::LineOrientation orientation) {
			AddTransportLine(orientation, 2, 1);
		}

		void AddBottomTransportLine(const TransportLineData::LineOrientation orientation) {
			AddTransportLine(orientation, 1, 2);
		}

		void AddLeftTransportLine(const TransportLineData::LineOrientation orientation) {
			AddTransportLine(orientation, 0, 1);
		}

		///
		/// \brief Validates that a tile at coords 1,1 with the placement orientation produces the expected line orientation
		void ValidateResultOrientation(const Orientation placement_orientation,
		                               const TransportLineData::LineOrientation expected_line_orientation) {
			EXPECT_EQ(lineProto_.OnRGetSpriteSet(placement_orientation, worldData_, {1, 1}),
			          static_cast<data::Sprite::SetT>(expected_line_orientation));
		}

		// ======================================================================
	private:
		void DispatchNeighborUpdate(const WorldCoord& emit_coords,
		                            const WorldCoord& receive_coords,
		                            const Orientation emit_orientation) {

			auto* tile = worldData_.GetTile(receive_coords);
			if (!tile)
				return;

			auto& layer = tile->GetLayer(game::ChunkTile::ChunkLayer::entity);
			if (!layer.prototypeData)
				return;

			static_cast<const Entity*>(layer.prototypeData)
				->OnNeighborUpdate(worldData_, logicData_, emit_coords, receive_coords, emit_orientation);
		}

	protected:
		///
		/// \brief Dispatches the appropriate events for when a transport line is built
		void TlBuildEvents(const WorldCoord& world_coords,
		                   const Orientation orientation) {
			auto& layer = worldData_.GetTile(world_coords)
			                        ->GetLayer(game::ChunkTile::ChunkLayer::entity);

			lineProto_.OnBuild(worldData_, logicData_, world_coords, layer, orientation);

			// Call on_neighbor_update for the 4 sides
			DispatchNeighborUpdate(world_coords,
			                       {world_coords.x, world_coords.y - 1}, Orientation::up);

			DispatchNeighborUpdate(world_coords,
			                       {world_coords.x + 1, world_coords.y}, Orientation::right);

			DispatchNeighborUpdate(world_coords,
			                       {world_coords.x, world_coords.y + 1}, Orientation::down);

			DispatchNeighborUpdate(world_coords,
			                       {world_coords.x - 1, world_coords.y}, Orientation::left);
		}

		///
		/// \brief Dispatches the appropriate events AFTER a transport line is removed 
		void TlRemoveEvents(const WorldCoord& world_coords) {

			auto& layer = worldData_.GetTile(world_coords)
			                        ->GetLayer(game::ChunkTile::ChunkLayer::entity);

			lineProto_.OnRemove(worldData_, logicData_, world_coords, layer);

			// Call on_neighbor_update for the 4 sides
			DispatchNeighborUpdate(world_coords,
			                       {world_coords.x, world_coords.y - 1}, Orientation::up);

			DispatchNeighborUpdate(world_coords,
			                       {world_coords.x + 1, world_coords.y}, Orientation::right);

			DispatchNeighborUpdate(world_coords,
			                       {world_coords.x, world_coords.y + 1}, Orientation::down);

			DispatchNeighborUpdate(world_coords,
			                       {world_coords.x - 1, world_coords.y}, Orientation::left);
		}

		// Bend

		///
		/// \param l_index index for left only segment in logic group
		/// \param r_index index for right only segment in logic group
		void ValidateBendToSideOnly(const size_t l_index = 2, const size_t r_index = 1) {
			game::Chunk& chunk = *worldData_.GetChunkC(0, 0);
			auto& logic_group  = chunk.GetLogicGroup(game::Chunk::LogicGroup::transport_line);

			ASSERT_EQ(logic_group.size(), 3);

			{
				auto& line_segment = GetSegment(logic_group[r_index]);
				EXPECT_EQ(line_segment.terminationType, jactorio::game::TransportSegment::TerminationType::right_only);
				EXPECT_EQ(line_segment.length, 2);
			}
			{
				auto& line_segment = GetSegment(logic_group[l_index]);
				EXPECT_EQ(line_segment.terminationType, jactorio::game::TransportSegment::TerminationType::left_only);
				EXPECT_EQ(line_segment.length, 2);
			}
		}

		// Grouping

		std::vector<game::ChunkTileLayer*>& GetTransportLines(
			const ChunkCoord& chunk_coords) {
			return worldData_.GetChunkC(chunk_coords.x, chunk_coords.y)
			                 ->GetLogicGroup(game::Chunk::LogicGroup::transport_line);
		}

		J_NODISCARD auto& GetLineData(const WorldCoord& world_coords) const {
			return *static_cast<const TransportLineData*>(
				worldData_.GetTile(world_coords)->GetLayer(
					game::ChunkTile::ChunkLayer::entity).GetUniqueData()
			);
		}

		auto GetLineSegmentIndex(const WorldCoord& world_coords) const {
			return GetLineData(world_coords).lineSegmentIndex;
		}


		///
		/// \param first Leading segment
		/// \param second Segment placed behind leading segment
		void GroupingValidate(const WorldCoord& first,
		                      const WorldCoord& second) {
			ASSERT_EQ(GetTransportLines({0, 0}).size(), 1);  // 0, 0 is chunk coordinate
			EXPECT_EQ(GetLineData(first).lineSegment->length, 2);

			EXPECT_EQ(GetLineSegmentIndex(first), 0);
			EXPECT_EQ(GetLineSegmentIndex(second), 1);

			EXPECT_EQ(GetLineData(first).lineSegment->targetSegment, nullptr);
		}

		void GroupBehindValidate(const WorldCoord& first,
		                         const WorldCoord& second) {
			GroupingValidate(first, second);
			EXPECT_EQ(GetLineData(first).lineSegment->itemOffset, 1);

			EXPECT_EQ(worldData_.LogicGetChunks().size(), 1);
		}
	};


	// ======================================================================
	// General tests

	TEST_F(TransportLineTest, OnBuildCreateTransportLineSegment) {
		// Should create a transport line segment and add its chunk to logic chunks
		worldData_.AddChunk(game::Chunk{-1, 0});

		auto& layer = worldData_.GetTile(-5, 0)
		                        ->GetLayer(game::ChunkTile::ChunkLayer::entity);
		layer.prototypeData = &lineProto_;

		TlBuildEvents({-5, 0}, Orientation::right);

		// ======================================================================

		// Added current chunk as a logic chunk
		ASSERT_EQ(worldData_.LogicGetChunks().size(), 1);

		auto& tile_layers = GetTransportLines({-1, 0});

		// Should have created a transport line structure
		ASSERT_EQ(tile_layers.size(), 1);
		ASSERT_TRUE(dynamic_cast<jactorio::data::TransportLineData*>(tile_layers.front()->GetUniqueData()));

		auto& line_segment = GetSegment(tile_layers[0]);
		EXPECT_EQ(line_segment.direction, jactorio::data::Orientation::right);
		EXPECT_EQ(line_segment.terminationType, jactorio::game::TransportSegment::TerminationType::straight);
		EXPECT_EQ(line_segment.length, 1);
	}

	TEST_F(TransportLineTest, OnRemoveDeleteTransportLineSegment) {
		// Removing a transport line needs to delete the transport line segment associated with it
		AddTransportLine({0, 0}, TransportLineData::LineOrientation::left);

		TlRemoveEvents({0, 0});

		// Transport line structure count should be 0 as it was removed
		EXPECT_TRUE(worldData_.GetChunkC({0, 0})->GetLogicGroup(jactorio::game::Chunk::LogicGroup::transport_line).empty());
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
		AddTransportLine({1, 1}, TransportLineData::LineOrientation::up);

		AddTransportLine({2, 0}, TransportLineData::LineOrientation::right);
		AddTransportLine({1, 0}, TransportLineData::LineOrientation::right);
		AddTransportLine({0, 0}, TransportLineData::LineOrientation::right);

		auto& tile_layers = GetTransportLines({0, 0});

		ASSERT_EQ(tile_layers.size(), 2);

		auto& line_segment = GetSegment(tile_layers[0]);
		EXPECT_EQ(line_segment.terminationType, jactorio::game::TransportSegment::TerminationType::right_only);
		EXPECT_EQ(line_segment.length, 2);
	}

	TEST_F(TransportLineTest, OnRemoveNeighborBend) {
		// Removing a transport line must update neighboring transport segments
		// The bottom segment goes from right_only to bend_right

		/*
		 * /> > > 
		 *    ^ 
		 */
		AddTransportLine({0, 0}, TransportLineData::LineOrientation::right);
		AddTransportLine({1, 0}, TransportLineData::LineOrientation::right);
		AddTransportLine({2, 0}, TransportLineData::LineOrientation::right);

		AddTransportLine({1, 1}, TransportLineData::LineOrientation::up);


		TlRemoveEvents({0, 0});

		auto& tile_layers = GetTransportLines({0, 0});

		ASSERT_EQ(tile_layers.size(), 2);

		auto& line_segment = GetSegment(tile_layers[1]);
		EXPECT_EQ(line_segment.terminationType, jactorio::game::TransportSegment::TerminationType::bend_right);
		EXPECT_EQ(line_segment.length, 2);
	}

	TEST_F(TransportLineTest, OnBuildUpdateNeighboringLines) {

		/*
		 * >
		 * ^
		 */
		AddTopTransportLine(TransportLineData::LineOrientation::right);

		auto& layer = worldData_.GetTile(1, 1)
		                        ->GetLayer(game::ChunkTile::ChunkLayer::entity);


		auto proto          = TransportBelt{};
		layer.prototypeData = &proto;


		// Should update line above, turn right to a up-right
		TlBuildEvents({1, 1}, Orientation::up);

		{
			auto& result_layer = worldData_.GetTile(1, 0)
			                               ->GetLayer(game::ChunkTile::ChunkLayer::entity);

			EXPECT_EQ(
				static_cast<jactorio::data::TransportLineData*>(result_layer.GetUniqueData())->orientation,
				jactorio::data::TransportLineData::LineOrientation::up_right
			);
		}
	}

	TEST_F(TransportLineTest, OnRemoveUpdateNeighboringLines) {
		// The on_remove event should update the orientations of the neighboring belts to if the current transport
		// line is not there

		/*
		 *  v
		 *  >
		 *  ^
		 */
		AddTopTransportLine(TransportLineData::LineOrientation::down);
		AddBottomTransportLine(TransportLineData::LineOrientation::up);
		AddTransportLine(TransportLineData::LineOrientation::right, 1, 1);  // Between the 2 above and below

		auto& layer = worldData_.GetTile(1, 2)
		                        ->GetLayer(game::ChunkTile::ChunkLayer::entity);
		layer.prototypeData = &lineProto_;


		// Removing the bottom line makes the center one bend down-right
		TlRemoveEvents({1, 2});

		{
			auto& result_layer = worldData_.GetTile(1, 1)
			                               ->GetLayer(game::ChunkTile::ChunkLayer::entity);

			EXPECT_EQ(
				static_cast<jactorio::data::TransportLineData*>(result_layer.GetUniqueData())->orientation,
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
		                             ->GetLayer(game::ChunkTile::ChunkLayer::entity);
		down_layer.prototypeData = &lineProto_;
		TlBuildEvents({0, 0}, Orientation::down);

		auto& left_layer = worldData_.GetTile(1, 0)
		                             ->GetLayer(game::ChunkTile::ChunkLayer::entity);
		left_layer.prototypeData = &lineProto_;
		TlBuildEvents({1, 0}, Orientation::left);

		auto& tile_layers = GetTransportLines({0, 0});

		ASSERT_EQ(tile_layers.size(), 2);

		auto& segment = GetSegment(tile_layers[1]);
		EXPECT_EQ(segment.terminationType, jactorio::game::TransportSegment::TerminationType::bend_left);

		// Should have lengthened segment and moved x 1 left
		EXPECT_EQ(segment.length, 2);
		EXPECT_EQ(static_cast<jactorio::data::TransportLineData*>(left_layer.GetUniqueData())->lineSegmentIndex, 1);
	}

	TEST_F(TransportLineTest, OnBuildBendingTransportLineSegmentLeading) {
		// Change the transport_line_segment termination type in accordance with orientation when placed ahead of existing line 

		auto& left_layer = worldData_.GetTile(1, 0)
		                             ->GetLayer(game::ChunkTile::ChunkLayer::entity);
		left_layer.prototypeData = &lineProto_;
		TlBuildEvents({1, 0}, Orientation::left);

		auto& down_layer = worldData_.GetTile(0, 0)
		                             ->GetLayer(game::ChunkTile::ChunkLayer::entity);
		down_layer.prototypeData = &lineProto_;
		TlBuildEvents({0, 0}, Orientation::down);

		auto& tile_layers = GetTransportLines({0, 0});

		ASSERT_EQ(tile_layers.size(), 2);

		auto& segment = GetSegment(tile_layers[0]);
		EXPECT_EQ(segment.terminationType, jactorio::game::TransportSegment::TerminationType::bend_left);

		// Should have lengthened segment and moved x 1 left
		EXPECT_EQ(segment.length, 2);
		EXPECT_EQ(static_cast<jactorio::data::TransportLineData*>(left_layer.GetUniqueData())->lineSegmentIndex, 1);
	}


	TEST_F(TransportLineTest, OnRemoveBend) {
		// After removing a transport line the transport line that connects to it with a bend must become straight,
		// decrement segment_length by 1, and shift its position

		/*
		 * v
		 * >
		 */
		AddTransportLine({0, 0}, TransportLineData::LineOrientation::down);
		AddTransportLine({0, 1}, TransportLineData::LineOrientation::right);


		TlRemoveEvents({0, 1});

		auto& tile_layers = GetTransportLines({0, 0});

		ASSERT_EQ(tile_layers.size(), 1);

		auto& segment = GetSegment(tile_layers[0]);
		EXPECT_EQ(segment.terminationType, jactorio::game::TransportSegment::TerminationType::straight);
		EXPECT_EQ(segment.length, 1);
	}

	TEST_F(TransportLineTest, SetOrientation) {
		// When the orientation is set, the member "set" should also be updated

		// Arbitrary segment is fine since no logic updates are performed
		const auto segment = std::make_shared<game::TransportSegment>(
			Orientation::left,
			game::TransportSegment::TerminationType::straight,
			1
		);

		TransportLineData line_data{segment};
		line_data.lineSegmentIndex = 1;  // Prevents it from attempting to delete line segment

		line_data.SetOrientation(TransportLineData::LineOrientation::down);
		EXPECT_EQ(line_data.set, static_cast<uint16_t>(jactorio::data::TransportLineData::LineOrientation::down));

		line_data.SetOrientation(TransportLineData::LineOrientation::left_down);
		EXPECT_EQ(line_data.set, static_cast<uint16_t>(jactorio::data::TransportLineData::LineOrientation::left_down));
	}


	TEST_F(TransportLineTest, OrientationUp1) {
		/*
		 * > ^
		 */
		AddLeftTransportLine(TransportLineData::LineOrientation::right);
		ValidateResultOrientation(Orientation::up,
		                          TransportLineData::LineOrientation::right_up);
	}

	TEST_F(TransportLineTest, OrientationUp2) {
		/*
		 *   ^ <
		 */
		AddRightTransportLine(TransportLineData::LineOrientation::left);
		ValidateResultOrientation(Orientation::up,
		                          TransportLineData::LineOrientation::left_up);
	}

	TEST_F(TransportLineTest, OrientationUp3) {
		/*
		 * > ^ <
		 */
		// Top and bottom points to one line, line should be straight

		AddLeftTransportLine(TransportLineData::LineOrientation::right);
		AddRightTransportLine(TransportLineData::LineOrientation::left);
		ValidateResultOrientation(Orientation::up,
		                          TransportLineData::LineOrientation::up);
	}

	TEST_F(TransportLineTest, OrientationUp4) {
		/*
		 * > ^ 
		 *   >
		 */
		AddLeftTransportLine(TransportLineData::LineOrientation::right);
		AddBottomTransportLine(TransportLineData::LineOrientation::right_down);
		ValidateResultOrientation(Orientation::up,
		                          TransportLineData::LineOrientation::right_up);
	}

	TEST_F(TransportLineTest, OrientationUp5) {
		/*
		 * > ^ 
		 *   ^
		 */
		AddLeftTransportLine(TransportLineData::LineOrientation::right);
		AddBottomTransportLine(TransportLineData::LineOrientation::up);
		ValidateResultOrientation(Orientation::up,
		                          TransportLineData::LineOrientation::up);
	}

	TEST_F(TransportLineTest, OrientationUp6) {
		/*
		 * < ^ <
		 */

		AddLeftTransportLine(TransportLineData::LineOrientation::left);
		AddRightTransportLine(TransportLineData::LineOrientation::left);
		ValidateResultOrientation(Orientation::up,
		                          TransportLineData::LineOrientation::left_up);
	}

	// ===

	TEST_F(TransportLineTest, OrientationRight1) {
		/*
		 *  v
		 *  >
		 */
		AddTopTransportLine(TransportLineData::LineOrientation::down);
		ValidateResultOrientation(Orientation::right,
		                          TransportLineData::LineOrientation::down_right);
	}

	TEST_F(TransportLineTest, OrientationRight2) {
		/*
		 * >
		 * ^
		 */
		AddBottomTransportLine(TransportLineData::LineOrientation::up);
		ValidateResultOrientation(Orientation::right,
		                          TransportLineData::LineOrientation::up_right);
	}

	TEST_F(TransportLineTest, OrientationRight3) {
		// Top and bottom points to one line, line should be straight

		AddTopTransportLine(TransportLineData::LineOrientation::down);
		AddBottomTransportLine(TransportLineData::LineOrientation::up);
		ValidateResultOrientation(Orientation::right,
		                          TransportLineData::LineOrientation::right);
	}

	TEST_F(TransportLineTest, OrientationRight4) {
		/*
		 *   v
		 * ^ >
		 */
		AddTopTransportLine(TransportLineData::LineOrientation::down);
		AddLeftTransportLine(TransportLineData::LineOrientation::up);  // Does not point to center
		ValidateResultOrientation(Orientation::right,
		                          TransportLineData::LineOrientation::down_right);
	}

	TEST_F(TransportLineTest, OrientationRight5) {
		/*
		 *   v
		 * > >
		 */
		AddTopTransportLine(TransportLineData::LineOrientation::down);
		AddLeftTransportLine(
			TransportLineData::LineOrientation::right);  // Points at center, center now straight 
		ValidateResultOrientation(Orientation::right,
		                          TransportLineData::LineOrientation::right);
	}

	TEST_F(TransportLineTest, OrientationRight6) {
		/*
		 * ^
		 * >
		 * ^
		 */
		AddTopTransportLine(TransportLineData::LineOrientation::up);
		AddBottomTransportLine(TransportLineData::LineOrientation::up);
		ValidateResultOrientation(Orientation::right,
		                          TransportLineData::LineOrientation::up_right);
	}

	// ===

	TEST_F(TransportLineTest, OrientationDown1) {
		AddLeftTransportLine(TransportLineData::LineOrientation::right);
		ValidateResultOrientation(Orientation::down,
		                          TransportLineData::LineOrientation::right_down);
	}

	TEST_F(TransportLineTest, OrientationDown2) {
		AddRightTransportLine(TransportLineData::LineOrientation::left);
		ValidateResultOrientation(Orientation::down,
		                          TransportLineData::LineOrientation::left_down);
	}

	TEST_F(TransportLineTest, OrientationDown3) {
		// Top and bottom points to one line, line should be straight

		AddLeftTransportLine(TransportLineData::LineOrientation::right);
		AddRightTransportLine(TransportLineData::LineOrientation::left);
		ValidateResultOrientation(Orientation::down,
		                          TransportLineData::LineOrientation::down);
	}

	TEST_F(TransportLineTest, OrientationDown4) {
		AddLeftTransportLine(TransportLineData::LineOrientation::right);
		AddTopTransportLine(TransportLineData::LineOrientation::left);
		ValidateResultOrientation(Orientation::down,
		                          TransportLineData::LineOrientation::right_down);
	}

	TEST_F(TransportLineTest, OrientationDown5) {
		AddLeftTransportLine(TransportLineData::LineOrientation::right);
		AddTopTransportLine(TransportLineData::LineOrientation::right_down);
		ValidateResultOrientation(Orientation::down,
		                          TransportLineData::LineOrientation::down);
	}

	TEST_F(TransportLineTest, OrientationDown6) {
		AddLeftTransportLine(TransportLineData::LineOrientation::left);
		AddRightTransportLine(TransportLineData::LineOrientation::left);
		ValidateResultOrientation(Orientation::down,
		                          TransportLineData::LineOrientation::left_down);
	}

	// ===

	TEST_F(TransportLineTest, OrientationLeft1) {
		AddTopTransportLine(TransportLineData::LineOrientation::down);
		ValidateResultOrientation(Orientation::left,
		                          TransportLineData::LineOrientation::down_left);
	}

	TEST_F(TransportLineTest, OrientationLeft2) {
		AddBottomTransportLine(TransportLineData::LineOrientation::up);
		ValidateResultOrientation(Orientation::left,
		                          TransportLineData::LineOrientation::up_left);
	}

	TEST_F(TransportLineTest, OrientationLeft3) {
		AddTopTransportLine(TransportLineData::LineOrientation::down);
		AddBottomTransportLine(TransportLineData::LineOrientation::up);
		ValidateResultOrientation(Orientation::left,
		                          TransportLineData::LineOrientation::left);
	}

	TEST_F(TransportLineTest, OrientationLeft4) {
		AddTopTransportLine(TransportLineData::LineOrientation::down);
		AddRightTransportLine(TransportLineData::LineOrientation::up);
		ValidateResultOrientation(Orientation::left,
		                          TransportLineData::LineOrientation::down_left);
	}

	TEST_F(TransportLineTest, OrientationLeft5) {
		AddTopTransportLine(TransportLineData::LineOrientation::down);
		AddRightTransportLine(TransportLineData::LineOrientation::left);
		ValidateResultOrientation(Orientation::left,
		                          TransportLineData::LineOrientation::left);
	}

	TEST_F(TransportLineTest, OrientationLeft6) {
		AddTopTransportLine(TransportLineData::LineOrientation::up);
		AddBottomTransportLine(TransportLineData::LineOrientation::up);
		ValidateResultOrientation(Orientation::left,
		                          TransportLineData::LineOrientation::up_left);
	}


	// ======================================================================
	// Side only

	// Change bend to side only as initially, it forms a bend without the line on top / bottom
	TEST_F(TransportLineTest, OnBuildUpChangeBendToSideOnly) {

		/*
		 *   ^
		 * > ^ < 
		 */
		BuildTransportLine({1, 0}, Orientation::up);
		BuildTransportLine({1, 1}, Orientation::up);

		BuildTransportLine({2, 1}, Orientation::left);
		BuildTransportLine({0, 1}, Orientation::right);

		ValidateBendToSideOnly();
		EXPECT_EQ(GetLineSegmentIndex({0, 1}), 1);
		EXPECT_EQ(GetLineSegmentIndex({2, 1}), 1);

		EXPECT_EQ(GetLineData({0, 1}).lineSegment->targetInsertOffset, 1);
		EXPECT_EQ(GetLineData({2, 1}).lineSegment->targetInsertOffset, 1);

		// Incremented 1 forwards	
		EXPECT_EQ(GetLineData({0, 1}).lineSegment->itemOffset, 1);
		EXPECT_EQ(GetLineData({2, 1}).lineSegment->itemOffset, 1);
	}

	TEST_F(TransportLineTest, OnBuildRightChangeBendToSideOnly) {

		/*
		 *   v
		 *   > > 
		 *   ^
		 */
		BuildTransportLine({1, 2}, Orientation::up);
		BuildTransportLine({1, 0}, Orientation::down);

		BuildTransportLine({1, 1}, Orientation::right);
		BuildTransportLine({2, 1}, Orientation::right);


		ValidateBendToSideOnly(1, 0);
		EXPECT_EQ(GetLineData({1, 0}).lineSegment->targetInsertOffset, 0);
		EXPECT_EQ(GetLineData({1, 2}).lineSegment->targetInsertOffset, 0);

		EXPECT_EQ(GetLineData({1, 0}).lineSegment->itemOffset, 1);
		EXPECT_EQ(GetLineData({1, 2}).lineSegment->itemOffset, 1);
	}

	TEST_F(TransportLineTest, OnBuildDownChangeBendToSideOnly) {

		/*
		 * > v < 
		 *   v 
		 */
		BuildTransportLine({1, 1}, Orientation::down);
		BuildTransportLine({1, 2}, Orientation::down);

		BuildTransportLine({0, 1}, Orientation::right);
		BuildTransportLine({2, 1}, Orientation::left);

		ValidateBendToSideOnly();
		EXPECT_EQ(GetLineData({0, 1}).lineSegment->targetInsertOffset, 0);
		EXPECT_EQ(GetLineData({2, 1}).lineSegment->targetInsertOffset, 0);
	}

	TEST_F(TransportLineTest, OnBuildLeftChangeBendToSideOnly) {

		/*
		 *   v
		 * < < 
		 *   ^
		 */
		BuildTransportLine({0, 1}, Orientation::left);
		BuildTransportLine({1, 1}, Orientation::left);

		BuildTransportLine({1, 0}, Orientation::down);
		BuildTransportLine({1, 2}, Orientation::up);

		ValidateBendToSideOnly();
		EXPECT_EQ(GetLineData({1, 0}).lineSegment->targetInsertOffset, 1);
		EXPECT_EQ(GetLineData({1, 2}).lineSegment->targetInsertOffset, 1);
	}

	TEST_F(TransportLineTest, OnBuildUpUpdateNeighboringSegmentToSideOnly) {
		/*
		 *   ^
		 *   ^
		 * > ^ < 
		 */
		/* Order:
		 *
		 *   3
		 *   4
		 * 1 5 2
		 */
		BuildTransportLine({0, 2}, Orientation::right);
		BuildTransportLine({2, 2}, Orientation::left);

		BuildTransportLine({1, 0}, Orientation::up);
		BuildTransportLine({1, 1}, Orientation::up);
		BuildTransportLine({1, 2}, Orientation::up);

		auto& tile_layers = GetTransportLines({0, 0});

		ASSERT_EQ(tile_layers.size(), 3);

		{
			auto& line_segment = GetSegment(tile_layers[0]);
			EXPECT_EQ(line_segment.terminationType, jactorio::game::TransportSegment::TerminationType::left_only);
		}
		{
			auto& line_segment = GetSegment(tile_layers[1]);
			EXPECT_EQ(line_segment.terminationType, jactorio::game::TransportSegment::TerminationType::right_only);
		}

		EXPECT_EQ(GetLineSegmentIndex({0, 2}), 1);
		EXPECT_EQ(GetLineSegmentIndex({2, 2}), 1);

		EXPECT_EQ(GetLineData({0, 2}).lineSegment->targetInsertOffset, 2);
		EXPECT_EQ(GetLineData({2, 2}).lineSegment->targetInsertOffset, 2);
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
		BuildTransportLine({1, 2}, Orientation::up);
		BuildTransportLine({1, 0}, Orientation::down);

		BuildTransportLine({1, 1}, Orientation::right);

		auto& tile_layers = GetTransportLines({0, 0});

		ASSERT_EQ(tile_layers.size(), 3);

		{
			auto& line_segment = GetSegment(tile_layers[0]);
			EXPECT_EQ(line_segment.terminationType, jactorio::game::TransportSegment::TerminationType::right_only);
		}
		{
			auto& line_segment = GetSegment(tile_layers[1]);
			EXPECT_EQ(line_segment.terminationType, jactorio::game::TransportSegment::TerminationType::left_only);
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
		BuildTransportLine({0, 0}, Orientation::right);
		BuildTransportLine({2, 0}, Orientation::left);

		BuildTransportLine({1, 0}, Orientation::down);

		auto& tile_layers = GetTransportLines({0, 0});

		ASSERT_EQ(tile_layers.size(), 3);

		{
			auto& line_segment = GetSegment(tile_layers[0]);
			EXPECT_EQ(line_segment.terminationType, jactorio::game::TransportSegment::TerminationType::right_only);
			EXPECT_EQ(line_segment.itemOffset, 1);  // Incremented 1 when turned side only
		}
		{
			auto& line_segment = GetSegment(tile_layers[1]);
			EXPECT_EQ(line_segment.terminationType, jactorio::game::TransportSegment::TerminationType::left_only);
			EXPECT_EQ(line_segment.itemOffset, 1);
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
		BuildTransportLine({0, 0}, Orientation::down);
		BuildTransportLine({0, 2}, Orientation::up);

		BuildTransportLine({0, 1}, Orientation::left);

		auto& tile_layers = GetTransportLines({0, 0});

		ASSERT_EQ(tile_layers.size(), 3);

		{
			auto& line_segment = GetSegment(tile_layers[0]);
			EXPECT_EQ(line_segment.terminationType, jactorio::game::TransportSegment::TerminationType::right_only);
		}
		{
			auto& line_segment = GetSegment(tile_layers[1]);
			EXPECT_EQ(line_segment.terminationType, jactorio::game::TransportSegment::TerminationType::left_only);
		}

		EXPECT_EQ(GetLineSegmentIndex({0, 0}), 1);
		EXPECT_EQ(GetLineSegmentIndex({0, 2}), 1);
	}


	// ======================================================================
	// Connecting segments

	TEST_F(TransportLineTest, OnRemoveSetNeighborTargetSegment) {
		// After removing a transport line, anything that points to it as a target_segment needs to be set to NULL
		BuildTransportLine({0, 0}, Orientation::left);
		BuildTransportLine({0, 1}, Orientation::up);


		TlRemoveEvents({0, 0});

		auto& tile_layers = GetTransportLines({0, 0});

		ASSERT_EQ(tile_layers.size(), 1);
		// Set back to nullptr
		EXPECT_EQ(GetSegment(tile_layers[0]).targetSegment, nullptr);
	}

	TEST_F(TransportLineTest, OnBuildConnectTransportLineSegmentsLeading) {
		// A transport line pointing to another one will set the target_segment
		/*
		 * ^ <
		 * 1 2
		 */

		{
			auto& layer = worldData_.GetTile(0, 0)
			                        ->GetLayer(game::ChunkTile::ChunkLayer::entity);
			layer.prototypeData = &lineProto_;
			TlBuildEvents({0, 0}, Orientation::up);
		}
		{
			// Second transport line should connect to first
			auto& layer = worldData_.GetTile(1, 0)
			                        ->GetLayer(game::ChunkTile::ChunkLayer::entity);
			layer.prototypeData = &lineProto_;
			TlBuildEvents({1, 0}, Orientation::left);
		}

		auto& tile_layers = GetTransportLines({0, 0});

		ASSERT_EQ(tile_layers.size(), 2);
		auto& line_segment = GetSegment(tile_layers[1]);
		EXPECT_EQ(line_segment.targetSegment, &GetSegment(tile_layers[0]));
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
			                        ->GetLayer(game::ChunkTile::ChunkLayer::entity);
			layer.prototypeData = &lineProto_;
			TlBuildEvents({1, 0}, Orientation::left);
		}
		{
			auto& layer = worldData_.GetTile(0, 0)
			                        ->GetLayer(game::ChunkTile::ChunkLayer::entity);
			layer.prototypeData = &lineProto_;
			TlBuildEvents({0, 0}, Orientation::up);
		}

		auto& tile_layers = GetTransportLines({0, 0});

		ASSERT_EQ(tile_layers.size(), 2);
		auto& line_segment = GetSegment(tile_layers[0]);
		EXPECT_EQ(line_segment.targetSegment, &GetSegment(tile_layers[1]));
	}

	TEST_F(TransportLineTest, OnBuildNoConnectTransportLineSegments) {
		// Do not connect transport line segments pointed at each other
		/*
		 * > <
		 */

		{
			auto& layer = worldData_.GetTile(0, 0)
			                        ->GetLayer(game::ChunkTile::ChunkLayer::entity);
			layer.prototypeData = &lineProto_;
			TlBuildEvents({0, 0}, Orientation::down);
		}
		{
			auto& layer = worldData_.GetTile(0, 1)
			                        ->GetLayer(game::ChunkTile::ChunkLayer::entity);
			layer.prototypeData = &lineProto_;
			TlBuildEvents({0, 1}, Orientation::up);
		}

		auto& tile_layers = GetTransportLines({0, 0});

		ASSERT_EQ(tile_layers.size(), 2);

		EXPECT_EQ(GetSegment(tile_layers[0]).targetSegment, nullptr);
		EXPECT_EQ(GetSegment(tile_layers[1]).targetSegment, nullptr);
	}

	// ======================================================================
	// Grouping

	TEST_F(TransportLineTest, OnBuildUpGroupAhead) {
		// When placed behind a line with the same orientation, join the previous line by extending its length
		/*
		 * ^
		 * ^
		 */
		BuildTransportLine({0, 0}, Orientation::up);
		BuildTransportLine({0, 1}, Orientation::up);

		GroupingValidate({0, 0}, {0, 1});
	}

	TEST_F(TransportLineTest, OnBuildUpGroupBehind) {
		// When placed ahead of a line with the same orientation, shift the head to the current position 
		/*
		 * ^
		 * ^
		 */
		BuildTransportLine({0, 1}, Orientation::up);
		BuildTransportLine({0, 0}, Orientation::up);

		GroupBehindValidate({0, 0}, {0, 1});
	}

	TEST_F(TransportLineTest, OnBuildUpGroupAheadCrossChunk) {
		// Since grouping ahead requires adjustment of a position within the current logic chunk, crossing chunks requires special logic
		worldData_.AddChunk(game::Chunk{0, -1});

		BuildTransportLine({0, -1}, Orientation::up);
		BuildTransportLine({0, 0}, Orientation::up);

		auto* segment = TransportLine::GetTransportSegment(worldData_, 0, 0);
		ASSERT_TRUE(segment);

		EXPECT_EQ(segment->get()->length, 1);
	}

	TEST_F(TransportLineTest, OnBuildUpGroupBehindCrossChunk) {
		// Since grouping ahead requires adjustment of a position within the current logic chunk, crossing chunks requires special logic
		worldData_.AddChunk(game::Chunk{0, -1});

		BuildTransportLine({0, 0}, Orientation::up);
		BuildTransportLine({0, -1}, Orientation::up);

		auto* segment = TransportLine::GetTransportSegment(worldData_, 0, -1);
		ASSERT_TRUE(segment);

		EXPECT_EQ(segment->get()->length, 1);
	}

	TEST_F(TransportLineTest, OnBuildRightGroupAhead) {
		/*
		 * > >
		 */
		BuildTransportLine({1, 0}, Orientation::right);
		BuildTransportLine({0, 0}, Orientation::right);

		GroupingValidate({1, 0}, {0, 0});
	}

	TEST_F(TransportLineTest, OnBuildRightGroupBehind) {
		/*
		 * > >
		 */
		BuildTransportLine({0, 0}, Orientation::right);
		BuildTransportLine({1, 0}, Orientation::right);

		GroupBehindValidate({1, 0}, {0, 0});
	}

	TEST_F(TransportLineTest, OnBuildDownGroupAhead) {
		/*
		 * v
		 * v
		 */
		BuildTransportLine({0, 1}, Orientation::down);
		BuildTransportLine({0, 0}, Orientation::down);

		GroupingValidate({0, 1}, {0, 0});
	}

	TEST_F(TransportLineTest, OnBuildDownGroupBehind) {
		/*
		 * v
		 * v
		 */
		BuildTransportLine({0, 0}, Orientation::down);
		BuildTransportLine({0, 1}, Orientation::down);

		GroupBehindValidate({0, 1}, {0, 0});
	}

	TEST_F(TransportLineTest, OnBuildLeftGroupAhead) {
		/*
		 * < <
		 */
		BuildTransportLine({0, 0}, Orientation::left);
		BuildTransportLine({1, 0}, Orientation::left);

		GroupingValidate({0, 0}, {1, 0});
	}

	TEST_F(TransportLineTest, OnBuildLeftGroupBehind) {
		/*
		 * < <
		 */
		BuildTransportLine({1, 0}, Orientation::left);
		BuildTransportLine({0, 0}, Orientation::left);

		GroupBehindValidate({0, 0}, {1, 0});
	}

	TEST_F(TransportLineTest, OnRemoveGroupBegin) {
		// Removing beginning of grouped transport segment
		// Create new segment, do not shorten segment ahead

		/*
		 * > >  
		 */
		AddTransportLine({0, 0}, TransportLineData::LineOrientation::right);
		AddTransportLine({1, 0}, TransportLineData::LineOrientation::right);

		TlRemoveEvents({1, 0});

		const auto& tile_layers = GetTransportLines({0, 0});
		ASSERT_EQ(tile_layers.size(), 1);
		EXPECT_EQ(GetSegment(tile_layers[0]).length, 1);

		EXPECT_EQ(worldData_.LogicGetChunks().size(), 1);
	}

	TEST_F(TransportLineTest, OnRemoveGroupBeginBend) {
		/*
		 * > > v
		 */
		AddTransportLine({0, 0}, TransportLineData::LineOrientation::right);
		AddTransportLine({1, 0}, TransportLineData::LineOrientation::right);
		AddTransportLine({2, 0}, TransportLineData::LineOrientation::down);

		TlRemoveEvents({1, 0});

		const auto& tile_layers = GetTransportLines({0, 0});
		ASSERT_EQ(tile_layers.size(), 2);
		EXPECT_EQ(GetSegment(tile_layers[0]).length, 1);

		EXPECT_EQ(worldData_.LogicGetChunks().size(), 1);
	}

	TEST_F(TransportLineTest, OnRemoveGroupMiddle) {
		// Removing middle of grouped transport segment
		// Create new segment, shorten segment ahead

		/*
		 * > /> > >
		 */
		AddTransportLine({0, 0}, TransportLineData::LineOrientation::right);
		AddTransportLine({1, 0}, TransportLineData::LineOrientation::right);
		AddTransportLine({2, 0}, TransportLineData::LineOrientation::right);
		AddTransportLine({3, 0}, TransportLineData::LineOrientation::right);

		TlRemoveEvents({1, 0});

		const auto& tile_layers = GetTransportLines({0, 0});
		ASSERT_EQ(tile_layers.size(), 2);
		EXPECT_EQ(GetSegment(tile_layers[0]).length, 2);

		auto& behind_segment = GetSegment(tile_layers[1]);
		EXPECT_EQ(behind_segment.length, 1);
		EXPECT_EQ(behind_segment.itemOffset, 0);  // Built 3 ahead: +3, remove at index 2: -2-1 = 0 

		EXPECT_EQ(worldData_.LogicGetChunks().size(), 1);
	}

	TEST_F(TransportLineTest, OnRemoveGroupMiddleUpdateTargetSegment) {
		// The new segment created when removing a group needs to update target segments so point to the newly created segment
		worldData_.AddChunk(game::Chunk(-1, 0));

		/*
		 * > > /> >
		 *   ^ 
		 */
		AddTransportLine({0, 0}, TransportLineData::LineOrientation::right);
		AddTransportLine({1, 0}, TransportLineData::LineOrientation::right);
		AddTransportLine({2, 0}, TransportLineData::LineOrientation::right);

		AddTransportLine({-1, 0}, TransportLineData::LineOrientation::right);
		AddTransportLine({0, 1}, TransportLineData::LineOrientation::up);

		TlRemoveEvents({1, 0});

		const auto& tile_layers = GetTransportLines({0, 0});
		ASSERT_EQ(tile_layers.size(), 3);
		EXPECT_EQ(GetLineData({0, 1}).lineSegment->targetSegment,
		          GetSegment(tile_layers[1]).targetSegment);


		const auto& tile_layers_left = GetTransportLines({-1, 0});
		ASSERT_EQ(tile_layers_left.size(), 1);
		EXPECT_EQ(GetLineData({-1, 0}).lineSegment->targetSegment,
		          GetSegment(tile_layers[1]).targetSegment);
	}

	TEST_F(TransportLineTest, OnRemoveGroupUpdateProperties) {
		// The segment index must be updated when a formally bend segment becomes straight
		// The itemOffset must be subtracted

		/*
		 * />
		 * ^ 
		 */
		BuildTransportLine({0, 1}, Orientation::up);
		BuildTransportLine({0, 0}, Orientation::right);

		const auto& struct_layer = GetTransportLines({0, 0});

		ASSERT_EQ(struct_layer.size(), 2);

		EXPECT_EQ(GetLineData({0, 1}).lineSegment->itemOffset, 1);


		// Remove


		TlRemoveEvents({0, 0});

		ASSERT_EQ(struct_layer.size(), 1);

		EXPECT_EQ(GetLineData({0, 1}).lineSegmentIndex, 0);
		EXPECT_EQ(GetLineData({0, 1}).lineSegment->itemOffset, 0);
	}

	TEST_F(TransportLineTest, OnRemoveGroupEnd) {
		// Removing end of grouped transport segment
		// Create no new segment, shorten segment ahead

		/*
		 * > >  
		 */
		AddTransportLine({0, 0}, TransportLineData::LineOrientation::right);
		AddTransportLine({1, 0}, TransportLineData::LineOrientation::right);

		TlRemoveEvents({0, 0});

		const auto& tile_layers = GetTransportLines({0, 0});
		ASSERT_EQ(tile_layers.size(), 1);
		EXPECT_EQ(GetSegment(tile_layers[0]).length, 1);
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
			                        ->GetLayer(game::ChunkTile::ChunkLayer::entity);
			layer.prototypeData = &lineProto_;
			TlBuildEvents({0, 0}, Orientation::right);
		}
		{
			auto& layer = worldData_.GetTile(1, 0)
			                        ->GetLayer(game::ChunkTile::ChunkLayer::entity);
			layer.prototypeData = &lineProto_;
			TlBuildEvents({1, 0}, Orientation::down);
		}
		{
			auto& layer = worldData_.GetTile(1, 1)
			                        ->GetLayer(game::ChunkTile::ChunkLayer::entity);
			layer.prototypeData = &lineProto_;
			TlBuildEvents({1, 1}, Orientation::left);
		}
		{
			auto& layer = worldData_.GetTile(0, 1)
			                        ->GetLayer(game::ChunkTile::ChunkLayer::entity);
			layer.prototypeData = &lineProto_;
			TlBuildEvents({0, 1}, Orientation::up);
		}


		auto& tile_layers = GetTransportLines({0, 0});
		ASSERT_EQ(tile_layers.size(), 4);

		// Right
		{
			auto& line_segment = GetSegment(tile_layers[0]);
			EXPECT_EQ(line_segment.terminationType, jactorio::game::TransportSegment::TerminationType::bend_right);
			EXPECT_EQ(line_segment.length, 2);
		}
		// Down
		{
			auto& line_segment = GetSegment(tile_layers[1]);
			EXPECT_EQ(line_segment.terminationType, jactorio::game::TransportSegment::TerminationType::bend_right);
			EXPECT_EQ(line_segment.length, 2);
		}

		// Left
		{
			auto& line_segment = GetSegment(tile_layers[2]);
			EXPECT_EQ(line_segment.terminationType, jactorio::game::TransportSegment::TerminationType::bend_right);
			EXPECT_EQ(line_segment.length, 2);
		}

		// Up
		{
			auto& line_segment = GetSegment(tile_layers[3]);
			EXPECT_EQ(line_segment.terminationType, jactorio::game::TransportSegment::TerminationType::bend_right);
			EXPECT_EQ(line_segment.length, 2);
		}
	}
}
