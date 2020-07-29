// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "data/prototype/entity/transport_line.h"

#include <cmath>

#include "data/prototype_manager.h"
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
                                                          const WorldCoordAxis world_x,
                                                          const WorldCoordAxis world_y) {
	const auto* tile = world_data.GetTile(world_x, world_y);
	if (!tile)  // No tile exists
		return nullptr;

	auto& layer = tile->GetLayer(game::ChunkTile::ChunkLayer::entity);

	if (!dynamic_cast<const TransportLine*>(  // Not an instance of transport line
		layer.prototypeData))
		return nullptr;

	return layer.GetUniqueData<TransportLineData>();
}

std::shared_ptr<game::TransportSegment>* data::TransportLine::GetTransportSegment(game::WorldData& world_data,
                                                                                  const WorldCoordAxis world_x,
                                                                                  const WorldCoordAxis world_y) {
	auto* tile = world_data.GetTile(world_x, world_y);
	if (tile) {
		auto& layer = tile->GetLayer(game::ChunkTile::ChunkLayer::entity);
		if (!layer.prototypeData || layer.prototypeData->Category() != DataCategory::transport_belt)
			return nullptr;

		auto* unique_data = layer.GetUniqueData<TransportLineData>();
		return &unique_data->lineSegment;
	}

	return nullptr;
}

void data::TransportLine::OnRDrawUniqueData(renderer::RendererLayer& layer, const SpriteUvCoordsT& uv_coords,
                                            const core::Position2<float>& pixel_offset,
                                            const UniqueDataBase* unique_data) const {
	auto& line_data = *static_cast<const TransportLineData*>(unique_data);

	// Only draw for the head of segments
	if (line_data.lineSegment->terminationType == game::TransportSegment::TerminationType::straight &&
		line_data.lineSegmentIndex != 0)
		return;

	if (line_data.lineSegment->terminationType != game::TransportSegment::TerminationType::straight &&
		line_data.lineSegmentIndex != 1)
		return;

	DrawTransportSegmentItems(layer, uv_coords,
	                          pixel_offset,
	                          *line_data.lineSegment);
}

data::Sprite::SetT data::TransportLine::OnRGetSpriteSet(const Orientation orientation, game::WorldData& world_data,
                                                        const WorldCoord& world_coords) const {
	auto* t_center = GetLineData(world_data, world_coords.x, world_coords.y - 1);
	auto* c_left   = GetLineData(world_data, world_coords.x - 1, world_coords.y);
	auto* c_right  = GetLineData(world_data, world_coords.x + 1, world_coords.y);
	auto* b_center = GetLineData(world_data, world_coords.x, world_coords.y + 1);

	return static_cast<uint16_t>(GetLineOrientation(orientation, t_center, c_right, b_center, c_left));
}

data::Sprite::FrameT data::TransportLine::OnRGetSpriteFrame(const UniqueDataBase& unique_data, const GameTickT game_tick) const {
	return AllOfSet(*sprite, game_tick);
}


