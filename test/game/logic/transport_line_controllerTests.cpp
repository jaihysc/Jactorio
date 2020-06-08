// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#include <gtest/gtest.h>

#include <memory>


#include "jactorioTests.h"
#include "data/prototype/entity/transport/transport_belt.h"
#include "game/logic/transport_line_controller.h"
#include "game/logic/transport_segment.h"
#include "game/world/world_data.h"

namespace jactorio::game
{
	// For line_logic and line_logic_precision
	void TestItemPositions(WorldData& world_data,
	                       const TransportSegment* up_segment,
	                       const TransportSegment* right_segment,
	                       const TransportSegment* down_segment,
	                       const TransportSegment* left_segment) {
		// Moving left at a speed of 0.01f per update, in 250 updates
		// Should reach end of transport line - Next update will change its direction to up

		// End of U | 5 - 2(0.7) / 0.01 = 360
		for (int i = 0; i < 360; ++i) {
			TransportLineLogicUpdate(world_data);
		}
		ASSERT_FLOAT_EQ(up_segment->right.lane.front().first.getAsDouble(), 0.f);
		ASSERT_EQ(up_segment->right.lane.size(), 1);

		// End of R | 4 - 2(0.7) / 0.01 = 260 updates
		for (int i = 0; i < 260; ++i) {
			TransportLineLogicUpdate(world_data);
		}
		ASSERT_FLOAT_EQ(right_segment->right.lane.front().first.getAsDouble(), 0.f);
		ASSERT_EQ(right_segment->right.lane.size(), 1);

		// End of D
		for (int i = 0; i < 360; ++i) {
			TransportLineLogicUpdate(world_data);
		}
		ASSERT_FLOAT_EQ(down_segment->right.lane.front().first.getAsDouble(), 0.f);
		ASSERT_EQ(down_segment->right.lane.size(), 1);

		// End of L 4 - 2(0.7)
		for (int i = 0; i < 260; ++i) {
			TransportLineLogicUpdate(world_data);
		}
		ASSERT_FLOAT_EQ(left_segment->right.lane.front().first.getAsDouble(), 0.f);
		ASSERT_EQ(left_segment->right.lane.size(), 1);
	}

	class TransportLineControllerTest : public testing::Test
	{
	protected:
		WorldData worldData_{};
		Chunk* chunk_ = nullptr;

		const std::unique_ptr<data::Item> itemProto_                   = std::make_unique<data::Item>();
		const std::unique_ptr<data::TransportBelt> transportBeltProto_ =
			std::make_unique<data::TransportBelt>();

		///
		/// \brief Creates a world, chunk and logic chunk at 0, 0
		void SetUp() override {
			chunk_ = worldData_.EmplaceChunk(0, 0);
			worldData_.LogicAddChunk(chunk_);
		}

		///
		/// \brief Creates tile UniqueData for TransportSegment
		void RegisterSegment(const Chunk::ChunkPair& world_coords,
		                     const std::shared_ptr<TransportSegment>& segment) {
			TestRegisterTransportSegment(worldData_, world_coords, segment, *transportBeltProto_.get());
		}
	};

	TEST_F(TransportLineControllerTest, LineLogic) {
		// Tests that items move as expected (within a chunk)

		transportBeltProto_->speed = 0.01f;

		// Segments (Logic chunk must be created first)
		const auto up_segment = std::make_shared<TransportSegment>(
			data::Orientation::up,
			TransportSegment::TerminationType::bend_right,
			5);
		const auto right_segment = std::make_shared<TransportSegment>(
			data::Orientation::right,
			TransportSegment::TerminationType::bend_right,
			4);
		const auto down_segment = std::make_shared<TransportSegment>(
			data::Orientation::down,
			TransportSegment::TerminationType::bend_right,
			5);
		auto left_segment = std::make_shared<TransportSegment>(
			data::Orientation::left,
			TransportSegment::TerminationType::bend_right,
			4);

		// What each transport segment empties into
		up_segment->targetSegment    = right_segment.get();
		right_segment->targetSegment = down_segment.get();
		down_segment->targetSegment  = left_segment.get();
		left_segment->targetSegment  = up_segment.get();

		RegisterSegment({0, 0}, up_segment);
		RegisterSegment({4, 0}, right_segment);
		RegisterSegment({4, 5}, down_segment);
		RegisterSegment({0, 5}, left_segment);

		// The actual lengths of the transport segments != the indicated length as it turns earlier

		// Insert item
		left_segment->AppendItem(false, 0.f, itemProto_.get());

		TestItemPositions(worldData_,
		                  up_segment.get(), right_segment.get(),
		                  down_segment.get(), left_segment.get());
	}

