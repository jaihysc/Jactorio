// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#include "data/prototype/entity/transport/transport_line.h"

#include "data/data_manager.h"
#include "game/logic/transport_line_structure.h"

std::pair<uint16_t, uint16_t> jactorio::data::TransportLine::MapPlacementOrientation(
	const Orientation orientation,
	game::WorldData& world_data,
	const game::WorldData::WorldPair& world_coords) const {

	auto* t_center = GetLineData(world_data, world_coords.first, world_coords.second - 1);
	auto* c_left   = GetLineData(world_data, world_coords.first - 1, world_coords.second);
	auto* c_right  = GetLineData(world_data, world_coords.first + 1, world_coords.second);
	auto* b_center = GetLineData(world_data, world_coords.first, world_coords.second + 1);

	return {static_cast<uint16_t>(GetLineOrientation(orientation, t_center, c_right, b_center, c_left)), 0};
}

// ======================================================================
// Data access

///
/// \return True if Line exists and points in direction 
#define NEIGHBOR_VALID(transport_line, direction)\
		((transport_line) && TransportLineData::ToOrientation((transport_line)->orientation) == Orientation::direction)

///
/// \brief Determines line orientation given placement orientation and neighbors
jactorio::data::TransportLineData::LineOrientation jactorio::data::TransportLine::GetLineOrientation(
	const Orientation orientation,
	TransportLineData* up,
	TransportLineData* right,
	TransportLineData* down,
	TransportLineData* left) {
	// Determine if there is a transport line neighboring the current one and a bend should be placed
	// The line above is valid for a bend since it faces towards the current tile

	switch (orientation) {
	case Orientation::up:
		if (!NEIGHBOR_VALID(down, up) && NEIGHBOR_VALID(left, right) != NEIGHBOR_VALID(right, left)) {
			if (NEIGHBOR_VALID(left, right))
				return TransportLineData::LineOrientation::right_up;
			return TransportLineData::LineOrientation::left_up;
		}
		return TransportLineData::LineOrientation::up;

	case Orientation::right:
		if (!NEIGHBOR_VALID(left, right) && NEIGHBOR_VALID(up, down) != NEIGHBOR_VALID(down, up)) {
			if (NEIGHBOR_VALID(up, down))
				return TransportLineData::LineOrientation::down_right;
			return TransportLineData::LineOrientation::up_right;
		}
		return TransportLineData::LineOrientation::right;

	case Orientation::down:
		if (!NEIGHBOR_VALID(up, down) && NEIGHBOR_VALID(left, right) != NEIGHBOR_VALID(right, left)) {
			if (NEIGHBOR_VALID(left, right))
				return TransportLineData::LineOrientation::right_down;
			return TransportLineData::LineOrientation::left_down;
		}
		return TransportLineData::LineOrientation::down;

	case Orientation::left:
		if (!NEIGHBOR_VALID(right, left) && NEIGHBOR_VALID(up, down) != NEIGHBOR_VALID(down, up)) {
			if (NEIGHBOR_VALID(up, down))
				return TransportLineData::LineOrientation::down_left;
			return TransportLineData::LineOrientation::up_left;
		}
		return TransportLineData::LineOrientation::left;

	default:
		assert(false); // Missing switch case
	}

	return TransportLineData::LineOrientation::up;
}

#undef NEIGHBOR_VALID


jactorio::data::TransportLineData* jactorio::data::TransportLine::GetLineData(const game::WorldData& world_data,
                                                                              const game::WorldData::WorldCoord world_x,
                                                                              const game::WorldData::WorldCoord world_y) {
	auto* tile = world_data.GetTile(world_x, world_y);
	if (!tile)  // No tile exists
		return nullptr;

	auto& layer = tile->GetLayer(game::ChunkTile::ChunkLayer::entity);

	if (!dynamic_cast<const TransportLine*>(  // Not an instance of transport line
		layer.prototypeData))
		return nullptr;

	return static_cast<TransportLineData*>(layer.uniqueData);
}

void jactorio::data::TransportLine::GetLineStructLayer(game::WorldData& world_data,
                                                       const game::WorldData::WorldCoord world_x,
                                                       const game::WorldData::WorldCoord world_y,
                                                       const std::function<void(game::ChunkStructLayer&, game::LogicChunk&)
                                                       >&
                                                       callback) {
	TransportLineData* line_data =
		GetLineData(world_data, world_x, world_y);
	if (!line_data)  // Transport lines does not exist
		return;

	game::LogicChunk* logic_chunk;
	try {
		logic_chunk = world_data.LogicGetChunk(
			world_data.GetChunk(world_x, world_y)
		);

		if (!logic_chunk)
			return;
	}
	catch (std::out_of_range&) {
		// Failed to find key (Logic chunk does not exist)
		return;
	}


	// Search for the transport line in the chunk struct layer where the
	// unique_data(Transport_line_data) == one referenced by line_data
	for (auto& layer : logic_chunk->GetStruct(game::LogicChunk::StructLayer::transport_line)) {
		if (layer.uniqueData == &line_data->lineSegment.get())
			callback(layer, *logic_chunk);
	}
}