void RemoveFromLogic(game::WorldData& world_data,
                     const WorldCoord& world_coords, game::TransportSegment& line_segment) {
	world_data.LogicRemove(
		game::Chunk::LogicGroup::transport_line,
		world_coords,
		[&](auto* t_layer) {
			return static_cast<data::TransportLineData*>(t_layer->GetUniqueData())->lineSegment.get() == &line_segment;
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

/// Up, right, down, left
using LineData4Way = data::TransportLineData*[4];


///
/// \brief Applies the necessary adjustments to shift a segment's head forwards or backwards
/// \tparam Forwards true to shift forwards, false to shift backwards
template <bool Forwards>
void ShiftSegmentHead(game::TransportSegment& line_segment) {
	if constexpr (Forwards) {
		line_segment.length++;
		line_segment.itemOffset++;
	}
	else {
		line_segment.length--;
		line_segment.itemOffset--;
	}
}

///
/// \brief Updates the world tiles which references a transport segment, props: line_segment_index, line_segment
/// \param world_coords Beginning tile to update
/// \param line_segment Beginning segment, traveling inverse Orientation line_segment.length tiles, <br>
/// all tiles set to reference this
/// \param offset Offsets segment id numbering, world_coords must be also adjusted to the appropriate offset when calling
void UpdateSegmentTiles(const game::WorldData& world_data,
                        const WorldCoord& world_coords,
                        const std::shared_ptr<game::TransportSegment>& line_segment,
                        const int offset = 0) {
	uint64_t x_offset = 0;
	uint64_t y_offset = 0;

	// Should be -1, 0, 1 depending on orientation
	uint64_t x_change = 0;
	uint64_t y_change = 0;
	OrientationIncrement(line_segment->direction, x_change, y_change, -1);

	// Adjust the segment index number of all following segments 
	for (int i = offset; i < line_segment->length; ++i) {
		auto* i_line_data = data::TransportLine::GetLineData(world_data,
		                                                     world_coords.x + x_offset,
		                                                     world_coords.y + y_offset);
		if (!i_line_data)
			continue;
		i_line_data->lineSegmentIndex = i;
		i_line_data->lineSegment      = line_segment;

		x_offset += x_change;
		y_offset += y_change;
	}
}

///
///	\brief Updates the orientation of current and neighboring transport lines 
void UpdateNeighborOrientation(const game::WorldData& world_data,
                               const WorldCoord& world_coords,
                               LineData4Way line_data_4,
                               data::TransportLineData* center) {
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
	auto* top    = data::TransportLine::GetLineData(world_data, world_coords.x, world_coords.y - 2);
	auto* right  = data::TransportLine::GetLineData(world_data, world_coords.x + 2, world_coords.y);
	auto* bottom = data::TransportLine::GetLineData(world_data, world_coords.x, world_coords.y + 2);
	auto* left   = data::TransportLine::GetLineData(world_data, world_coords.x - 2, world_coords.y);

	auto* t_left  = data::TransportLine::GetLineData(world_data, world_coords.x - 1, world_coords.y - 1);
	auto* t_right = data::TransportLine::GetLineData(world_data, world_coords.x + 1, world_coords.y - 1);

	auto* b_left  = data::TransportLine::GetLineData(world_data, world_coords.x - 1, world_coords.y + 1);
	auto* b_right = data::TransportLine::GetLineData(world_data, world_coords.x + 1, world_coords.y + 1);

	// Top neighbor
	if (line_data_4[0])
		line_data_4[0]->SetOrientation(
			data::TransportLine::GetLineOrientation(
				data::TransportLineData::ToOrientation(line_data_4[0]->orientation),
				top, t_right, center, t_left)
		);
	// Right
	if (line_data_4[1])
		line_data_4[1]->SetOrientation(
			data::TransportLine::GetLineOrientation(data::TransportLineData::ToOrientation(line_data_4[1]->orientation),
			                                        t_right, right, b_right, center)
		);
	// Bottom
	if (line_data_4[2])
		line_data_4[2]->SetOrientation(
			data::TransportLine::GetLineOrientation(
				data::TransportLineData::ToOrientation(line_data_4[2]->orientation),
				center, b_right, bottom, b_left)
		);
	// Left
	if (line_data_4[3])
		line_data_4[3]->SetOrientation(
			data::TransportLine::GetLineOrientation(
				data::TransportLineData::ToOrientation(line_data_4[3]->orientation),
				t_left, center, b_left, left)
		);
}

///
/// \brief Calls func or side_only_func depending on the line_orientation, provides parameters on how neighboring lines
/// should be modified.
/// \tparam IsNeighborUpdate If true, length and itemOffset will also be updated
/// \remark This does not move across logic chunks and may make the position negative
template <bool IsNeighborUpdate>
void UpdateNeighborTermination(game::WorldData& world_data,
                               const WorldCoordAxis world_x,
                               const WorldCoordAxis world_y,
                               const data::TransportLineData::LineOrientation line_orientation) {

	auto bend_update = [](game::WorldData& world_data,
	                      int world_x, int world_y,
	                      const game::TransportSegment::TerminationType termination_type) {
		auto* line_segment = data::TransportLine::GetTransportSegment(world_data, world_x, world_y);
		if (line_segment) {
			if constexpr (!IsNeighborUpdate) {
				ShiftSegmentHead<true>(**line_segment);
			}
			line_segment->get()->terminationType = termination_type;

			UpdateSegmentTiles(world_data, {world_x, world_y}, *line_segment, 1);
		}
	};

	auto side_update = [](game::WorldData& world_data,
	                      int world_x, int world_y,
	                      const data::Orientation direction,
	                      const game::TransportSegment::TerminationType termination_type) {
		auto* line_segment = data::TransportLine::GetTransportSegment(world_data, world_x, world_y);
		if (line_segment) {
			// Segment does not match required direction
			if (line_segment->get()->direction != direction)
				return;

			if constexpr (!IsNeighborUpdate) {
				ShiftSegmentHead<true>(**line_segment);
			}
			line_segment->get()->terminationType = termination_type;

			UpdateSegmentTiles(world_data, {world_x, world_y}, *line_segment, 1);
		}
	};

	switch (line_orientation) {
		// Up
	case data::TransportLineData::LineOrientation::up_left:
		bend_update(world_data, world_x, world_y + 1,
		            game::TransportSegment::TerminationType::bend_left);
		break;
	case data::TransportLineData::LineOrientation::up_right:
		bend_update(world_data, world_x, world_y + 1,
		            game::TransportSegment::TerminationType::bend_right);
		break;

		// Right
	case data::TransportLineData::LineOrientation::right_up:
		bend_update(world_data, world_x - 1, world_y,
		            game::TransportSegment::TerminationType::bend_left);
		break;
	case data::TransportLineData::LineOrientation::right_down:
		bend_update(world_data, world_x - 1, world_y,
		            game::TransportSegment::TerminationType::bend_right);
		break;

		// Down
	case data::TransportLineData::LineOrientation::down_right:
		bend_update(world_data, world_x, world_y - 1,
		            game::TransportSegment::TerminationType::bend_left);
		break;
	case data::TransportLineData::LineOrientation::down_left:
		bend_update(world_data, world_x, world_y - 1,
		            game::TransportSegment::TerminationType::bend_right);
		break;

		// Left
	case data::TransportLineData::LineOrientation::left_down:
		bend_update(world_data, world_x + 1, world_y,
		            game::TransportSegment::TerminationType::bend_left);
		break;
	case data::TransportLineData::LineOrientation::left_up:
		bend_update(world_data, world_x + 1, world_y,
		            game::TransportSegment::TerminationType::bend_right);
		break;

		// Straight (Check for transport lines on both sides to make side only)
	case data::TransportLineData::LineOrientation::up:
		side_update(world_data, world_x - 1, world_y,
		            data::Orientation::right,
		            game::TransportSegment::TerminationType::left_only);
		side_update(world_data, world_x + 1, world_y,
		            data::Orientation::left,
		            game::TransportSegment::TerminationType::right_only);
		break;
	case data::TransportLineData::LineOrientation::right:
		side_update(world_data, world_x, world_y - 1,
		            data::Orientation::down,
		            game::TransportSegment::TerminationType::left_only);
		side_update(world_data, world_x, world_y + 1,
		            data::Orientation::up,
		            game::TransportSegment::TerminationType::right_only);
		break;
	case data::TransportLineData::LineOrientation::down:
		side_update(world_data, world_x - 1, world_y,
		            data::Orientation::right,
		            game::TransportSegment::TerminationType::right_only);
		side_update(world_data, world_x + 1, world_y,
		            data::Orientation::left,
		            game::TransportSegment::TerminationType::left_only);
		break;
	case data::TransportLineData::LineOrientation::left:
		side_update(world_data, world_x, world_y - 1,
		            data::Orientation::down,
		            game::TransportSegment::TerminationType::right_only);
		side_update(world_data, world_x, world_y + 1,
		            data::Orientation::up,
		            game::TransportSegment::TerminationType::left_only);
		break;

	default:
		assert(false);  // Missing switch case
		break;

	}
}

// ======================================================================
// Build

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

	auto& origin_segment = *origin_data.lineSegment;

	data::TransportLineData* neighbor_data =
		data::TransportLine::GetLineData(world_data, neighbor_world_x, neighbor_world_y);
	if (neighbor_data) {
		game::TransportSegment& neighbor_segment = *neighbor_data->lineSegment;

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
/// \brief If a transport segment exists at world_x, world_y and has a terminal type == Required,
/// it will be changed to New
template <game::TransportSegment::TerminationType Required,
          game::TransportSegment::TerminationType New>
void TryChangeTerminationType(const game::WorldData& world_data,
                              const int32_t world_x, const int32_t world_y) {
	data::TransportLineData* line_data = data::TransportLine::GetLineData(world_data, world_x, world_y);
	if (line_data) {
		if (line_data->lineSegment->terminationType == Required) {
			line_data->lineSegment->terminationType = New;
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
                        const WorldCoord& world_coords,
                        const std::shared_ptr<game::TransportSegment>& line_segment) {
	using namespace jactorio;

	if (!line_data)
		return;

	switch (line_data->orientation) {
	case BendLeft:
		line_segment->terminationType = game::TransportSegment::TerminationType::bend_left;
		goto shift_segment;

	case BendRight:
		line_segment->terminationType = game::TransportSegment::TerminationType::bend_right;
		goto shift_segment;

	case LeftOnly:
		line_segment->terminationType = game::TransportSegment::TerminationType::left_only;

		// Change segment opposite to current one, which may also point to the same target segment
#define LEFT_ONLY_CHANGE_TERMINATION_TYPE\
			TryChangeTerminationType<game::TransportSegment::TerminationType::bend_right,\
			                         game::TransportSegment::TerminationType::right_only>(

		if constexpr (Orientation == data::Orientation::up) {
			LEFT_ONLY_CHANGE_TERMINATION_TYPE
				world_data, world_coords.x, world_coords.y - 2);
		}
		else if constexpr (Orientation == data::Orientation::right) {
			LEFT_ONLY_CHANGE_TERMINATION_TYPE
				world_data, world_coords.x + 2, world_coords.y);
		}
		else if constexpr (Orientation == data::Orientation::down) {
			LEFT_ONLY_CHANGE_TERMINATION_TYPE
				world_data, world_coords.x, world_coords.y + 2);
		}
		else if constexpr (Orientation == data::Orientation::left) {
			LEFT_ONLY_CHANGE_TERMINATION_TYPE
				world_data, world_coords.x - 2, world_coords.y);
		}

#undef LEFT_ONLY_CHANGE_TERMINATION_TYPE
		goto shift_segment;

	case RightOnly:
		line_segment->terminationType = game::TransportSegment::TerminationType::right_only;

#define RIGHT_ONLY_CHANGE_TERMINATION_TYPE\
			TryChangeTerminationType<game::TransportSegment::TerminationType::bend_left,\
			                         game::TransportSegment::TerminationType::left_only>(

		// Check 2 units up and see if there is segment bending left, if so change it to straight
		if constexpr (Orientation == data::Orientation::up) {
			RIGHT_ONLY_CHANGE_TERMINATION_TYPE
				world_data, world_coords.x, world_coords.y - 2);
		}
		else if constexpr (Orientation == data::Orientation::right) {
			RIGHT_ONLY_CHANGE_TERMINATION_TYPE
				world_data, world_coords.x + 2, world_coords.y);
		}
		else if constexpr (Orientation == data::Orientation::down) {
			RIGHT_ONLY_CHANGE_TERMINATION_TYPE
				world_data, world_coords.x, world_coords.y + 2);
		}
		else if constexpr (Orientation == data::Orientation::left) {
			RIGHT_ONLY_CHANGE_TERMINATION_TYPE
				world_data, world_coords.x - 2, world_coords.y);
		}

#undef RIGHT_ONLY_CHANGE_TERMINATION_TYPE


	shift_segment:
		ShiftSegmentHead<true>(*line_segment);
		UpdateSegmentTiles(world_data, world_coords, line_segment, 1);
		break;

	default:
		break;
	}
}

