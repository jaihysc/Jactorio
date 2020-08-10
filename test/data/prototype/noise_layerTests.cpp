// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "data/prototype/tile/noise_layer.h"
#include "data/prototype/tile/tile.h"

namespace jactorio::data
{
	TEST(NoiseLayer, GetSetTileNoiseRange) {
		{
			auto noise_layer = jactorio::data::NoiseLayer<Tile>();

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
			EXPECT_EQ(noise_layer.Get(-1), nullptr);
			EXPECT_EQ(noise_layer.Get(1.6), nullptr);

			EXPECT_EQ(noise_layer.Get(0), &tile_proto);
			EXPECT_EQ(noise_layer.Get(0.9), &tile_proto);

			EXPECT_EQ(noise_layer.Get(1), &tile_proto2);
			EXPECT_EQ(noise_layer.Get(1.5), &tile_proto2);
		}
		{
			auto noise_layer = jactorio::data::NoiseLayer<Tile>(-1, true);

			auto tile_proto  = Tile();
			auto tile_proto2 = Tile();

			// Setup tile ranges
			// 0 inclusive
			noise_layer.SetStartNoise(0);
			// 1 inclusive if at the end
			noise_layer.Add(1, &tile_proto);
			noise_layer.Add(1.5, &tile_proto2);


			// Fetch tile based on range
			// With normalize out of range values to min/max
			EXPECT_EQ(noise_layer.Get(-1), &tile_proto);
			EXPECT_EQ(noise_layer.Get(1.6), &tile_proto2);

			EXPECT_EQ(noise_layer.Get(0), &tile_proto);
			EXPECT_EQ(noise_layer.Get(0.9), &tile_proto);

			EXPECT_EQ(noise_layer.Get(1), &tile_proto2);
			EXPECT_EQ(noise_layer.Get(1.5), &tile_proto2);
		}
	}

	TEST(NoiseLayer, GetSetNoiseStartVal) {
		auto noise_layer = jactorio::data::NoiseLayer<Tile>();

		// Defaults to -1
		EXPECT_EQ(noise_layer.GetStartNoise(), -1);

		noise_layer.SetStartNoise(0);
		EXPECT_EQ(0, noise_layer.GetStartNoise());

		noise_layer.SetStartNoise(1);
		EXPECT_EQ(1, noise_layer.GetStartNoise());
	}

	TEST(NoiseLayer, GetNoiseRangeStartEnd) {
		{
			auto noise_layer = jactorio::data::NoiseLayer<Tile>();

			auto tile_proto  = Tile();
			auto tile_proto2 = Tile();

			noise_layer.SetStartNoise(0);
			noise_layer.Add(1, &tile_proto);
			noise_layer.Add(1.5, &tile_proto2);

			EXPECT_EQ(noise_layer.GetStartNoise(), 0.f);
			EXPECT_EQ(noise_layer.GetEndNoise(), 1.5f);
		}
		{
			auto noise_layer = jactorio::data::NoiseLayer<Tile>();

			// Defaults to -1
			EXPECT_EQ(noise_layer.GetStartNoise(), -1.f);
			EXPECT_EQ(noise_layer.GetEndNoise(), -1.f);
		}
	}

	TEST(NoiseLayer, GetValNoiseRange) {
		auto noise_layer      = jactorio::data::NoiseLayer<Tile>();
		noise_layer.normalize = false;

		auto tile_proto  = Tile();
		auto tile_proto2 = Tile();

		noise_layer.SetStartNoise(0);
		noise_layer.Add(1, &tile_proto);
		noise_layer.Add(1.5, &tile_proto2);

		EXPECT_FLOAT_EQ(noise_layer.GetValNoiseRange(0.1).first, 0);
		EXPECT_FLOAT_EQ(noise_layer.GetValNoiseRange(0.1).second, 1);

		EXPECT_FLOAT_EQ(noise_layer.GetValNoiseRange(1.1).first, 1);
		EXPECT_FLOAT_EQ(noise_layer.GetValNoiseRange(1.1).second, 1.5);
	}
}
