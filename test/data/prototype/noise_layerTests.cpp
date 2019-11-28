#include <gtest/gtest.h>

#include "data/prototype/noise_layer.h"

namespace data::prototype
{
    TEST(noise_layer, get_set_tile_noise_range) {
        auto noise_layer = jactorio::data::Noise_layer();

        auto tile_proto = jactorio::data::Tile();
        auto tile_proto2 = jactorio::data::Tile();

        // Setup tile ranges
        // 0 inclusive
        noise_layer.set_start_val(0);
        // 1 inclusive if at the end
        noise_layer.add_tile(1, &tile_proto);
        noise_layer.add_tile(1.5, &tile_proto2);


        // Fetch tile based on range
        // Out of range returns nullptr
        EXPECT_EQ(noise_layer.get_tile(-1), nullptr);
        EXPECT_EQ(noise_layer.get_tile(1.6), nullptr);

        EXPECT_EQ(noise_layer.get_tile(0), &tile_proto);
        EXPECT_EQ(noise_layer.get_tile(0.9), &tile_proto);

        EXPECT_EQ(noise_layer.get_tile(1), &tile_proto2);
        EXPECT_EQ(noise_layer.get_tile(1.5), &tile_proto2);
    }

    TEST(noise_layer, get_set_noise_start_val) {
        auto noise_layer = jactorio::data::Noise_layer();

        // Defaults to -1
        EXPECT_EQ(noise_layer.get_start_val(), -1);

        noise_layer.set_start_val(0);
        EXPECT_EQ(0, noise_layer.get_start_val());

        noise_layer.set_start_val(1);
        EXPECT_EQ(1, noise_layer.get_start_val());
    }
}