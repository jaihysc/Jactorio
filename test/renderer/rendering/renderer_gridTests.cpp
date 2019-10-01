#include <gtest/gtest.h>

#include <iterator>

#include "renderer/rendering/renderer_grid.h"


TEST(Renderer_grid, genDisplayRenderGrid) {
	const auto grid = jactorio_renderer_rendering::Renderer_grid::gen_render_grid(10, 10);

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