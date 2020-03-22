// 
// transport_line.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 03/21/2020
// Last modified: 03/22/2020
// 

#include "data/prototype/entity/transport/transport_line.h"

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

///
/// \brief Attempts to retrieve transport line data at world coordinates
/// \return pointer to data or nullptr if non existent
jactorio::data::Transport_line_data* get_line_data(jactorio::game::World_data& world_data,
                                                   const int world_x, const int world_y) {
	auto* tile = world_data.get_tile_world_coords(world_x, world_y);
	if (!tile)  // No tile exists
		return nullptr;

	auto& layer = tile->get_layer(jactorio::game::Chunk_tile::chunkLayer::entity);

	if (!dynamic_cast<jactorio::data::Transport_line*>(  // Not an instance of transport line
		layer.prototype_data))
		return nullptr;

	return static_cast<jactorio::data::Transport_line_data*>(layer.unique_data);
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


void jactorio::data::Transport_line::on_build(game::World_data& world_data, const std::pair<int, int> world_coords,
                                              game::Chunk_tile_layer* tile_layer, const uint16_t frame,
                                              const placementOrientation orientation) const {
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
	auto* t_center = get_line_data(world_data, world_coords.first, world_coords.second - 1);
	auto* t_right = get_line_data(world_data, world_coords.first + 1, world_coords.second - 1);

	auto* c_left = get_line_data(world_data, world_coords.first - 1, world_coords.second);
	auto* c_right = get_line_data(world_data, world_coords.first + 1, world_coords.second);

	auto* b_left = get_line_data(world_data, world_coords.first - 1, world_coords.second + 1);
	auto* b_center = get_line_data(world_data, world_coords.first, world_coords.second + 1);
	auto* b_right = get_line_data(world_data, world_coords.first + 1, world_coords.second + 1);

	// Calculate center first so center unique_data is initialized	
	{
		auto* data = new Transport_line_data();
		auto line_orientation = get_line_orientation(orientation, t_center, c_right, b_center, c_left);

		data->set = static_cast<uint16_t>(line_orientation);
		data->frame = frame;
		data->orientation = line_orientation;
		tile_layer->unique_data = data;
	}

	auto* center = static_cast<Transport_line_data*>(tile_layer->unique_data);

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