// ======================================================================
// Build / Remove / Neighbor update

void jactorio::data::TransportLine::UpdateNeighboringOrientation(const game::WorldData& world_data,
                                                                 const game::WorldData::WorldPair& world_coords,
                                                                 TransportLineData* t_center,
                                                                 TransportLineData* c_right,
                                                                 TransportLineData* b_center,
                                                                 TransportLineData* c_left,
                                                                 TransportLineData* center) {
	// Building a belt will update all neighboring belts (X), which thus requires tiles (*)
	/*
	 *     ---
	 *     |*|        Top
	 *   -------      
	 *   |*|x|*|      Top left, top center, top right 
	 * -----------
	 * |*|x|x|x|*|    Left, Center left, Center right, Right
	 * -----------
	 *   |*|x|*|      Bottom left, bottom center, bottom right
	 *   ------- 
	 *     |*|        Bottom
	 *     ---
	 */
	auto* top    = GetLineData(world_data, world_coords.first, world_coords.second - 2);
	auto* right  = GetLineData(world_data, world_coords.first + 2, world_coords.second);
	auto* bottom = GetLineData(world_data, world_coords.first, world_coords.second + 2);
	auto* left   = GetLineData(world_data, world_coords.first - 2, world_coords.second);

	auto* t_left  = GetLineData(world_data, world_coords.first - 1, world_coords.second - 1);
	auto* t_right = GetLineData(world_data, world_coords.first + 1, world_coords.second - 1);

	auto* b_left  = GetLineData(world_data, world_coords.first - 1, world_coords.second + 1);
	auto* b_right = GetLineData(world_data, world_coords.first + 1, world_coords.second + 1);

	// Top neighbor
	if (t_center)
		t_center->SetOrientation(
			GetLineOrientation(TransportLineData::ToOrientation(t_center->orientation), top, t_right, center, t_left));
	// Right
	if (c_right)
		c_right->SetOrientation(
			GetLineOrientation(TransportLineData::ToOrientation(c_right->orientation), t_right, right, b_right, center));
	// Bottom
	if (b_center)
		b_center->SetOrientation(
			GetLineOrientation(TransportLineData::ToOrientation(b_center->orientation), center, b_right, bottom, b_left));
	// Left
	if (c_left)
		c_left->SetOrientation(
			GetLineOrientation(TransportLineData::ToOrientation(c_left->orientation), t_left, center, b_left, left));
}

void jactorio::data::TransportLine::UpdateNeighborLines(
	game::WorldData& world_data,
	const int32_t world_x, const int32_t world_y,
	const TransportLineData::LineOrientation line_orientation,
	const UpdateFunc& func,
	const UpdateSideOnlyFunc& side_only_func) {

	switch (line_orientation) {
		// Up
	case TransportLineData::LineOrientation::up_left:
		func(world_data, world_x, world_y + 1,
		     0, -1,
		     game::TransportLineSegment::TerminationType::bend_left);
		break;
	case TransportLineData::LineOrientation::up_right:
		func(world_data, world_x, world_y + 1,
		     0, -1,
		     game::TransportLineSegment::TerminationType::bend_right);
		break;

		// Right
	case TransportLineData::LineOrientation::right_up:
		func(world_data, world_x - 1, world_y,
		     1, 0,
		     game::TransportLineSegment::TerminationType::bend_left);
		break;
	case TransportLineData::LineOrientation::right_down:
		func(world_data, world_x - 1, world_y,
		     1, 0,
		     game::TransportLineSegment::TerminationType::bend_right);
		break;

		// Down
	case TransportLineData::LineOrientation::down_right:
		func(world_data, world_x, world_y - 1,
		     0, 1,
		     game::TransportLineSegment::TerminationType::bend_left);
		break;
	case TransportLineData::LineOrientation::down_left:
		func(world_data, world_x, world_y - 1,
		     0, 1,
		     game::TransportLineSegment::TerminationType::bend_right);
		break;

		// Left
	case TransportLineData::LineOrientation::left_down:
		func(world_data, world_x + 1, world_y,
		     -1, 0,
		     game::TransportLineSegment::TerminationType::bend_left);
		break;
	case TransportLineData::LineOrientation::left_up:
		func(world_data, world_x + 1, world_y,
		     -1, 0,
		     game::TransportLineSegment::TerminationType::bend_right);
		break;

		// Straight (Check for transport lines on both sides to make side only)
	case TransportLineData::LineOrientation::up:
		side_only_func(world_data, world_x - 1, world_y,
		               1, 0,
		               Orientation::right,
		               game::TransportLineSegment::TerminationType::left_only);
		side_only_func(world_data, world_x + 1, world_y,
		               -1, 0,
		               Orientation::left,
		               game::TransportLineSegment::TerminationType::right_only);
		break;
	case TransportLineData::LineOrientation::right:
		side_only_func(world_data, world_x, world_y - 1,
		               0, 1,
		               Orientation::down,
		               game::TransportLineSegment::TerminationType::left_only);
		side_only_func(world_data, world_x, world_y + 1,
		               0, -1,
		               Orientation::up,
		               game::TransportLineSegment::TerminationType::right_only);
		break;
	case TransportLineData::LineOrientation::down:
		side_only_func(world_data, world_x - 1, world_y,
		               1, 0,
		               Orientation::right,
		               game::TransportLineSegment::TerminationType::right_only);
		side_only_func(world_data, world_x + 1, world_y,
		               -1, 0,
		               Orientation::left,
		               game::TransportLineSegment::TerminationType::left_only);
		break;
	case TransportLineData::LineOrientation::left:
		side_only_func(world_data, world_x, world_y - 1,
		               0, 1,
		               Orientation::down,
		               game::TransportLineSegment::TerminationType::right_only);
		side_only_func(world_data, world_x, world_y + 1,
		               0, -1,
		               Orientation::up,
		               game::TransportLineSegment::TerminationType::left_only);
		break;

		// 
	default:
		assert(false);  // Missing switch case
		break;

	}
}