	/*
	TEST_F(TransportLineControllerTest, LineLogicPrecision) {
		// Tests for data type precision representing the distance between items

		const auto item_proto = std::make_unique<jactorio::data::Item>();
		const auto transport_belt_proto = std::make_unique<jactorio::data::Transport_belt>();
		transport_belt_proto->speed = 0.01f;

		// Segments (Logic chunk must be created first)
		auto* up_segment = new jactorio::game::Transport_line_segment(
			jactorio::data::Orientation::up,
			jactorio::game::Transport_line_segment::terminationType::bend_right,
			5);
		auto* right_segment = new jactorio::game::Transport_line_segment(
			jactorio::data::Orientation::right,
			jactorio::game::Transport_line_segment::terminationType::bend_right,
			4);
		auto* down_segment = new jactorio::game::Transport_line_segment(
			jactorio::data::Orientation::down,
			jactorio::game::Transport_line_segment::terminationType::bend_right,
			5);
		auto* left_segment = new jactorio::game::Transport_line_segment(
			jactorio::data::Orientation::left,
			jactorio::game::Transport_line_segment::terminationType::bend_right,
			4);

		// What each transport segment empties into
		up_segment->target_segment = right_segment;
		right_segment->target_segment = down_segment;
		down_segment->target_segment = left_segment;
		left_segment->target_segment = up_segment;
		{
			auto& up = logic_chunk_->get_struct(jactorio::game::Logic_chunk::structLayer::transport_line)
			                      .emplace_back(transport_belt_proto.get(), 0, 0);
			up.unique_data = up_segment;

			auto& right = logic_chunk_->get_struct(jactorio::game::Logic_chunk::structLayer::transport_line)
			                         .emplace_back(transport_belt_proto.get(), 4, 0);
			right.unique_data = right_segment;

			auto& down = logic_chunk_->get_struct(jactorio::game::Logic_chunk::structLayer::transport_line)
			                        .emplace_back(transport_belt_proto.get(), 4, 5);
			down.unique_data = down_segment;

			auto& left = logic_chunk_->get_struct(jactorio::game::Logic_chunk::structLayer::transport_line)
			                        .emplace_back(transport_belt_proto.get(), 0, 5);
			left.unique_data = left_segment;
		}


		// Insert item
		left_segment->append_item(false, 0.f, item_proto.get());

		// Should manage to make 100 000 laps
		for (int i = 0; i < 100000; ++i) {
			test_item_positions(world_data_, up_segment, right_segment, down_segment, left_segment);

			if (HasFatalFailure()) {
				printf("Precision failed on lap %d/100000\n", i + 1);
				FAIL();
			}
		}
	}
	*/

