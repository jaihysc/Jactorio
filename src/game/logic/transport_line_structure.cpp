// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#include "game/logic/transport_line_structure.h"

#include <decimal.h>

#include "core/data_type.h"
#include "game/logic/transport_line_controller.h"

bool jactorio::game::TransportLineSegment::
CanInsert(const bool left_side, const TransportLineOffset& start_offset) {
	std::deque<TransportLineItem>& side = left_side ? this->left : this->right;

	// Check if start_offset already has an item
	TransportLineOffset offset{0};

	for (const auto& item : side) {
		// Item is not compressed with the previous item
		if (item.first >
			dec::decimal_cast<kTransportLineDecimalPlace>(kItemSpacing)) {
			//  OFFSET item_spacing               item_spacing   OFFSET + item.first
			//     | -------------- |   GAP FOR ITEM   | ------------ |
			if (dec::decimal_cast<kTransportLineDecimalPlace>(kItemSpacing) + offset <=
				start_offset &&
				start_offset <=
				offset + item.first - dec::decimal_cast<kTransportLineDecimalPlace>(kItemSpacing)) {

				return true;
			}
		}

		offset += item.first;

		// Offset past start_offset, not possible to be true past this
		if (offset > start_offset)
			return false;
	}

	// Account for the item width of the last item if not the firs item
	if (!side.empty())
		offset += dec::decimal_cast<kTransportLineDecimalPlace>(kItemSpacing);

	return offset <= start_offset;
}

void jactorio::game::TransportLineSegment::AppendItem(const bool insert_left, double offset, const data::Item* item) {
	std::deque<TransportLineItem>& target_queue = insert_left ? this->left : this->right;

	// A minimum distance of item_spacing is maintained between items (AFTER the initial item)
	if (offset < kItemSpacing && !target_queue.empty())
		offset = kItemSpacing;

	target_queue.emplace_back(offset, item);
	insert_left
		? lBackItemDistance += TransportLineOffset{offset}
		: rBackItemDistance += TransportLineOffset{offset};
}

void jactorio::game::TransportLineSegment::InsertItem(const bool insert_left, const double offset, const data::Item* item) {
	std::deque<TransportLineItem>& target_queue = insert_left ? this->left : this->right;

	TransportLineOffset target_offset{offset};
	TransportLineOffset counter_offset;

	std::deque<TransportLineItem>::iterator it;
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
		? lBackItemDistance = target_offset
		: rBackItemDistance = target_offset;

loop_exit:
	// Modify target offset relative to previous item
	target_offset -= counter_offset;


	// A minimum distance of item_spacing is maintained between items
	//	if (target_offset < dec::decimal_cast<transport_line_decimal_place>(item_spacing) && !target_queue.empty())
	//		target_offset = item_spacing;

	target_queue.emplace(it, target_offset, item);
}

bool jactorio::game::TransportLineSegment::TryInsertItem(const bool insert_left, const double offset,
                                                         const data::Item* item) {
	if (!CanInsert(insert_left, dec::decimal_cast<kTransportLineDecimalPlace>(offset)))
		return false;

	// Reenable transport segment
	if (insert_left) {
		if (!IsActiveLeft())
			lIndex = 0;
	}
	else {
		if (!IsActiveRight())
			rIndex = 0;
	}

	InsertItem(insert_left, offset, item);
	return true;
}
