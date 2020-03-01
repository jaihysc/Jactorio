//
// Created on 2/29/2020.
// This file is subject to the terms and conditions defined in 'LICENSE' included in the source code package
//

#include <core/float_math.h>
#include <game/logic/transport_line_controller.h>
#include "game/logic/transport_line_structure.h"

bool jactorio::game::Transport_line_segment::can_insert(bool left_side, float start_offset) {
	std::deque<transport_line_item>& side = left_side ? this->left : this->right;

	// Check if start_offset already has an item
	float offset = 0.f;
	for (const auto& item : side) {
		// Item is not compressed with the previous item
		if (item.first > transport_line_c::item_spacing + core::transport_line_epsilon) {
			//  OFFSET item_spacing               item_spacing   OFFSET + item.first
			//     | -------------- |   GAP FOR ITEM   | ------------ |
			if (offset + transport_line_c::item_spacing <= start_offset &&
				start_offset <= offset + item.first - transport_line_c::item_spacing) {
				return true;
			}
		}

		offset += item.first;

		// Offset past start_offset, not possible to be true past this
		if (offset > start_offset + jactorio::core::transport_line_epsilon)
			return false;
	}
	// Account for the item width of the last item
	offset += jactorio::game::transport_line_c::item_spacing;

	return offset < start_offset + jactorio::core::transport_line_epsilon;
}