	TEST_F(TransportLineControllerTest, LineLogicFast) {
		// Same as line logic, but belts are faster (0.06), which seems to break the current logic at the time of writing
		const auto j_belt_speed = 0.06f;

		transportBeltProto_->speed = j_belt_speed;  // <---

		// Segments (Logic chunk must be created first)
		auto up_segment = std::make_shared<TransportSegment>(
			data::Orientation::up,
			TransportSegment::TerminationType::bend_right,
			5);
		const auto right_segment = std::make_shared<TransportSegment>(
			data::Orientation::right,
			TransportSegment::TerminationType::bend_right,
			5);
		const auto down_segment = std::make_shared<TransportSegment>(
			data::Orientation::down,
			TransportSegment::TerminationType::bend_right,
			5);
		auto left_segment = std::make_shared<TransportSegment>(
			data::Orientation::left,
			TransportSegment::TerminationType::bend_right,
			5);

		// What each transport segment empties into
		up_segment->targetSegment    = right_segment.get();
		right_segment->targetSegment = down_segment.get();
		down_segment->targetSegment  = left_segment.get();
		left_segment->targetSegment  = up_segment.get();

		RegisterSegment({0, 0}, up_segment);
		RegisterSegment({4, 0}, right_segment);
		RegisterSegment({4, 5}, down_segment);
		RegisterSegment({0, 5}, left_segment);

		// Logic
		left_segment->AppendItem(true, 0.f, itemProto_.get());
		left_segment->AppendItem(true, kItemSpacing, itemProto_.get());
		left_segment->AppendItem(true, kItemSpacing, itemProto_.get());

		// 1 update
		// first item moved to up segment
		TransportLineLogicUpdate(worldData_);
		ASSERT_EQ(up_segment->left.lane.size(), 1);
		ASSERT_EQ(left_segment->left.lane.size(), 2);

		EXPECT_FLOAT_EQ(up_segment->left.lane[0].first.getAsDouble(), 4.40 - j_belt_speed);
		EXPECT_FLOAT_EQ(left_segment->left.lane[0].first.getAsDouble(), 0.25 - j_belt_speed);
		EXPECT_FLOAT_EQ(left_segment->left.lane[1].first.getAsDouble(), 0.25);

		// 2 updates | 0.12
		for (int i = 0; i < 2; ++i) {
			TransportLineLogicUpdate(worldData_);
		}
		ASSERT_EQ(up_segment->left.lane.size(), 1);
		ASSERT_EQ(left_segment->left.lane.size(), 2);

		EXPECT_FLOAT_EQ(up_segment->left.lane[0].first.getAsDouble(), 4.40 - (3 * j_belt_speed));
		EXPECT_FLOAT_EQ(left_segment->left.lane[0].first.getAsDouble(), 0.25 - (3 * j_belt_speed));
		EXPECT_FLOAT_EQ(left_segment->left.lane[1].first.getAsDouble(), 0.25);


		// 2 updates | Total distance = 4(0.06) = 0.24
		// second item moved to up segment
		for (int i = 0; i < 2; ++i) {
			TransportLineLogicUpdate(worldData_);
		}
		ASSERT_EQ(up_segment->left.lane.size(), 2);
		ASSERT_EQ(left_segment->left.lane.size(), 1);

		EXPECT_FLOAT_EQ(up_segment->left.lane[0].first.getAsDouble(), 4.40 - (5 * j_belt_speed));
		EXPECT_FLOAT_EQ(up_segment->left.lane[1].first.getAsDouble(), 0.25);  // Spacing maintained
		// Item 2 was 0.01 -> -0.05
		// | -0.05 - 0.20 | = 0.25 Maintains distance
		EXPECT_FLOAT_EQ(left_segment->left.lane[0].first.getAsDouble(), 0.20);
	}


	TEST_F(TransportLineControllerTest, LineLogicRightBend) {
		// Validates the correct handling of multiple items across transport lines
		// The spacing between items should be maintained

		transportBeltProto_->speed = 0.01f;

		/*
		 *    --------- RIGHT -------- >
		 *    ^
		 *    |
		 *    | UP
		 *    |
		 *    |
		 */

		auto up_segment = std::make_shared<TransportSegment>(
			data::Orientation::up,
			TransportSegment::TerminationType::bend_right,
			4);
		auto right_segment = std::make_shared<TransportSegment>(
			data::Orientation::right,
			TransportSegment::TerminationType::straight,
			4);

		up_segment->targetSegment = right_segment.get();

		RegisterSegment({0, 0}, up_segment);
		RegisterSegment({3, 0}, right_segment);

		// Offset is distance from beginning, or previous item
		up_segment->AppendItem(true, 0.f, itemProto_.get());
		up_segment->AppendItem(true, 1, itemProto_.get());
		up_segment->AppendItem(true, 1, itemProto_.get());
		static_assert(kItemSpacing < 1);  // Tested positions would otherwise be invalid

		// Logic
		// Should transfer the first item
		TransportLineLogicUpdate(worldData_);


		EXPECT_EQ(up_segment->left.lane.size(), 2);
		EXPECT_FLOAT_EQ(up_segment->left.lane[0].first.getAsDouble(), 0.99f);
		EXPECT_FLOAT_EQ(up_segment->left.lane[1].first.getAsDouble(), 1.f);

		EXPECT_EQ(right_segment->left.lane.size(), 1);
		// Moved forward once 4 - 0.3 - 0.01
		EXPECT_FLOAT_EQ(right_segment->left.lane[0].first.getAsDouble(), 3.69f);

		// Transfer second item after (1 / 0.01) + 1 update - 1 update (Already moved once above)
		for (int i = 0; i < 100; ++i) {
			TransportLineLogicUpdate(worldData_);
		}

		EXPECT_EQ(up_segment->left.lane.size(), 1);
		EXPECT_EQ(right_segment->left.lane.size(), 2);
		// Spacing of 1 tile between the items is maintained across belts
		EXPECT_FLOAT_EQ(right_segment->left.lane[0].first.getAsDouble(), 2.69f);
		EXPECT_FLOAT_EQ(right_segment->left.lane[1].first.getAsDouble(), 1.f);


		// Third item
		for (int i = 0; i < 100; ++i) {
			TransportLineLogicUpdate(worldData_);
		}
		EXPECT_EQ(up_segment->left.lane.size(), 0);
		EXPECT_EQ(right_segment->left.lane.size(), 3);

		EXPECT_FLOAT_EQ(right_segment->left.lane[0].first.getAsDouble(), 1.69f);
		EXPECT_FLOAT_EQ(right_segment->left.lane[1].first.getAsDouble(), 1.f);
		EXPECT_FLOAT_EQ(right_segment->left.lane[2].first.getAsDouble(), 1.f);
	}

