#include <gtest/gtest.h>

#include <iterator>

#include "renderer/rendering/renderer_grid.h"


TEST(Renderer_grid, genRenderGrid) {
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

TEST(Renderer_grid, gen_render_tile_grid) {
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

TEST(Renderer_grid, genTextureGrid) {
	// This test will fail if genRenderGridIndices fails
	const auto grid = jactorio::renderer::renderer_grid::gen_texture_grid(12);

	// 0.f, 1.f,  // bottom left
	// 1.f, 1.f,  // bottom right
	// 1.f, 0.f,  // upper right
	// 0.f, 0.f,  // upper left

	// Tile 0, 0 rendering positions
	EXPECT_EQ(grid[0], 0.f);
	EXPECT_EQ(grid[1], 1.f);

	EXPECT_EQ(grid[2], 1.f);
	EXPECT_EQ(grid[3], 1.f);

	EXPECT_EQ(grid[4], 1.f);
	EXPECT_EQ(grid[5], 0.f);

	EXPECT_EQ(grid[6], 0.f);
	EXPECT_EQ(grid[7], 0.f);

	delete[] grid;
}

TEST(Renderer_grid, genRenderGridIndices) {
	const auto grid = jactorio::renderer::renderer_grid::
		gen_render_grid_indices(11, 11);
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


	// // tile at position 0, 0
	// EXPECT_EQ(grid[0], 0);
	// EXPECT_EQ(grid[1], 1);
	// EXPECT_EQ(grid[2], 12);
	//
	// EXPECT_EQ(grid[3], 12);
	// EXPECT_EQ(grid[4], 11);
	// EXPECT_EQ(grid[5], 0 );
	//
	// // tile at position 2, 1
	// EXPECT_EQ(grid[72], 13);  // x + y * width
	// EXPECT_EQ(grid[73], 14);  // Previous number + 1
	// EXPECT_EQ(grid[74], 25);  // Previous number + width
	//
	// EXPECT_EQ(grid[75], 25);  // Previous number
	// EXPECT_EQ(grid[76], 24);  // Previous number - 1
	// EXPECT_EQ(grid[77], 13);  // First number
	//
	// // tile at position 10, 10
	// EXPECT_EQ(grid[72], 13);  // x + y * width
	// EXPECT_EQ(grid[73], 14);  // Previous number + 1
	// EXPECT_EQ(grid[74], 25);  // Previous number + width
	//
	// EXPECT_EQ(grid[75], 25);  // Previous number
	// EXPECT_EQ(grid[76], 24);  // Previous number - 1
	// EXPECT_EQ(grid[77], 13);  // First number
	delete[] grid;
}
