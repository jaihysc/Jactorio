#include <gtest/gtest.h>

#include "renderer/rendering/renderer_manager.h"

TEST(renderer_manager, Render_data_generation) {
	std::string img_ids[] {
		"tpotato"
	};
	
	jactorio::renderer::renderer_manager::add_draw_item(10, 10, 300, 300, "p");

	auto data = jactorio::renderer::renderer_manager::get_render_data();
}