// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#include "data/prototype/entity/transport/transport_line.h"

#include <cmath>

#include "data/data_manager.h"
#include "game/logic/transport_segment.h"
#include "renderer/rendering/data_renderer.h"

using namespace jactorio;

data::Orientation data::TransportLineData::ToOrientation(const LineOrientation line_orientation) {
	switch (line_orientation) {
	case LineOrientation::up:
	case LineOrientation::right_up:
	case LineOrientation::left_up:
		return Orientation::up;

	case LineOrientation::right:
	case LineOrientation::up_right:
	case LineOrientation::down_right:
		return Orientation::right;

	case LineOrientation::down:
	case LineOrientation::right_down:
	case LineOrientation::left_down:
		return Orientation::down;

	case LineOrientation::left:
	case LineOrientation::up_left:
	case LineOrientation::down_left:
		return Orientation::left;

	default:
		assert(false);  // Missing switch case
		return Orientation::up;
	}
}

void data::TransportLineData::OnDrawUniqueData(renderer::RendererLayer& layer,
                                               const float x_offset, const float y_offset) {
	// Only draw for the head of segments
	if (lineSegment.get().terminationType == game::TransportSegment::TerminationType::straight &&
		lineSegmentIndex != 0)
		return;

	if (lineSegment.get().terminationType != game::TransportSegment::TerminationType::straight &&
		lineSegmentIndex != 1)
		return;

	DrawTransportSegmentItems(layer,
	                          x_offset, y_offset,
	                          this->lineSegment.get());
}

//

