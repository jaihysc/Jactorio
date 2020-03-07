//
// Created on 2/29/2020.
// This file is subject to the terms and conditions defined in 'LICENSE' included in the source code package
//

#include "core/data_type.h"
#include "game/logic/transport_line_controller.h"
#include "game/logic/transport_line_structure.h"

bool jactorio::game::Transport_line_segment::can_insert(bool left_side, const transport_line_offset& start_offset) {
	std::deque<transport_line_item>& side = left_side ? this->left : this->right;

	// Check if start_offset already has an item
	transport_line_offset offset{0};

	for (const auto& item : side) {
		// Item is not compressed with the previous item
		if (item.first >
			dec::decimal_cast<jactorio::transport_line_decimal_place>(transport_line_c::item_spacing)) {
			//  OFFSET item_spacing               item_spacing   OFFSET + item.first
			//     | -------------- |   GAP FOR ITEM   | ------------ |
			if (dec::decimal_cast<jactorio::transport_line_decimal_place>(transport_line_c::item_spacing) + offset <=
				start_offset &&
				start_offset <=
				offset + item.first - dec::decimal_cast<jactorio::transport_line_decimal_place>(transport_line_c::item_spacing)) {

				return true;
			}
		}

		offset += item.first;

		// Offset past start_offset, not possible to be true past this
		if (offset > start_offset)
			return false;
	}
	// Account for the item width of the last item
	offset += dec::decimal_cast<jactorio::transport_line_decimal_place>(jactorio::game::transport_line_c::item_spacing);

	return offset <= start_offset;
}