	TEST_F(TransportLineControllerTest, LineLogicCompressedRightBend) {
		// Same as line_logic_right_bend, but items are compressed

		transportBeltProto_->speed = 0.01f;

		/*
		 * COMPRESSED
		 *    --------- RIGHT -------- >
		 *    ^
		 *    |
		 *    | UP
		 *    |
		 *    |
		 */

		auto up_segment = std::make_shared<TransportSegment>(
			data::Orientation::up,
			TransportSegment::TerminationType::bend_right,
			4);
		auto right_segment = std::make_shared<TransportSegment>(
			data::Orientation::right,
			TransportSegment::TerminationType::straight,
			4);

		up_segment->targetSegment = right_segment.get();

		RegisterSegment({0, 0}, up_segment);
		RegisterSegment({3, 0}, right_segment);

		// Offset is distance from beginning, or previous item
		up_segment->AppendItem(true, 0.f, itemProto_.get());
		up_segment->AppendItem(true, kItemSpacing, itemProto_.get());

		// First item
		TransportLineLogicUpdate(worldData_);


		EXPECT_EQ(up_segment->left.lane.size(), 1);
		EXPECT_FLOAT_EQ(up_segment->left.lane[0].first.getAsDouble(), 0.24f);

		EXPECT_EQ(right_segment->left.lane.size(), 1);
		// Moved forward once 4 - 0.3 - 0.01
		EXPECT_FLOAT_EQ(right_segment->left.lane[0].first.getAsDouble(), 3.69f);


		// Transfer second item after (0.25 / 0.01) + 1 update - 1 update (Already moved once above)
		for (int i = 0; i < 25; ++i) {
			TransportLineLogicUpdate(worldData_);
		}

		EXPECT_EQ(up_segment->left.lane.size(), 0);
		EXPECT_EQ(right_segment->left.lane.size(), 2);
		// Spacing is maintained across belts
		EXPECT_FLOAT_EQ(right_segment->left.lane[0].first.getAsDouble(), 3.44f);
		EXPECT_FLOAT_EQ(right_segment->left.lane[1].first.getAsDouble(), 0.25f);
	}

	TEST_F(TransportLineControllerTest, LineLogicTransitionStraight) {
		// Transferring from a straight segment traveling left to another one traveling left
		/*
		 * < ------ LEFT (1) ------		< ------ LEFT (2) -------
		 */

		transportBeltProto_->speed = 0.01f;

		auto segment_1 = std::make_shared<TransportSegment>(
			data::Orientation::left,
			TransportSegment::TerminationType::straight,
			4);
		auto segment_2 = std::make_shared<TransportSegment>(
			data::Orientation::left,
			TransportSegment::TerminationType::straight,
			4);

		segment_2->targetSegment = segment_1.get();

		RegisterSegment({0, 0}, segment_1);
		RegisterSegment({3, 0}, segment_2);

		// Insert item on left + right side
		segment_2->AppendItem(true, 0.02f, itemProto_.get());
		segment_2->AppendItem(false, 0.02f, itemProto_.get());

		// Travel to the next belt in 0.02 / 0.01 + 1 updates
		for (int i = 0; i < 3; ++i) {
			TransportLineLogicUpdate(worldData_);
		}

		EXPECT_EQ(segment_2->left.lane.size(), 0);
		EXPECT_EQ(segment_2->right.lane.size(), 0);
		// 3.99 tiles from the end of this transport line
		EXPECT_FLOAT_EQ(segment_1->left.lane[0].first.getAsDouble(), 3.99);
		EXPECT_FLOAT_EQ(segment_1->right.lane[0].first.getAsDouble(), 3.99);
	}