///
/// \brief Change current line segment termination type to a bend depending on neighbor termination orientation
void UpdateTermination(game::WorldData& world_data,
                       const WorldCoord& world_coords,
                       LineData4Way& line_data,
                       const std::shared_ptr<game::TransportSegment>& line_segment) {
	switch (line_segment->direction) {

	case data::Orientation::up:
		UpdateSegmentProps<data::Orientation::up,
		                   data::TransportLineData::LineOrientation::up_left,
		                   data::TransportLineData::LineOrientation::up_right,
		                   data::TransportLineData::LineOrientation::right,
		                   data::TransportLineData::LineOrientation::left>(line_data[0],
		                                                                   world_data, world_coords, line_segment);
		break;
	case data::Orientation::right:
		UpdateSegmentProps<data::Orientation::right,
		                   data::TransportLineData::LineOrientation::right_up,
		                   data::TransportLineData::LineOrientation::right_down,
		                   data::TransportLineData::LineOrientation::down,
		                   data::TransportLineData::LineOrientation::up>(line_data[1],
		                                                                 world_data, world_coords, line_segment);
		break;
	case data::Orientation::down:
		UpdateSegmentProps<data::Orientation::down,
		                   data::TransportLineData::LineOrientation::down_right,
		                   data::TransportLineData::LineOrientation::down_left,
		                   data::TransportLineData::LineOrientation::left,
		                   data::TransportLineData::LineOrientation::right>(line_data[2],
		                                                                    world_data, world_coords, line_segment);
		break;
	case data::Orientation::left:
		UpdateSegmentProps<data::Orientation::left,
		                   data::TransportLineData::LineOrientation::left_down,
		                   data::TransportLineData::LineOrientation::left_up,
		                   data::TransportLineData::LineOrientation::up,
		                   data::TransportLineData::LineOrientation::down>(line_data[3],
		                                                                   world_data, world_coords, line_segment);
		break;

	default:
		assert(false);  // Missing switch case
	}
}