std::pair<uint16_t, uint16_t> data::TransportLine::MapPlacementOrientation(
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
data::TransportLineData::LineOrientation data::TransportLine::GetLineOrientation(
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

data::TransportLineData* data::TransportLine::GetLineData(const game::WorldData& world_data,
                                                          const game::WorldData::WorldCoord world_x,
                                                          const game::WorldData::WorldCoord world_y) {
	const auto* tile = world_data.GetTile(world_x, world_y);
	if (!tile)  // No tile exists
		return nullptr;

	auto& layer = tile->GetLayer(game::ChunkTile::ChunkLayer::entity);

	if (!dynamic_cast<const TransportLine*>(  // Not an instance of transport line
		layer.prototypeData))
		return nullptr;

	return static_cast<TransportLineData*>(layer.uniqueData);
}

game::TransportSegment* data::TransportLine::GetTransportSegment(game::WorldData& world_data,
                                                                 const game::WorldData::WorldCoord world_x,
                                                                 const game::WorldData::WorldCoord world_y) {
	auto* tile = world_data.GetTile(world_x, world_y);
	if (tile) {
		auto& layer = tile->GetLayer(game::ChunkTile::ChunkLayer::entity);
		if (!layer.prototypeData || layer.prototypeData->Category() != DataCategory::transport_belt)
			return nullptr;

		auto* unique_data = layer.uniqueData;
		return &static_cast<data::TransportLineData*>(unique_data)->lineSegment.get();
	}

	return nullptr;
}

void data::TransportLine::RemoveFromLogic(game::WorldData& world_data,
                                          const game::WorldData::WorldPair& world_coords, game::TransportSegment& line_segment) {
	world_data.LogicRemove(
		game::Chunk::LogicGroup::transport_line,
		world_coords,
		[&](auto* t_layer) {
			return &static_cast<TransportLineData*>(t_layer->uniqueData)->lineSegment.get() == &line_segment;
		});
}


//
//
//
//
//
//
//
//
//
//
//
//
//
//
//

// ======================================================================
// Build / Remove / Neighbor update

void data::TransportLine::UpdateNeighboringOrientation(const game::WorldData& world_data,
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

void data::TransportLine::UpdateNeighborLines(
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
		     game::TransportSegment::TerminationType::bend_left);
		break;
	case TransportLineData::LineOrientation::up_right:
		func(world_data, world_x, world_y + 1,
		     0, -1,
		     game::TransportSegment::TerminationType::bend_right);
		break;

		// Right
	case TransportLineData::LineOrientation::right_up:
		func(world_data, world_x - 1, world_y,
		     1, 0,
		     game::TransportSegment::TerminationType::bend_left);
		break;
	case TransportLineData::LineOrientation::right_down:
		func(world_data, world_x - 1, world_y,
		     1, 0,
		     game::TransportSegment::TerminationType::bend_right);
		break;

		// Down
	case TransportLineData::LineOrientation::down_right:
		func(world_data, world_x, world_y - 1,
		     0, 1,
		     game::TransportSegment::TerminationType::bend_left);
		break;
	case TransportLineData::LineOrientation::down_left:
		func(world_data, world_x, world_y - 1,
		     0, 1,
		     game::TransportSegment::TerminationType::bend_right);
		break;

		// Left
	case TransportLineData::LineOrientation::left_down:
		func(world_data, world_x + 1, world_y,
		     -1, 0,
		     game::TransportSegment::TerminationType::bend_left);
		break;
	case TransportLineData::LineOrientation::left_up:
		func(world_data, world_x + 1, world_y,
		     -1, 0,
		     game::TransportSegment::TerminationType::bend_right);
		break;

		// Straight (Check for transport lines on both sides to make side only)
	case TransportLineData::LineOrientation::up:
		side_only_func(world_data, world_x - 1, world_y,
		               1, 0,
		               Orientation::right,
		               game::TransportSegment::TerminationType::left_only);
		side_only_func(world_data, world_x + 1, world_y,
		               -1, 0,
		               Orientation::left,
		               game::TransportSegment::TerminationType::right_only);
		break;
	case TransportLineData::LineOrientation::right:
		side_only_func(world_data, world_x, world_y - 1,
		               0, 1,
		               Orientation::down,
		               game::TransportSegment::TerminationType::left_only);
		side_only_func(world_data, world_x, world_y + 1,
		               0, -1,
		               Orientation::up,
		               game::TransportSegment::TerminationType::right_only);
		break;
	case TransportLineData::LineOrientation::down:
		side_only_func(world_data, world_x - 1, world_y,
		               1, 0,
		               Orientation::right,
		               game::TransportSegment::TerminationType::right_only);
		side_only_func(world_data, world_x + 1, world_y,
		               -1, 0,
		               Orientation::left,
		               game::TransportSegment::TerminationType::left_only);
		break;
	case TransportLineData::LineOrientation::left:
		side_only_func(world_data, world_x, world_y - 1,
		               0, 1,
		               Orientation::down,
		               game::TransportSegment::TerminationType::right_only);
		side_only_func(world_data, world_x, world_y + 1,
		               0, -1,
		               Orientation::up,
		               game::TransportSegment::TerminationType::left_only);
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
/// \brief If a transport segment exists at world_x, world_y and has a terminal type == Required,
/// it will be changed to New
template <game::TransportSegment::TerminationType Required,
          game::TransportSegment::TerminationType New>
void TryChangeTerminationType(const game::WorldData& world_data,
                              const int32_t world_x, const int32_t world_y) {
	using namespace jactorio;

	data::TransportLineData* line_data = data::TransportLine::GetLineData(world_data, world_x, world_y);
	if (line_data) {
		if (line_data->lineSegment.get().terminationType == Required) {
			line_data->lineSegment.get().terminationType = New;
		}
	}
}

///
/// \brief Updates the world tiles which references a transport segment, props: line_segment_index, line_segment
/// \param world_coords Beginning tile to update
/// \param line_segment Beginning segment, traveling inverse Orientation line_segment.length tiles, <br>
/// all tiles set to reference this
/// \param offset Offsets segment id numbering, world_coords must be also adjusted to the appropriate offset when calling
void UpdateSegmentTiles(const game::WorldData& world_data,
                        const game::WorldData::WorldPair& world_coords,
                        game::TransportSegment& line_segment,
                        const int offset = 0) {
	uint64_t x_offset = 0;
	uint64_t y_offset = 0;

	// Should be -1, 0, 1 depending on orientation
	uint64_t x_change = 0;
	uint64_t y_change = 0;
	OrientationIncrement(line_segment.direction, x_change, y_change, -1);

	// Adjust the segment index number of all following segments 
	for (int i = offset; i < line_segment.length; ++i) {
		auto* i_line_data = data::TransportLine::GetLineData(world_data,
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

///
/// \brief Determines the member target_segment of Transport_line_segment
/// \tparam OriginConnect Orientation required for origin_segment to connect to neighbor segment
/// \tparam TargetConnect Orientation required for neighbor segment to connect to origin segment
/// \param orientation Current orientation
template <data::Orientation OriginConnect, data::Orientation TargetConnect>
void UpdateLineTargets(game::WorldData& world_data,
                       data::TransportLineData& origin_data,
                       const data::Orientation orientation,
                       const int32_t neighbor_world_x, const int32_t neighbor_world_y) {
	using namespace jactorio;

	auto& origin_segment = origin_data.lineSegment.get();

	data::TransportLineData* neighbor_data =
		data::TransportLine::GetLineData(world_data, neighbor_world_x, neighbor_world_y);
	if (neighbor_data) {
		game::TransportSegment& neighbor_segment = neighbor_data->lineSegment;

		// Do not attempt to connect to itself
		if (&origin_segment == &neighbor_segment)
			return;

		const bool origin_valid   = orientation == OriginConnect;
		const bool neighbor_valid = neighbor_segment.direction == TargetConnect;

		// Only 1 can be valid at a time (does not both point to each other)
		// Either origin feeds into neighbor, or neighbor feeds into origin depending on which one is valid
		if (origin_valid == neighbor_valid)
			return;

		if (origin_valid) {
			origin_segment.targetSegment = &neighbor_segment;

			origin_segment.targetInsertOffset = neighbor_data->lineSegmentIndex;
			neighbor_segment.GetOffsetAbs(origin_segment.targetInsertOffset);
		}
		else {
			neighbor_segment.targetSegment = &origin_segment;

			neighbor_segment.targetInsertOffset = origin_data.lineSegmentIndex;
			origin_segment.GetOffsetAbs(neighbor_segment.targetInsertOffset);
		}
	}
}

///
/// \brief Changes the provided line segment's properties based on its 4 neighbors
template <data::Orientation Orientation,
          data::TransportLineData::LineOrientation BendLeft,
          data::TransportLineData::LineOrientation BendRight,
          data::TransportLineData::LineOrientation LeftOnly,
          data::TransportLineData::LineOrientation RightOnly>
void UpdateSegmentProps(data::TransportLineData* line_data,
                        game::WorldData& world_data,
                        const game::WorldData::WorldPair& world_coords,
                        game::TransportSegment& line_segment) {
	using namespace jactorio;

	if (!line_data)
		return;

	switch (line_data->orientation) {
	case BendLeft:
		line_segment.terminationType = game::TransportSegment::TerminationType::bend_left;
		goto shift_segment;

	case BendRight:
		line_segment.terminationType = game::TransportSegment::TerminationType::bend_right;
		goto shift_segment;

	case LeftOnly:
		line_segment.terminationType = game::TransportSegment::TerminationType::left_only;

#define LEFT_ONLY_CHANGE_TERMINATION_TYPE\
			TryChangeTerminationType<game::TransportSegment::TerminationType::bend_right,\
			                         game::TransportSegment::TerminationType::right_only>(

		if constexpr (Orientation == data::Orientation::up) {
			LEFT_ONLY_CHANGE_TERMINATION_TYPE
				world_data, world_coords.first, world_coords.second - 2);
		}
		else if constexpr (Orientation == data::Orientation::right) {
			LEFT_ONLY_CHANGE_TERMINATION_TYPE
				world_data, world_coords.first + 2, world_coords.second);
		}
		else if constexpr (Orientation == data::Orientation::down) {
			LEFT_ONLY_CHANGE_TERMINATION_TYPE
				world_data, world_coords.first, world_coords.second + 2);
		}
		else if constexpr (Orientation == data::Orientation::left) {
			LEFT_ONLY_CHANGE_TERMINATION_TYPE
				world_data, world_coords.first - 2, world_coords.second);
		}

#undef LEFT_ONLY_CHANGE_TERMINATION_TYPE
		goto shift_segment;

	case RightOnly:
		line_segment.terminationType = game::TransportSegment::TerminationType::right_only;

#define RIGHT_ONLY_CHANGE_TERMINATION_TYPE\
			TryChangeTerminationType<game::TransportSegment::TerminationType::bend_left,\
			                         game::TransportSegment::TerminationType::left_only>(

		// Check 2 units up and see if there is segment bending left, if so change it to straight
		if constexpr (Orientation == data::Orientation::up) {
			RIGHT_ONLY_CHANGE_TERMINATION_TYPE
				world_data, world_coords.first, world_coords.second - 2);
		}
		else if constexpr (Orientation == data::Orientation::right) {
			RIGHT_ONLY_CHANGE_TERMINATION_TYPE
				world_data, world_coords.first + 2, world_coords.second);
		}
		else if constexpr (Orientation == data::Orientation::down) {
			RIGHT_ONLY_CHANGE_TERMINATION_TYPE
				world_data, world_coords.first, world_coords.second + 2);
		}
		else if constexpr (Orientation == data::Orientation::left) {
			RIGHT_ONLY_CHANGE_TERMINATION_TYPE
				world_data, world_coords.first - 2, world_coords.second);
		}

#undef RIGHT_ONLY_CHANGE_TERMINATION_TYPE


	shift_segment:
		line_segment.length++;
		UpdateSegmentTiles(world_data, world_coords, line_segment, 1);
		break;

	default:
		break;
	}
}

