#include "data/prototype/noise_layer.h"

#include <vector>

jactorio::data::Noise_layer* jactorio::data::Noise_layer::add_tile(const float end_range, Tile* tile_ptr) {
	// Will not add if start_val is not set
	if (noise_range_tile_ranges_.empty())
		return this;

	noise_range_tile_ranges_.push_back(end_range);
	noise_range_tiles_.push_back(tile_ptr);

	return this;
}

jactorio::data::Tile* jactorio::data::Noise_layer::get_tile(float val) {
	if (normalize_val) {
		// Normalize out of range of max-min within noise_layer
		const float start_val = get_start_val();
		const float end_val = get_max_noise_val();

		if (val > end_val)
			val = end_val;
		if (val < start_val)
			val = start_val;
	}

	bool last_value = true;
	for (unsigned int i = noise_range_tile_ranges_.size() - 1; i > 0; --i) {
		// Less than
		bool less_than = false;
		if (last_value) {
			// ending range inclusive only for the last value
			last_value = false;
			if (val <= noise_range_tile_ranges_[i])
				less_than = true;
		}
		else {
			if (val < noise_range_tile_ranges_[i])
				less_than = true;
		}
		if (!less_than)
			continue;

		// Greater than
		if (val >= noise_range_tile_ranges_[i - 1]) {
			return noise_range_tiles_[i - 1];
		}

	}
	// Not found
	return nullptr;
}