// ======================================================================
// Build

///
/// \brief If a transport segment exists at world_x, world_y and terminationType == termination_type, its terminationType will
/// be updated to new_termination_type
void TryChangeTerminationType(const jactorio::game::WorldData& world_data,
                              const int32_t world_x, const int32_t world_y,
                              const jactorio::game::TransportLineSegment::TerminationType termination_type,
                              const jactorio::game::TransportLineSegment::TerminationType new_termination_type) {
	using namespace jactorio;

	data::TransportLineData* line_data = data::TransportLine::GetLineData(world_data, world_x, world_y);
	if (line_data) {
		if (line_data->lineSegment.get().terminationType == termination_type) {
			line_data->lineSegment.get().terminationType = new_termination_type;
		}
	}
}

///
/// \brief Determines the member target_segment of Transport_line_segment
/// \param orientation Current orientation
/// \param origin_connect_orientation Orientation required for origin_segment to connect to neighbor segment
/// \param target_connect_orientation Orientation required for neighbor segment to connect to origin segment
void UpdateLineTargets(jactorio::game::WorldData& world_data,
                       jactorio::game::TransportLineSegment& origin_segment,
                       const jactorio::data::Orientation orientation, const int32_t neighbor_world_x,
                       const int32_t neighbor_world_y,
                       const jactorio::data::Orientation origin_connect_orientation,
                       const jactorio::data::Orientation target_connect_orientation) {
	using namespace jactorio;

	data::TransportLineData* neighbor_line_data = data::TransportLine::GetLineData(
		world_data, neighbor_world_x, neighbor_world_y);
	if (neighbor_line_data) {
		game::TransportLineSegment& neighbor_segment = neighbor_line_data->lineSegment;

		// Do not attempt to connect to itself
		if (&origin_segment == &neighbor_segment)
			return;

		const bool origin_valid   = orientation == origin_connect_orientation;
		const bool neighbor_valid = neighbor_segment.direction == target_connect_orientation;

		// Only 1 can be valid at a time (does not both point to each other)
		// Either origin feeds into neighbor, or neighbor feeds into origin depending on which one is valid
		if (origin_valid == neighbor_valid)
			return;

		if (origin_valid)
			origin_segment.targetSegment = &neighbor_segment;
		else
			neighbor_segment.targetSegment = &origin_segment;
	}
}


