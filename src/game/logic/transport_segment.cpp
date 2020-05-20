// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#include "game/logic/transport_segment.h"

#include <decimal.h>

#include "game/logic/transport_line_controller.h"

bool jactorio::game::TransportLane::IsActive() const {
	return !(lane.empty() || index >= lane.size());
}

bool jactorio::game::TransportLane::CanInsert(TransportLineOffset start_offset, const ItemOffsetT item_offset) {
	start_offset += TransportLineOffset(item_offset);
	assert(start_offset.getAsDouble() >= 0);

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

void jactorio::game::TransportLane::AppendItem(BeginOffsetT offset, const data::Item* item) {
	// A minimum distance of item_spacing is maintained between items (AFTER the initial item)
	if (offset < kItemSpacing && !lane.empty())
		offset = kItemSpacing;

	lane.emplace_back(offset, item);
	backItemDistance += TransportLineOffset{offset};
}

void jactorio::game::TransportLane::InsertItem(BeginOffsetT offset, const data::Item* item, const ItemOffsetT item_offset) {
	offset += item_offset;
	assert(offset >= 0);

	TransportLineOffset target_offset{offset};  // Location where item will be inserted
	TransportLineOffset counter_offset;  // Running tally of offset from beginning

	std::deque<TransportLineItem>::iterator it;
	for (auto i = 0u; i < lane.size(); ++i) {
		counter_offset += lane[i].first;

		// Ends at location where item should be inserted
		// Target: 0.4
		// 0.3   0.2(0.5)
		//     ^ Ends here
		if (counter_offset > target_offset) {
			it = lane.begin() + i;

			// Modify offset of next item to be relative to what will be the newly inserted item
			counter_offset -= lane[i].first;  // Back to distance to previous item

			// Modify insert offset to be relative to previous item, and following item to be relative to newly inserted item
			target_offset -= counter_offset;
			lane[i].first -= target_offset;
			goto loop_exit;
		}
	}
	// Failed to find a greater item, insert at back
	backItemDistance = target_offset;

	it = lane.end();
	target_offset -= counter_offset;

loop_exit:
	assert(target_offset.getAsDouble() >= 0);
	lane.emplace(it, target_offset, item);
}

bool jactorio::game::TransportLane::TryInsertItem(const BeginOffsetT offset, const data::Item* item,
                                                  const ItemOffsetT item_offset) {
	if (!CanInsert(dec::decimal_cast<kTransportLineDecimalPlace>(offset), item_offset))
		return false;

	// Reenable transport segment if disabled
	if (!IsActive())
		index = 0;

	InsertItem(offset, item, item_offset);
	return true;
}

const jactorio::data::Item* jactorio::game::TransportLane::TryPopItem(const BeginOffsetT offset, const BeginOffsetT epsilon) {

	const TransportLineOffset target_offset{offset - epsilon};
	TransportLineOffset offset_counter{0};

	// Iterate past offset - epsilon
	size_t iteration = 0;
	for (auto& item_pair : lane) {
		offset_counter += item_pair.first;

		if (offset_counter >= target_offset) {
			// Return first item if it is within upper bounds offset + epsilon
			if (item_pair.first < TransportLineOffset(offset + epsilon)) {
				// Adjust the next item to preserve spacing
				if (iteration + 1 < lane.size()) {
					lane[iteration + 1].first += item_pair.first;
				}

				const data::Item* item = item_pair.second;
				lane.erase(lane.begin() + iteration);

				return item;
			}
			// Past upper epsilon bound
			return nullptr;
		}

		++iteration;
	}
	// Failed to meet lower epsilon
	return nullptr;
}

// ======================================================================

bool jactorio::game::TransportSegment::CanInsert(const bool left_side, const TransportLineOffset& start_offset) {
	return left_side ? left.CanInsert(start_offset, 0) : right.CanInsert(start_offset, 0);
}

bool jactorio::game::TransportSegment::IsActive(const bool left_side) const {
	return left_side ? left.IsActive() : right.IsActive();
}

void jactorio::game::TransportSegment::AppendItem(const bool left_side, const BeginOffsetT offset, const data::Item* item) {
	left_side ? left.AppendItem(offset, item) : right.AppendItem(offset, item);
}

void jactorio::game::TransportSegment::InsertItem(const bool left_side, const BeginOffsetT offset, const data::Item* item) {
	left_side ? left.InsertItem(offset, item, 0) : right.InsertItem(offset, item, 0);
}

bool jactorio::game::TransportSegment::TryInsertItem(const bool left_side, const BeginOffsetT offset, const data::Item* item) {
	return left_side ? left.TryInsertItem(offset, item, 0) : right.TryInsertItem(offset, item, 0);
}

// With itemOffset applied

bool jactorio::game::TransportSegment::CanInsertAbs(const bool left_side, const TransportLineOffset& start_offset) {
	return left_side ? left.CanInsert(start_offset, itemOffset) : right.CanInsert(start_offset, itemOffset);
}

void jactorio::game::TransportSegment::InsertItemAbs(const bool left_side, const BeginOffsetT offset, const data::Item* item) {
	left_side ? left.InsertItem(offset, item, itemOffset) : right.InsertItem(offset, item, itemOffset);
}

bool jactorio::game::TransportSegment::TryInsertItemAbs(const bool left_side, const BeginOffsetT offset,
                                                        const data::Item* item) {
	return left_side ? left.TryInsertItem(offset, item, itemOffset) : right.TryInsertItem(offset, item, itemOffset);
}

const jactorio::data::Item* jactorio::game::TransportSegment::TryPopItemAbs(const bool left_side,
                                                                            const BeginOffsetT offset, const BeginOffsetT epsilon) {
	return left_side ? left.TryPopItem(offset, epsilon) : right.TryPopItem(offset, epsilon);
}

void jactorio::game::TransportSegment::GetOffsetAbs(BeginOffsetT& val) const {
	val -= itemOffset;
}