void data::TransportLine::UpdateSegmentHead(game::WorldData& world_data,
                                            const game::WorldData::WorldPair& world_coords,
                                            LineData4Way& line_data,
                                            game::TransportSegment& line_segment) {
	switch (line_segment.direction) {

	case Orientation::up:
		UpdateSegmentProps<Orientation::up,
		                   TransportLineData::LineOrientation::up_left,
		                   TransportLineData::LineOrientation::up_right,
		                   TransportLineData::LineOrientation::right,
		                   TransportLineData::LineOrientation::left>(line_data[0],
		                                                             world_data, world_coords, line_segment);
		break;
	case Orientation::right:
		UpdateSegmentProps<Orientation::right,
		                   TransportLineData::LineOrientation::right_up,
		                   TransportLineData::LineOrientation::right_down,
		                   TransportLineData::LineOrientation::down,
		                   TransportLineData::LineOrientation::up>(line_data[1],
		                                                           world_data, world_coords, line_segment);
		break;
	case Orientation::down:
		UpdateSegmentProps<Orientation::down,
		                   TransportLineData::LineOrientation::down_right,
		                   TransportLineData::LineOrientation::down_left,
		                   TransportLineData::LineOrientation::left,
		                   TransportLineData::LineOrientation::right>(line_data[2],
		                                                              world_data, world_coords, line_segment);
		break;
	case Orientation::left:
		UpdateSegmentProps<Orientation::left,
		                   TransportLineData::LineOrientation::left_down,
		                   TransportLineData::LineOrientation::left_up,
		                   TransportLineData::LineOrientation::up,
		                   TransportLineData::LineOrientation::down>(line_data[3],
		                                                             world_data, world_coords, line_segment);
		break;

	default:
		assert(false);  // Missing switch case
	}
}

