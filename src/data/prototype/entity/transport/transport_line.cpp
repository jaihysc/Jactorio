// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#include "data/prototype/entity/transport/transport_line.h"

#include "data/data_manager.h"
#include "game/logic/transport_line_structure.h"

std::pair<uint16_t, uint16_t> jactorio::data::Transport_line::map_placement_orientation(
	const Orientation orientation,
	game::World_data& world_data,
	const game::World_data::world_pair& world_coords) const {

	auto* t_center = get_line_data(world_data, world_coords.first, world_coords.second - 1);
	auto* c_left   = get_line_data(world_data, world_coords.first - 1, world_coords.second);
	auto* c_right  = get_line_data(world_data, world_coords.first + 1, world_coords.second);
	auto* b_center = get_line_data(world_data, world_coords.first, world_coords.second + 1);

	return {static_cast<uint16_t>(get_line_orientation(orientation, t_center, c_right, b_center, c_left)), 0};
}

// ======================================================================
// Data access

///
/// \return True if Line exists and points in direction 
#define NEIGHBOR_VALID(transport_line, direction)\
		((transport_line) && Transport_line_data::to_orientation((transport_line)->orientation) == Orientation::direction)

///
/// \brief Determines line orientation given placement orientation and neighbors
jactorio::data::Transport_line_data::LineOrientation jactorio::data::Transport_line::get_line_orientation(
	const Orientation orientation,
	Transport_line_data* up,
	Transport_line_data* right,
	Transport_line_data* down,
	Transport_line_data* left) {
	// Determine if there is a transport line neighboring the current one and a bend should be placed
	// The line above is valid for a bend since it faces towards the current tile

	switch (orientation) {
	case Orientation::up:
		if (!NEIGHBOR_VALID(down, up) && NEIGHBOR_VALID(left, right) != NEIGHBOR_VALID(right, left)) {
			if (NEIGHBOR_VALID(left, right))
				return Transport_line_data::LineOrientation::right_up;
			return Transport_line_data::LineOrientation::left_up;
		}
		return Transport_line_data::LineOrientation::up;

	case Orientation::right:
		if (!NEIGHBOR_VALID(left, right) && NEIGHBOR_VALID(up, down) != NEIGHBOR_VALID(down, up)) {
			if (NEIGHBOR_VALID(up, down))
				return Transport_line_data::LineOrientation::down_right;
			return Transport_line_data::LineOrientation::up_right;
		}
		return Transport_line_data::LineOrientation::right;

	case Orientation::down:
		if (!NEIGHBOR_VALID(up, down) && NEIGHBOR_VALID(left, right) != NEIGHBOR_VALID(right, left)) {
			if (NEIGHBOR_VALID(left, right))
				return Transport_line_data::LineOrientation::right_down;
			return Transport_line_data::LineOrientation::left_down;
		}
		return Transport_line_data::LineOrientation::down;

	case Orientation::left:
		if (!NEIGHBOR_VALID(right, left) && NEIGHBOR_VALID(up, down) != NEIGHBOR_VALID(down, up)) {
			if (NEIGHBOR_VALID(up, down))
				return Transport_line_data::LineOrientation::down_left;
			return Transport_line_data::LineOrientation::up_left;
		}
		return Transport_line_data::LineOrientation::left;

	default:
		assert(false); // Missing switch case
	}

	return Transport_line_data::LineOrientation::up;
}

#undef NEIGHBOR_VALID


jactorio::data::Transport_line_data* jactorio::data::Transport_line::get_line_data(const game::World_data& world_data,
                                                                                   const game::World_data::world_coord world_x,
                                                                                   const game::World_data::world_coord world_y) {
	auto* tile = world_data.get_tile(world_x, world_y);
	if (!tile)  // No tile exists
		return nullptr;

	auto& layer = tile->get_layer(game::Chunk_tile::chunkLayer::entity);

	if (!dynamic_cast<const Transport_line*>(  // Not an instance of transport line
		layer.prototype_data))
		return nullptr;

	return static_cast<Transport_line_data*>(layer.unique_data);
}

