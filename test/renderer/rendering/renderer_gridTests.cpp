// 
// renderer_gridTests.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 10/22/2019
// Last modified: 03/15/2020
// 

#include <gtest/gtest.h>

#include "renderer/rendering/renderer_grid.h"

namespace renderer
{
	TEST(renderer_grid, gen_render_tile_grid) {
		using namespace jactorio::renderer;
		Renderer_layer r_layer{};

		// Will automatically resize up but not down
		renderer_grid::gen_render_grid(&r_layer, 10, 10, 16);

		auto grid = r_layer.get_buf_vertex().ptr;

		// 0, 0
		EXPECT_EQ(grid[0], 0.f);
		EXPECT_EQ(grid[1], 0.f);

		EXPECT_EQ(grid[2], 16.f);
		EXPECT_EQ(grid[3], 0.f);

		EXPECT_EQ(grid[4], 16.f);
		EXPECT_EQ(grid[5], 16.f);

		EXPECT_EQ(grid[6], 0.f);
		EXPECT_EQ(grid[7], 16.f);

		// 9, 9
		EXPECT_EQ(grid[792], 144.f);
		EXPECT_EQ(grid[793], 144.f);

		EXPECT_EQ(grid[794], 160.f);
		EXPECT_EQ(grid[795], 144.f);

		EXPECT_EQ(grid[796], 160.f);
		EXPECT_EQ(grid[797], 160.f);

		EXPECT_EQ(grid[798], 144.f);
		EXPECT_EQ(grid[799], 160.f);
	}

	TEST(renderer_grid, genRenderGrid) {
		const auto grid = jactorio::renderer::renderer_grid::
			gen_render_grid(10, 10);

		// 0, 0
		EXPECT_EQ(grid[0], 0.f);
		EXPECT_EQ(grid[1], 0.f);

		// 9, 1
		EXPECT_EQ(grid[38], 9.f);
		EXPECT_EQ(grid[39], 1.f);

		// 9, 9
		EXPECT_EQ(grid[198], 9.f);
		EXPECT_EQ(grid[199], 9.f);

		delete[] grid;
	}

	TEST(renderer_grid, gen_render_tile_grid_old) {
		const auto grid = jactorio::renderer::renderer_grid::
			gen_render_tile_grid(10, 10, 16);

		// 0, 0
		EXPECT_EQ(grid[0], 0.f);
		EXPECT_EQ(grid[1], 0.f);

		EXPECT_EQ(grid[2], 16.f);
		EXPECT_EQ(grid[3], 0.f);

		EXPECT_EQ(grid[4], 16.f);
		EXPECT_EQ(grid[5], 16.f);

		EXPECT_EQ(grid[6], 0.f);
		EXPECT_EQ(grid[7], 16.f);

		// 9, 9
		EXPECT_EQ(grid[792], 144.f);
		EXPECT_EQ(grid[793], 144.f);

		EXPECT_EQ(grid[794], 160.f);
		EXPECT_EQ(grid[795], 144.f);

		EXPECT_EQ(grid[796], 160.f);
		EXPECT_EQ(grid[797], 160.f);

		EXPECT_EQ(grid[798], 144.f);
		EXPECT_EQ(grid[799], 160.f);

		delete[] grid;
	}

	TEST(renderer_grid, genTextureGrid) {
		// This test will fail if genRenderGridIndices fails
		const auto grid = jactorio::renderer::renderer_grid::gen_texture_grid(12);

		// 0.f, 1.f,  // bottom left
		// 1.f, 1.f,  // bottom right
		// 1.f, 0.f,  // upper right
		// 0.f, 0.f,  // upper left

		// Tile 0, 0 rendering positions
		// EXPECT_EQ(grid[0], 0.f);
		// EXPECT_EQ(grid[1], 1.f);
		//
		// EXPECT_EQ(grid[2], 1.f);
		// EXPECT_EQ(grid[3], 1.f);
		//
		// EXPECT_EQ(grid[4], 1.f);
		// EXPECT_EQ(grid[5], 0.f);
		//
		// EXPECT_EQ(grid[6], 0.f);
		// EXPECT_EQ(grid[7], 0.f);

		// Do not draw the entire spritemap, instead draw nothing by having 0, 0 UV coords
		EXPECT_EQ(grid[0], 0.f);
		EXPECT_EQ(grid[1], 0.f);

		EXPECT_EQ(grid[2], 0.f);
		EXPECT_EQ(grid[3], 0.f);

		EXPECT_EQ(grid[4], 0.f);
		EXPECT_EQ(grid[5], 0.f);

		EXPECT_EQ(grid[6], 0.f);
		EXPECT_EQ(grid[7], 0.f);

		delete[] grid;
	}

	TEST(renderer_grid, gen_render_grid_indices) {
		const auto grid = jactorio::renderer::renderer_grid::gen_render_grid_indices(121);
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