data::TransportLineData* data::TransportLine::InitTransportSegment(game::WorldData& world_data,
                                                                   const game::WorldData::WorldPair&
                                                                   world_coords,
                                                                   const Orientation orientation,
                                                                   game::ChunkTileLayer& tile_layer,
                                                                   LineData4Way& line_data) const {

	static_assert(static_cast<int>(Orientation::left) == 3);  // Indexing line_data will be out of range 

	auto& origin_chunk = *world_data.GetChunk(world_coords);

	game::TransportSegment* line_segment;
	int line_segment_index = 0;

	enum class InitSegmentStatus
	{
		new_segment,
		group_ahead, // Segment ahead of current location
		group_behind // Segment behind current location
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
			if (&origin_chunk != world_data.GetChunk(behind_coords)) {
				// Different memory addresses = different logic chunks
				status = InitSegmentStatus::new_segment;
			}
		}
	}
	else {
		// Group ahead

		game::WorldData::WorldPair ahead_coords = world_coords;
		OrientationIncrement(orientation, ahead_coords.first, ahead_coords.second, 1);

		status = InitSegmentStatus::group_ahead;
		if (&origin_chunk != world_data.GetChunk(ahead_coords)) {
			// Different memory addresses = different logic chunks
			status = InitSegmentStatus::new_segment;
		}
	}

	// ======================================================================

	switch (status) {
	case InitSegmentStatus::new_segment:
		line_segment = new game::TransportSegment{
			orientation,
			game::TransportSegment::TerminationType::straight,
			1
		};

		world_data.LogicRegister(game::Chunk::LogicGroup::transport_line,
		                         world_coords,
		                         game::ChunkTile::ChunkLayer::entity);
		break;

	case InitSegmentStatus::group_behind:
		// The transport segment's position is adjusted by init_transport_segment
		// Move the segment head behind forwards to current position
		line_segment = &line_data[i_index]->lineSegment.get();
		line_segment->length++;
		line_segment->itemOffset++;
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
	if (status == InitSegmentStatus::group_behind) {
		// Remove old head from logic group, add new head which is now 1 tile ahead
		auto& chunk = *world_data.GetChunk(world_coords);

		RemoveFromLogic(world_data, world_coords, *line_segment);
		chunk.GetLogicGroup(game::Chunk::LogicGroup::transport_line).emplace_back(&tile_layer);

		// Renumber
		UpdateSegmentTiles(world_data, world_coords, *line_segment);
	}

	return unique_data;
}