void jactorio::data::Transport_line::get_line_struct_layer(game::World_data& world_data,
                                                                     const game::World_data::world_coord world_x,
                                                                     const game::World_data::world_coord world_y,
                                                                     const std::function<void(game::Chunk_struct_layer&)>&
                                                                     callback) {
	Transport_line_data* line_data =
		get_line_data(world_data, world_x, world_y);
	if (!line_data)  // Transport lines does not exist
		return;

	std::vector<game::Chunk_struct_layer>* transport_structures;
	try {
		transport_structures =
			&world_data.logic_get_chunk(world_data.get_chunk(world_x, world_y))->
			            get_struct(game::Logic_chunk::structLayer::transport_line);

	}
	catch (std::out_of_range&) {
		// Failed to find key (Logic chunk does not exist)
		return;
	}


	// Search for the transport line in the chunk struct layer where the
	// unique_data(Transport_line_data) == one referenced by line_data
	for (auto& layer : *transport_structures) {
		if (layer.unique_data == &line_data->line_segment)
			callback(layer);
	}
}

// ======================================================================
// Build / Remove / Neighbor update

void jactorio::data::Transport_line::update_neighboring_orientation(const game::World_data& world_data,
                                                                    const game::World_data::world_pair& world_coords,
                                                                    Transport_line_data* t_center,
                                                                    Transport_line_data* c_right,
                                                                    Transport_line_data* b_center,
                                                                    Transport_line_data* c_left,
                                                                    Transport_line_data* center) {
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
	auto* top    = get_line_data(world_data, world_coords.first, world_coords.second - 2);
	auto* right  = get_line_data(world_data, world_coords.first + 2, world_coords.second);
	auto* bottom = get_line_data(world_data, world_coords.first, world_coords.second + 2);
	auto* left   = get_line_data(world_data, world_coords.first - 2, world_coords.second);

	auto* t_left  = get_line_data(world_data, world_coords.first - 1, world_coords.second - 1);
	auto* t_right = get_line_data(world_data, world_coords.first + 1, world_coords.second - 1);

	auto* b_left  = get_line_data(world_data, world_coords.first - 1, world_coords.second + 1);
	auto* b_right = get_line_data(world_data, world_coords.first + 1, world_coords.second + 1);

	// Top neighbor
	if (t_center)
		t_center->set_orientation(
			get_line_orientation(Transport_line_data::to_orientation(t_center->orientation), top, t_right, center, t_left));
	// Right
	if (c_right)
		c_right->set_orientation(
			get_line_orientation(Transport_line_data::to_orientation(c_right->orientation), t_right, right, b_right, center));
	// Bottom
	if (b_center)
		b_center->set_orientation(
			get_line_orientation(Transport_line_data::to_orientation(b_center->orientation), center, b_right, bottom, b_left));
	// Left
	if (c_left)
		c_left->set_orientation(
			get_line_orientation(Transport_line_data::to_orientation(c_left->orientation), t_left, center, b_left, left));
}