void jactorio::data::TransportLine::UpdateSegmentHead(game::WorldData& world_data,
                                                      const game::WorldData::WorldPair& world_coords,
                                                      LineData4Way& line_data,
                                                      game::TransportLineSegment& line_segment) {
	auto* up    = line_data[0];
	auto* right = line_data[1];
	auto* down  = line_data[2];
	auto* left  = line_data[3];

	float* segment_sl_x = nullptr;  // _sl_ = Struct layer
	float* segment_sl_y = nullptr;
	GetLineStructLayer(world_data,
	                   world_coords.first, world_coords.second,
	                   [&](auto& s_layer, auto&) {
		                   segment_sl_x = &s_layer.positionX;
		                   segment_sl_y = &s_layer.positionY;
	                   });
	assert(segment_sl_x != nullptr);
	assert(segment_sl_y != nullptr);

	switch (line_segment.direction) {

#define TL_DOWNSHIFT\
	--(*segment_sl_y);\
	line_segment.length++;\
	UpdateSegmentTiles(world_data, world_coords, line_segment, 1)

	case Orientation::up:
		if (up) {
			switch (up->orientation) {
			case TransportLineData::LineOrientation::up_left:
				line_segment.terminationType = game::TransportLineSegment::TerminationType::bend_left;
				TL_DOWNSHIFT;
				break;
			case TransportLineData::LineOrientation::up_right:
				line_segment.terminationType = game::TransportLineSegment::TerminationType::bend_right;
				TL_DOWNSHIFT;
				break;

			case TransportLineData::LineOrientation::right:
				line_segment.terminationType = game::TransportLineSegment::TerminationType::right_only;
				TL_DOWNSHIFT;

				// Check 2 units up and see if there is segment bending left, if so change it to straight
				TryChangeTerminationType(world_data,
				                         world_coords.first, world_coords.second - 2,
				                         game::TransportLineSegment::TerminationType::bend_left,
				                         game::TransportLineSegment::TerminationType::left_only);
				break;
			case TransportLineData::LineOrientation::left:
				line_segment.terminationType = game::TransportLineSegment::TerminationType::left_only;
				TL_DOWNSHIFT;
				TryChangeTerminationType(world_data,
				                         world_coords.first, world_coords.second - 2,
				                         game::TransportLineSegment::TerminationType::bend_right,
				                         game::TransportLineSegment::TerminationType::right_only);
				break;

			default:
				break;
			}
		}
		break;
#undef TL_DOWNSHIFT
#define TL_RIGHTSHIFT\
	++(*segment_sl_x);\
	line_segment.length++;\
	UpdateSegmentTiles(world_data, world_coords, line_segment, 1)

	case Orientation::right:
		if (right) {
			switch (right->orientation) {
			case TransportLineData::LineOrientation::right_up:
				line_segment.terminationType = game::TransportLineSegment::TerminationType::bend_left;
				TL_RIGHTSHIFT;
				break;
			case TransportLineData::LineOrientation::right_down:
				line_segment.terminationType = game::TransportLineSegment::TerminationType::bend_right;
				TL_RIGHTSHIFT;
				break;

			case TransportLineData::LineOrientation::down:
				line_segment.terminationType = game::TransportLineSegment::TerminationType::right_only;
				TL_RIGHTSHIFT;
				TryChangeTerminationType(world_data, world_coords.first + 2, world_coords.second,
				                         game::TransportLineSegment::TerminationType::bend_left,
				                         game::TransportLineSegment::TerminationType::left_only);
				break;
			case TransportLineData::LineOrientation::up:
				line_segment.terminationType = game::TransportLineSegment::TerminationType::left_only;
				TL_RIGHTSHIFT;
				TryChangeTerminationType(world_data, world_coords.first + 2, world_coords.second,
				                         game::TransportLineSegment::TerminationType::bend_right,
				                         game::TransportLineSegment::TerminationType::right_only);
				break;

			default:
				break;
			}
		}
		break;
#undef TL_RIGHTSHIFT
#define TL_UPSHIFT\
	++(*segment_sl_y);\
	line_segment.length++;\
	UpdateSegmentTiles(world_data, world_coords, line_segment, 1)

	case Orientation::down:
		if (down) {
			switch (down->orientation) {
			case TransportLineData::LineOrientation::down_right:
				line_segment.terminationType = game::TransportLineSegment::TerminationType::bend_left;
				TL_UPSHIFT;
				break;
			case TransportLineData::LineOrientation::down_left:
				line_segment.terminationType = game::TransportLineSegment::TerminationType::bend_right;
				TL_UPSHIFT;
				break;

			case TransportLineData::LineOrientation::left:
				line_segment.terminationType = game::TransportLineSegment::TerminationType::right_only;
				TL_UPSHIFT;
				TryChangeTerminationType(world_data, world_coords.first, world_coords.second + 2,
				                         game::TransportLineSegment::TerminationType::bend_left,
				                         game::TransportLineSegment::TerminationType::left_only);
				break;
			case TransportLineData::LineOrientation::right:
				line_segment.terminationType = game::TransportLineSegment::TerminationType::left_only;
				TL_UPSHIFT;
				TryChangeTerminationType(world_data, world_coords.first, world_coords.second + 2,
				                         game::TransportLineSegment::TerminationType::bend_right,
				                         game::TransportLineSegment::TerminationType::right_only);
				break;

			default:
				break;
			}
		}
		break;
#undef TL_UPSHIFT
#define TL_LEFTSHIFT\
	--(*segment_sl_x);\
	line_segment.length++;\
	UpdateSegmentTiles(world_data, world_coords, line_segment, 1)

	case Orientation::left:
		if (left) {
			switch (left->orientation) {
			case TransportLineData::LineOrientation::left_down:
				line_segment.terminationType = game::TransportLineSegment::TerminationType::bend_left;
				TL_LEFTSHIFT;
				break;
			case TransportLineData::LineOrientation::left_up:
				line_segment.terminationType = game::TransportLineSegment::TerminationType::bend_right;
				TL_LEFTSHIFT;
				break;

			case TransportLineData::LineOrientation::up:
				line_segment.terminationType = game::TransportLineSegment::TerminationType::right_only;
				TL_LEFTSHIFT;
				TryChangeTerminationType(world_data, world_coords.first - 2, world_coords.second,
				                         game::TransportLineSegment::TerminationType::bend_left,
				                         game::TransportLineSegment::TerminationType::left_only);
				break;

			case TransportLineData::LineOrientation::down:
				line_segment.terminationType = game::TransportLineSegment::TerminationType::left_only;
				TL_LEFTSHIFT;
				TryChangeTerminationType(world_data, world_coords.first - 2, world_coords.second,
				                         game::TransportLineSegment::TerminationType::bend_right,
				                         game::TransportLineSegment::TerminationType::right_only);
				break;

			default:
				break;
			}
		}
		break;
#undef TL_LEFTSHIFT

	default:
		assert(false);  // Missing switch case
	}
}

