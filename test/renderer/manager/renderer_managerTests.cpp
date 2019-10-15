#include <gtest/gtest.h>

#include "renderer/rendering/renderer_manager.h"

TEST(renderer_manager, Render_data_generation) {
	std::string img_ids[] {
		"tpotato"
	};
	
	jactorio::renderer::Renderer_manager::gen_render_data(1, 1, img_ids);

	auto data = jactorio::renderer::Renderer_manager::get_render_data();
}