void data::TransportLine::OnBuild(game::WorldData& world_data,
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
	TransportLineData& line_data         = *InitTransportSegment(world_data, world_coords, orientation, tile_layer, line_data_4);
	game::TransportSegment& line_segment = line_data.lineSegment;

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
		   const game::TransportSegment::TerminationType termination_type) {

			auto* line_segment = GetTransportSegment(world_data, world_x, world_y);
			if (line_segment) {
				line_segment->length++;
				line_segment->terminationType = termination_type;

				UpdateSegmentTiles(world_data, {world_x, world_y}, *line_segment, 1);
			}
		}, [](game::WorldData& world_data,
		      const int world_x, const int world_y,
		      const float world_offset_x, const float world_offset_y,
		      const Orientation direction,
		      const game::TransportSegment::TerminationType termination_type) {

			auto* line_segment = GetTransportSegment(world_data, world_x, world_y);
			if (line_segment) {
				if (line_segment->direction != direction)
					return;

				line_segment->length++;
				line_segment->terminationType = termination_type;

				UpdateSegmentTiles(world_data, {world_x, world_y}, *line_segment, 1);
			}
		});

	// ======================================================================
	// Set the target_segment to the neighbor it is pointing to, or the neighbor's target segment which is pointing to this
	if (up)
		UpdateLineTargets<Orientation::up, Orientation::down>(
			world_data, line_data,
			orientation,
			world_coords.first, world_coords.second - 1);
	if (right)
		UpdateLineTargets<Orientation::right, Orientation::left>(
			world_data, line_data,
			orientation,
			world_coords.first + 1, world_coords.second);
	if (down)
		UpdateLineTargets<Orientation::down, Orientation::up>(
			world_data, line_data,
			orientation,
			world_coords.first, world_coords.second + 1
		);
	if (left)
		UpdateLineTargets<Orientation::left, Orientation::right>(
			world_data, line_data,
			orientation,
			world_coords.first - 1, world_coords.second
		);
}

