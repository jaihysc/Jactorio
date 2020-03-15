// 
// transport_line_structure.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 02/29/2020
// Last modified: 03/15/2020
// 

#include "core/data_type.h"
#include "game/logic/transport_line_controller.h"
#include "game/logic/transport_line_structure.h"

bool jactorio::game::Transport_line_segment::
can_insert(const bool left_side, const transport_line_offset& start_offset) {
	std::deque<transport_line_item>& side = left_side ? this->left : this->right;

	// Check if start_offset already has an item
	transport_line_offset offset{0};

	for (const auto& item : side) {
		// Item is not compressed with the previous item
		if (item.first >
			dec::decimal_cast<transport_line_decimal_place>(transport_line_c::item_spacing)) {
			//  OFFSET item_spacing               item_spacing   OFFSET + item.first
			//     | -------------- |   GAP FOR ITEM   | ------------ |
			if (dec::decimal_cast<transport_line_decimal_place>(transport_line_c::item_spacing) + offset <=
				start_offset &&
				start_offset <=
				offset + item.first - dec::decimal_cast<transport_line_decimal_place>(transport_line_c::item_spacing)) {

				return true;
			}
		}

		offset += item.first;

		// Offset past start_offset, not possible to be true past this
		if (offset > start_offset)
			return false;
	}
	// Account for the item width of the last item
	offset += dec::decimal_cast<transport_line_decimal_place>(transport_line_c::item_spacing);

	return offset <= start_offset;
}

void jactorio::game::Transport_line_segment::append_item(const bool insert_left, double offset, data::Item* item) {
	std::deque<transport_line_item>& target_queue = insert_left ? this->left : this->right;

	// A minimum distance of transport_line_c::item_spacing is maintained between items (AFTER the initial item)
	if (offset < transport_line_c::item_spacing && !target_queue.empty())
		offset = transport_line_c::item_spacing;

	target_queue.emplace_back(offset, item);
	insert_left
		? l_back_item_distance += transport_line_offset{offset}
		: r_back_item_distance += transport_line_offset{offset};
}

void jactorio::game::Transport_line_segment::
insert_item(const bool insert_left, const double offset, data::Item* item) {
	std::deque<transport_line_item>& target_queue = insert_left ? this->left : this->right;

	transport_line_offset target_offset{offset};
	transport_line_offset counter_offset;

	std::deque<transport_line_item>::iterator it;
	for (auto i = 0u; i < target_queue.size(); ++i) {
		if (target_offset < target_queue[i].first) {
			it = target_queue.begin() + i;

			// Modify offset of next item to be relative to what will be the newly inserted item
			it->first -= target_offset;
			goto loop_exit;
		}
		counter_offset += target_queue[i].first;
	}
	// Failed to find a greater item
	it = target_queue.end();
	insert_left
		? l_back_item_distance = target_offset
		: r_back_item_distance = target_offset;

loop_exit:
	// Modify target offset relative to previous item
	target_offset -= counter_offset;


	// A minimum distance of transport_line_c::item_spacing is maintained between items
	//	if (target_offset < dec::decimal_cast<transport_line_decimal_place>(transport_line_c::item_spacing) && !target_queue.empty())
	//		target_offset = transport_line_c::item_spacing;

	target_queue.emplace(it, target_offset, item);
}
