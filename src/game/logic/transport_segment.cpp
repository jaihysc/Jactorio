// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/logic/transport_segment.h"

#include <decimal.h>

#include "game/logic/transport_line_controller.h"

using namespace jactorio;

bool game::TransportLane::IsActive() const {
	return !(lane.empty() || index >= lane.size());
}

bool game::TransportLane::CanInsert(TransportLineOffset start_offset, const IntOffsetT item_offset) {
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

void game::TransportLane::AppendItem(FloatOffsetT offset, const data::Item* item) {
	// A minimum distance of item_spacing is maintained between items (AFTER the initial item)
	if (offset < kItemSpacing && !lane.empty())
		offset = kItemSpacing;

	lane.emplace_back(offset, item);
	backItemDistance += TransportLineOffset{offset};
}

void game::TransportLane::InsertItem(FloatOffsetT offset, const data::Item* item, const IntOffsetT item_offset) {
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

bool game::TransportLane::TryInsertItem(const FloatOffsetT offset, const data::Item* item,
                                        const IntOffsetT item_offset) {
	if (!CanInsert(dec::decimal_cast<kTransportLineDecimalPlace>(offset), item_offset))
		return false;

	// Reenable transport segment if disabled
	if (!IsActive())
		index = 0;

	InsertItem(offset, item, item_offset);
	return true;
}

std::pair<size_t, game::TransportLineItem> game::TransportLane::GetItem(const FloatOffsetT offset,
                                                                        const FloatOffsetT epsilon) const {
	const TransportLineOffset target_offset{offset - epsilon};
	TransportLineOffset offset_counter{0};

	// Iterate past offset - epsilon
	size_t iteration = 0;
	for (const auto& item_pair : lane) {
		offset_counter += item_pair.first;

		if (offset_counter >= target_offset) {
			// Return first item if it is within upper bounds offset + epsilon
			if (item_pair.first < TransportLineOffset(offset + epsilon)) {
				return {iteration, item_pair};
			}
			// Past upper epsilon bound
			return {0, {}};
		}

		++iteration;
	}
	// Failed to meet lower epsilon
	return {0, {}};
}

const data::Item* game::TransportLane::TryPopItem(const FloatOffsetT offset, const FloatOffsetT epsilon) {
	const auto result = GetItem(offset, epsilon);

	if (result.second.second == nullptr)
		return nullptr;

	const auto iteration = result.first;
	const auto item_pair = result.second;


	if (iteration + 1 < lane.size()) {
		lane[iteration + 1].first += item_pair.first;
	}

	lane.erase(lane.begin() + iteration);
	return item_pair.second;
}

// ======================================================================

bool game::TransportSegment::CanInsert(const bool left_side, const TransportLineOffset& start_offset) {
	return left_side ? left.CanInsert(start_offset, 0) : right.CanInsert(start_offset, 0);
}

bool game::TransportSegment::IsActive(const bool left_side) const {
	return left_side ? left.IsActive() : right.IsActive();
}

void game::TransportSegment::AppendItem(const bool left_side, const FloatOffsetT offset, const data::Item* item) {
	left_side ? left.AppendItem(offset, item) : right.AppendItem(offset, item);
}

void game::TransportSegment::InsertItem(const bool left_side, const FloatOffsetT offset, const data::Item* item) {
	left_side ? left.InsertItem(offset, item, 0) : right.InsertItem(offset, item, 0);
}

bool game::TransportSegment::TryInsertItem(const bool left_side, const FloatOffsetT offset, const data::Item* item) {
	return left_side ? left.TryInsertItem(offset, item, 0) : right.TryInsertItem(offset, item, 0);
}

// With itemOffset applied

bool game::TransportSegment::CanInsertAbs(const bool left_side, const TransportLineOffset& start_offset) {
	return left_side ? left.CanInsert(start_offset, itemOffset) : right.CanInsert(start_offset, itemOffset);
}

void game::TransportSegment::InsertItemAbs(const bool left_side, const FloatOffsetT offset, const data::Item* item) {
	left_side ? left.InsertItem(offset, item, itemOffset) : right.InsertItem(offset, item, itemOffset);
}

bool game::TransportSegment::TryInsertItemAbs(const bool left_side, const FloatOffsetT offset,
                                              const data::Item* item) {
	return left_side ? left.TryInsertItem(offset, item, itemOffset) : right.TryInsertItem(offset, item, itemOffset);
}

std::pair<size_t, game::TransportLineItem> game::TransportSegment::GetItemAbs(const bool left_side,
                                                                              const FloatOffsetT offset,
                                                                              const FloatOffsetT epsilon) const {
	return left_side ? left.GetItem(offset, epsilon) : right.GetItem(offset, epsilon);
}

const data::Item* game::TransportSegment::TryPopItemAbs(const bool left_side,
                                                        const FloatOffsetT offset,
                                                        const FloatOffsetT epsilon) {
	return left_side ? left.TryPopItem(offset, epsilon) : right.TryPopItem(offset, epsilon);
}

void game::TransportSegment::GetOffsetAbs(IntOffsetT& val) const {
	val -= itemOffset;
}

void game::TransportSegment::GetOffsetAbs(FloatOffsetT& val) const {
	val -= itemOffset;
}
