// 
// transport_line.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 03/21/2020
// Last modified: 03/27/2020
// 

#include "data/prototype/entity/transport/transport_line.h"


#include "data/data_manager.h"
#include "game/logic/transport_line_structure.h"

///
/// \brief Converts lineOrientation to placementOrientation
jactorio::data::placementOrientation to_placement_orientation(
	const jactorio::data::Transport_line_data::lineOrientation line_orientation) {
	switch (line_orientation) {

	case jactorio::data::Transport_line_data::lineOrientation::up:
	case jactorio::data::Transport_line_data::lineOrientation::right_up:
	case jactorio::data::Transport_line_data::lineOrientation::left_up:
		return jactorio::data::placementOrientation::up;

	case jactorio::data::Transport_line_data::lineOrientation::right:
	case jactorio::data::Transport_line_data::lineOrientation::up_right:
	case jactorio::data::Transport_line_data::lineOrientation::down_right:
		return jactorio::data::placementOrientation::right;

	case jactorio::data::Transport_line_data::lineOrientation::down:
	case jactorio::data::Transport_line_data::lineOrientation::right_down:
	case jactorio::data::Transport_line_data::lineOrientation::left_down:
		return jactorio::data::placementOrientation::down;

	case jactorio::data::Transport_line_data::lineOrientation::left:
	case jactorio::data::Transport_line_data::lineOrientation::up_left:
	case jactorio::data::Transport_line_data::lineOrientation::down_left:
		return jactorio::data::placementOrientation::left;

	default:
		assert(false);  // Missing switch case
		return jactorio::data::placementOrientation::up;
	}
}

///
/// \return True if Line exists and points in direction 
#define NEIGHBOR_VALID(transport_line, direction)\
		((transport_line) && to_placement_orientation((transport_line)->orientation) == placementOrientation::direction)

///
/// \brief Determines line orientation given placement orientation and neighbors
jactorio::data::Transport_line_data::lineOrientation get_line_orientation(
	const jactorio::data::placementOrientation orientation,
	jactorio::data::Transport_line_data* up,
	jactorio::data::Transport_line_data* right,
	jactorio::data::Transport_line_data* down,
	jactorio::data::Transport_line_data* left
) {
	// Determine if there is a transport line neighboring the current one and a bend should be placed
	// The line above is valid for a bend since it faces towards the current tile
	using namespace jactorio::data;

	switch (orientation) {
	case placementOrientation::up:
		if (!NEIGHBOR_VALID(down, up) && NEIGHBOR_VALID(left, right) != NEIGHBOR_VALID(right, left)) {
			if (NEIGHBOR_VALID(left, right))
				return Transport_line_data::lineOrientation::right_up;
			return Transport_line_data::lineOrientation::left_up;
		}
		return Transport_line_data::lineOrientation::up;

	case placementOrientation::right:
		if (!NEIGHBOR_VALID(left, right) && NEIGHBOR_VALID(up, down) != NEIGHBOR_VALID(down, up)) {
			if (NEIGHBOR_VALID(up, down))
				return Transport_line_data::lineOrientation::down_right;
			return Transport_line_data::lineOrientation::up_right;
		}
		return Transport_line_data::lineOrientation::right;

	case placementOrientation::down:
		if (!NEIGHBOR_VALID(up, down) && NEIGHBOR_VALID(left, right) != NEIGHBOR_VALID(right, left)) {
			if (NEIGHBOR_VALID(left, right))
				return Transport_line_data::lineOrientation::right_down;
			return Transport_line_data::lineOrientation::left_down;
		}
		return Transport_line_data::lineOrientation::down;

	case placementOrientation::left:
		if (!NEIGHBOR_VALID(right, left) && NEIGHBOR_VALID(up, down) != NEIGHBOR_VALID(down, up)) {
			if (NEIGHBOR_VALID(up, down))
				return Transport_line_data::lineOrientation::down_left;
			return Transport_line_data::lineOrientation::up_left;
		}
		return Transport_line_data::lineOrientation::left;

	default:
		assert(false); // Missing switch case
	}

	return Transport_line_data::lineOrientation::up;
}


std::pair<uint16_t, uint16_t> jactorio::data::Transport_line::map_placement_orientation(
	const placementOrientation orientation, game::World_data& world_data,
	const std::pair<int, int> world_coords) const {

	auto* t_center = get_line_data(world_data, world_coords.first, world_coords.second - 1);
	auto* c_left = get_line_data(world_data, world_coords.first - 1, world_coords.second);
	auto* c_right = get_line_data(world_data, world_coords.first + 1, world_coords.second);
	auto* b_center = get_line_data(world_data, world_coords.first, world_coords.second + 1);

	return {static_cast<uint16_t>(get_line_orientation(orientation, t_center, c_right, b_center, c_left)), 0};
}


