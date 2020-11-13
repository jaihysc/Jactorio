// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "proto/noise_layer.h"
#include "proto/tile.h"

namespace jactorio::proto
{
    TEST(NoiseLayer, GetSetTileNoiseRange) {
        {
            auto noise_layer = NoiseLayer<Tile>();

            auto tile_proto  = Tile();
            auto tile_proto2 = Tile();

            // Setup tile ranges
            // 0 inclusive
            noise_layer.SetStartNoise(0);
            // 1 inclusive if at the end
            noise_layer.Add(1, &tile_proto);
            noise_layer.Add(1.5, &tile_proto2);


            // Fetch tile based on range
            // Out of range returns nullptr
            EXPECT_EQ(noise_layer.Get(-1.f), nullptr);
            EXPECT_EQ(noise_layer.Get(1.6f), nullptr);

            EXPECT_EQ(noise_layer.Get(0.f), &tile_proto);
            EXPECT_EQ(noise_layer.Get(0.9f), &tile_proto);

            EXPECT_EQ(noise_layer.Get(1.f), &tile_proto2);
            EXPECT_EQ(noise_layer.Get(1.5f), &tile_proto2);
        }
        {
            auto noise_layer = NoiseLayer<Tile>(-1, true);

            auto tile_proto  = Tile();
            auto tile_proto2 = Tile();

            // Setup tile ranges
            // 0 inclusive
            noise_layer.SetStartNoise(0);
            // 1 inclusive if at the end
            noise_layer.Add(1.f, &tile_proto);
            noise_layer.Add(1.5f, &tile_proto2);


            // Fetch tile based on range
            // With normalize out of range values to min/max
            EXPECT_EQ(noise_layer.Get(-1.f), &tile_proto);
            EXPECT_EQ(noise_layer.Get(1.6f), &tile_proto2);

            EXPECT_EQ(noise_layer.Get(0.f), &tile_proto);
            EXPECT_EQ(noise_layer.Get(0.9f), &tile_proto);

            EXPECT_EQ(noise_layer.Get(1.f), &tile_proto2);
            EXPECT_EQ(noise_layer.Get(1.5f), &tile_proto2);
        }
    }

    TEST(NoiseLayer, GetSetNoiseStartVal) {
        auto noise_layer = NoiseLayer<Tile>();

        // Defaults to -1
        EXPECT_EQ(noise_layer.GetStartNoise(), -1);

        noise_layer.SetStartNoise(0);
        EXPECT_EQ(0, noise_layer.GetStartNoise());

        noise_layer.SetStartNoise(1);
        EXPECT_EQ(1, noise_layer.GetStartNoise());
    }

    TEST(NoiseLayer, GetNoiseRangeStartEnd) {
        {
            auto noise_layer = NoiseLayer<Tile>();

            auto tile_proto  = Tile();
            auto tile_proto2 = Tile();

            noise_layer.SetStartNoise(0);
            noise_layer.Add(1, &tile_proto);
            noise_layer.Add(1.5, &tile_proto2);

            EXPECT_EQ(noise_layer.GetStartNoise(), 0.f);
            EXPECT_EQ(noise_layer.GetEndNoise(), 1.5f);
        }
        {
            auto noise_layer = NoiseLayer<Tile>();

            // Defaults to -1
            EXPECT_EQ(noise_layer.GetStartNoise(), -1.f);
            EXPECT_EQ(noise_layer.GetEndNoise(), -1.f);
        }
    }

    TEST(NoiseLayer, GetValNoiseRange) {
        auto noise_layer      = NoiseLayer<Tile>();
        noise_layer.normalize = false;

        auto tile_proto  = Tile();
        auto tile_proto2 = Tile();

        noise_layer.SetStartNoise(0.f);
        noise_layer.Add(1.f, &tile_proto);
        noise_layer.Add(1.5f, &tile_proto2);

        EXPECT_FLOAT_EQ(noise_layer.GetValNoiseRange(0.1f).first, 0);
        EXPECT_FLOAT_EQ(noise_layer.GetValNoiseRange(0.1f).second, 1);

        EXPECT_FLOAT_EQ(noise_layer.GetValNoiseRange(1.1f).first, 1);
        EXPECT_FLOAT_EQ(noise_layer.GetValNoiseRange(1.1f).second, 1.5);
    }
} // namespace jactorio::proto