	TEST_F(TransportLineControllerTest, LineLogicStopAtEndOfLine) {
		// When no target_segment is provided:
		// First Item will stop at the end of line (Distance is 0)
		// Trailing items will stop at item_width from the previous item

		transportBeltProto_->speed = 0.01f;

		auto segment = std::make_shared<TransportSegment>(
			data::Orientation::left,
			TransportSegment::TerminationType::straight,
			10);

		RegisterSegment({0, 0}, segment);

		segment->AppendItem(true, 0.5f, itemProto_.get());
		segment->AppendItem(true, kItemSpacing, itemProto_.get());
		segment->AppendItem(true, kItemSpacing + 1.f, itemProto_.get());

		// Will reach distance 0 after 0.5 / 0.01 updates
		for (int i = 0; i < 50; ++i) {
			TransportLineLogicUpdate(worldData_);
		}

		EXPECT_EQ(segment->left.index, 0);
		EXPECT_FLOAT_EQ(segment->left.lane[0].first.getAsDouble(), 0);

		// On the next update, with no target segment, first item is kept at 0, second item untouched
		// move index to 2 (was 0) as it has a distance greater than item_width
		TransportLineLogicUpdate(worldData_);


		EXPECT_EQ(segment->left.index, 2);
		EXPECT_FLOAT_EQ(segment->left.lane[0].first.getAsDouble(), 0);
		EXPECT_FLOAT_EQ(segment->left.lane[1].first.getAsDouble(), jactorio::game::kItemSpacing);
		EXPECT_FLOAT_EQ(segment->left.lane[2].first.getAsDouble(), jactorio::game::kItemSpacing + 0.99f);

		// After 0.2 + 0.99 / 0.01 updates, the Third item will not move in following updates
		for (int j = 0; j < 99; ++j) {
			TransportLineLogicUpdate(worldData_);
		}
		EXPECT_FLOAT_EQ(segment->left.lane[2].first.getAsDouble(), jactorio::game::kItemSpacing);

		TransportLineLogicUpdate(worldData_);

		// Index set to 0, checking if a valid target exists to move items forward
		EXPECT_EQ(segment->left.index, 0);

		EXPECT_FLOAT_EQ(segment->left.lane[2].first.getAsDouble(), jactorio::game::kItemSpacing);


		// Updates do nothing since all items are compressed
		for (int k = 0; k < 50; ++k) {
			TransportLineLogicUpdate(worldData_);
		}
	}

	TEST_F(TransportLineControllerTest, LineLogicStopAtFilledTargetSegment) {
		// For the right lane:

		transportBeltProto_->speed = 0.01f;

		/*
		 *    --------- RIGHT -------- >
		 *    ^
		 *    |
		 *    | UP
		 *    |
		 *    |
		 */

		auto up_segment = std::make_shared<TransportSegment>(
			data::Orientation::up,
			TransportSegment::TerminationType::bend_right,
			4);
		auto right_segment = std::make_shared<TransportSegment>(
			data::Orientation::right,
			TransportSegment::TerminationType::straight,
			4);

		up_segment->targetSegment = right_segment.get();

		RegisterSegment({0, 0}, up_segment);
		RegisterSegment({3, 0}, right_segment);

		// RIGHT LINE: 14 items can be fit on the right lane: (4 - 0.7) / kItemSpacing{0.25} = 13.2
		for (int i = 0; i < 14; ++i) {
			right_segment->AppendItem(false, 0.f, itemProto_.get());
		}

		// Items on up line should stop
		up_segment->AppendItem(false, 0.f, itemProto_.get());

		// WIll not move after an arbitrary number of updates
		for (int i = 0; i < 34; ++i) {
			TransportLineLogicUpdate(worldData_);
		}

		EXPECT_FLOAT_EQ(up_segment->right.lane.front().first.getAsDouble(), 0);
	}

	TEST_F(TransportLineControllerTest, LineLogicItemSpacing) {
		// A minimum distance of kItemSpacing is maintained between items

		auto right_segment = std::make_shared<TransportSegment>(
			data::Orientation::right,
			TransportSegment::TerminationType::bend_right,
			4);

		RegisterSegment({0, 0}, right_segment);

		right_segment->AppendItem(true, 0.f, itemProto_.get());
		right_segment->AppendItem(true, 0.f, itemProto_.get());  // Insert behind previous item

		// Check that second item has a minimum distance of kItemSpacing
		EXPECT_FLOAT_EQ(right_segment->left.lane[0].first.getAsDouble(), 0.f);
		EXPECT_FLOAT_EQ(right_segment->left.lane[1].first.getAsDouble(), jactorio::game::kItemSpacing);
	}