void jactorio::data::Transport_line::update_neighboring_orientation(game::World_data& world_data,
                                                                    const std::pair<int, int> world_coords,
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
	auto* top = get_line_data(world_data, world_coords.first, world_coords.second - 2);
	auto* right = get_line_data(world_data, world_coords.first + 2, world_coords.second);
	auto* bottom = get_line_data(world_data, world_coords.first, world_coords.second + 2);
	auto* left = get_line_data(world_data, world_coords.first - 2, world_coords.second);

	auto* t_left = get_line_data(world_data, world_coords.first - 1, world_coords.second - 1);
	// auto* t_center = get_line_data(world_data, world_coords.first, world_coords.second - 1);
	auto* t_right = get_line_data(world_data, world_coords.first + 1, world_coords.second - 1);

	// auto* c_left = get_line_data(world_data, world_coords.first - 1, world_coords.second);
	// auto* center = static_cast<Transport_line_data*>(center_layer.unique_data);
	// auto* c_right = get_line_data(world_data, world_coords.first + 1, world_coords.second);

	auto* b_left = get_line_data(world_data, world_coords.first - 1, world_coords.second + 1);
	// auto* b_center = get_line_data(world_data, world_coords.first, world_coords.second + 1);
	auto* b_right = get_line_data(world_data, world_coords.first + 1, world_coords.second + 1);

	// Top neighbor
	if (t_center)
		t_center->set_orientation(
			get_line_orientation(to_placement_orientation(t_center->orientation), top, t_right, center, t_left));
	// Right
	if (c_right)
		c_right->set_orientation(
			get_line_orientation(to_placement_orientation(c_right->orientation), t_right, right, b_right, center));
	// Bottom
	if (b_center)
		b_center->set_orientation(
			get_line_orientation(to_placement_orientation(b_center->orientation), center, b_right, bottom, b_left));
	// Left
	if (c_left)
		c_left->set_orientation(
			get_line_orientation(to_placement_orientation(c_left->orientation), t_left, center, b_left, left));
}

///
/// \brief Determines the member target_segment of Transport_line_segment
/// \param orientation Current orientation
/// \param origin_connect_orientation Orientation required for origin_segment to connect to neighbor segment
/// \param target_connect_orientation Orientation required for neighbor segment to connect to origin segment
void update_transport_line_targets(jactorio::game::World_data& world_data,
                                   jactorio::game::Transport_line_segment* origin_segment,
                                   const jactorio::data::placementOrientation orientation, const int32_t neighbor_world_x,
                                   const int32_t neighbor_world_y,
                                   const jactorio::data::placementOrientation origin_connect_orientation,
                                   const jactorio::data::placementOrientation target_connect_orientation) {
	using namespace jactorio;

	data::Transport_line_data* neighbor_line_data = data::Transport_line::get_line_data(
		world_data, neighbor_world_x, neighbor_world_y);
	if (neighbor_line_data) {
		game::Transport_line_segment& neighbor_segment = neighbor_line_data->line_segment;

		const bool origin_valid = orientation == origin_connect_orientation;
		const bool neighbor_valid =
			static_cast<data::placementOrientation>(neighbor_segment.direction) == target_connect_orientation;

		// Only 1 can be valid at a time
		if (origin_valid == neighbor_valid)
			return;

		if (origin_valid)
			origin_segment->target_segment = &neighbor_segment;
		else
			neighbor_segment.target_segment = origin_segment;
	}
}

///
/// \brief Finds transport line at world_x, world_y and moves it by world_offset_x, world_offset_y <br>
/// It will also set the termination_type to the parameter and increase segment length by 1
void update_transport_segment(jactorio::game::World_data& world_data,
                              const int world_x, const int world_y,
                              const float world_offset_x, const float world_offset_y,
                              const jactorio::game::Transport_line_segment::terminationType termination_type) {

	using namespace jactorio;

	auto& transport_structures =
		world_data.logic_get_all_chunks()
		          .at(world_data.get_chunk_world_coords(world_x, world_y))
		          .get_struct(game::Logic_chunk::structLayer::transport_line);

	data::Transport_line_data* line_data = data::Transport_line::get_line_data(world_data, world_x, world_y);

	// Search for the transport line in the chunk struct layer where the
	// unique_data(Transport_line_data) == one referenced by line_data
	for (auto& layer : transport_structures) {
		if (layer.unique_data == &line_data->line_segment) {
			layer.position_x += world_offset_x;
			layer.position_y += world_offset_y;

			auto* line_segment = static_cast<game::Transport_line_segment*>(layer.unique_data);

			line_segment->segment_length++;
			line_segment->termination_type = termination_type;
		}
	}
}


// ======================================================================