void jactorio::data::TransportLine::UpdateSegmentTiles(const game::WorldData& world_data,
                                                       const game::WorldData::WorldPair& world_coords,
                                                       game::TransportLineSegment& line_segment,
                                                       const int offset) {
	uint64_t x_offset = 0;
	uint64_t y_offset = 0;

	// Should be -1, 0, 1 depending on orientation
	uint64_t x_change = 0;
	uint64_t y_change = 0;
	OrientationIncrement(line_segment.direction, x_change, y_change, -1);

	// Adjust the segment index number of all following segments 
	for (int i = offset; i < line_segment.length; ++i) {
		auto* i_line_data = GetLineData(world_data,
		                                world_coords.first + x_offset,
		                                world_coords.second + y_offset);
		if (!i_line_data)
			continue;
		i_line_data->lineSegmentIndex = i;
		i_line_data->lineSegment      = std::ref(line_segment);

		x_offset += x_change;
		y_offset += y_change;
	}
}

jactorio::data::TransportLineData* jactorio::data::TransportLine::InitTransportSegment(game::WorldData& world_data,
                                                                                       const game::WorldData::WorldPair&
                                                                                       world_coords,
                                                                                       const Orientation orientation,
                                                                                       game::ChunkTileLayer& tile_layer,
                                                                                       LineData4Way& line_data) const {

	static_assert(static_cast<int>(Orientation::left) == 3);  // Indexing line_data will be out of range 

	auto* origin_l_chunk = world_data.LogicGetChunk(world_data.GetChunk(world_coords));  // May be nullptr

	game::TransportLineSegment* line_segment;
	int line_segment_index = 0;

	enum class InitSegmentStatus
	{
		new_segment,
		group_ahead,
		group_behind
	} status;
	const auto index  = static_cast<int>(orientation);
	const int i_index = InvertOrientation(index);

	if (!line_data[index] ||
		line_data[index]->lineSegment.get().direction != orientation) {

		status = InitSegmentStatus::new_segment;  // If failed to group with ahead, this is chosen

		// Failed to group with ahead, try to group with segment behind
		if (line_data[i_index] &&
			line_data[i_index]->lineSegment.get().direction == orientation) {
			// Group behind

			game::WorldData::WorldPair behind_coords = world_coords;
			OrientationIncrement(orientation, behind_coords.first, behind_coords.second, -1);

			// Within the same logic chunk = Can group behind
			status = InitSegmentStatus::group_behind;
			GetLineStructLayer(world_data,
			                   behind_coords.first, behind_coords.second,
			                   [&](auto& s, auto& l_chunk) {
				                   // Different memory addresses = different logic chunks
				                   if (origin_l_chunk != &l_chunk) {
					                   status = InitSegmentStatus::new_segment;
					                   return;
				                   }
				                   OrientationIncrement(orientation,
				                                        s.positionX, s.positionY);
			                   });
		}
	}
	else {
		// Group ahead

		game::WorldData::WorldPair ahead_coords = world_coords;
		OrientationIncrement(orientation, ahead_coords.first, ahead_coords.second, 1);

		status = InitSegmentStatus::group_ahead;
		GetLineStructLayer(world_data,
		                   ahead_coords.first, ahead_coords.second,
		                   [&](auto&, auto& l_chunk) {
			                   // Different memory addresses = different logic chunks
			                   if (origin_l_chunk != &l_chunk) {
				                   status = InitSegmentStatus::new_segment;
			                   }
		                   });

	}

	// ======================================================================

	switch (status) {
	case InitSegmentStatus::new_segment:
		line_segment = new game::TransportLineSegment{
			orientation,
			game::TransportLineSegment::TerminationType::straight,
			1
		};

		// Add the transport line segment to logic chunk IF is the first of a NEW segment
		{
			auto* chunk = world_data.GetChunk(world_coords);

			auto& struct_layer =
				world_data.LogicAddChunk(chunk)
				          .GetStruct(game::LogicChunk::StructLayer::transport_line)
				          .emplace_back(this,
				                        game::WorldData::ToStructCoord(world_coords.first),
				                        game::WorldData::ToStructCoord(world_coords.second));

			struct_layer.uniqueData = line_segment;
		}
		break;

	case InitSegmentStatus::group_behind:
		// The transport segment's position is adjusted by init_transport_segment
		// Move the segment head behind forwards to current position
		line_segment = &line_data[static_cast<int>(InvertOrientation(orientation))]->lineSegment.get();
		line_data[i_index]->lineSegment.get().length++;
		break;

	case InitSegmentStatus::group_ahead:
		line_segment = &line_data[index]->lineSegment.get();

		line_data[index]->lineSegment.get().length++;
		line_segment_index = line_data[index]->lineSegmentIndex + 1;
		break;

	default:
		assert(false);
		break;
	}

	// Create unique data at tile
	tile_layer.uniqueData = new TransportLineData(*line_segment);

	auto* unique_data             = static_cast<TransportLineData*>(tile_layer.uniqueData);
	unique_data->lineSegmentIndex = line_segment_index;

	// Line data is not initialized yet inside switch
	if (status == InitSegmentStatus::group_behind)
		UpdateSegmentTiles(world_data, world_coords, *line_segment);

	return unique_data;
}