	TEST_F(TransportLineControllerTest, LineLogicTransitionSideLeft) {
		// Belt feeding into only one side of another belt
		/*
		 *                           Right     Left
		 *                             |    -    |
		 *                             |    -    |
		 *        --------- A ----->   |    -    |
		 *        --------- B ----->   | v  -    | Downwards belt
		 *                             |    -    |
		 *                             |    -    |
		 *                             |  v -    |
		 */
		// A first, fill entire lane, if A is not compressed, B moves

		transportBeltProto_->speed = 0.05;


		// Segments (Logic chunk must be created first)
		auto right_segment = std::make_shared<TransportSegment>(
			data::Orientation::right,
			TransportSegment::TerminationType::right_only,
			5);
		auto down_segment = std::make_shared<TransportSegment>(
			data::Orientation::down,
			TransportSegment::TerminationType::straight,
			10);

		right_segment->targetSegment      = down_segment.get();
		right_segment->targetInsertOffset = 8;  // 8 + 1 = 9

		down_segment->itemOffset = 1;

		RegisterSegment({4, 0}, right_segment);
		RegisterSegment({4, 9}, down_segment);

		// Insert items
		for (int i = 0; i < 3; ++i) {
			right_segment->AppendItem(true, 0.f, itemProto_.get());
			right_segment->AppendItem(false, 0.f, itemProto_.get());
		}

		// Logic tests
		TransportLineLogicUpdate(worldData_);

		// Since the target belt is empty, both A + B inserts into right lane
		EXPECT_EQ(right_segment->left.lane.size(), 2);
		EXPECT_EQ(right_segment->left.lane[0].first.getAsDouble(), 0.2);  // 0.25 - 0.05

		EXPECT_EQ(right_segment->right.lane.size(), 2);
		EXPECT_EQ(right_segment->right.lane[0].first.getAsDouble(), 0.2);


		ASSERT_EQ(down_segment->right.lane.size(), 2);
		// 10 - 0.7 - 0.05
		EXPECT_FLOAT_EQ(down_segment->right.lane[0].first.getAsDouble(), 9.25f);
		// (10 - 0.3 - 0.05) - (10 - 0.7 - 0.05)
		EXPECT_FLOAT_EQ(down_segment->right.lane[1].first.getAsDouble(), 0.4f);


		// ======================================================================
		// End on One update prior to transitioning
		for (int j = 0; j < 4; ++j) {
			TransportLineLogicUpdate(worldData_);
		}
		EXPECT_EQ(right_segment->left.lane[0].first.getAsDouble(), 0.0);
		EXPECT_EQ(right_segment->right.lane[0].first.getAsDouble(), 0.0);

		EXPECT_FLOAT_EQ(down_segment->right.lane[0].first.getAsDouble(), 9.05f);


		// ======================================================================
		// Transition items
		TransportLineLogicUpdate(worldData_);
		EXPECT_EQ(right_segment->left.lane.size(), 1);
		EXPECT_EQ(right_segment->left.lane[0].first.getAsDouble(), 0.2);  // 0.25 - 0.05


		// ======================================================================
		// Right lane (B) stops, left (A) takes priority
		EXPECT_EQ(right_segment->right.lane.size(), 2);  // Unmoved
		EXPECT_EQ(right_segment->right.lane[0].first.getAsDouble(), 0.f);

		ASSERT_EQ(down_segment->right.lane.size(), 3);
		EXPECT_FLOAT_EQ(down_segment->right.lane[0].first.getAsDouble(), 9.00f);
		EXPECT_FLOAT_EQ(down_segment->right.lane[1].first.getAsDouble(), 0.4f);
		EXPECT_FLOAT_EQ(down_segment->right.lane[2].first.getAsDouble(), 0.25f);


		// ======================================================================
		// Transition third item for Lane A, should wake up lane B after passing
		for (int j = 0; j < 4 + 13 + 1; ++j) {  // 0.20 / 0.05 + (0.40 + 0.25) / 0.05 + 1 for transition
			TransportLineLogicUpdate(worldData_);
		}
		EXPECT_EQ(right_segment->left.lane.size(), 0);
		EXPECT_EQ(right_segment->right.lane.size(), 1);  // Woke and moved

		ASSERT_EQ(down_segment->right.lane.size(), 5);
		EXPECT_FLOAT_EQ(down_segment->right.lane[0].first.getAsDouble(), 8.10f);
		EXPECT_FLOAT_EQ(down_segment->right.lane[3].first.getAsDouble(), 0.25f);

	}

