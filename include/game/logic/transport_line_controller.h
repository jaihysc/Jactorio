#ifndef GAME_LOGIC_TRANSPORT_LINE_CONTROLLER_H
#define GAME_LOGIC_TRANSPORT_LINE_CONTROLLER_H

#include "game/world/logic_chunk.h"

#include "data/prototype/item/item.h"
#include "transport_line_structure.h"

#include <queue>
#include <utility>

/**
 * Transport line logic for anything moving items
 */
namespace jactorio::game::transport_line_c
{
	// Update tiles are iterated over by the update logic, since items are stored per chunk. It
	// iterates through all items of a chunk, checking if any of the conditions are met
	// with a 32 x 32 chunk and 9 items per chunk: (9216 iterations max) (~120 chunks average)
	//
	// After an item exits a chunk, it needs to be moved to the next chunk

	// A list of positions is stored them behind a std::map per chunk,
	// with a custom float comparison function accounting for epsilon
	// This gives a faster O(log n) speed over O(n) iterating over every update tile

	/*
	 * Measured performance (Debug config):
	 * Copying 120 000 objects: 7ms
	 */

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

	// Width of one item on a belt (in tiles)
	constexpr float item_width = 0.4f;
	// Distance left between each item when transport line is fully compressed (in tiles)
	constexpr float item_spacing = 0.25f;

	// Number of tiles to offset items in order to line up on the L / R sides of the belt for all 4 directions
	// Direction is direction of item movement for the transport line

	constexpr float line_base_offset_left = 0.3;
	constexpr float line_base_offset_right = 0.7;

	constexpr float line_up_l_item_offset_x = line_base_offset_left - item_width / 2;
	constexpr float line_up_r_item_offset_x = line_base_offset_right - item_width / 2;

	constexpr float line_right_l_item_offset_y = line_base_offset_left - item_width / 2;
	constexpr float line_right_r_item_offset_y = line_base_offset_right - item_width / 2;

	constexpr float line_down_l_item_offset_x = line_base_offset_right - item_width / 2;
	constexpr float line_down_r_item_offset_x = line_base_offset_left - item_width / 2;

	constexpr float line_left_l_item_offset_y = line_base_offset_right - item_width / 2;
	constexpr float line_left_r_item_offset_y = line_base_offset_left - item_width / 2;

	// Bend left
	constexpr float line_up_bl_l_item_offset_y = line_base_offset_right - item_width / 2;
	constexpr float line_up_bl_r_item_offset_y = line_base_offset_left - item_width / 2;

	constexpr float line_right_bl_l_item_offset_x = line_base_offset_left - item_width / 2;
	constexpr float line_right_bl_r_item_offset_x = line_base_offset_right - item_width / 2;

	constexpr float line_down_bl_l_item_offset_y = line_base_offset_left - item_width / 2;
	constexpr float line_down_bl_r_item_offset_y = line_base_offset_right - item_width / 2;

	constexpr float line_left_bl_l_item_offset_x = line_base_offset_right - item_width / 2;
	constexpr float line_left_bl_r_item_offset_x = line_base_offset_left - item_width / 2;

	// Bend right
	constexpr float line_up_br_l_item_offset_y = line_base_offset_left - item_width / 2;
	constexpr float line_up_br_r_item_offset_y = line_base_offset_right - item_width / 2;

	constexpr float line_right_br_l_item_offset_x = line_base_offset_right - item_width / 2;
	constexpr float line_right_br_r_item_offset_x = line_base_offset_left - item_width / 2;

	constexpr float line_down_br_l_item_offset_y = line_base_offset_right - item_width / 2;
	constexpr float line_down_br_r_item_offset_y = line_base_offset_left - item_width / 2;

	constexpr float line_left_br_l_item_offset_x = line_base_offset_left - item_width / 2;
	constexpr float line_left_br_r_item_offset_x = line_base_offset_right - item_width / 2;


	/**
	 * Holds items which are to be transferred to another transport line
	 */
	struct Segment_transition_item
	{
		Segment_transition_item(Transport_line_segment* target_segment, Transport_line_segment* previous_segment,
								std::vector<transport_line_item>&& items)
			: target_segment(target_segment), previous_segment(previous_segment), items(std::move(items)) {
		}

		game::Transport_line_segment* target_segment;
		game::Transport_line_segment* previous_segment;

		// The offset in transport_line_item is negative to indicate it belongs to the left side, positive for right
		std::vector<transport_line_item> items;
	};

	/**
	 * Updates belt logic for a logic chunk
	 * @param queue Queue of items which needs to be moved into another line segment as it has crossed this one's end <br>
	 * @param l_chunk Chunk to update
	 */
	void logic_update(std::queue<Segment_transition_item>& queue, Logic_chunk* l_chunk);

	/**
	 * Attempts to move items in the queue to their new transport line segments
	 * The queue is empty and will be usable after calling this
	 * @param queue  Queue to process
	 */
	void logic_process_queued_items(std::queue<Segment_transition_item>& queue);


	// Item insertion
	/**
	 * Inserts item onto the specified side of a belt behind the last item
	 * @param insert_left True to insert item on left size of belt
	 * @param offset Number of tiles to offset from previous item or the end of the transport line segment when there are no items
	 */
	void belt_insert_item(bool insert_left, game::Transport_line_segment* belt, float offset, data::Item* item);
}

#endif // GAME_LOGIC_TRANSPORT_LINE_CONTROLLER_H