jactorio::data::Transport_line_data* jactorio::data::Transport_line::get_line_data(game::World_data& world_data,
                                                                                   const int world_x, const int world_y) {
	auto* tile = world_data.get_tile_world_coords(world_x, world_y);
	if (!tile)  // No tile exists
		return nullptr;

	auto& layer = tile->get_layer(game::Chunk_tile::chunkLayer::entity);

	if (!dynamic_cast<const Transport_line*>(  // Not an instance of transport line
		layer.prototype_data))
		return nullptr;

	return static_cast<Transport_line_data*>(layer.unique_data);
}


void jactorio::data::Transport_line::on_build(game::World_data& world_data, const std::pair<int, int> world_coords,
                                              game::Chunk_tile_layer& tile_layer, const uint16_t frame,
                                              const placementOrientation orientation) const {

	// ======================================================================
	// Update neighbor rendering orientation
	auto* t_center = get_line_data(world_data, world_coords.first, world_coords.second - 1);
	auto* c_right = get_line_data(world_data, world_coords.first + 1, world_coords.second);
	auto* c_left = get_line_data(world_data, world_coords.first - 1, world_coords.second);
	auto* b_center = get_line_data(world_data, world_coords.first, world_coords.second + 1);


	auto* line_segment = new game::Transport_line_segment{
		static_cast<const game::Transport_line_segment::moveDir>(orientation),
		game::Transport_line_segment::terminationType::straight,
		1
	};

	// Create transport line data at world_coords
	Transport_line_data::lineOrientation line_orientation =
		get_line_orientation(orientation, t_center, c_right, b_center, c_left);
	{
		auto* data = new Transport_line_data(*line_segment);

		data->set = static_cast<uint16_t>(line_orientation);
		data->frame = frame;
		data->orientation = line_orientation;
		tile_layer.unique_data = data;

		// Take the 4 transport lines neighboring the center as parameters to avoid recalculating them
		update_neighboring_orientation(
			world_data, world_coords, t_center,
			c_right, b_center, c_left, static_cast<Transport_line_data*>(tile_layer.unique_data));
	}

	// ======================================================================
	// Change current line segment termination type to a bend depending on neighbor termination orientation
	// The location where the line_segment will be added to is adjusted according to the termination type of neighbors 
	int32_t line_segment_world_x = world_coords.first;
	int32_t line_segment_world_y = world_coords.second;

	switch (orientation) {
	case placementOrientation::up:
		if (t_center)
			switch (t_center->orientation) {
			case Transport_line_data::lineOrientation::up_left:
				line_segment->termination_type = game::Transport_line_segment::terminationType::bend_left;
				line_segment_world_y--;
				line_segment->segment_length++;
				break;
			case Transport_line_data::lineOrientation::up_right:
				line_segment->termination_type = game::Transport_line_segment::terminationType::bend_right;
				line_segment_world_y--;
				line_segment->segment_length++;
				break;

			case Transport_line_data::lineOrientation::right:
				line_segment->termination_type = game::Transport_line_segment::terminationType::right_only;
				break;
			case Transport_line_data::lineOrientation::left:
				line_segment->termination_type = game::Transport_line_segment::terminationType::left_only;
				break;

			default:
				break;
			}
		break;
	case placementOrientation::right:
		if (c_right)
			switch (c_right->orientation) {
			case Transport_line_data::lineOrientation::right_up:
				line_segment->termination_type = game::Transport_line_segment::terminationType::bend_left;
				line_segment_world_x++;
				line_segment->segment_length++;
				break;
			case Transport_line_data::lineOrientation::right_down:
				line_segment->termination_type = game::Transport_line_segment::terminationType::bend_right;
				line_segment_world_x++;
				line_segment->segment_length++;
				break;

			case Transport_line_data::lineOrientation::up:
				line_segment->termination_type = game::Transport_line_segment::terminationType::right_only;
				break;

			case Transport_line_data::lineOrientation::down:
				line_segment->termination_type = game::Transport_line_segment::terminationType::left_only;
				break;

			default:
				break;
			}
		break;
	case placementOrientation::down:
		if (b_center)
			switch (b_center->orientation) {
			case Transport_line_data::lineOrientation::down_right:
				line_segment->termination_type = game::Transport_line_segment::terminationType::bend_left;
				line_segment_world_y++;
				line_segment->segment_length++;
				break;
			case Transport_line_data::lineOrientation::down_left:
				line_segment->termination_type = game::Transport_line_segment::terminationType::bend_right;
				line_segment_world_y++;
				line_segment->segment_length++;
				break;

			case Transport_line_data::lineOrientation::right:
				line_segment->termination_type = game::Transport_line_segment::terminationType::right_only;
				break;

			case Transport_line_data::lineOrientation::left:
				line_segment->termination_type = game::Transport_line_segment::terminationType::left_only;
				break;

			default:
				break;
			}
		break;
	case placementOrientation::left:
		if (c_left)
			switch (c_left->orientation) {
			case Transport_line_data::lineOrientation::left_down:
				line_segment->termination_type = game::Transport_line_segment::terminationType::bend_left;
				line_segment_world_x--;
				line_segment->segment_length++;
				break;
			case Transport_line_data::lineOrientation::left_up:
				line_segment->termination_type = game::Transport_line_segment::terminationType::bend_right;
				line_segment_world_x--;
				line_segment->segment_length++;
				break;

			case Transport_line_data::lineOrientation::up:
				line_segment->termination_type = game::Transport_line_segment::terminationType::left_only;
				break;

			case Transport_line_data::lineOrientation::down:
				line_segment->termination_type = game::Transport_line_segment::terminationType::right_only;
				break;

			default:
				break;
			}
		break;

	default:
		assert(false);  // Missing switch case
	}

	// Add the transport line segment to logic chunk
	{
		auto* chunk = world_data.get_chunk_world_coords(line_segment_world_x, line_segment_world_y);

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
		update_transport_line_targets(world_data, line_segment, orientation,
		                              world_coords.first, world_coords.second - 1,
		                              placementOrientation::up,
		                              placementOrientation::down);
	if (c_right)
		update_transport_line_targets(world_data, line_segment, orientation,
		                              world_coords.first + 1, world_coords.second,
		                              placementOrientation::right,
		                              placementOrientation::left);
	if (b_center)
		update_transport_line_targets(world_data, line_segment, orientation,
		                              world_coords.first, world_coords.second + 1,
		                              placementOrientation::down,
		                              placementOrientation::up);
	if (c_left)
		update_transport_line_targets(world_data, line_segment, orientation,
		                              world_coords.first - 1, world_coords.second,
		                              placementOrientation::left,
		                              placementOrientation::right);

	// ======================================================================
	// Change the neighboring line segment termination type to a bend depending on Transport_line_data orientation
	// Since the line_orientation was changed to a bend, it is confirmed that segments exist at neighboring locations
	// NOTE! This does not move across logic chunks and may make the position negative
	switch (line_orientation) {
		// Up
	case Transport_line_data::lineOrientation::up_left:
		update_transport_segment(world_data, world_coords.first, world_coords.second + 1,
		                         0, -1,
		                         game::Transport_line_segment::terminationType::bend_left);
		break;
	case Transport_line_data::lineOrientation::up_right:
		update_transport_segment(world_data, world_coords.first, world_coords.second + 1,
		                         0, -1,
		                         game::Transport_line_segment::terminationType::bend_right);
		break;

		// Right
	case Transport_line_data::lineOrientation::right_up:
		update_transport_segment(world_data, world_coords.first - 1, world_coords.second,
		                         1, 0,
		                         game::Transport_line_segment::terminationType::bend_left);
		break;
	case Transport_line_data::lineOrientation::right_down:
		update_transport_segment(world_data, world_coords.first - 1, world_coords.second,
		                         1, 0,
		                         game::Transport_line_segment::terminationType::bend_right);
		break;

		// Down
	case Transport_line_data::lineOrientation::down_right:
		update_transport_segment(world_data, world_coords.first, world_coords.second - 1,
		                         0, 1,
		                         game::Transport_line_segment::terminationType::bend_left);
		break;
	case Transport_line_data::lineOrientation::down_left:
		update_transport_segment(world_data, world_coords.first, world_coords.second - 1,
		                         0, 1,
		                         game::Transport_line_segment::terminationType::bend_right);
		break;

		// Left
	case Transport_line_data::lineOrientation::left_down:
		update_transport_segment(world_data, world_coords.first + 1, world_coords.second,
		                         -1, 0,
		                         game::Transport_line_segment::terminationType::bend_left);
		break;
	case Transport_line_data::lineOrientation::left_up:
		update_transport_segment(world_data, world_coords.first + 1, world_coords.second,
		                         -1, 0,
		                         game::Transport_line_segment::terminationType::bend_right);
		break;

	default:
		break;
	}

}

void jactorio::data::Transport_line::on_remove(game::World_data& world_data, const std::pair<int, int> world_coords,
                                               game::Chunk_tile_layer& /*tile_layer*/) const {
	auto* t_center = get_line_data(world_data, world_coords.first, world_coords.second - 1);
	auto* c_left = get_line_data(world_data, world_coords.first - 1, world_coords.second);
	auto* c_right = get_line_data(world_data, world_coords.first + 1, world_coords.second);
	auto* b_center = get_line_data(world_data, world_coords.first, world_coords.second + 1);

	update_neighboring_orientation(world_data, world_coords, t_center,
	                               c_right, b_center, c_left, nullptr);
}