void jactorio::data::TransportLine::OnBuild(game::WorldData& world_data,
                                            const game::WorldData::WorldPair& world_coords,
                                            game::ChunkTileLayer& tile_layer,
                                            const Orientation orientation) const {
	auto* up    = GetLineData(world_data, world_coords.first, world_coords.second - 1);
	auto* right = GetLineData(world_data, world_coords.first + 1, world_coords.second);
	auto* down  = GetLineData(world_data, world_coords.first, world_coords.second + 1);
	auto* left  = GetLineData(world_data, world_coords.first - 1, world_coords.second);
	TransportLineData* line_data_4[4]{up, right, down, left};

	// ======================================================================
	// Create data
	TransportLineData& line_data = *InitTransportSegment(world_data, world_coords, orientation, tile_layer, line_data_4);
	game::TransportLineSegment& line_segment = line_data.lineSegment;

	const TransportLineData::LineOrientation line_orientation = GetLineOrientation(orientation, up, right, down, left);

	line_data.set         = static_cast<uint16_t>(line_orientation);
	line_data.orientation = line_orientation;


	// ======================================================================
	// Update neighbor rendering orientation
	// This has to be done PRIOR to adjusting for bends

	// Take the 4 transport lines neighboring the center as parameters to avoid recalculating them
	UpdateNeighboringOrientation(
		world_data, world_coords,
		up, right, down, left,
		static_cast<TransportLineData*>(tile_layer.uniqueData));

	// Change current line segment termination type to a bend depending on neighbor termination orientation
	UpdateSegmentHead(world_data, world_coords, line_data_4, line_segment);

	// Updates the termination type and length of neighboring lines
	UpdateNeighborLines(
		world_data,
		world_coords.first, world_coords.second,
		line_orientation,

		[](game::WorldData& world_data,
		   const int world_x, const int world_y,
		   const float world_offset_x, const float world_offset_y,
		   const game::TransportLineSegment::TerminationType termination_type) {

			GetLineStructLayer(world_data, world_x, world_y, [&](auto& layer, auto&) {
				layer.positionX += world_offset_x;
				layer.positionY += world_offset_y;

				auto& line_segment = *static_cast<game::TransportLineSegment*>(
					layer.uniqueData);

				line_segment.length++;
				line_segment.terminationType = termination_type;

				UpdateSegmentTiles(world_data, {world_x, world_y}, line_segment, 1);
			});
		}, [](game::WorldData& world_data,
		      const int world_x, const int world_y,
		      const float world_offset_x, const float world_offset_y,
		      const Orientation direction,
		      const game::TransportLineSegment::TerminationType termination_type) {

			GetLineStructLayer(world_data, world_x, world_y, [&](auto& layer, auto&) {
				auto& line_segment = *static_cast<game::TransportLineSegment*>(layer.uniqueData);
				if (line_segment.direction != direction)
					return;

				layer.positionX += world_offset_x;
				layer.positionY += world_offset_y;

				line_segment.length++;
				line_segment.terminationType = termination_type;

				UpdateSegmentTiles(world_data, {world_x, world_y}, line_segment, 1);
			});
		});

	// ======================================================================
	// Set the target_segment to the neighbor it is pointing to, or the neighbor's target segment which is pointing to this
	if (up)
		UpdateLineTargets(world_data, line_segment, orientation,
		                  world_coords.first, world_coords.second - 1,
		                  Orientation::up,
		                  Orientation::down);
	if (right)
		UpdateLineTargets(world_data, line_segment, orientation,
		                  world_coords.first + 1, world_coords.second,
		                  Orientation::right,
		                  Orientation::left);
	if (down)
		UpdateLineTargets(world_data, line_segment, orientation,
		                  world_coords.first, world_coords.second + 1,
		                  Orientation::down,
		                  Orientation::up);
	if (left)
		UpdateLineTargets(world_data, line_segment, orientation,
		                  world_coords.first - 1, world_coords.second,
		                  Orientation::left,
		                  Orientation::right);
}