void jactorio::data::Transport_line::update_neighboring_transport_segment(
	game::World_data& world_data,
	const int32_t world_x, const int32_t world_y,
	const Transport_line_data::LineOrientation line_orientation,
	const update_segment_func& func,
	const update_segment_side_only_func& side_only_func) {

	switch (line_orientation) {
		// Up
	case Transport_line_data::LineOrientation::up_left:
		func(world_data, world_x, world_y + 1,
		     0, -1,
		     game::Transport_line_segment::TerminationType::bend_left);
		break;
	case Transport_line_data::LineOrientation::up_right:
		func(world_data, world_x, world_y + 1,
		     0, -1,
		     game::Transport_line_segment::TerminationType::bend_right);
		break;

		// Right
	case Transport_line_data::LineOrientation::right_up:
		func(world_data, world_x - 1, world_y,
		     1, 0,
		     game::Transport_line_segment::TerminationType::bend_left);
		break;
	case Transport_line_data::LineOrientation::right_down:
		func(world_data, world_x - 1, world_y,
		     1, 0,
		     game::Transport_line_segment::TerminationType::bend_right);
		break;

		// Down
	case Transport_line_data::LineOrientation::down_right:
		func(world_data, world_x, world_y - 1,
		     0, 1,
		     game::Transport_line_segment::TerminationType::bend_left);
		break;
	case Transport_line_data::LineOrientation::down_left:
		func(world_data, world_x, world_y - 1,
		     0, 1,
		     game::Transport_line_segment::TerminationType::bend_right);
		break;

		// Left
	case Transport_line_data::LineOrientation::left_down:
		func(world_data, world_x + 1, world_y,
		     -1, 0,
		     game::Transport_line_segment::TerminationType::bend_left);
		break;
	case Transport_line_data::LineOrientation::left_up:
		func(world_data, world_x + 1, world_y,
		     -1, 0,
		     game::Transport_line_segment::TerminationType::bend_right);
		break;

		// Straight (Check for transport lines on both sides to make side only)
	case Transport_line_data::LineOrientation::up:
		side_only_func(world_data, world_x - 1, world_y,
		               1, 0,
		               Orientation::right,
		               game::Transport_line_segment::TerminationType::left_only);
		side_only_func(world_data, world_x + 1, world_y,
		               -1, 0,
		               Orientation::left,
		               game::Transport_line_segment::TerminationType::right_only);
		break;
	case Transport_line_data::LineOrientation::right:
		side_only_func(world_data, world_x, world_y - 1,
		               0, 1,
		               Orientation::down,
		               game::Transport_line_segment::TerminationType::left_only);
		side_only_func(world_data, world_x, world_y + 1,
		               0, -1,
		               Orientation::up,
		               game::Transport_line_segment::TerminationType::right_only);
		break;
	case Transport_line_data::LineOrientation::down:
		side_only_func(world_data, world_x - 1, world_y,
		               1, 0,
		               Orientation::right,
		               game::Transport_line_segment::TerminationType::right_only);
		side_only_func(world_data, world_x + 1, world_y,
		               -1, 0,
		               Orientation::left,
		               game::Transport_line_segment::TerminationType::left_only);
		break;
	case Transport_line_data::LineOrientation::left:
		side_only_func(world_data, world_x, world_y - 1,
		               0, 1,
		               Orientation::down,
		               game::Transport_line_segment::TerminationType::right_only);
		side_only_func(world_data, world_x, world_y + 1,
		               0, -1,
		               Orientation::up,
		               game::Transport_line_segment::TerminationType::left_only);
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
void try_change_termination_type(jactorio::game::World_data& world_data,
                                 const int32_t world_x, const int32_t world_y,
                                 const jactorio::game::Transport_line_segment::TerminationType termination_type,
                                 const jactorio::game::Transport_line_segment::TerminationType new_termination_type) {
	using namespace jactorio;

	data::Transport_line_data* line_data = data::Transport_line::get_line_data(world_data, world_x, world_y);
	if (line_data) {
		if (line_data->line_segment.termination_type == termination_type) {
			line_data->line_segment.termination_type = new_termination_type;
		}
	}
}

///
/// \brief Determines the member target_segment of Transport_line_segment
/// \param orientation Current orientation
/// \param origin_connect_orientation Orientation required for origin_segment to connect to neighbor segment
/// \param target_connect_orientation Orientation required for neighbor segment to connect to origin segment
void update_line_targets(jactorio::game::World_data& world_data,
                         jactorio::game::Transport_line_segment* origin_segment,
                         const jactorio::data::Orientation orientation, const int32_t neighbor_world_x,
                         const int32_t neighbor_world_y,
                         const jactorio::data::Orientation origin_connect_orientation,
                         const jactorio::data::Orientation target_connect_orientation) {
	using namespace jactorio;

	data::Transport_line_data* neighbor_line_data = data::Transport_line::get_line_data(
		world_data, neighbor_world_x, neighbor_world_y);
	if (neighbor_line_data) {
		game::Transport_line_segment& neighbor_segment = neighbor_line_data->line_segment;

		const bool origin_valid   = orientation == origin_connect_orientation;
		const bool neighbor_valid = neighbor_segment.direction == target_connect_orientation;

		// Only 1 can be valid at a time
		if (origin_valid == neighbor_valid)
			return;

		if (origin_valid)
			origin_segment->target_segment = &neighbor_segment;
		else
			neighbor_segment.target_segment = origin_segment;
	}
}


void jactorio::data::Transport_line::update_termination_type(game::World_data& world_data,
                                                             const game::World_data::world_pair& world_coords,
                                                             const Orientation orientation,
                                                             Transport_line_data* up, Transport_line_data* right,
                                                             Transport_line_data* down, Transport_line_data* left,
                                                             game::Transport_line_segment* line_segment,
                                                             int32_t& line_segment_world_x, int32_t& line_segment_world_y) {
	switch (orientation) {

#define TL_DOWNSHIFT\
	line_segment_world_y--;\
	line_segment->segment_length++

	case Orientation::up:
		if (up) {
			switch (up->orientation) {
			case Transport_line_data::LineOrientation::up_left:
				line_segment->termination_type = game::Transport_line_segment::TerminationType::bend_left;
				TL_DOWNSHIFT;
				break;
			case Transport_line_data::LineOrientation::up_right:
				line_segment->termination_type = game::Transport_line_segment::TerminationType::bend_right;
				TL_DOWNSHIFT;
				break;

			case Transport_line_data::LineOrientation::right:
				line_segment->termination_type = game::Transport_line_segment::TerminationType::right_only;
				TL_DOWNSHIFT;

				// Check 2 units up and see if there is segment bending left, if so change it to straight
				try_change_termination_type(world_data,
				                            world_coords.first, world_coords.second - 2,
				                            game::Transport_line_segment::TerminationType::bend_left,
				                            game::Transport_line_segment::TerminationType::left_only);
				break;
			case Transport_line_data::LineOrientation::left:
				line_segment->termination_type = game::Transport_line_segment::TerminationType::left_only;
				TL_DOWNSHIFT;
				try_change_termination_type(world_data,
				                            world_coords.first, world_coords.second - 2,
				                            game::Transport_line_segment::TerminationType::bend_right,
				                            game::Transport_line_segment::TerminationType::right_only);
				break;

			default:
				break;
			}
		}
		break;
#undef TL_DOWNSHIFT
#define TL_RIGHTSHIFT\
	line_segment_world_x++;\
	line_segment->segment_length++

	case Orientation::right:
		if (right) {
			switch (right->orientation) {
			case Transport_line_data::LineOrientation::right_up:
				line_segment->termination_type = game::Transport_line_segment::TerminationType::bend_left;
				TL_RIGHTSHIFT;
				break;
			case Transport_line_data::LineOrientation::right_down:
				line_segment->termination_type = game::Transport_line_segment::TerminationType::bend_right;
				TL_RIGHTSHIFT;
				break;

			case Transport_line_data::LineOrientation::down:
				line_segment->termination_type = game::Transport_line_segment::TerminationType::right_only;
				TL_RIGHTSHIFT;
				try_change_termination_type(world_data, world_coords.first + 2, world_coords.second,
				                            game::Transport_line_segment::TerminationType::bend_left,
				                            game::Transport_line_segment::TerminationType::left_only);
				break;
			case Transport_line_data::LineOrientation::up:
				line_segment->termination_type = game::Transport_line_segment::TerminationType::left_only;
				TL_RIGHTSHIFT;
				try_change_termination_type(world_data, world_coords.first + 2, world_coords.second,
				                            game::Transport_line_segment::TerminationType::bend_right,
				                            game::Transport_line_segment::TerminationType::right_only);
				break;

			default:
				break;
			}
		}
		break;
#undef TL_RIGHTSHIFT
#define TL_UPSHIFT\
	line_segment_world_y++;\
	line_segment->segment_length++

	case Orientation::down:
		if (down) {
			switch (down->orientation) {
			case Transport_line_data::LineOrientation::down_right:
				line_segment->termination_type = game::Transport_line_segment::TerminationType::bend_left;
				TL_UPSHIFT;
				break;
			case Transport_line_data::LineOrientation::down_left:
				line_segment->termination_type = game::Transport_line_segment::TerminationType::bend_right;
				TL_UPSHIFT;
				break;

			case Transport_line_data::LineOrientation::left:
				line_segment->termination_type = game::Transport_line_segment::TerminationType::right_only;
				TL_UPSHIFT;
				try_change_termination_type(world_data, world_coords.first, world_coords.second + 2,
				                            game::Transport_line_segment::TerminationType::bend_left,
				                            game::Transport_line_segment::TerminationType::left_only);
				break;
			case Transport_line_data::LineOrientation::right:
				line_segment->termination_type = game::Transport_line_segment::TerminationType::left_only;
				TL_UPSHIFT;
				try_change_termination_type(world_data, world_coords.first, world_coords.second + 2,
				                            game::Transport_line_segment::TerminationType::bend_right,
				                            game::Transport_line_segment::TerminationType::right_only);
				break;

			default:
				break;
			}
		}
		break;
#undef TL_UPSHIFT
#define TL_LEFTSHIFT\
	line_segment_world_x--;\
	line_segment->segment_length++

	case Orientation::left:
		if (left) {
			switch (left->orientation) {
			case Transport_line_data::LineOrientation::left_down:
				line_segment->termination_type = game::Transport_line_segment::TerminationType::bend_left;
				TL_LEFTSHIFT;
				break;
			case Transport_line_data::LineOrientation::left_up:
				line_segment->termination_type = game::Transport_line_segment::TerminationType::bend_right;
				TL_LEFTSHIFT;
				break;

			case Transport_line_data::LineOrientation::up:
				line_segment->termination_type = game::Transport_line_segment::TerminationType::right_only;
				TL_LEFTSHIFT;
				try_change_termination_type(world_data, world_coords.first - 2, world_coords.second,
				                            game::Transport_line_segment::TerminationType::bend_left,
				                            game::Transport_line_segment::TerminationType::left_only);
				break;

			case Transport_line_data::LineOrientation::down:
				line_segment->termination_type = game::Transport_line_segment::TerminationType::left_only;
				TL_LEFTSHIFT;
				try_change_termination_type(world_data, world_coords.first - 2, world_coords.second,
				                            game::Transport_line_segment::TerminationType::bend_right,
				                            game::Transport_line_segment::TerminationType::right_only);
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

void jactorio::data::Transport_line::on_build(game::World_data& world_data,
                                              const game::World_data::world_pair& world_coords,
                                              game::Chunk_tile_layer& tile_layer,
                                              const uint16_t frame,
                                              const Orientation orientation) const {
	// ======================================================================
	auto* t_center = get_line_data(world_data, world_coords.first, world_coords.second - 1);
	auto* c_right  = get_line_data(world_data, world_coords.first + 1, world_coords.second);
	auto* c_left   = get_line_data(world_data, world_coords.first - 1, world_coords.second);
	auto* b_center = get_line_data(world_data, world_coords.first, world_coords.second + 1);


	auto* line_segment = new game::Transport_line_segment{
		static_cast<const Orientation>(orientation),
		game::Transport_line_segment::TerminationType::straight,
		1
	};

	// Create transport line data at world_coords
	Transport_line_data::LineOrientation line_orientation =
		get_line_orientation(orientation, t_center, c_right, b_center, c_left);
	{
		tile_layer.unique_data = new Transport_line_data(*line_segment);
		auto* data             = static_cast<Transport_line_data*>(tile_layer.unique_data);

		data->set         = static_cast<uint16_t>(line_orientation);
		data->frame       = frame;
		data->orientation = line_orientation;
	}

	// ======================================================================
	// Update neighbor rendering orientation
	// This has to be done PRIOR to adjusting for bends

	// Take the 4 transport lines neighboring the center as parameters to avoid recalculating them
	update_neighboring_orientation(
		world_data, world_coords,
		t_center, c_right, b_center, c_left,
		static_cast<Transport_line_data*>(tile_layer.unique_data));

	// ======================================================================
	// Change current line segment termination type to a bend depending on neighbor termination orientation
	// The location where the line_segment will be added to is adjusted according to the termination type of neighbors 
	int32_t line_segment_world_x = world_coords.first;
	int32_t line_segment_world_y = world_coords.second;

	update_termination_type(world_data,
	                        world_coords,
	                        orientation,
	                        t_center, c_right, b_center, c_left,
	                        line_segment,
	                        line_segment_world_x, line_segment_world_y);

	// Add the transport line segment to logic chunk
	{
		auto* chunk = world_data.get_chunk(line_segment_world_x, line_segment_world_y);

		auto& struct_layer =
			world_data.logic_add_chunk(chunk)
			          .get_struct(game::Logic_chunk::structLayer::transport_line)
			          .emplace_back(this,
			                        game::Chunk_struct_layer::to_position(chunk->get_position().first,
			                                                              line_segment_world_x),
			                        game::Chunk_struct_layer::to_position(chunk->get_position().second,
			                                                              line_segment_world_y));

		struct_layer.unique_data = line_segment;
	}

	// ======================================================================
	// Set the target_segment to the neighbor it is pointing to, or the neighbor's target segment which is pointing to this
	if (t_center)
		update_line_targets(world_data, line_segment, orientation,
		                    world_coords.first, world_coords.second - 1,
		                    Orientation::up,
		                    Orientation::down);
	if (c_right)
		update_line_targets(world_data, line_segment, orientation,
		                    world_coords.first + 1, world_coords.second,
		                    Orientation::right,
		                    Orientation::left);
	if (b_center)
		update_line_targets(world_data, line_segment, orientation,
		                    world_coords.first, world_coords.second + 1,
		                    Orientation::down,
		                    Orientation::up);
	if (c_left)
		update_line_targets(world_data, line_segment, orientation,
		                    world_coords.first - 1, world_coords.second,
		                    Orientation::left,
		                    Orientation::right);

	// ======================================================================

	update_neighboring_transport_segment(
		world_data,
		world_coords.first, world_coords.second,
		line_orientation,

		[](game::World_data& world_data,
		   const int world_x, const int world_y,
		   const float world_offset_x, const float world_offset_y,
		   const game::Transport_line_segment::TerminationType termination_type) {

			get_line_struct_layer(world_data, world_x, world_y, [&](auto& layer) {
				layer.position_x += world_offset_x;
				layer.position_y += world_offset_y;

				auto* line_segment = static_cast<game::Transport_line_segment*>(
					layer.unique_data);

				line_segment->segment_length++;
				line_segment->termination_type = termination_type;

			});
		}, [](game::World_data& world_data,
		      const int world_x, const int world_y,
		      const float world_offset_x, const float world_offset_y,
		      const Orientation direction,
		      const game::Transport_line_segment::TerminationType termination_type) {

			get_line_struct_layer(world_data, world_x, world_y, [&](auto& layer) {
				auto* line_segment = static_cast<game::Transport_line_segment*>(layer.unique_data);
				if (line_segment->direction != direction)
					return;

				layer.position_x += world_offset_x;
				layer.position_y += world_offset_y;

				line_segment->segment_length++;
				line_segment->termination_type = termination_type;

			});
		});
}

// ======================================================================
// Neighbor update
void jactorio::data::Transport_line::on_neighbor_update(game::World_data& world_data,
                                                        const game::World_data::world_pair& emit_world_coords,
                                                        const game::World_data::world_pair& receive_world_coords,
                                                        Orientation emit_orientation) const {
	// Run stuff here that on_build and on_remove both calls

	auto* line_data = get_line_data(world_data, receive_world_coords.first, receive_world_coords.second);
	if (!line_data)  // Transport line does not exist here
		return;

	// ======================================================================

	const update_segment_func func =
		[](game::World_data& world_data,
		   const int world_x, const int world_y,
		   float /*world_offset_x*/, float /*world_offset_y*/,
		   game::Transport_line_segment::TerminationType termination_type) {

		get_line_struct_layer(world_data, world_x, world_y, [&](auto& layer) {
			auto* line_segment             = static_cast<game::Transport_line_segment*>(layer.unique_data);
			line_segment->termination_type = termination_type;

		});

	};

	const update_segment_side_only_func side_only_func =
		[](game::World_data& world_data,
		   const int world_x, const int world_y,
		   float /*world_offset_x*/, float /*world_offset_y*/,
		   Orientation direction,
		   game::Transport_line_segment::TerminationType termination_type) {

		get_line_struct_layer(world_data, world_x, world_y, [&](auto& layer) {
			auto* line_segment = static_cast<game::Transport_line_segment*>(layer.unique_data);
			if (line_segment->direction != direction)
				return;

			line_segment->termination_type = termination_type;

		});

	};

	update_neighboring_transport_segment(world_data,
	                                     receive_world_coords.first, receive_world_coords.second,
	                                     line_data->orientation,
	                                     func, side_only_func);
}


// ======================================================================
// Remove

///
/// \brief Updated neighboring segments after transport line is removed 
/// \param world_coords Coords of transport line removed
/// \param line_data Neighboring line segment
/// \param target Removed line segment
void nullptr_target_segment(jactorio::game::World_data& world_data,
                            const std::pair<jactorio::game::World_data::world_coord, jactorio::game::World_data::world_coord>
                            world_coords,
                            jactorio::data::Transport_line_data* line_data, jactorio::data::Transport_line_data* target) {

	if (line_data && line_data->line_segment.target_segment == &target->line_segment) {
		jactorio::game::Transport_line_segment& line_segment = line_data->line_segment;
		line_segment.target_segment                          = nullptr;

		// If bends / side only, set to straight & decrement length	
		switch (line_segment.termination_type) {
		case jactorio::game::Transport_line_segment::TerminationType::bend_left:
		case jactorio::game::Transport_line_segment::TerminationType::bend_right:
		case jactorio::game::Transport_line_segment::TerminationType::right_only:
		case jactorio::game::Transport_line_segment::TerminationType::left_only:
			line_segment.segment_length--;
			line_segment.termination_type = jactorio::game::Transport_line_segment::TerminationType::straight;

			// Move the neighboring line segments back if they are not straight
			switch (line_segment.direction) {
			case jactorio::data::Orientation::up:
				jactorio::data::Transport_line::get_line_struct_layer(
					world_data, world_coords.first, world_coords.second + 1, [](auto& layer) {
						++layer.position_y;
					});
				break;
			case jactorio::data::Orientation::right:
				jactorio::data::Transport_line::get_line_struct_layer(
					world_data, world_coords.first - 1, world_coords.second, [](auto& layer) {
						--layer.position_x;
					});
				break;
			case jactorio::data::Orientation::down:
				jactorio::data::Transport_line::get_line_struct_layer(
					world_data, world_coords.first, world_coords.second - 1, [](auto& layer) {
						--layer.position_y;
					});
				break;
			case jactorio::data::Orientation::left:
				jactorio::data::Transport_line::get_line_struct_layer(
					world_data, world_coords.first + 1, world_coords.second, [](auto& layer) {
						++layer.position_x;
					});
				break;

			default:
				assert(false);  // Missing case for bending
				break;
			}
			break;

		default:
			// Does not bend
			break;
		}
	}
}

void jactorio::data::Transport_line::on_remove(game::World_data& world_data,
                                               const game::World_data::world_pair& world_coords,
                                               game::Chunk_tile_layer& tile_layer) const {
	auto* t_center = get_line_data(world_data, world_coords.first, world_coords.second - 1);
	auto* c_left   = get_line_data(world_data, world_coords.first - 1, world_coords.second);
	auto* c_right  = get_line_data(world_data, world_coords.first + 1, world_coords.second);
	auto* b_center = get_line_data(world_data, world_coords.first, world_coords.second + 1);

	update_neighboring_orientation(world_data, world_coords,
	                               t_center, c_right, b_center, c_left, nullptr);


	// Set neighboring transport line segments which points to this segment's target_segment to nullptr
	nullptr_target_segment(world_data, world_coords,
	                       t_center, static_cast<Transport_line_data*>(tile_layer.unique_data));
	nullptr_target_segment(world_data, world_coords,
	                       c_left, static_cast<Transport_line_data*>(tile_layer.unique_data));
	nullptr_target_segment(world_data, world_coords,
	                       c_right, static_cast<Transport_line_data*>(tile_layer.unique_data));
	nullptr_target_segment(world_data, world_coords,
	                       b_center, static_cast<Transport_line_data*>(tile_layer.unique_data));

	game::Logic_chunk& logic_chunk =
		*world_data.logic_get_chunk(world_data.get_chunk(world_coords.first, world_coords.second));

	// Remove transport line segment referenced in Transport_line_data
	std::vector<game::Chunk_struct_layer>& struct_layer = logic_chunk.get_struct(game::Logic_chunk::structLayer::transport_line);
	struct_layer.erase(
		std::remove_if(struct_layer.begin(), struct_layer.end(), [&](game::Chunk_struct_layer& i) {
			return static_cast<game::Transport_line_segment*>(i.unique_data) ==
				&static_cast<Transport_line_data*>(tile_layer.unique_data)->line_segment;
		}),
		struct_layer.end());
}
