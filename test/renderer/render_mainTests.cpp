#include <gtest/gtest.h>

#include "renderer/render_main.h"

TEST(render_main, refresh_rate_get_set) {
	// Defaults to 60
	EXPECT_EQ(jactorio::renderer::get_render_refresh_rate(), 60);

	
	jactorio::renderer::set_render_refresh_rate(123);
	EXPECT_EQ(jactorio::renderer::get_render_refresh_rate(), 123);
}