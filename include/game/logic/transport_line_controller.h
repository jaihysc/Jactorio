#ifndef GAME_LOGIC_TRANSPORT_LINE_CONTROLLER_H
#define GAME_LOGIC_TRANSPORT_LINE_CONTROLLER_H

#include "game/world/logic_chunk.h"

#include "data/prototype/item/item.h"

#include <queue>

/**
 * Transport line logic for anything moving items
 */
namespace jactorio::game::transport_line_c
{
	// Each item on belt gets a SINGLE float of the distance to the next item / end of belt
	//
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
	 *		This makes it feasible to store items for transport belts based on the chunk(or tile)
	 *		they reside in
	 *
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
	 *
	 * A right item will occupy the entire space from 0.1 to 0.5
	 * A left item will occupy the entire space from 0.5 to 0.9
	 */

	// Width of one item on a belt (in tiles)
	constexpr float item_width = 0.4f;

	// Destination chunk's item entity layer pointer, pointer to items which needs to be moved there
	using chunk_transition_item_queue = std::pair<std::vector<Chunk_object_layer>*, std::vector<Chunk_object_layer>>;

	/**
	 * Updates belt logic for a logic chunk
	 * Item will be queued for chunk transition when location is less than 0 + epsilon or greater than 32 - epsilon
	 * @param chunk_transition_items Queue of items which needs to be moved into another chunk as it has crossed chunk boundaries <br>
	 * @param l_chunk Chunk to update
	 */
	void logic_update(std::queue<chunk_transition_item_queue>& chunk_transition_items, Logic_chunk* l_chunk);

	/**
	 * Will append everything from chunk_transition_item_queue.second into the layer at chunk_transition_item_queue.first <br>
	 * The queue is empty and will be usable after calling this
	 * @param chunk_transition_items  Queue to process
	 */
	void logic_process_queued_items(std::queue<chunk_transition_item_queue>& chunk_transition_items);


	// When inserting an item, it will insert it onto the center of the belt

	// Inserts an item onto a belt LEFT side
	void belt_insert_item_l(int tile_x, int tile_y, data::Item* item);

	// Inserts an item onto a belt RIGHT side
	void belt_insert_item_r(int tile_x, int tile_y, data::Item* item);


	/**
	 * Adds item to chunk with specified offsets (Prefer belt_insert_item_x() over this)
	 * Will not add chunk to logic chunks list automatically
	 */
	void chunk_insert_item(Chunk* chunk, float position_x, float position_y, data::Item* item);
}

#endif // GAME_LOGIC_TRANSPORT_LINE_CONTROLLER_H