	TEST_F(TransportLineControllerTest, LineLogicTransitionSideRight) {
		// Belt feeding into only one side of another belt moving updards
		/*
		 * Left     Right
		 *  |    -    |
		 *  |    -    |
		 *  |    -    |	<------------ A -------------
		 *  |  ^ -    | <------------ B -------------
		 *  |    -    |
		 *  |    -    |
		 *  |    -    |
		 */
		// B first, fill entire lane, if B is not compressed, A moves

		transportBeltProto_->speed = 0.05;


		// Segments (Logic chunk must be created first)
		auto left_segment = std::make_shared<TransportSegment>(
			data::Orientation::left,
			TransportSegment::TerminationType::right_only,
			5);
		auto down_segment = std::make_shared<TransportSegment>(
			data::Orientation::down,
			TransportSegment::TerminationType::straight,
			20);

		left_segment->targetSegment = down_segment.get();

		left_segment->targetInsertOffset = -1;  // Will insert into up_segment with offset of 9 absolute
		down_segment->itemOffset         = 10;

		RegisterSegment({4, 0}, left_segment);
		RegisterSegment({4, 9}, down_segment);


		// Insert items
		for (int i = 0; i < 3; ++i) {
			left_segment->AppendItem(true, 0.f, itemProto_.get());
			left_segment->AppendItem(false, 0.f, itemProto_.get());
		}

		// Logic tests
		TransportLineLogicUpdate(worldData_);

		// Since the target belt is empty, both A + B inserts into right lane
		EXPECT_EQ(left_segment->left.lane.size(), 2);
		EXPECT_EQ(left_segment->left.lane[0].first.getAsDouble(), 0.2);  // 0.25 - 0.05

		EXPECT_EQ(left_segment->right.lane.size(), 2);
		EXPECT_EQ(left_segment->right.lane[0].first.getAsDouble(), 0.2);


		ASSERT_EQ(down_segment->right.lane.size(), 2);
		// 10 - 0.7 - 0.05
		EXPECT_FLOAT_EQ(down_segment->right.lane[0].first.getAsDouble(), 9.25f);
		// (10 - 0.3 - 0.05) - (10 - 0.7 - 0.05)
		EXPECT_FLOAT_EQ(down_segment->right.lane[1].first.getAsDouble(), 0.4f);


		// ======================================================================
		// End on One update prior to transitioning
		for (int j = 0; j < 4; ++j) {
			TransportLineLogicUpdate(worldData_);
		}
		EXPECT_EQ(left_segment->left.lane[0].first.getAsDouble(), 0.0);
		EXPECT_EQ(left_segment->right.lane[0].first.getAsDouble(), 0.0);

		EXPECT_FLOAT_EQ(down_segment->right.lane[0].first.getAsDouble(), 9.05f);


		// ======================================================================
		// Transition items
		TransportLineLogicUpdate(worldData_);
		EXPECT_EQ(left_segment->left.lane.size(), 1);
		EXPECT_EQ(left_segment->left.lane[0].first.getAsDouble(), 0.2);  // 0.25 - 0.05


		// ======================================================================
		// Right lane (B) stops, left (A) takes priority
		EXPECT_EQ(left_segment->right.lane.size(), 2);  // Unmoved
		EXPECT_EQ(left_segment->right.lane[0].first.getAsDouble(), 0.f);

		ASSERT_EQ(down_segment->right.lane.size(), 3);
		EXPECT_FLOAT_EQ(down_segment->right.lane[0].first.getAsDouble(), 9.00f);
		EXPECT_FLOAT_EQ(down_segment->right.lane[1].first.getAsDouble(), 0.4f);
		EXPECT_FLOAT_EQ(down_segment->right.lane[2].first.getAsDouble(), 0.25f);


		// ======================================================================
		// Transition third item for Lane A, should wake up lane B after passing
		for (int j = 0; j < 4 + 13 + 1; ++j) {  // 0.20 / 0.05 + (0.40 + 0.25) / 0.05 + 1 for transition
			TransportLineLogicUpdate(worldData_);
		}
		EXPECT_EQ(left_segment->left.lane.size(), 0);
		EXPECT_EQ(left_segment->right.lane.size(), 1);  // Woke and moved

		ASSERT_EQ(down_segment->right.lane.size(), 5);
		EXPECT_FLOAT_EQ(down_segment->right.lane[0].first.getAsDouble(), 8.10f);
		EXPECT_FLOAT_EQ(down_segment->right.lane[3].first.getAsDouble(), 0.25f);

	}

