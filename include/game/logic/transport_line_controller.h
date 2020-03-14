// 
// transport_line_controller.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 02/13/2020
// Last modified: 03/12/2020
// 

#ifndef JACTORIO_INCLUDE_GAME_LOGIC_TRANSPORT_LINE_CONTROLLER_H
#define JACTORIO_INCLUDE_GAME_LOGIC_TRANSPORT_LINE_CONTROLLER_H

#include "game/world/logic_chunk.h"
#include "game/world/world_data.h"

/**
 * Transport line logic for anything moving items
 */
namespace jactorio::game::transport_line_c
{
	/* Placement of items on transport line (Expressed as decimal percentages of a tile)
	 * | R Padding 0.0
	 * |
	 * ------------------------------------------------- 0.1
	 *
	 * <<<<<< center of R item <<<<<<<<<<<<<<<<<<<<<<<< 0.3
	 *
	 * ====== CENTER OF BELT ========================== 0.5
	 *
	 * <<<<<< center of L item <<<<<<<<<<<<<<<<<<<<<<<< 0.7
	 * 
	 * ------------------------------------------------- 0.9
	 * |
	 * | L Padding 1.0
	 *
	 * With an item_width of 0.4f:
	 * A right item will occupy the entire space from 0.1 to 0.5
	 * A left item will occupy the entire space from 0.5 to 0.9
	 */

	/*
	 * Item wakeup:
	 *
	 * After a transport line lane stops, it cannot wake up by itself, another transport line or lane must call the member update_wakeup
	 * in transport_line_structure
	 */

	// Width of one item on a belt (in tiles)
	constexpr double item_width = 0.4f;
	// Distance left between each item when transport line is fully compressed (in tiles)
	constexpr double item_spacing = 0.25f;

	// Number of tiles to offset items in order to line up on the L / R sides of the belt for all 4 directions
	// Direction is direction of item movement for the transport line

	constexpr double line_base_offset_left = 0.3;
	constexpr double line_base_offset_right = 0.7;

	constexpr double line_left_up_straight_item_offset = 0.25;
	constexpr double line_right_down_straight_item_offset = 0.75;

	constexpr double line_up_l_item_offset_x = line_base_offset_left - item_width / 2;
	constexpr double line_up_r_item_offset_x = line_base_offset_right - item_width / 2;

	constexpr double line_right_l_item_offset_y = line_base_offset_left - item_width / 2;
	constexpr double line_right_r_item_offset_y = line_base_offset_right - item_width / 2;

	constexpr double line_down_l_item_offset_x = line_base_offset_right - item_width / 2;
	constexpr double line_down_r_item_offset_x = line_base_offset_left - item_width / 2;

	constexpr double line_left_l_item_offset_y = line_base_offset_right - item_width / 2;
	constexpr double line_left_r_item_offset_y = line_base_offset_left - item_width / 2;

	// Bend left
	constexpr double line_up_bl_l_item_offset_y = line_base_offset_right - item_width / 2;
	constexpr double line_up_bl_r_item_offset_y = line_base_offset_left - item_width / 2;

	constexpr double line_right_bl_l_item_offset_x = line_base_offset_left - item_width / 2;
	constexpr double line_right_bl_r_item_offset_x = line_base_offset_right - item_width / 2;

	constexpr double line_down_bl_l_item_offset_y = line_base_offset_left - item_width / 2;
	constexpr double line_down_bl_r_item_offset_y = line_base_offset_right - item_width / 2;

	constexpr double line_left_bl_l_item_offset_x = line_base_offset_right - item_width / 2;
	constexpr double line_left_bl_r_item_offset_x = line_base_offset_left - item_width / 2;

	// Bend right
	constexpr double line_up_br_l_item_offset_y = line_base_offset_left - item_width / 2;
	constexpr double line_up_br_r_item_offset_y = line_base_offset_right - item_width / 2;

	constexpr double line_right_br_l_item_offset_x = line_base_offset_right - item_width / 2;
	constexpr double line_right_br_r_item_offset_x = line_base_offset_left - item_width / 2;

	constexpr double line_down_br_l_item_offset_y = line_base_offset_right - item_width / 2;
	constexpr double line_down_br_r_item_offset_y = line_base_offset_left - item_width / 2;

	constexpr double line_left_br_l_item_offset_x = line_base_offset_left - item_width / 2;
	constexpr double line_left_br_r_item_offset_x = line_base_offset_right - item_width / 2;

	// Feed side (left and right lanes are the same)
	constexpr double line_up_single_side_item_offset_y = line_base_offset_right - item_width / 2;
	constexpr double line_right_single_side_item_offset_x = line_base_offset_left - item_width / 2;
	constexpr double line_down_single_side_item_offset_y = line_base_offset_left - item_width / 2;
	constexpr double line_left_single_side_item_offset_x = line_base_offset_right - item_width / 2;

	/**
	 * Moves items for transport lines
	 * @param l_chunk Chunk to update
	 */
	void logic_update_move_items(Logic_chunk* l_chunk);

	/**
	 * Transitions items on transport lines to other lines and modifies whether of not the line is active
	 * @param l_chunk Chunk to update
	 */
	void logic_update_transition_items(Logic_chunk* l_chunk);


	/**
	 * Updates belt logic for a logic chunk
	 */
	void transport_line_logic_update(World_data& world_data);
}

#endif //JACTORIO_INCLUDE_GAME_LOGIC_TRANSPORT_LINE_CONTROLLER_H
