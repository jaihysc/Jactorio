// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#include "game/logic/transport_line_structure.h"

#include <decimal.h>

#include "game/logic/transport_line_controller.h"

bool jactorio::game::TransportLane::IsActive() const {
	return !(lane.empty() || index >= lane.size());
}

bool jactorio::game::TransportLane::CanInsert(TransportLineOffset start_offset, const ItemOffsetT item_offset) {
	start_offset += TransportLineOffset(item_offset);
	TransportLineOffset offset(0);

	// Check if start_offset already has an item
	for (const auto& item : lane) {
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
	if (!lane.empty())
		offset += dec::decimal_cast<kTransportLineDecimalPlace>(kItemSpacing);

	return offset <= start_offset;
}

void jactorio::game::TransportLane::AppendItem(InsertOffsetT offset, const data::Item* item) {
	// A minimum distance of item_spacing is maintained between items (AFTER the initial item)
	if (offset < kItemSpacing && !lane.empty())
		offset = kItemSpacing;

	lane.emplace_back(offset, item);
	backItemDistance += TransportLineOffset{offset};
}

void jactorio::game::TransportLane::InsertItem(InsertOffsetT offset, const data::Item* item, const ItemOffsetT item_offset) {
	offset += item_offset;
	
	TransportLineOffset target_offset{offset};
	TransportLineOffset counter_offset;

	std::deque<TransportLineItem>::iterator it;
	for (auto i = 0u; i < lane.size(); ++i) {
		if (target_offset < lane[i].first) {
			it = lane.begin() + i;

			// Modify offset of next item to be relative to what will be the newly inserted item
			it->first -= target_offset;
			goto loop_exit;
		}
		counter_offset += lane[i].first;
	}
	// Failed to find a greater item
	it = lane.end();

	backItemDistance = target_offset;

loop_exit:
	// Modify target offset relative to previous item
	target_offset -= counter_offset;

	lane.emplace(it, target_offset, item);
}

bool jactorio::game::TransportLane::TryInsertItem(const InsertOffsetT offset, const data::Item* item, const ItemOffsetT item_offset) {
	if (!CanInsert(dec::decimal_cast<kTransportLineDecimalPlace>(offset), item_offset))
		return false;

	// Reenable transport segment if disabled
	if (!IsActive())
		index = 0;

	InsertItem(offset, item, item_offset);
	return true;
}

// ======================================================================

// item_offset is currently unused, replace 0 with itemOffset if needed in the future

bool jactorio::game::TransportSegment::CanInsert(const bool left_side, const TransportLineOffset& start_offset) {
	return left_side ? left.CanInsert(start_offset, 0) : right.CanInsert(start_offset, 0);
}

bool jactorio::game::TransportSegment::IsActive(const bool left_side) const {
	return left_side ? left.IsActive() : right.IsActive();
}

void jactorio::game::TransportSegment::AppendItem(const bool left_side, const InsertOffsetT offset, const data::Item* item) {
	return left_side ? left.AppendItem(offset, item) : right.AppendItem(offset, item);
}

void jactorio::game::TransportSegment::InsertItem(const bool left_side, const InsertOffsetT offset, const data::Item* item) {
	return left_side ? left.InsertItem(offset, item, 0) : right.InsertItem(offset, item, 0);
}

bool jactorio::game::TransportSegment::TryInsertItem(const bool left_side, const InsertOffsetT offset, const data::Item* item) {
	return left_side ? left.TryInsertItem(offset, item, 0) : right.TryInsertItem(offset, item, 0);
}

void jactorio::game::TransportSegment::AdjustInsertionOffset(InsertOffsetT& val) const {
	val -= itemOffset;
}