	TEST_F(TransportLineControllerTest, LineLogicNewSegmentAddedAhead) {
		//     2      1
		// < ----- < -----
		
		transportBeltProto_->speed = 0.04f;

		// Segments (Logic chunk must be created first)
		auto left_segment = std::make_shared<TransportSegment>(
			data::Orientation::left,
			TransportSegment::TerminationType::straight,
			2);

		RegisterSegment({2, 1}, left_segment);

		// One item stopped, one still moving
		left_segment->AppendItem(true, 0, itemProto_.get());
		TransportLineLogicUpdate(worldData_);
		EXPECT_EQ(left_segment.get()->left.index, 0);

		left_segment->AppendItem(true, 2, itemProto_.get());
		TransportLineLogicUpdate(worldData_);
		EXPECT_EQ(left_segment.get()->left.index, 1);


		// ======================================================================
		auto left_segment_2 = std::make_shared<TransportSegment>(
			data::Orientation::left,
			TransportSegment::TerminationType::straight,
			1);

		left_segment->targetSegment = left_segment_2.get();

		RegisterSegment({1, 1}, left_segment_2);

		// Update neighboring segments as a new segment was placed
		transportBeltProto_->OnNeighborUpdate(worldData_, 
											  {1, 1}, {2, 1},
											  data::Orientation::right);

		EXPECT_EQ(left_segment.get()->left.index, 0);
	}

	TEST_F(TransportLineControllerTest, BackItemDistance) {
		/*
		 * ^
		 * |
		 * |
		 * 
		 * ^
		 * |
		 * |
		 */

		transportBeltProto_->speed = 0.05;


		// Segments (Logic chunk must be created first)
		auto up_segment_1 = std::make_shared<TransportSegment>(data::Orientation::up,
		                                                       TransportSegment::TerminationType::straight,
		                                                       1);
		auto up_segment_2 = std::make_shared<TransportSegment>(data::Orientation::up,
		                                                       TransportSegment::TerminationType::straight,
		                                                       1);

		up_segment_2->targetSegment = up_segment_1.get();

		RegisterSegment({0, 0}, up_segment_1);
		RegisterSegment({0, 1}, up_segment_2);

		up_segment_2->AppendItem(true, 0.05, itemProto_.get());
		EXPECT_FLOAT_EQ(up_segment_2->left.backItemDistance.getAsDouble(), 0.05);

		TransportLineLogicUpdate(worldData_);
		EXPECT_FLOAT_EQ(up_segment_2->left.backItemDistance.getAsDouble(), 0);

		// Segment 1
		TransportLineLogicUpdate(worldData_);
		EXPECT_FLOAT_EQ(up_segment_2->left.backItemDistance.getAsDouble(), 0);

		EXPECT_FLOAT_EQ(up_segment_1->left.backItemDistance.getAsDouble(), 0.95);  // First segment now

		for (int i = 0; i < 19; ++i) {
			TransportLineLogicUpdate(worldData_);
		}
		EXPECT_FLOAT_EQ(up_segment_1->left.backItemDistance.getAsDouble(), 0);

		// Remains at 0
		TransportLineLogicUpdate(worldData_);
		EXPECT_FLOAT_EQ(up_segment_1->left.backItemDistance.getAsDouble(), 0);


		// ======================================================================
		// Fill the first segment up to 4 items
		up_segment_1->AppendItem(true, 0, itemProto_.get());
		up_segment_1->AppendItem(true, 0, itemProto_.get());
		up_segment_1->AppendItem(true, 0, itemProto_.get());
		EXPECT_FLOAT_EQ(up_segment_1->left.backItemDistance.getAsDouble(), 0.75);


		// Will not enter since segment 1 is full
		up_segment_2->AppendItem(true, 0.05, itemProto_.get());
		TransportLineLogicUpdate(worldData_);
		TransportLineLogicUpdate(worldData_);
		TransportLineLogicUpdate(worldData_);
		EXPECT_FLOAT_EQ(up_segment_1->left.backItemDistance.getAsDouble(), 0.75);
		EXPECT_FLOAT_EQ(up_segment_2->left.backItemDistance.getAsDouble(), 0);
	}
}