///
/// \brief Initializes line data and groups transport segments
/// Sets the transport segment grouped / newly created with in tile_layer and returns it
/// \return Created data for at tile_layer, was a new transport segment created
data::TransportLineData& InitTransportSegment(game::WorldData& world_data,
                                              const WorldCoord& world_coords,
                                              const data::Orientation orientation,
                                              game::ChunkTileLayer& tile_layer,
                                              LineData4Way& line_data) {
	/*
	 * Transport line grouping rules:
	 *
	 * < < < [1, 2, 3] - Direction [order];
	 * Line ahead:
	 *		- Extends length of transport line segment
	 *
	 * < < < [3, 2, 1]
	 * Line behind:
	 *		- Moves head of transport segment, shift leading item 1 tile back
	 *		
	 * < < < [1, 3, 2]
	 * Line ahead and behind:
	 *		- Behaves as line ahead
	 */

	static_assert(static_cast<int>(data::Orientation::left) == 3);  // Indexing line_data will be out of range 

	auto& origin_chunk = *world_data.GetChunkW(world_coords);

	std::shared_ptr<game::TransportSegment> line_segment;
	int line_segment_index = 0;

	enum class InitSegmentStatus
	{
		new_segment,
		group_ahead, // Segment ahead of current location
		group_behind // Segment behind current location
	} status;
	const auto index  = static_cast<int>(orientation);
	const int i_index = data::InvertOrientation(index);

	if (!line_data[index] ||
		line_data[index]->lineSegment->direction != orientation) {

		status = InitSegmentStatus::new_segment;  // If failed to group with ahead, this is chosen

		// Failed to group with ahead, try to group with segment behind
		if (line_data[i_index] &&
			line_data[i_index]->lineSegment->direction == orientation) {
			// Group behind

			WorldCoord behind_coords = world_coords;
			OrientationIncrement(orientation, behind_coords.x, behind_coords.y, -1);

			// Within the same logic chunk = Can group behind
			status = InitSegmentStatus::group_behind;
			if (&origin_chunk != world_data.GetChunkW(behind_coords)) {
				// Different memory addresses = different logic chunks
				status = InitSegmentStatus::new_segment;
			}
		}
	}
	else {
		// Group ahead

		WorldCoord ahead_coords = world_coords;
		OrientationIncrement(orientation, ahead_coords.x, ahead_coords.y, 1);

		status = InitSegmentStatus::group_ahead;
		if (&origin_chunk != world_data.GetChunkW(ahead_coords)) {
			// Different memory addresses = different logic chunks
			status = InitSegmentStatus::new_segment;
		}
	}

	// ======================================================================

	switch (status) {
	case InitSegmentStatus::new_segment:
		line_segment = std::make_shared<game::TransportSegment>(
			orientation,
			game::TransportSegment::TerminationType::straight,
			1
		);

		world_data.LogicRegister(game::Chunk::LogicGroup::transport_line,
		                         world_coords,
		                         game::ChunkTile::ChunkLayer::entity);
		break;

	case InitSegmentStatus::group_behind:
		// The transport segment's position is adjusted by init_transport_segment
		// Move the segment head behind forwards to current position
		line_segment = line_data[i_index]->lineSegment;
		ShiftSegmentHead<true>(*line_segment);
		break;

	case InitSegmentStatus::group_ahead:
		line_segment = line_data[index]->lineSegment;

		line_data[index]->lineSegment->length++;  // Lengthening its tail, not head
		line_segment_index = line_data[index]->lineSegmentIndex + 1;
		break;

	default:
		assert(false);
		break;
	}

	// Create unique data at tile
	auto* unique_data             = tile_layer.MakeUniqueData<data::TransportLineData>(line_segment);
	unique_data->lineSegmentIndex = line_segment_index;

	// Line data is not initialized yet inside switch
	if (status == InitSegmentStatus::group_behind) {
		// Remove old head from logic group, add new head which is now 1 tile ahead
		RemoveFromLogic(world_data, world_coords, *line_segment);
		world_data.LogicRegister(game::Chunk::LogicGroup::transport_line,
		                         world_coords,
		                         game::ChunkTile::ChunkLayer::entity);

		// Renumber
		UpdateSegmentTiles(world_data, world_coords, line_segment);
	}

	return *unique_data;
}

