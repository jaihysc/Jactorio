// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 11/24/2019

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_NOISE_LAYER_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_NOISE_LAYER_H
#pragma once

#include "jactorio.h"

#include <vector>

#include "data/prototype/entity/entity.h"
#include "data/prototype/entity/resource_entity.h"
#include "data/prototype/prototype_base.h"

namespace jactorio::data
{
	///
	/// \remark MOST values will be in range -1, 1. SOME VALUES will be greater/less than -1, 1
	/// T is type stored in noise_layer
	template <typename T>
	class Noise_layer final : public Prototype_base
	{
	public:
		J_NODISCARD dataCategory category() const override {
			bool constexpr is_tile = std::is_same<T, Tile>::value;
			bool constexpr is_entity = std::is_same<T, Entity>::value || std::is_same<
				T, Resource_entity>::value;

			static_assert(is_tile || is_entity);

			if constexpr (is_tile)
				return dataCategory::noise_layer_tile;
			if constexpr (is_entity)
				return dataCategory::noise_layer_entity;

			return dataCategory::none;
		};

		Noise_layer() {
			set_start_val(-1.f);
		}

		Noise_layer(const float starting_val, const bool normalize_val)
			: normalize_val(normalize_val) {
			set_start_val(starting_val);
		}

		// ======================================================================

		// Perlin noise properties
		// See http://libnoise.sourceforge.net/glossary/ for usage of the following parameters
		PYTHON_PROP_REF_I(Noise_layer, int, octave_count, 8);
		PYTHON_PROP_REF_I(Noise_layer, double, frequency, 0.25f);
		PYTHON_PROP_REF_I(Noise_layer, double, persistence, 0.5f);

		// ======================================================================

		/// Affects number of resources generated for Resource entities
		PYTHON_PROP_REF_I(Noise_layer, double, richness, 1.f);

		/**
		 * If true, input values outside of the noise_layer range will be brought to the lowest/highest
		 * If false, input values outside of the noise_layer range returns nullptr
		 */
		PYTHON_PROP_REF_I(Noise_layer, bool, normalize_val, false);

	private:
		// The inclusive value where the noise range begins
		std::vector<float> noise_range_tile_ranges_;
		// Size is 1 less than noise_range_tile_ranges
		std::vector<T*> noise_range_tiles_;

	public:
		/**
		 * Retrieves the starting value <br>
		 * Default value is -1
		 * @return
		 */
		J_NODISCARD float get_start_val() const {
			return noise_range_tile_ranges_[0];
		}

		Noise_layer* set_start_val(const float start_val) {
			// Start_val is first value in the noise_range_tile_ranges vector
			// If already in the vector, modify the first value
			if (noise_range_tile_ranges_.empty())
				noise_range_tile_ranges_.push_back(start_val);
			else
				noise_range_tile_ranges_[0] = start_val;

			return this;
		}

		/**
		 * Returns maximum noise value accepted
		 */
		float get_max_noise_val() {
			// Last item will have highest value
			return noise_range_tile_ranges_[noise_range_tile_ranges_.size() - 1];
		}


		/**
		 * Adds a tile range, the order which this is called affects the order of the tile range <br>
		 * Will not add if start_val is not set
		 * @param end_range Exclusive ending range, unless it is last item
		 * @param tile_ptr Pointer to tile prototype or inheritors
		 */
		Noise_layer* add(const float end_range, T* tile_ptr) {
			// Will not add if start_val is not set
			if (noise_range_tile_ranges_.empty())
				return this;

			noise_range_tile_ranges_.push_back(end_range);
			noise_range_tiles_.push_back(tile_ptr);

			return this;
		}

		/**
		 * Fetches the tile at the current range <br>
		 * normalize_if_val_out_of_range is false, Nullptr if out of range <br>
		 * normalize_if_val_out_of_range is true, Min/max value tile if out of range
		 */
		T* get(float val) {
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


		void post_load_validate() const override {
			J_DATA_ASSERT(1 <= octave_count, "A minimum of 1 octaves is required");
			J_DATA_ASSERT(0 < frequency, "Frequency must be greater than 0");
			J_DATA_ASSERT(0 < persistence, "Persistence must be greater than 0");

			J_DATA_ASSERT(0 < richness, "Richness must be greater than 0");
		}
	};
}

#endif //JACTORIO_INCLUDE_DATA_PROTOTYPE_NOISE_LAYER_H