// ======================================================================
// Neighbor update
void jactorio::data::TransportLine::OnNeighborUpdate(game::WorldData& world_data,
                                                     const game::WorldData::WorldPair& /*emit_world_coords*/,
                                                     const game::WorldData::WorldPair& receive_world_coords,
                                                     Orientation /*emit_orientation*/) const {
	// Run stuff here that on_build and on_remove both calls

	auto* line_data = GetLineData(world_data, receive_world_coords.first, receive_world_coords.second);
	if (!line_data)  // Transport line does not exist here
		return;

	// ======================================================================

	const UpdateFunc func =
		[](game::WorldData& world_data,
		   const int world_x, const int world_y,
		   float /*world_offset_x*/, float /*world_offset_y*/,
		   game::TransportLineSegment::TerminationType termination_type) {

		GetLineStructLayer(world_data, world_x, world_y, [&](auto& layer, auto&) {
			auto* line_segment            = static_cast<game::TransportLineSegment*>(layer.uniqueData);
			line_segment->terminationType = termination_type;
		});
	};

	const UpdateSideOnlyFunc side_only_func =
		[](game::WorldData& world_data,
		   const int world_x, const int world_y,
		   float /*world_offset_x*/, float /*world_offset_y*/,
		   Orientation direction,
		   game::TransportLineSegment::TerminationType termination_type) {

		GetLineStructLayer(world_data, world_x, world_y, [&](auto& layer, auto&) {
			auto* line_segment = static_cast<game::TransportLineSegment*>(layer.uniqueData);
			if (line_segment->direction != direction)
				return;

			line_segment->terminationType = termination_type;

		});
	};

	UpdateNeighborLines(world_data,
	                    receive_world_coords.first, receive_world_coords.second,
	                    line_data->orientation,
	                    func, side_only_func);
}


// ======================================================================
// Remove

void jactorio::data::TransportLine::DisconnectTargetSegment(game::WorldData& world_data,
                                                            const game::WorldData::WorldPair& world_coords,
                                                            TransportLineData* line_data,
                                                            TransportLineData* target) {

	if (line_data && line_data->lineSegment.get().targetSegment == &target->lineSegment.get()) {
		game::TransportLineSegment& line_segment = line_data->lineSegment;
		line_segment.targetSegment               = nullptr;

		game::WorldData::WorldPair neighbor_world_coords = world_coords;

		// If bends / side only, set to straight & decrement length	
		switch (line_segment.terminationType) {
		case game::TransportLineSegment::TerminationType::bend_left:
		case game::TransportLineSegment::TerminationType::bend_right:
		case game::TransportLineSegment::TerminationType::right_only:
		case game::TransportLineSegment::TerminationType::left_only:
			line_segment.length--;
			line_segment.terminationType = game::TransportLineSegment::TerminationType::straight;

			// Move the neighboring line segments back if they are not straight
			OrientationIncrement(line_segment.direction,
			                     neighbor_world_coords.first, neighbor_world_coords.second, -1.f);

			GetLineStructLayer(
				world_data,
				neighbor_world_coords.first, neighbor_world_coords.second,
				[&](auto& s_layer, auto&) {
					OrientationIncrement(line_segment.direction,
					                     s_layer.positionX, s_layer.positionY, -1.f);
				});

			// Renumber from index 0
			UpdateSegmentTiles(world_data, neighbor_world_coords, line_segment);
			break;

		default:
			// Does not bend
			break;
		}
	}
}