void data::TransportLine::OnBuild(game::WorldData& world_data,
                                  game::LogicData& logic_data,
                                  const WorldCoord& world_coords,
                                  game::ChunkTileLayer& tile_layer, const Orientation orientation) const {
	auto* up    = GetLineData(world_data, world_coords.x, world_coords.y - 1);
	auto* right = GetLineData(world_data, world_coords.x + 1, world_coords.y);
	auto* down  = GetLineData(world_data, world_coords.x, world_coords.y + 1);
	auto* left  = GetLineData(world_data, world_coords.x - 1, world_coords.y);
	TransportLineData* line_data_4[4]{up, right, down, left};

	// ======================================================================
	// Create data
	TransportLineData& line_data = InitTransportSegment(world_data, world_coords, orientation, tile_layer, line_data_4);
	const std::shared_ptr<game::TransportSegment>& line_segment = line_data.lineSegment;

	const TransportLineData::LineOrientation line_orientation = GetLineOrientation(orientation, up, right, down, left);

	line_data.set         = static_cast<uint16_t>(line_orientation);
	line_data.orientation = line_orientation;


	// ======================================================================
	// Update neighbor rendering orientation
	// This has to be done PRIOR to adjusting for bends

	// Take the 4 transport lines neighboring the center as parameters to avoid recalculating them
	UpdateNeighborOrientation(
		world_data, world_coords,
		line_data_4,
		static_cast<TransportLineData*>(tile_layer.GetUniqueData()));

	UpdateTermination(world_data, world_coords, line_data_4, line_segment);

	// Updates the termination type and length of neighboring lines
	UpdateNeighborTermination<false>(world_data,
	                                 world_coords.x, world_coords.y,
	                                 line_orientation);

	// ======================================================================
	// Set the target_segment to the neighbor it is pointing to, or the neighbor's target segment which is pointing to this
	if (up)
		UpdateLineTargets<Orientation::up, Orientation::down>(
			world_data, line_data,
			orientation,
			world_coords.x, world_coords.y - 1);
	if (right)
		UpdateLineTargets<Orientation::right, Orientation::left>(
			world_data, line_data,
			orientation,
			world_coords.x + 1, world_coords.y);
	if (down)
		UpdateLineTargets<Orientation::down, Orientation::up>(
			world_data, line_data,
			orientation,
			world_coords.x, world_coords.y + 1
		);
	if (left)
		UpdateLineTargets<Orientation::left, Orientation::right>(
			world_data, line_data,
			orientation,
			world_coords.x - 1, world_coords.y
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
                                           game::LogicData& logic_data,
                                           const WorldCoord& /*emit_world_coords*/,
                                           const WorldCoord& receive_world_coords,
                                           Orientation /*emit_orientation*/) const {
	// Run stuff here that on_build and on_remove both calls

	auto* line_data = GetLineData(world_data, receive_world_coords.x, receive_world_coords.y);
	if (!line_data)  // Transport line does not exist here
		return;

	// ======================================================================

	// Reset segment lane item index to 0, since the head items MAY now have somewhere to go
	line_data->lineSegment->left.index  = 0;
	line_data->lineSegment->right.index = 0;

	UpdateNeighborTermination<true>(world_data,
	                                receive_world_coords.x, receive_world_coords.y,
	                                line_data->orientation);
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

///
/// \brief Updates neighboring segments after transport line is removed 
/// \param world_coords Coords of transport line removed
/// \param line_data Neighboring line segment
/// \param target Removed line segment
void DisconnectTargetSegment(game::WorldData& world_data,
                             const WorldCoord& world_coords,
                             data::TransportLineData* target,
                             data::TransportLineData* line_data) {

	if (line_data &&
		line_data->lineSegment->targetSegment == target->lineSegment.get()) {

		const std::shared_ptr<game::TransportSegment>& line_segment = line_data->lineSegment;
		line_segment->targetSegment                                 = nullptr;

		WorldCoord neighbor_world_coords = world_coords;

		// If bends / side only, set to straight & decrement length	
		switch (line_segment->terminationType) {
		case game::TransportSegment::TerminationType::bend_left:
		case game::TransportSegment::TerminationType::bend_right:
		case game::TransportSegment::TerminationType::right_only:
		case game::TransportSegment::TerminationType::left_only:
			ShiftSegmentHead<false>(*line_segment);
			line_segment->terminationType = game::TransportSegment::TerminationType::straight;

			// Move the neighboring line segments back if they are not straight
			OrientationIncrement(line_segment->direction,
			                     neighbor_world_coords.x, neighbor_world_coords.y, -1.f);

			// Renumber from index 0
			UpdateSegmentTiles(world_data, neighbor_world_coords, line_segment);
			break;

		default:
			// Does not bend
			break;
		}
	}
}

double ToChunkOffset(const WorldCoordAxis world_coord) {
	return fabs(game::WorldData::ToChunkCoord(world_coord) * game::Chunk::kChunkWidth - world_coord);
}

void data::TransportLine::OnRemove(game::WorldData& world_data,
                                   game::LogicData& logic_data,
                                   const WorldCoord& world_coords, game::ChunkTileLayer& tile_layer) const {
	auto* up    = GetLineData(world_data, world_coords.x, world_coords.y - 1);
	auto* right = GetLineData(world_data, world_coords.x + 1, world_coords.y);
	auto* down  = GetLineData(world_data, world_coords.x, world_coords.y + 1);
	auto* left  = GetLineData(world_data, world_coords.x - 1, world_coords.y);
	TransportLineData* line_data_4[4]{up, right, down, left};

	UpdateNeighborOrientation(world_data, world_coords, line_data_4, nullptr);

	auto* line_data = static_cast<TransportLineData*>(tile_layer.GetUniqueData());

	// Set neighboring transport line segments which points to this segment's target_segment to nullptr
	DisconnectTargetSegment(world_data, world_coords, line_data, up);
	DisconnectTargetSegment(world_data, world_coords, line_data, right);
	DisconnectTargetSegment(world_data, world_coords, line_data, down);
	DisconnectTargetSegment(world_data, world_coords, line_data, left);

	auto& chunk = *world_data.GetChunkW(world_coords);

	// o_ = old
	// n_ = new

	auto& o_line_data          = *static_cast<TransportLineData*>(tile_layer.GetUniqueData());
	const auto& o_line_segment = o_line_data.lineSegment;

	auto n_seg_coords = world_coords;

	OrientationIncrement(o_line_segment->direction,
	                     n_seg_coords.x, n_seg_coords.y, -1);

	// Create new segment at behind cords if not the end of a segment
	const auto n_seg_length = o_line_segment->length - o_line_data.lineSegmentIndex - 1;
	if (n_seg_length > 0) {
		// Create new segment
		const auto n_segment = std::make_shared<game::TransportSegment>(o_line_segment->direction,
		                                                                game::TransportSegment::TerminationType::straight,
		                                                                n_seg_length);
		n_segment->itemOffset = o_line_segment->itemOffset - o_line_data.lineSegmentIndex - 1;


		// Add to be considered for logic updates
		chunk.GetLogicGroup(game::Chunk::LogicGroup::transport_line).emplace_back(
			&world_data.GetTile(n_seg_coords)->GetLayer(game::ChunkTile::ChunkLayer::entity)
		);

		// ======================================================================

		// Update trailing segments to use new segment and renumber
		UpdateSegmentTiles(world_data, n_seg_coords, n_segment);

		// Update other segments leading into old segment
		// TODO improve this algorithm for updating target segments
		for (int i = 0; i < game::Chunk::kChunkArea; ++i) {
			auto& layer = chunk.Tiles()[i].GetLayer(game::ChunkTile::ChunkLayer::entity);
			if (!layer.prototypeData || layer.prototypeData->Category() != DataCategory::transport_belt)
				continue;

			const auto& line_segment = layer.GetUniqueData<TransportLineData>()->lineSegment.get();


			const auto position_x = i % game::Chunk::kChunkWidth;
			const auto position_y = i / game::Chunk::kChunkWidth;
			bool valid_neighbor   = false;  // Neighbor must be BEHIND the segment which was removed
			switch (o_line_segment->direction) {
			case Orientation::up:
				valid_neighbor = position_y > ToChunkOffset(world_coords.y);
				break;
			case Orientation::right:
				valid_neighbor = position_x < ToChunkOffset(world_coords.x);
				break;
			case Orientation::down:
				valid_neighbor = position_y < ToChunkOffset(world_coords.y);
				break;
			case Orientation::left:
				valid_neighbor = position_x > ToChunkOffset(world_coords.x);
				break;

			default:
				assert(false);
			}

			if (valid_neighbor && line_segment->targetSegment == o_line_segment.get()) {
				line_segment->targetSegment = n_segment.get();
			}
		}


		// Update segment in neighboring logic chunk leading into old_segment
		ChunkCoord neighbor_chunk_coords = chunk.GetPosition();
		OrientationIncrement(o_line_segment->direction,
		                     neighbor_chunk_coords.x, neighbor_chunk_coords.y, -1);

		auto* neighbor_l_chunk = world_data.GetChunkC(neighbor_chunk_coords);
		if (neighbor_l_chunk) {
			for (auto& t_line : neighbor_l_chunk->GetLogicGroup(game::Chunk::LogicGroup::transport_line)) {
				const auto& i_segment = t_line->GetUniqueData<TransportLineData>()->lineSegment;

				if (i_segment->targetSegment == o_line_segment.get()) {
					i_segment->targetSegment = n_segment.get();
				}
			}
		}
	}

	// Remove original transport line segment referenced in Transport_line_data if is head of segment
	// If not head, reduce the length of original segment to index + 1
	if (o_line_data.lineSegmentIndex == 0 ||
		(o_line_data.lineSegmentIndex == 1 && o_line_segment->terminationType !=  // Head of bending segments start at 1
			game::TransportSegment::TerminationType::straight)) {

		RemoveFromLogic(world_data, world_coords, *o_line_segment);
	}
	else {
		o_line_segment->length = o_line_data.lineSegmentIndex;
	}
}
