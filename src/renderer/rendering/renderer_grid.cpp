#include "renderer/rendering/renderer_grid.h"

float* jactorio_renderer_rendering::Renderer_grid::gen_render_grid(
	unsigned short x_max,
	unsigned short y_max) {
	
	const auto grid = new float[
		static_cast<unsigned long long>(x_max) * y_max * 2];

	unsigned int index = 0;
	for (unsigned int y = 0; y < x_max; ++y) {
		for (unsigned int x = 0; x < y_max; ++x) {
			// 1 coordinate position (vertex)
			grid[index++] = static_cast<float>(x);
			grid[index++] = static_cast<float>(y);
		}
	}
	
	return grid;
}
