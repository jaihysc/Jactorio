// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 12/06/2019

#include <gtest/gtest.h>

#include "data/prototype/noise_layer.h"
#include "data/prototype/tile/tile.h"

namespace data::prototype
{
	TEST(NoiseLayer, GetSetTileNoiseRange) {
		{
			auto noise_layer = jactorio::data::NoiseLayer<jactorio::data::Tile>();

			auto tile_proto  = jactorio::data::Tile();
			auto tile_proto2 = jactorio::data::Tile();

			// Setup tile ranges
			// 0 inclusive
			noise_layer.SetStartVal(0);
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
			auto noise_layer = jactorio::data::NoiseLayer<jactorio::data::Tile>(-1, true);

			auto tile_proto  = jactorio::data::Tile();
			auto tile_proto2 = jactorio::data::Tile();

			// Setup tile ranges
			// 0 inclusive
			noise_layer.SetStartVal(0);
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
		auto noise_layer = jactorio::data::NoiseLayer<jactorio::data::Tile>();

		// Defaults to -1
		EXPECT_EQ(noise_layer.GetStartVal(), -1);

		noise_layer.SetStartVal(0);
		EXPECT_EQ(0, noise_layer.GetStartVal());

		noise_layer.SetStartVal(1);
		EXPECT_EQ(1, noise_layer.GetStartVal());
	}

	TEST(NoiseLayer, GetNoiseRangeMinMax) {
		{
			auto noise_layer = jactorio::data::NoiseLayer<jactorio::data::Tile>();

			auto tile_proto  = jactorio::data::Tile();
			auto tile_proto2 = jactorio::data::Tile();

			noise_layer.SetStartVal(0);
			noise_layer.Add(1, &tile_proto);
			noise_layer.Add(1.5, &tile_proto2);

			EXPECT_EQ(noise_layer.GetStartVal(), 0.f);
			EXPECT_EQ(noise_layer.GetMaxNoiseVal(), 1.5f);
		}
		{
			auto noise_layer = jactorio::data::NoiseLayer<jactorio::data::Tile>();

			// Defaults to -1
			EXPECT_EQ(noise_layer.GetStartVal(), -1.f);
			EXPECT_EQ(noise_layer.GetMaxNoiseVal(), -1.f);
		}
	}
}
