// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_PROTO_NOISE_LAYER_H
#define JACTORIO_INCLUDE_PROTO_NOISE_LAYER_H
#pragma once

#include "jactorio.h"

#include <type_traits>
#include <vector>

#include "proto/abstract/entity.h"
#include "proto/framework/framework_base.h"
#include "proto/resource_entity.h"
#include "proto/tile.h"

namespace jactorio::proto
{
    /// \remark MOST values will be in range -1, 1. SOME VALUES will be greater/less than -1, 1
    /// \tparam T Type stored in noise_layer
    template <typename T>
    class NoiseLayer final : public FrameworkBase
    {
        static_assert(std::is_base_of<FrameworkBase, T>::value);

    public:
        using NoiseValT = float;

        J_NODISCARD Category GetCategory() const override {
            bool constexpr is_tile   = std::is_same<T, Tile>::value;
            bool constexpr is_entity = std::is_same<T, Entity>::value || std::is_same<T, ResourceEntity>::value;

            static_assert(is_tile || is_entity);

            if constexpr (is_tile)
                return Category::noise_layer_tile;
            if constexpr (is_entity)
                return Category::noise_layer_entity;

            return Category::none;
        }

        NoiseLayer() {
            SetStartNoise(-1.f);
        }

        NoiseLayer(const NoiseValT starting_val, const bool normalize_val) : normalize(normalize_val) {
            SetStartNoise(starting_val);
        }

        // ======================================================================
        // Perlin noise properties

        // See http://libnoise.sourceforge.net/glossary/ for usage of the following parameters
        PYTHON_PROP_REF_I(ProtoIntT, octaveCount, 8);
        PYTHON_PROP_REF_I(ProtoFloatT, frequency, 0.25);
        PYTHON_PROP_REF_I(ProtoFloatT, persistence, 0.5);

        /// Affects number of resources generated for Resource entities
        PYTHON_PROP_REF_I(ProtoFloatT, richness, 100.f);

        /// If true, input values outside of the noise_layer range will be brought to the lowest/highest
        /// If false, input values outside of the noise_layer range returns nullptr
        PYTHON_PROP_REF_I(bool, normalize, false);

        // ======================================================================
        // Noise value

        /// \return Starting noise value
        /// Default value is -1
        J_NODISCARD NoiseValT GetStartNoise() const {
            return noiseRanges_[0];
        }

        NoiseLayer* SetStartNoise(const NoiseValT start_val) {
            // Start_val is first value in the noise_range_tile_ranges vector
            // If already in the vector, modify the first value
            if (noiseRanges_.empty())
                noiseRanges_.push_back(start_val);
            else
                noiseRanges_[0] = start_val;

            return this;
        }

        /// \return Maximum noise value accepted
        J_NODISCARD NoiseValT GetEndNoise() const {
            // Last item will have highest value
            return noiseRanges_[noiseRanges_.size() - 1];
        }


        // ======================================================================
        // T value


        /// Adds a tile range, the order which this is called affects the order of the tile range
        /// \remark Will not add if start_val is not set
        /// \param end_range Exclusive ending range, unless it is last item
        /// \param prototype FrameworkBase or inheritors
        NoiseLayer* Add(const NoiseValT end_range, T* prototype) {
            // Will not add if start_val is not set
            if (noiseRanges_.empty())
                return this;

            noiseRanges_.push_back(end_range);
            prototypes_.push_back(prototype);

            return this;
        }


        /// Fetches the tile at the current range
        /// normalize is false, Nullptr if out of range
        /// normalize is true, Min/max value tile if out of range
        J_NODISCARD T* Get(NoiseValT val) const {
            TryNormalizeNoiseVal(val);

            const auto upper_bound = IterateNoiseRange(val);
            if (upper_bound == 0)
                return nullptr;

            return prototypes_[upper_bound - 1];
        }

        /// Gets start end range for a noise value
        /// \return inclusive start, exclusive end range unless it is last item, {0, 0} if invalid
        J_NODISCARD std::pair<NoiseValT, NoiseValT> GetValNoiseRange(NoiseValT val) const {
            TryNormalizeNoiseVal(val);

            const auto upper_bound = IterateNoiseRange(val);
            if (upper_bound == 0)
                return {};

            return {noiseRanges_[upper_bound - 1], noiseRanges_[upper_bound]};
        }


        void PostLoadValidate(const data::PrototypeManager& /*proto*/) const override {
            J_PROTO_ASSERT(1 <= octaveCount, "A minimum of 1 octaves is required");
            J_PROTO_ASSERT(0 < frequency, "Frequency must be greater than 0");
            J_PROTO_ASSERT(0 < persistence, "Persistence must be greater than 0");

            J_PROTO_ASSERT(0 < richness, "Richness must be greater than 0");
        }

    private:
        /// If normalize is set, normalizes provided value
        void TryNormalizeNoiseVal(NoiseValT& val) const {
            // Ending range must be inclusive for last noise value so normalize works

            if (normalize) {
                // Normalize out of range of max-min within noise_layer
                const NoiseValT start_val = GetStartNoise();
                const NoiseValT end_val   = GetEndNoise();

                if (val > end_val)
                    val = end_val;
                if (val < start_val)
                    val = start_val;
            }
        }

        /// \return upper noise range bound index, 0 if not found
        J_NODISCARD size_t IterateNoiseRange(const NoiseValT val) const noexcept {
            bool last_value = true;
            for (auto i = noiseRanges_.size() - 1; i > 0; --i) {
                // Within upper bound
                if (last_value) {
                    last_value = false;
                    if (val > noiseRanges_[i])
                        continue;
                }
                else {
                    if (val >= noiseRanges_[i])
                        continue;
                }

                // Within lower bound
                if (val >= noiseRanges_[i - 1]) {
                    return i;
                }
            }

            return 0;
        }

        /// Tiles associated with noise [i] to [i + 1]
        /// Size is 1 less than noise_range_tile_ranges
        std::vector<T*> prototypes_;

        /// The inclusive value where the noise range begins
        std::vector<NoiseValT> noiseRanges_;
    };
} // namespace jactorio::proto

#endif // JACTORIO_INCLUDE_PROTO_NOISE_LAYER_H