void jactorio::data::TransportLine::OnRemove(game::WorldData& world_data,
                                             const game::WorldData::WorldPair& world_coords,
                                             game::ChunkTileLayer& tile_layer) const {
	auto* t_center = GetLineData(world_data, world_coords.first, world_coords.second - 1);
	auto* c_left   = GetLineData(world_data, world_coords.first - 1, world_coords.second);
	auto* c_right  = GetLineData(world_data, world_coords.first + 1, world_coords.second);
	auto* b_center = GetLineData(world_data, world_coords.first, world_coords.second + 1);

	UpdateNeighboringOrientation(world_data, world_coords,
	                             t_center, c_right, b_center, c_left, nullptr);


	// Set neighboring transport line segments which points to this segment's target_segment to nullptr
	DisconnectTargetSegment(world_data, world_coords,
	                        t_center, static_cast<TransportLineData*>(tile_layer.uniqueData));
	DisconnectTargetSegment(world_data, world_coords,
	                        c_left, static_cast<TransportLineData*>(tile_layer.uniqueData));
	DisconnectTargetSegment(world_data, world_coords,
	                        c_right, static_cast<TransportLineData*>(tile_layer.uniqueData));
	DisconnectTargetSegment(world_data, world_coords,
	                        b_center, static_cast<TransportLineData*>(tile_layer.uniqueData));

	game::LogicChunk& logic_chunk =
		*world_data.LogicGetChunk(world_coords.first, world_coords.second);

	// o_ = old
	// n_ = new

	auto& o_line_data    = *static_cast<TransportLineData*>(tile_layer.uniqueData);
	auto& o_line_segment = o_line_data.lineSegment.get();

	auto n_seg_coords = world_coords;
	OrientationIncrement(o_line_segment.direction,
	                     n_seg_coords.first, n_seg_coords.second, -1);

	// Create new segment at behind cords if not the end of a segment
	const auto n_seg_length = o_line_segment.length - o_line_data.lineSegmentIndex - 1;
	if (n_seg_length > 0) {
		game::ChunkTileLayer& n_tile = world_data.GetTile(n_seg_coords)->GetLayer(game::ChunkTile::ChunkLayer::entity);

		// Create new segment
		static_cast<TransportLineData*>(n_tile.uniqueData)->lineSegment =
			std::ref(*new game::TransportLineSegment(o_line_segment.direction,
			                                         game::TransportLineSegment::TerminationType::straight,
			                                         n_seg_length
				)
			);

		auto& n_line_data    = *static_cast<TransportLineData*>(n_tile.uniqueData);
		auto& n_line_segment = n_line_data.lineSegment.get();

		// Renumber trailing segments
		UpdateSegmentTiles(world_data, n_seg_coords, n_line_segment);

		// Update other segments leading into old segment
		for (auto& t_line : logic_chunk.GetStruct(game::LogicChunk::StructLayer::transport_line)) {
			auto& i_segment_data = *static_cast<game::TransportLineSegment*>(t_line.uniqueData);

			bool valid_neighbor = false;  // Neighbor must be BEHIND the segment which was removed
			switch (o_line_segment.direction) {
			case Orientation::up:
				valid_neighbor = t_line.positionY > game::WorldData::ToStructCoord(world_coords.second);
				break;
			case Orientation::right:
				valid_neighbor = t_line.positionX < game::WorldData::ToStructCoord(world_coords.first);
				break;
			case Orientation::down:
				valid_neighbor = t_line.positionY < game::WorldData::ToStructCoord(world_coords.second);
				break;
			case Orientation::left:
				valid_neighbor = t_line.positionX > game::WorldData::ToStructCoord(world_coords.first);
				break;

			default:
				assert(false);
			}

			if (valid_neighbor && i_segment_data.targetSegment == &o_line_segment) {
				i_segment_data.targetSegment = &n_line_segment;
			}
		}
		// Update segment in neighboring logic chunk leading into old_segment
		game::Chunk::ChunkPair neighbor_chunk_coords = logic_chunk.chunk->GetPosition();
		OrientationIncrement(o_line_segment.direction,
		                     neighbor_chunk_coords.first, neighbor_chunk_coords.second, -1);

		auto* neighbor_l_chunk = world_data.LogicGetChunk(neighbor_chunk_coords);
		if (neighbor_l_chunk) {
			for (auto& t_line : neighbor_l_chunk->GetStruct(game::LogicChunk::StructLayer::transport_line)) {
				auto* i_segment_data = static_cast<game::TransportLineSegment*>(t_line.uniqueData);

				if (i_segment_data->targetSegment == &o_line_segment) {
					i_segment_data->targetSegment = &n_line_segment;
				}

			}
		}


		// All segments are guaranteed to be within the same logic chunk
		auto& struct_layer =
			logic_chunk.GetStruct(game::LogicChunk::StructLayer::transport_line)
			           .emplace_back(this,
			                         game::WorldData::ToStructCoord(n_seg_coords.first),
			                         game::WorldData::ToStructCoord(n_seg_coords.second));

		struct_layer.uniqueData = &n_line_segment;
	}

	// Remove original transport line segment referenced in Transport_line_data if is head of segment
	// If not head, reduce the length of original segment to index + 1
	if (o_line_data.lineSegmentIndex == 0 ||
		(o_line_data.lineSegmentIndex == 1 && o_line_segment.terminationType !=  // Head of bending segments start at 1
			game::TransportLineSegment::TerminationType::straight)) {

		std::vector<game::ChunkStructLayer>& struct_layer = logic_chunk.GetStruct(
			game::LogicChunk::StructLayer::transport_line);
		struct_layer.erase(
			std::remove_if(struct_layer.begin(), struct_layer.end(), [&](game::ChunkStructLayer& s_layer) {
				return static_cast<game::TransportLineSegment*>(s_layer.uniqueData) == &o_line_segment;
			}),
			struct_layer.end());
	}
	else {
		o_line_segment.length = o_line_data.lineSegmentIndex;
	}
}
