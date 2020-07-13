// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 11/24/2019

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_NOISE_LAYER_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_NOISE_LAYER_H
#pragma once

#include "jactorio.h"

#include <vector>

#include "data/prototype/prototype_base.h"
#include "data/prototype/entity/entity.h"
#include "data/prototype/entity/resource_entity.h"

namespace jactorio::data
{
	///
	/// \remark MOST values will be in range -1, 1. SOME VALUES will be greater/less than -1, 1
	/// T is type stored in noise_layer
	template <typename T>
	class NoiseLayer final : public PrototypeBase
	{
	public:
		J_NODISCARD DataCategory Category() const override {
			bool constexpr is_tile   = std::is_same<T, Tile>::value;
			bool constexpr is_entity = std::is_same<T, Entity>::value || std::is_same<
				T, ResourceEntity>::value;

			static_assert(is_tile || is_entity);

			if constexpr (is_tile)
				return DataCategory::noise_layer_tile;
			if constexpr (is_entity)
				return DataCategory::noise_layer_entity;

			return DataCategory::none;
		};

		NoiseLayer() {
			SetStartVal(-1.f);
		}

		NoiseLayer(const float starting_val, const bool normalize_val)
			: normalize(normalize_val) {
			SetStartVal(starting_val);
		}

		// ======================================================================

		// Perlin noise properties
		// See http://libnoise.sourceforge.net/glossary/ for usage of the following parameters
		PYTHON_PROP_REF_I(NoiseLayer, int, octaveCount, 8);
		PYTHON_PROP_REF_I(NoiseLayer, double, frequency, 0.25f);
		PYTHON_PROP_REF_I(NoiseLayer, double, persistence, 0.5f);

		// ======================================================================

		/// Affects number of resources generated for Resource entities
		PYTHON_PROP_REF_I(NoiseLayer, double, richness, 1.f);

		///
		/// If true, input values outside of the noise_layer range will be brought to the lowest/highest
		/// If false, input values outside of the noise_layer range returns nullptr
		PYTHON_PROP_REF_I(NoiseLayer, bool, normalize, false);

	private:
		// The inclusive value where the noise range begins
		std::vector<float> noiseRangeTileRanges_;
		// Size is 1 less than noise_range_tile_ranges
		std::vector<T*> noiseRangeTiles_;

	public:
		///
		/// \brief Retrieves the starting value <br>
		/// Default value is -1
		J_NODISCARD float GetStartVal() const {
			return noiseRangeTileRanges_[0];
		}

		NoiseLayer* SetStartVal(const float start_val) {
			// Start_val is first value in the noise_range_tile_ranges vector
			// If already in the vector, modify the first value
			if (noiseRangeTileRanges_.empty())
				noiseRangeTileRanges_.push_back(start_val);
			else
				noiseRangeTileRanges_[0] = start_val;

			return this;
		}

		///
		/// \brief Returns maximum noise value accepted
		J_NODISCARD float GetMaxNoiseVal() const {
			// Last item will have highest value
			return noiseRangeTileRanges_[noiseRangeTileRanges_.size() - 1];
		}


		///
		/// \brief Adds a tile range, the order which this is called affects the order of the tile range <br>
		/// /remark Will not add if start_val is not set
		/// \param end_range Exclusive ending range, unless it is last item
		/// \param tile_ptr Pointer to tile prototype or inheritors
		NoiseLayer* Add(const float end_range, T* tile_ptr) {
			// Will not add if start_val is not set
			if (noiseRangeTileRanges_.empty())
				return this;

			noiseRangeTileRanges_.push_back(end_range);
			noiseRangeTiles_.push_back(tile_ptr);

			return this;
		}

		///
		/// \brief Fetches the tile at the current range <br>
		/// normalize_if_val_out_of_range is false, Nullptr if out of range <br>
		/// normalize_if_val_out_of_range is true, Min/max value tile if out of range
		J_NODISCARD T* Get(float val) const {
			if (normalize) {
				// Normalize out of range of max-min within noise_layer
				const float start_val = GetStartVal();
				const float end_val   = GetMaxNoiseVal();

				if (val > end_val)
					val = end_val;
				if (val < start_val)
					val = start_val;
			}

			bool last_value = true;
			for (auto i = noiseRangeTileRanges_.size() - 1; i > 0; --i) {
				// Less than
				bool less_than = false;
				if (last_value) {
					// ending range inclusive only for the last value
					last_value = false;
					if (val <= noiseRangeTileRanges_[i])
						less_than = true;
				}
				else {
					if (val < noiseRangeTileRanges_[i])
						less_than = true;
				}
				if (!less_than)
					continue;

				// Greater than
				if (val >= noiseRangeTileRanges_[i - 1]) {
					return noiseRangeTiles_[i - 1];
				}

			}
			// Not found
			return nullptr;
		}


		void PostLoadValidate(const PrototypeManager&) const override {
			J_DATA_ASSERT(1 <= octaveCount, "A mnimum of 1 octaves is required");
			J_DATA_ASSERT(0 < frequency, "Frequency must be greater than 0");
			J_DATA_ASSERT(0 < persistence, "Persistence must be greater than 0");

			J_DATA_ASSERT(0 < richness, "Richness must be greater than 0");
		}
	};
}

#endif //JACTORIO_INCLUDE_DATA_PROTOTYPE_NOISE_LAYER_H
