// 
// noise_layerTests.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 12/06/2019
// Last modified: 03/15/2020
// 

#include <gtest/gtest.h>

#include "data/prototype/noise_layer.h"
#include "data/prototype/tile/tile.h"

namespace data::prototype
{
	TEST(noise_layer, get_set_tile_noise_range) {
		{
			auto noise_layer = jactorio::data::Noise_layer<jactorio::data::Tile>();

			auto tile_proto = jactorio::data::Tile();
			auto tile_proto2 = jactorio::data::Tile();

			// Setup tile ranges
			// 0 inclusive
			noise_layer.set_start_val(0);
			// 1 inclusive if at the end
			noise_layer.add(1, &tile_proto);
			noise_layer.add(1.5, &tile_proto2);


			// Fetch tile based on range
			// Out of range returns nullptr
			EXPECT_EQ(noise_layer.get(-1), nullptr);
			EXPECT_EQ(noise_layer.get(1.6), nullptr);

			EXPECT_EQ(noise_layer.get(0), &tile_proto);
			EXPECT_EQ(noise_layer.get(0.9), &tile_proto);

			EXPECT_EQ(noise_layer.get(1), &tile_proto2);
			EXPECT_EQ(noise_layer.get(1.5), &tile_proto2);
		}
		{
			auto noise_layer = jactorio::data::Noise_layer<jactorio::data::Tile>(-1, true);

			auto tile_proto = jactorio::data::Tile();
			auto tile_proto2 = jactorio::data::Tile();

			// Setup tile ranges
			// 0 inclusive
			noise_layer.set_start_val(0);
			// 1 inclusive if at the end
			noise_layer.add(1, &tile_proto);
			noise_layer.add(1.5, &tile_proto2);


			// Fetch tile based on range
			// With normalize out of range values to min/max
			EXPECT_EQ(noise_layer.get(-1), &tile_proto);
			EXPECT_EQ(noise_layer.get(1.6), &tile_proto2);

			EXPECT_EQ(noise_layer.get(0), &tile_proto);
			EXPECT_EQ(noise_layer.get(0.9), &tile_proto);

			EXPECT_EQ(noise_layer.get(1), &tile_proto2);
			EXPECT_EQ(noise_layer.get(1.5), &tile_proto2);
		}
	}

	TEST(noise_layer, get_set_noise_start_val) {
		auto noise_layer = jactorio::data::Noise_layer<jactorio::data::Tile>();

		// Defaults to -1
		EXPECT_EQ(noise_layer.get_start_val(), -1);

		noise_layer.set_start_val(0);
		EXPECT_EQ(0, noise_layer.get_start_val());

		noise_layer.set_start_val(1);
		EXPECT_EQ(1, noise_layer.get_start_val());
	}

	TEST(noise_layer, get_noise_range_min_max) {
		{
			auto noise_layer = jactorio::data::Noise_layer<jactorio::data::Tile>();

			auto tile_proto = jactorio::data::Tile();
			auto tile_proto2 = jactorio::data::Tile();

			noise_layer.set_start_val(0);
			noise_layer.add(1, &tile_proto);
			noise_layer.add(1.5, &tile_proto2);

			EXPECT_EQ(noise_layer.get_start_val(), 0.f);
			EXPECT_EQ(noise_layer.get_max_noise_val(), 1.5f);
		}
		{
			auto noise_layer = jactorio::data::Noise_layer<jactorio::data::Tile>();

			// Defaults to -1
			EXPECT_EQ(noise_layer.get_start_val(), -1.f);
			EXPECT_EQ(noise_layer.get_max_noise_val(), -1.f);
		}
	}
}
