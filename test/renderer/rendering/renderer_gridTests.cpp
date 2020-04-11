// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/22/2019

#include <gtest/gtest.h>

#include "renderer/rendering/renderer_grid.h"

namespace renderer
{
	TEST(renderer_grid, gen_render_grid_indices) {
		unsigned int* grid = nullptr;
		jactorio::core::Capturing_guard<void()> guard([&] { delete[] grid; });

		grid = jactorio::renderer::renderer_grid::gen_render_grid_indices(121);
		// Indices generation pattern:
		// top left
		// top right
		// bottom right
		// bottom left

		// 0, 0
		EXPECT_EQ(grid[0], 0);
		EXPECT_EQ(grid[1], 1);
		EXPECT_EQ(grid[2], 2);

		EXPECT_EQ(grid[3], 2);
		EXPECT_EQ(grid[4], 3);
		EXPECT_EQ(grid[5], 0);

		// 10, 10
		EXPECT_EQ(grid[720], 480);
		EXPECT_EQ(grid[721], 481);
		EXPECT_EQ(grid[722], 482);

		EXPECT_EQ(grid[723], 482);
		EXPECT_EQ(grid[724], 483);
		EXPECT_EQ(grid[725], 480);
	}
}