//
//
//
//
//
//
//
//
//
//
//
//
//
//
//

// ======================================================================
// Neighbor update
void data::TransportLine::OnNeighborUpdate(game::WorldData& world_data,
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
		   const game::TransportSegment::TerminationType termination_type) {

		auto* line_segment = GetTransportSegment(world_data, world_x, world_y);
		if (line_segment) {
			line_segment->terminationType = termination_type;
		}
	};

	const UpdateSideOnlyFunc side_only_func =
		[](game::WorldData& world_data,
		   const int world_x, const int world_y,
		   float /*world_offset_x*/, float /*world_offset_y*/,
		   const Orientation direction,
		   const game::TransportSegment::TerminationType termination_type) {

		auto* line_segment = GetTransportSegment(world_data, world_x, world_y);
		if (line_segment) {
			if (line_segment->direction != direction)
				return;

			line_segment->terminationType = termination_type;
		}
	};

	UpdateNeighborLines(world_data,
	                    receive_world_coords.first, receive_world_coords.second,
	                    line_data->orientation,
	                    func, side_only_func);
}

//
//
//
//
//
//
//
//
//
//
//
//
//
//
//

// ======================================================================
// Remove

void data::TransportLine::DisconnectTargetSegment(game::WorldData& world_data,
                                                  const game::WorldData::WorldPair& world_coords,
                                                  TransportLineData* target,
                                                  TransportLineData* line_data) {

	if (line_data && line_data->lineSegment.get().targetSegment == &target->lineSegment.get()) {
		game::TransportSegment& line_segment = line_data->lineSegment;
		line_segment.targetSegment           = nullptr;

		game::WorldData::WorldPair neighbor_world_coords = world_coords;

		// If bends / side only, set to straight & decrement length	
		switch (line_segment.terminationType) {
		case game::TransportSegment::TerminationType::bend_left:
		case game::TransportSegment::TerminationType::bend_right:
		case game::TransportSegment::TerminationType::right_only:
		case game::TransportSegment::TerminationType::left_only:
			line_segment.length--;
			line_segment.terminationType = game::TransportSegment::TerminationType::straight;

			// Move the neighboring line segments back if they are not straight
			OrientationIncrement(line_segment.direction,
			                     neighbor_world_coords.first, neighbor_world_coords.second, -1.f);

			// Renumber from index 0
			UpdateSegmentTiles(world_data, neighbor_world_coords, line_segment);
			break;

		default:
			// Does not bend
			break;
		}
	}
}

double ToChunkOffset(const game::WorldData::WorldCoord world_coord) {
	return fabs(game::WorldData::ToChunkCoord(world_coord) * game::Chunk::kChunkWidth - world_coord);
}

void data::TransportLine::OnRemove(game::WorldData& world_data,
                                   const game::WorldData::WorldPair& world_coords,
                                   game::ChunkTileLayer& tile_layer) const {
	auto* t_center = GetLineData(world_data, world_coords.first, world_coords.second - 1);
	auto* c_left   = GetLineData(world_data, world_coords.first - 1, world_coords.second);
	auto* c_right  = GetLineData(world_data, world_coords.first + 1, world_coords.second);
	auto* b_center = GetLineData(world_data, world_coords.first, world_coords.second + 1);

	UpdateNeighboringOrientation(world_data, world_coords,
	                             t_center, c_right, b_center, c_left, nullptr);

	auto* line_data = static_cast<TransportLineData*>(tile_layer.uniqueData);

	// Set neighboring transport line segments which points to this segment's target_segment to nullptr
	DisconnectTargetSegment(world_data, world_coords, line_data, t_center);
	DisconnectTargetSegment(world_data, world_coords, line_data, c_left);
	DisconnectTargetSegment(world_data, world_coords, line_data, c_right);
	DisconnectTargetSegment(world_data, world_coords, line_data, b_center);

	game::Chunk& chunk = *world_data.GetChunk(world_coords);

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
		// Create new segment
		auto& n_segment = *new game::TransportSegment(o_line_segment.direction,
		                                              game::TransportSegment::TerminationType::straight,
		                                              n_seg_length);
		n_segment.itemOffset = o_line_segment.itemOffset - o_line_data.lineSegmentIndex - 1;


		// Add to be considered for logic updates
		chunk.GetLogicGroup(game::Chunk::LogicGroup::transport_line).emplace_back(
			&world_data.GetTile(n_seg_coords)->GetLayer(game::ChunkTile::ChunkLayer::entity)
		);

		// ======================================================================

		// Update trailing segments to use new segment and renumber
		UpdateSegmentTiles(world_data, n_seg_coords, n_segment);  // <- First tile takes ownership of n_segment

		// Update other segments leading into old segment
		// TODO improve this algorithm for updating target segments
		for (int i = 0; i < game::Chunk::kChunkArea; ++i) {
			auto& layer = chunk.Tiles()[i].GetLayer(game::ChunkTile::ChunkLayer::entity);
			if (!layer.prototypeData || layer.prototypeData->Category() != DataCategory::transport_belt)
				continue;

			auto& line_segment = static_cast<TransportLineData*>(layer.uniqueData)->lineSegment.get();


			const auto position_x = i % game::Chunk::kChunkWidth;
			const auto position_y = i / game::Chunk::kChunkWidth;
			bool valid_neighbor   = false;  // Neighbor must be BEHIND the segment which was removed
			switch (o_line_segment.direction) {
			case Orientation::up:
				valid_neighbor = position_y > ToChunkOffset(world_coords.second);
				break;
			case Orientation::right:
				valid_neighbor = position_x < ToChunkOffset(world_coords.first);
				break;
			case Orientation::down:
				valid_neighbor = position_y < ToChunkOffset(world_coords.second);
				break;
			case Orientation::left:
				valid_neighbor = position_x > ToChunkOffset(world_coords.first);
				break;

			default:
				assert(false);
			}

			if (valid_neighbor && line_segment.targetSegment == &o_line_segment) {
				line_segment.targetSegment = &n_segment;
			}
		}


		// Update segment in neighboring logic chunk leading into old_segment
		game::Chunk::ChunkPair neighbor_chunk_coords = chunk.GetPosition();
		OrientationIncrement(o_line_segment.direction,
		                     neighbor_chunk_coords.first, neighbor_chunk_coords.second, -1);

		auto* neighbor_l_chunk = world_data.GetChunkC(neighbor_chunk_coords);
		if (neighbor_l_chunk) {
			for (auto& t_line : neighbor_l_chunk->GetLogicGroup(game::Chunk::LogicGroup::transport_line)) {
				auto& i_segment = static_cast<TransportLineData*>(t_line->uniqueData)->lineSegment.get();

				if (i_segment.targetSegment == &o_line_segment) {
					i_segment.targetSegment = &n_segment;
				}
			}
		}
	}

	// Remove original transport line segment referenced in Transport_line_data if is head of segment
	// If not head, reduce the length of original segment to index + 1
	if (o_line_data.lineSegmentIndex == 0 ||
		(o_line_data.lineSegmentIndex == 1 && o_line_segment.terminationType !=  // Head of bending segments start at 1
			game::TransportSegment::TerminationType::straight)) {

		RemoveFromLogic(world_data, world_coords, o_line_segment);
	}
	else {
		o_line_segment.length = o_line_data.lineSegmentIndex;
	}
}
